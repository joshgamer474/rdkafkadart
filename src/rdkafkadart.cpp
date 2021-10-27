#include <rdkafkadart.h>
#include <consumer.h>

void* create_consumer(const char* broker,
  void (*cmsg_callback)(void* consumer, const char* topic,
      uint8_t* data, uint64_t len, int64_t offset))
{
    return new Consumer(broker, NULL, cmsg_callback);
}

void consume(void* consumer, const char** topics, int topics_len, int timeout_ms)
{
    const std::vector<std::string> topicsvec(topics, topics + topics_len);
    Consumer* con = static_cast<Consumer*>(consumer);
    con->start(topicsvec, timeout_ms);
}

void destroy_consumer(void* consumer)
{
    Consumer* con = static_cast<Consumer*>(consumer);
    con->stop();
    delete con;
}

const char* get_topics_from_consumer(void* consumer)
{
    Consumer* con = static_cast<Consumer*>(consumer);
    return con->get_alltopicsstr().data();
}