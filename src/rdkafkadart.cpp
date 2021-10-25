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

const char** get_topics_from_consumer(void* consumer)
{
    Consumer* con = static_cast<Consumer*>(consumer);
    const auto topics = con->get_alltopics();
    std::vector<const char*> topicsptr(topics.size());
    for (auto& topic : topics)
    {
        topicsptr.push_back(topic.data());
    }
    return topicsptr.data();
}