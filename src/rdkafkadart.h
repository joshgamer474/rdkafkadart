#ifndef _RDKAFKADART_H_
#define _RDKAFKADART_H_

#include <map>
#include <string>
#include <vector>

#ifdef RdkafkaDart_EXPORTS
#define RDK_EXPORT extern "C" __declspec(dllexport)
#else
#define RDK_EXPORT extern "C" __declspec(dllimport)
#endif

RDK_EXPORT void* create_consumer(char* broker, char** topics, int topics_len);
RDK_EXPORT void consume(void* consumer, int timeout_ms = 100);
RDK_EXPORT void destroy_consumer(void* consumer);
RDK_EXPORT const char** get_topics_from_consumer(void* consumer);

#endif