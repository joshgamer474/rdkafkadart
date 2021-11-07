#include <rdkafkadart.h>
#include <chrono>
#include <memory>

#include <consumer.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

/*
#ifdef RdkafkaDart_ANDROID
static auto filesink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>
    ("/data/data/com.example.pokestonks_mobile/rdkafka.log", 1479120392, 1, true);
#elif RdkafkaDart_IOS
static auto filesink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>
    ("~/tmp/rdkafka.log", 1479120392, 1, true);
#else
static std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> filesink =
    std::make_shared<spdlog::sinks::rotating_file_sink_mt>
        ("logs/rdkafka.log", 1479120392, 1, true);
#endif
*/
static std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> filesink;
static std::shared_ptr<spdlog::logger> logger;// =
    //std::make_shared<spdlog::logger>("RdkafkaDart", filesink);

void set_logpath(const char* logpath)
{
    const std::string path(logpath);
    const std::string fulllogpath = path + "/rdkafka.log";
    // Reinit filesink
    filesink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>
        (fulllogpath, 1479120392, 1, true);
    // Reinit logger
    logger = std::make_shared<spdlog::logger>("RdkafkaDart", filesink);
}

void* create_consumer(const char* broker,
  void (*cmsg_callback)(void* consumer, const char* topic,
      uint8_t* data, uint64_t len, int64_t offset))
{
    if (logger == nullptr)
    {
#ifdef RdkafkaDart_ANDROID
        throw std::invalid_argument("Did not configure log path using set_logpath()...");
#elif RdkafkaDart_IOS
        throw std::invalid_argument("Did not configure log path using set_logpath()...");
#else
        set_logpath("logs");
#endif
    }
    logger->set_level(spdlog::level::debug);
    logger->info("create_consumer() creating consumer");
    void* ret = new Consumer(broker, NULL, cmsg_callback, filesink);
    logger->info("create_consumer() created consumer {} to broker {}", ret, broker);
    logger->flush();
    return ret;
}

void consume(void* consumer, const char** topics, int topics_len, int timeout_ms)
{
    const std::vector<std::string> topicsvec(topics, topics + topics_len);
    logger->info("consume()ing {0} topics to consumer {1}: {2}",
        topicsvec.size(),
        consumer,
        *topics);
    logger->flush();
    Consumer* con = static_cast<Consumer*>(consumer);
    con->start(topicsvec, timeout_ms);
}

void destroy_consumer(void* consumer)
{
    logger->info("destroy_consumer() consumer {}",
        consumer);
    logger->flush();
    Consumer* con = static_cast<Consumer*>(consumer);
    con->stop();
    delete con;
}

const char* get_topics_from_consumer(void* consumer)
{
    Consumer* con = static_cast<Consumer*>(consumer);
    logger->info("get_topics_from_consumer() consumer {}, topics: {}",
        consumer,
        con->get_alltopicsstr().c_str());
    return con->get_alltopicsstr().data();
}