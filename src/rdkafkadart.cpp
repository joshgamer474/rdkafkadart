#include <rdkafkadart.h>
#include <chrono>
#include <map>
#include <memory>

#include <consumer.h>
#include <producer.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

static std::map<void*, std::unique_ptr<Consumer>> consumers_map =
    std::map<void*, std::unique_ptr<Consumer>>();
static std::map<void*, std::unique_ptr<Producer>> producers_map =
    std::map<void*, std::unique_ptr<Producer>>();
static std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> filesink;
static std::shared_ptr<spdlog::logger> logger;
static spdlog::level::level_enum log_level = spdlog::level::debug;

// Initializes the spdlog logger and rotating file sink filesink
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

// Set the log level for each logger
void set_loglevel(const char* loglevel)
{
    if (logger == nullptr)
    {
        return;
    }
    const std::string lvl(loglevel);
    if (lvl.compare("err") == 0)
    {
        log_level = spdlog::level::err;
    }
    else if (lvl.compare("warn") == 0)
    {
        log_level = spdlog::level::info;
    }
    else if (lvl.compare("info") == 0)
    {
        log_level = spdlog::level::info;
    }
    else if (lvl.compare("debug") == 0)
    {
        log_level = spdlog::level::debug;
    }
    else if (lvl.compare("trace") == 0)
    {
        log_level = spdlog::level::trace;
    }
    logger->set_level(log_level);
}

/*
  Kafka Consumer methods
*/

void* create_consumer(const char* broker,
  void (*cmsg_callback)(void* consumer, const char* topic,
      uint8_t* data, uint64_t len, int64_t offset),
  const int64_t start_offset)
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
    logger->info("create_consumer() creating consumer");
    std::unique_ptr<Consumer> con = std::make_unique<Consumer>(
        broker,
        nullptr,
        cmsg_callback, filesink,
        log_level
    );
    con->set_start_offset(start_offset);
    void* ret = con.get();
    consumers_map[ret] = std::move(con);
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
    if (consumers_map.find(consumer) != consumers_map.end())
    {
        consumers_map.erase(consumer);
    }
}

const char* get_topics_from_consumer(void* consumer)
{
    Consumer* con = static_cast<Consumer*>(consumer);
    logger->info("get_topics_from_consumer() consumer {}, topics: {}",
        consumer,
        con->get_alltopicsstr().c_str());
    return con->get_alltopicsstr().data();
}

void ack(void* consumer)
{
    Consumer* con = static_cast<Consumer*>(consumer);
    con->ack();
}

void ack_all(void* consumer)
{
    Consumer* con = static_cast<Consumer*>(consumer);
    con->ack_all();
}

/*
  Kafka Producer methods
*/

void* create_producer(const char* broker)
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
    logger->info("create_producer() creating producer");
    std::unique_ptr<Producer> prod = std::make_unique<Producer>(broker, nullptr, filesink, log_level);
    void* ret = prod.get();
    producers_map[ret] = std::move(prod);
    //void* ret = new Producer(broker, NULL, filesink, log_level);
    logger->info("create_producer() created producer {} to broker {}", ret, broker);
    logger->flush();
    return ret;
}

void produce(void* producer, const char* topic, uint8_t* data, uint64_t len)
{
    const std::vector<uint8_t> datavec(data, data + len);
    logger->info("produce_msg() topic {}, data len: {}",
        topic,
        len);
    logger->flush();
    Producer* pro = static_cast<Producer*>(producer);
    pro->produce(topic, datavec);
}

void destroy_producer(void* producer)
{
    logger->info("destroy_producer() producer {}", producer);
    logger->flush();
    if (producers_map.find(producer) != producers_map.end())
    {
        producers_map.erase(producer);
    }
}