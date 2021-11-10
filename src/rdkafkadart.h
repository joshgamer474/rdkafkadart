#ifndef _RDKAFKADART_H_
#define _RDKAFKADART_H_

#include <map>
#include <string>
#include <vector>

#if defined(_MSC_VER)
#define EXPORT extern "C" __declspec(dllexport)
#define IMPORT extern "C" __declspec(dllimport)
#elif defined(__GNUC__)
#define EXPORT extern "C" __attribute__((visibility("default")))
#define IMPORT
#else
#define EXPORT
#define IMPORT
#endif

#ifdef RdkafkaDart_EXPORTS
#define RDK_EXPORT EXPORT
#else
#define RDK_EXPORT IMPORT
#endif

// Log path init method
RDK_EXPORT void set_logpath(const char* logpath);
RDK_EXPORT void set_loglevel(const char* loglevel);

// Kafka Consumer methods
RDK_EXPORT void* create_consumer(const char* broker,
    void (*cmsg_callback)(void* consumer, const char* topic,
      uint8_t* data, uint64_t len, int64_t offset) = nullptr);
RDK_EXPORT void consume(void* consumer, const char** topics, int topics_len, int timeout_ms = 100);
RDK_EXPORT void destroy_consumer(void* consumer);
RDK_EXPORT const char* get_topics_from_consumer(void* consumer);
RDK_EXPORT void ack(void* consumer);
RDK_EXPORT void ack_all(void* consumer);

// Kafka Producer methods
RDK_EXPORT void* create_producer(const char* broker);
RDK_EXPORT void produce(void* producer, const char* topic, uint8_t* data, uint64_t len);
RDK_EXPORT void destroy_producer(void* producer);
#endif