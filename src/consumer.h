#ifndef _CONSUMER_H_
#define _CONSUMER_H_
#include <functional>
#include <map>
#include <string>
#include <stddef.h>
#include <thread>
#include <deque>
#include <vector>
#include <librdkafka/rdkafkacpp.h>

class Consumer {
public:
    Consumer(std::string broker,
        std::function<void(std::string topic, std::vector<uint8_t>)> msg_callback = nullptr,
        std::function<void(void* consumer, const char* topic, uint8_t* data,
            uint64_t len, int64_t offset)> cmsg_callback = nullptr);
    virtual ~Consumer();

    void start(const std::vector<std::string>& topics, int timeout_ms=100);
    void stop();
    bool is_running();
    const std::vector<std::string>& get_alltopics();
    const std::string& get_alltopicsstr();

    std::size_t msgs_consumed;

private:
    void init();
    void consume(int timeout_ms=100);
    RdKafka::ErrorCode consume_msg(std::string topic, RdKafka::Message* msg, void* opaque);
    void clear_queuedmsgs();
    void clear_sentmsgs();

    bool run;
    bool done_consuming;
    int32_t partition;
    int64_t start_offset;
    std::thread consume_thread;
    std::string broker;
    std::map<std::string, RdKafka::Topic*> topic_handles;
    std::map<std::string, size_t> msgs_consumed_map;
    std::deque<RdKafka::Message*> queued_msgs;
    std::deque<RdKafka::Message*> sent_msgs;
    std::string errstr;
    RdKafka::Conf* conf;
    RdKafka::Conf* tconf;
    RdKafka::Consumer* consumer;
    std::vector<std::string> alltopics;
    std::string alltopicsstr;

    std::function<void(std::string topic, std::vector<uint8_t> data)> msg_callback;
    std::function<void(void* consumer, const char* topic,
        uint8_t* data, uint64_t len, int64_t offset)> cmsg_callback;
};
#endif