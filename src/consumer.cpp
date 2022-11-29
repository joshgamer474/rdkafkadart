#include <consumer.h>
#include <iostream>
#include <stdlib.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

Consumer::Consumer(std::string broker,
    std::function<void(std::string topic, std::vector<uint8_t>)> msg_callback,
    std::function<void(void* consumer, const char* topic,
        uint8_t* data, uint64_t len, int64_t offset)> cmsg_callback,
    std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> logsink,
    spdlog::level::level_enum loglevel)
    : broker(broker),
    msgs_consumed(0),
    run(false),
    done_consuming(false),
    stop_consumer_thread(false),
    partition(0),
    start_offset(RdKafka::Topic::OFFSET_BEGINNING),
    msg_callback(msg_callback),
    cmsg_callback(cmsg_callback),
    topic_handles(std::map<std::string, RdKafka::Topic*>()),
    msgs_consumed_map(std::map<std::string, size_t>())
{
    if (logsink)
    {
        logger = std::make_shared<spdlog::logger>("Consumer", logsink);
    }
    else
    {
        long long int rnd = rand() % (715701992318);
        logger = spdlog::stdout_color_mt(std::to_string(rnd));
    }
    logger->set_level(loglevel);

    init();
}

Consumer::~Consumer()
{
    logger->info("~Consumer()");
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
    logger->info("Created consumer to Kafka broker: {}", broker.c_str());

    // Get metadata from Kafka server
    RdKafka::Metadata* metadata;
    RdKafka::ErrorCode ret = consumer->metadata(true, NULL, &metadata, 200);
    if (ret != RdKafka::ERR_NO_ERROR)
    {
        logger->error("Failed to acquire metadata, err: {}",
          RdKafka::err2str(ret));
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
    logger->info("Acquired Kafka server metadata, topics {}",
      alltopicsstr.c_str());
}

void Consumer::set_start_offset(const int64_t _start_offset) {
  start_offset = _start_offset;
}

void Consumer::start(const std::vector<std::string>& topics, int timeout_ms)
{
    run = true;
    done_consuming = false;
    stop_consumer_thread = false;
    // Clear previous topichandles
    clear_topichandles();

    // Start consumer for topic+partition at start offset
    for (auto& topic : topics)
    {
        std::lock_guard<std::mutex> lg(topic_handles_mutex);
        // Create topic handle
        logger->debug("Creating topic handle for topic {}", topic.c_str());
        topic_handles[topic] = RdKafka::Topic::create(consumer, topic, tconf, errstr);
        msgs_consumed_map[topic] = 0;
        // Start consuming topic handle
        logger->debug("Starting topic handle for topic {}", topic.c_str());
        RdKafka::ErrorCode resp = consumer->start(topic_handles[topic], partition, start_offset);
        if (resp != RdKafka::ERR_NO_ERROR) {
            logger->error("Failed to start consumer, err: {}",
                RdKafka::err2str(resp));
            std::cerr << "Failed to start consumer: " <<
                RdKafka::err2str(resp) << std::endl;
            break;
        }
    }
    // Start consuming topics
    consume(timeout_ms);

    logger->debug("Waiting until consuming is complete, queued_msgs.size(): {}, done_consuming: {}, cmsg_callback: {}",
        queued_msgs.size(),
        done_consuming,
        cmsg_callback != nullptr);
    // Wait until consuming is complete if cmsg_callback is being used
    while ((!queued_msgs.empty() || done_consuming != true)
        && cmsg_callback != nullptr)
    {
        // Consume queued RdKafka::Messages
        while (!queued_msgs.empty())
        {
            std::lock_guard<std::mutex> lg(queued_msgs_mutex);
            RdKafka::Message* msg = queued_msgs.front();
            // Remove msg from queued_msgs
            queued_msgs.pop_front();
            logger->trace("Doing cmsg_callback() for msg {}", (void*)msg);
            // Call c callback for each message consumed
            cmsg_callback(
                this,
                msg->topic_name().c_str(),
                (unsigned char*)msg->payload(),
                msg->len(),
                msg->offset());
            // msgs will be deleted on consumer destruction
            std::lock_guard<std::mutex> lgs(sent_msgs_mutex);
            // Push to sent_msgs queue to be deleted later
            sent_msgs.push_back(msg);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    logger->debug("Reached bottom of start()");
}

void Consumer::consume(const int timeout_ms)
{
    if (consume_thread)
    {
        return;
    }
    logger->info("Starting consume_thread");
    consume_thread = std::make_unique<std::thread>([&, timeout_ms]()
    {
        bool consumed_msg = false;
        while (!stop_consumer_thread)
        {
            consumed_msg = false;
            if (topic_handles.empty() || consumer == nullptr)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            // Poll for kafka consumer events
            consumer->poll(0);

            // Consume each topic one at a time
            std::lock_guard<std::mutex> lg(topic_handles_mutex);
            for (auto& pair : topic_handles)
            {
                if (pair.second == nullptr)
                {
                    break;
                }

                // Consume topic for message
                logger->debug("Consuming msgs on topic {}", pair.first.c_str());
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
                        consumed_msg = true;
                    }
                    else
                    {
                        break;
                    }

                    if (cmsg_callback == nullptr)
                    {   // Delete message memory immediately
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

                logger->debug("Consumed {} msgs on topic {}",
                    msgs_consumed_map[pair.first],
                    pair.first);
            } // end for(topic : topic_handles)

          // Check if any msgs were consumed
          // If no msgs were available, sleep the consumer thread and retry consuming
          if (!consumed_msg) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
          }

        } // end while(!stop_consumer_thread)

        logger->info("consume_thread has finished running");
        done_consuming = true;
    });
}

RdKafka::ErrorCode Consumer::consume_msg(std::string topic, RdKafka::Message* msg, void* opaque)
{
    const RdKafka::Headers *headers;
    RdKafka::MessageTimestamp timestamp;
    void* buf;
    size_t len = 0;
    unsigned char* charbuf;
    std::vector<uint8_t> bufvec;

    switch (msg->err())
    {
        case RdKafka::ERR__TIMED_OUT:
            break;
        case RdKafka::ERR_NO_ERROR:
            // Forward msg through C++ msg_callback() immediately
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
            // Queue msg for C cmsg_callback() in parent thread
            if (cmsg_callback != nullptr)
            {
                std::lock_guard<std::mutex> lg(queued_msgs_mutex);
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
    logger->info("stop() called, stopping consumer");
    run = false;
    stop_consumer_thread = true;
    clear_topichandles();
    if (consume_thread &&
        consume_thread->joinable())
    {
        logger->info("Joining consume_thread");
        consume_thread->join();
        consume_thread = nullptr;
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

void Consumer::clear_topichandles()
{
    if (consumer == nullptr)
    {
        return;
    }
    for (auto& pair : topic_handles)
    {
        logger->info("Consumed {} messages on topic {}",
            msgs_consumed_map[pair.first],
            pair.first.c_str());
        consumer->stop(pair.second, partition);
        std::lock_guard<std::mutex> lg(topic_handles_mutex);
        if (pair.second) {
          delete pair.second;
        }
    }
    topic_handles.clear();
}

void Consumer::clear_queuedmsgs()
{
    logger->debug("Clearing queued messages queue");
    while (!queued_msgs.empty())
    {
        std::lock_guard<std::mutex> lg(queued_msgs_mutex);
        RdKafka::Message* msg = queued_msgs.front();
        queued_msgs.pop_front();
        if (msg)
        {
            delete msg;
        }
    }
}

void Consumer::clear_sentmsgs()
{
    logger->debug("Clearing sent messages queue");
    while (!sent_msgs.empty())
    {
        ack();
    }
}

void Consumer::ack_all()
{
    clear_sentmsgs();
}

void Consumer::ack()
{
    if (sent_msgs.empty())
    {
        return;
    }
    std::lock_guard<std::mutex> lg(sent_msgs_mutex);
    RdKafka::Message* msg = sent_msgs.front();
    sent_msgs.pop_front();
    if (msg)
    {
        delete msg;
    }
}