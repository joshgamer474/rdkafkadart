#include <gtest/gtest.h>
#include <chrono>
#include <string>
#include <vector>

#include <consumer.h>

void msg_callback(std::string topic, std::vector<uint8_t> data)
{
    printf("Receieved %s msg length: %zu, %s\n",
        topic.c_str(),
        data.size(),
        std::string(data.begin(), data.begin() + data.size()).c_str());
}

TEST(ConsumerTest, ConsumerGetTopicsTest)
{
    std::unique_ptr<Consumer> consumer = std::make_unique<Consumer>
        ("192.168.1.55:9092", "SM7a_description", msg_callback);
    const auto alltopics = consumer->get_alltopics();
    printf("Found %zu topics\n", alltopics.size());
    EXPECT_GT(alltopics.size(), 0);
}

TEST(ConsumerTest, ConsumeTest)
{
    std::unique_ptr<Consumer> consumer = std::make_unique<Consumer>
        ("192.168.1.55:9092", "SM7a_description", msg_callback);
    consumer->start();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    consumer->stop();
    EXPECT_GT(consumer->msgs_consumed, 0);
}

TEST(ConsumerTest, ConsumeMultipleTest)
{
    std::vector<std::string> topics = { "SM7a_description", "SM7a" };
    std::unique_ptr<Consumer> consumer = std::make_unique<Consumer>
        ("192.168.1.55:9092", topics, msg_callback);
    consumer->start();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    consumer->stop();
    EXPECT_GT(consumer->msgs_consumed, 0);
}