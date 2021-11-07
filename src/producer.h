#ifndef _PRODUCER_H_
#define _PRODUCER_H_
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <librdkafka/rdkafkacpp.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

class Producer {
public:
    Producer(std::string broker,
        std::function<void(std::string topic, std::vector<uint8_t>)> msg_callback = nullptr,
        std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> logsink = nullptr,
        spdlog::level::level_enum loglevel = spdlog::level::err);
    virtual ~Producer();

    RdKafka::ErrorCode produce(std::string topic, const std::vector<uint8_t>& data);
    std::size_t msgs_produced;

private:
    void init();

    std::shared_ptr<spdlog::logger> logger;
    bool run;
    int32_t partition;
    int64_t start_offset;
    std::thread consume_thread;
    std::string broker;
    std::string errstr;
    RdKafka::Conf* conf;
    RdKafka::Conf* tconf;
    RdKafka::Producer* producer;

    std::function<void(std::string, std::vector<uint8_t>)> msg_callback;
};
#endif