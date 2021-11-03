#include <gtest/gtest.h>
#include <chrono>
#include <string>
#include <vector>

#include <consumer.h>
#include <rdkafkadart.h>

/*
// Kafka Consumer methods
RDK_EXPORT void* create_consumer(char* broker, char** topics, int topics_len,
  void (*ccmsg_callback)(void* consumer, const char* topic, uint8_t* data, uint64_t len));
RDK_EXPORT void consume(void* consumer, int timeout_ms = 100);
RDK_EXPORT void destroy_consumer(void* consumer);
RDK_EXPORT const char* get_topics_from_consumer(void* consumer);
*/

void cmsg_callback(void* consumer, const char* topic,
    uint8_t* data, uint64_t len, int64_t offset)
{
    const std::string datastr = reinterpret_cast<char *>(data);
    printf("Receieved %s msg length: %zu, %s\n",
        topic,
        len,
        datastr.c_str());
}

const std::string broker = "192.168.1.55:9092";

TEST(RdkafkaDart, RdkafkaDartCreateConsumer)
{
    void* consumer = create_consumer(broker.c_str(),
        cmsg_callback);

    Consumer* cons = reinterpret_cast<Consumer*>(consumer);
    const uint64_t msgs_consumed = cons->msgs_consumed;
    const bool is_running = cons->is_running();
    destroy_consumer(consumer);

    EXPECT_FALSE(is_running);
    EXPECT_EQ(msgs_consumed, 0);
}

TEST(RdkafkaDart, RdkafkaDartGetTopicsTest)
{
    void* consumer = create_consumer(broker.c_str(),
        cmsg_callback);

    Consumer* cons = reinterpret_cast<Consumer*>(consumer);
    const std::string topicsstr = cons->get_alltopicsstr();
    const uint64_t msgs_consumed = cons->msgs_consumed;
    const bool is_running = cons->is_running();
    destroy_consumer(consumer);

    EXPECT_GT(topicsstr.size(), 0);
    EXPECT_FALSE(is_running);
    EXPECT_EQ(msgs_consumed, 0);
}

TEST(RdkafkaDart, RdkafkaDartConsumerConsume)
{
    const std::vector<std::string> topics = {
        "SM11b",
        "SM11b_description"
    };

    void* consumer = create_consumer(broker.c_str(),
        cmsg_callback);
    Consumer* cons = reinterpret_cast<Consumer*>(consumer);
    // Consume topics async with synchronous cmsg_callback()
    cons->start(topics);
    // Stop consumer thread after consuming is completed
    cons->stop();
    const uint64_t msgs_consumed = cons->msgs_consumed;
    const bool is_running = cons->is_running();
    destroy_consumer(consumer);

    EXPECT_FALSE(is_running);
    EXPECT_GT(msgs_consumed, 0);
}

TEST(RdkafkaDart, RdkafkaDartConsumerConsumeStressTest)
{
    const std::vector<std::string> topics = {
        "SM11b",
        "SM11b_description"
    };
    const std::vector<std::string> topics2 = {
        "SM11",
        "SM11_description"
    };
    const std::vector<std::string> topics3 = {
        "SM10",
        "SM10_description"
    };
    void* consumer = create_consumer(broker.c_str(),
        cmsg_callback);

    Consumer* cons = reinterpret_cast<Consumer*>(consumer);
    // Consume topics async with synchronous cmsg_callback()
    cons->start(topics);
    // Stop consumer thread after consuming is completed
    //cons->stop();
    // Consume new set of topics
    cons->start(topics2);
    // Consume new set of topics again
    cons->start(topics3);
    // Stop consuming
    cons->stop();

    // Get checkable variables
    const uint64_t msgs_consumed = cons->msgs_consumed;
    const bool is_running = cons->is_running();

    // Destroy consumer
    destroy_consumer(consumer);

    EXPECT_FALSE(is_running);
    EXPECT_GT(msgs_consumed, 0);
}

TEST(RdkafkaDart, RdkafkaDartConsumerTestDestruction)
{
    // Create consumer
    void* consumer = create_consumer(broker.c_str(),
        cmsg_callback);
    Consumer* con = static_cast<Consumer*>(consumer);

    // Destroy consumer
    delete con;

    // Create new consumer
    const std::vector<std::string> topics = {
        "SM11b",
        "SM11b_description"
    };
    consumer = create_consumer(broker.c_str(),
        cmsg_callback);

    con = static_cast<Consumer*>(consumer);
    // Consume topics async with synchronous cmsg_callback()
    con->start(topics);
    // Stop consumer thread after consuming is completed
    con->stop();
    // Restart consumer
    con->start(topics);
    con->stop();
    const uint64_t msgs_consumed = con->msgs_consumed;
    const bool is_running = con->is_running();
    destroy_consumer(consumer);

    EXPECT_FALSE(is_running);
    EXPECT_GT(msgs_consumed, 0);
}