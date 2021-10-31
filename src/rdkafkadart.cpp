#include <rdkafkadart.h>
#include <consumer.h>
#include <spdlog/async.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#ifdef RdkafkaDart_ANDROID
static auto file_logger = spdlog::basic_logger_mt<spdlog::async_factory>("rdkafkadart", "rdkafka.log", true);
#else
static auto file_logger = spdlog::basic_logger_mt<spdlog::async_factory>("rdkafkadart", "logs/rdkafka.log", true);
#endif

void* create_consumer(const char* broker,
  void (*cmsg_callback)(void* consumer, const char* topic,
      uint8_t* data, uint64_t len, int64_t offset))
{
    void* ret = new Consumer(broker, NULL, cmsg_callback);
    file_logger->info("create_consumer() consumer {}, to broker {}", ret, broker);
    return ret;
}

void consume(void* consumer, const char** topics, int topics_len, int timeout_ms)
{
    const std::vector<std::string> topicsvec(topics, topics + topics_len);
    file_logger->info("consume()ing {0} topics to consumer {1}: {2}",
        topicsvec.size(),
        consumer,
        *topics);
    Consumer* con = static_cast<Consumer*>(consumer);
    con->start(topicsvec, timeout_ms);
}

void destroy_consumer(void* consumer)
{
    file_logger->info("destroy_consumer() consumer {}",
        consumer);
    Consumer* con = static_cast<Consumer*>(consumer);
    con->stop();
    delete con;
}

const char* get_topics_from_consumer(void* consumer)
{
    Consumer* con = static_cast<Consumer*>(consumer);
    file_logger->info("get_topics_from_consumer() consumer {}, topics: {}",
        consumer,
        con->get_alltopicsstr().c_str());
    return con->get_alltopicsstr().data();
}