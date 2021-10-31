#include <producer.h>
#include <iostream>
#include <spdlog/spdlog.h>

Producer::Producer(std::string broker,
    std::function<void(std::string topic, std::vector<uint8_t>)> msg_callback)
    : broker(broker),
    msgs_produced(0),
    run(false),
    partition(RdKafka::Topic::PARTITION_UA),
    start_offset(RdKafka::Topic::OFFSET_BEGINNING),
    msg_callback(msg_callback)
{
    init();
}

Producer::~Producer()
{
    spdlog::info("~Producer()");
    //printf("Produced %zu messages\n", msgs_produced);
}

void Producer::init()
{
    // Init configs
    conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

    conf->set("metadata.broker.list", broker, errstr);
    conf->set("default_topic_conf", tconf, errstr);

    // Create Producer
    spdlog::info("Creating producer to broker {}", broker.c_str());
    producer = RdKafka::Producer::create(conf, errstr);
}

void Producer::produce(std::string topic, const std::vector<uint8_t>& data)
{
    spdlog::debug("Producing to topic {0} {1} bytes",
        topic.c_str(),
        data.size());
    RdKafka::ErrorCode resp = producer->produce(topic, partition,
        RdKafka::Producer::RK_MSG_COPY,
        const_cast<uint8_t *>(data.data()), data.size(),
        NULL,
        0,
        0,
        NULL,
        NULL);
    if (resp != RdKafka::ERR_NO_ERROR)
    {
        spdlog::error("Failed to produce to topic {0}, error {1}",
            topic.c_str(),
            RdKafka::err2str(resp));
        std::cerr << "% produce() failed: " << RdKafka::err2str(resp) << std::endl;
    }
    msgs_produced++;
    producer->poll(0);
}