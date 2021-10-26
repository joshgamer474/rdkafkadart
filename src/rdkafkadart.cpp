#include <rdkafkadart.h>
#include <consumer.h>

void* create_consumer(char* broker, char** topics, int topics_len,
  void (*cmsg_callback)(const char* topic, uint8_t* data, uint64_t len))
{
    std::vector<std::string> topicsstr(topics, topics + topics_len);
    return new Consumer(broker, topicsstr, NULL, cmsg_callback);
}

void consume(void* consumer, int timeout_ms)
{
    Consumer* con = static_cast<Consumer*>(consumer);
    con->start(timeout_ms);
}

void destroy_consumer(void* consumer)
{
    Consumer* con = static_cast<Consumer*>(consumer);
    delete con;
}

const char* get_topics_from_consumer(void* consumer)
{
    Consumer* con = static_cast<Consumer*>(consumer);
    return con->get_alltopicsstr().data();
}