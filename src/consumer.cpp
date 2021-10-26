#include <consumer.h>
#include <iostream>

Consumer::Consumer(std::string broker, std::string topic,
    std::function<void(std::string topic, std::vector<uint8_t>)> msg_callback)
    : broker(broker),
    msgs_consumed(0),
    run(false),
    partition(0),
    start_offset(RdKafka::Topic::OFFSET_BEGINNING),
    msg_callback(msg_callback),
    msgs_consumed_map(std::map<std::string, size_t>())
{
    init({ topic });
}

Consumer::Consumer(std::string broker, std::vector<std::string> topics,
    std::function<void(std::string topic, std::vector<uint8_t>)> msg_callback)
    : broker(broker),
    msgs_consumed(0),
    run(false),
    partition(0),
    start_offset(RdKafka::Topic::OFFSET_BEGINNING),
    msg_callback(msg_callback),
    msgs_consumed_map(std::map<std::string, size_t>())
{
    init(topics);
}

Consumer::~Consumer()
{
    //printf("Consumed %zu messages\n", msgs_consumed);
    stop();
}

void Consumer::init(std::vector<std::string> topics)
{
    // Init configs
    conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

    conf->set("metadata.broker.list", broker, errstr);
    conf->set("enable.partition.eof", "true", errstr);

    // Create consumer
    consumer = RdKafka::Consumer::create(conf, errstr);

    // Get metadata from Kafka server
    RdKafka::Metadata* metadata;
    RdKafka::ErrorCode ret = consumer->metadata(true, NULL, &metadata, 200);
    if (ret != RdKafka::ERR_NO_ERROR)
    {
        std::cerr << "%% Failed to acquire metadata: "
            << RdKafka::err2str(ret) << std::endl;
        return;
    }
    // Save all found Kafka topics
    for (auto topic : *metadata->topics())
    {
        alltopics.push_back(topic->topic());
        alltopicsstr += topic->topic() + ',';
    }
    alltopicsstr.pop_back();

    // Create topic handle(s)
    for (const std::string topic : topics) {
        topic_handles[topic] = RdKafka::Topic::create(consumer, topic, tconf, errstr);
        msgs_consumed_map[topic] = 0;
    }
}

void Consumer::start(int timeout_ms)
{
    run = true;
    // Start consumer for topic+partition at start offset
    for (auto& pair : topic_handles) {
        RdKafka::ErrorCode resp = consumer->start(pair.second, partition, start_offset);
        if (resp != RdKafka::ERR_NO_ERROR) {
            std::cerr << "Failed to start consumer: " <<
                RdKafka::err2str(resp) << std::endl;
            break;
        }
    }
    // Start consuming topics
    consume(timeout_ms);
}

void Consumer::consume(int timeout_ms)
{
    consume_thread = std::thread([&]()
    {
        // Poll for kafka consumer events
        consumer->poll(0);

        // Consume each topic one at a time
        for (auto& pair : topic_handles)
        {
            // Consume topic for message
            RdKafka::Message *msg = consumer->consume(pair.second, partition, timeout_ms);

            // Consume all messages on a topic until there are no more messages to consume
            while (msg != nullptr && run == true)
            {
                // Process message
                if (msg != nullptr)
                {
                    RdKafka::ErrorCode ret = consume_msg(pair.first, msg, NULL);
                    if (ret == RdKafka::ErrorCode::ERR__PARTITION_EOF)
                    {   // Reached end of partition for this topic
                        break;
                    }
                }
                else
                {
                    break;
                }

                // Delete message
                delete msg;

                // Poll for more kafka consumer events
                consumer->poll(0);

                // Consume next message
                msg = consumer->consume(pair.second, partition, timeout_ms);
            }

            if (msg)
            {
                delete msg;
            }

            //printf("Consumed %zu messages on topic %s\n",
            //    msgs_consumed_map[pair.first], pair.first);
        }
        printf("consume_thread finished running\n");
    });
}

RdKafka::ErrorCode Consumer::consume_msg(std::string topic, RdKafka::Message* msg, void* opaque)
{
    const RdKafka::Headers *headers;
    RdKafka::MessageTimestamp timestamp;
    const void* buf;
    size_t len = 0;
    unsigned char* charbuf;
    std::vector<uint8_t> bufvec;

    if (msg->err() != RdKafka::ERR_NO_ERROR &&
        msg->err() != RdKafka::ERR__TIMED_OUT)
    {
        printf("consume_msg() msg->err(): %i %s\n", msg->err(), msg->errstr().c_str());
    }

    switch (msg->err())
    {
        case RdKafka::ERR__TIMED_OUT:
            break;
        case RdKafka::ERR_NO_ERROR:
            headers = msg->headers();
            timestamp = msg->timestamp();
            buf = msg->payload();
            len = msg->len();
            //bufvec = static_cast<std::vector<uint8_t> const*>(buf);
            charbuf = (unsigned char*) buf;
            // Copy buf* into vector<uint8_t>
            bufvec = std::vector<uint8_t>(charbuf, charbuf + len);
            /*printf("broadcasting consume_msg() for msg %zu, len %zu\n",
                msgs_consumed_map[topic],
                msg->len());*/
            // Call msg callback with data
            if (msg_callback != nullptr) {
                msg_callback(topic, bufvec);
            }
            msgs_consumed_map[topic]++;
            msgs_consumed++;
            break;
        case RdKafka::ERR__PARTITION_EOF:
            // Last message read, hit end of partition for topic
            break;
        default:
            // Errors
            std::cerr << "Consume failed: " << msg->errstr() << std::endl;
            run = false;
    }
    return msg->err();
}

void Consumer::stop()
{
    run = false;
    if (consumer)
    {
        for (auto& pair : topic_handles)
        {
            printf("Consumed %zu messages on topic %s\n",
                msgs_consumed_map[pair.first],
                pair.first.c_str());
            consumer->stop(pair.second, partition);
        }
    }
    if (consume_thread.joinable())
    {
        printf("joining consume_thread\n");
        consume_thread.join();
    }
}

bool Consumer::is_running()
{
    return run;
}

const std::vector<std::string>& Consumer::get_alltopics()
{
    return alltopics;
}

const std::string& Consumer::get_alltopicsstr()
{
    return alltopicsstr;
}