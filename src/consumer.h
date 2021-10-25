#ifndef _CONSUMER_H_
#define _CONSUMER_H_
#include <functional>
#include <map>
#include <string>
#include <thread>
#include <vector>
#include <stddef.h>

#include <librdkafka/rdkafkacpp.h>

class Consumer {
public:
    Consumer(std::string broker, std::string topic,
        std::function<void(std::string topic, std::vector<uint8_t>)> msg_callback = nullptr);
    Consumer(std::string broker, std::vector<std::string> topics,
        std::function<void(std::string topic, std::vector<uint8_t>)> msg_callback = nullptr);
    virtual ~Consumer();

    void start(int timeout_ms=100);
    void stop();
    bool is_running();
    std::vector<std::string> get_alltopics();

    std::size_t msgs_consumed;

private:
    void init(std::vector<std::string> topics);
    void consume(int timeout_ms=100);
    RdKafka::ErrorCode consume_msg(std::string topic, RdKafka::Message* msg, void* opaque);

    bool run;
    int32_t partition;
    int64_t start_offset;
    std::thread consume_thread;
    std::string broker;
    std::map<std::string, RdKafka::Topic*> topic_handles;
    std::map<std::string, size_t> msgs_consumed_map;
    std::string errstr;
    RdKafka::Conf* conf;
    RdKafka::Conf* tconf;
    RdKafka::Consumer* consumer;
    std::vector<std::string> alltopics;

    std::function<void(std::string, std::vector<uint8_t>)> msg_callback;
};
#endif