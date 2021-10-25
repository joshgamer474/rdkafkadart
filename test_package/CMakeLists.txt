PROJECT(RdkafkaTestPackage)
cmake_minimum_required(VERSION 2.8.12)

set(CMAKE_BINARY_DIR ${CMAKE_BINARY_DIR}/test_package)

set(UNIT_TEST_SOURCE
    src/consumer.cpp
    src/producer.cpp)

include_directories(src)

add_executable(RdkafkaTestPackage
    ${UNIT_TEST_SOURCE})

target_link_libraries(RdkafkaTestPackage
    RdkafkaDart
    CONAN_PKG::gtest)

message("CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")