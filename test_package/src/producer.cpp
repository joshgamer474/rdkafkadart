#include <gtest/gtest.h>
#include <chrono>
#include <vector>

#include <producer.h>
#include <consumer.h>

std::vector<std::vector<uint8_t>> msgs_consumed = std::vector<std::vector<uint8_t>>();

void consumer_msg_callback(std::string topic, std::vector<uint8_t> data)
{
    printf("Receieved %s msg length: %zu, %s\n",
        topic.c_str(),
        data.size(),
        std::string(data.begin(), data.end()).c_str());
    msgs_consumed.push_back(data);
}

TEST(ProducerTest, ProduceTest)
{
    const std::string broker = "192.168.1.55:9093";
    const std::vector<uint8_t> testdata = { 0, 1, 2, 3, 4, 5, 6, 7 };
    std::unique_ptr<Producer> producer = std::make_unique<Producer>(broker);
    producer->produce("p", testdata);

    // Create consumer to verify that producer produced message
    std::unique_ptr<Consumer> consumer = std::make_unique<Consumer>(broker, "p", consumer_msg_callback);
    consumer->start();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    consumer->stop();
    
    // Check if consumed messages contain produced messages
    bool consumed_produced_test_data = false;
    for (const std::vector<uint8_t> data : msgs_consumed)
    {
        if (data == testdata)
        {
            consumed_produced_test_data = true;
        }
    }
    EXPECT_GT(producer->msgs_produced, 0);
    EXPECT_GT(consumer->msgs_consumed, 0);
    EXPECT_TRUE(consumed_produced_test_data);
}