#include <consumer.h>
#include <iostream>

Consumer::Consumer(std::string broker,
    std::function<void(std::string topic, std::vector<uint8_t>)> msg_callback,
    std::function<void(void* consumer, const char* topic,
        uint8_t* data, uint64_t len, int64_t offset)> cmsg_callback)
    : broker(broker),
    msgs_consumed(0),
    run(false),
    done_consuming(false),
    partition(0),
    start_offset(RdKafka::Topic::OFFSET_BEGINNING),
    msg_callback(msg_callback),
    cmsg_callback(cmsg_callback),
    msgs_consumed_map(std::map<std::string, size_t>())
{
    init();
}

Consumer::~Consumer()
{
    //printf("Consumed %zu messages\n", msgs_consumed);
    stop();
    // Delete remaining consumed Rdkafka::Messages
    clear_queuedmsgs();
    clear_sentmsgs();
}

void Consumer::init()
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
}

void Consumer::start(const std::vector<std::string>& topics, int timeout_ms)
{
    run = true;
    done_consuming = false;
    // Start consumer for topic+partition at start offset
    for (auto& topic : topics) {
        // Create topic handle
        topic_handles[topic] = RdKafka::Topic::create(consumer, topic, tconf, errstr);
        msgs_consumed_map[topic] = 0;
        // Start consuming topic handle
        RdKafka::ErrorCode resp = consumer->start(topic_handles[topic], partition, start_offset);
        if (resp != RdKafka::ERR_NO_ERROR) {
            std::cerr << "Failed to start consumer: " <<
                RdKafka::err2str(resp) << std::endl;
            break;
        }
    }
    // Start consuming topics
    consume(timeout_ms);

    // Wait until consuming is complete if cmsg_callback is being used
    while ((!queued_msgs.empty() || done_consuming != true)
        && cmsg_callback != nullptr)
    {
        // Consume queued RdKafka::Messages
        while (!queued_msgs.empty())
        {
          RdKafka::Message* msg = queued_msgs.front();
          // Call c callback for each message consumed
          cmsg_callback(
              this,
              msg->topic_name().c_str(),
              (unsigned char*)msg->payload(),
              msg->len(),
              msg->offset());
          sent_msgs.push_back(msg);
          queued_msgs.pop_front();
          // msgs will be deleted on consumer destruction
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
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

                if (cmsg_callback == nullptr)
                {   // Delete message
                    delete msg;
                }

                // Poll for more kafka consumer events
                consumer->poll(0);

                // Consume next message
                msg = consumer->consume(pair.second, partition, timeout_ms);
            }

            if (msg && cmsg_callback == nullptr)
            {
                delete msg;
            }

            //printf("Consumed %zu messages on topic %s\n",
            //    msgs_consumed_map[pair.first], pair.first);
        }
        done_consuming = true;
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
            if (msg_callback != nullptr)
            {   // Get data from *msg
                headers = msg->headers();
                timestamp = msg->timestamp();
                buf = msg->payload();
                len = msg->len();
                charbuf = (unsigned char*) buf;
                // Copy buf* into vector<uint8_t>
                bufvec = std::vector<uint8_t>(charbuf, charbuf + len);

                // Call msg callback with data
                msg_callback(topic, bufvec);
            }
            if (cmsg_callback != nullptr)
            {
              queued_msgs.push_back(msg);
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

void Consumer::clear_queuedmsgs()
{
    while (!queued_msgs.empty())
    {
        RdKafka::Message* msg = queued_msgs.front();
        if (msg)
        {
            delete msg;
        }
        queued_msgs.pop_front();
    }
}

void Consumer::clear_sentmsgs()
{
    while (!sent_msgs.empty())
    {
        RdKafka::Message* msg = sent_msgs.front();
        if (msg)
        {
            delete msg;
        }
        sent_msgs.pop_front();
    }
}