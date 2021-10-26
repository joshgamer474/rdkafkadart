#include <rdkafkadart.h>
#include <consumer.h>
#include <producer.h>

void* create_consumer(char* broker, char** topics, int topics_len)
{
    std::vector<std::string> topicsstr(topics, topics + topics_len);
    return new Consumer(broker, topicsstr);
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