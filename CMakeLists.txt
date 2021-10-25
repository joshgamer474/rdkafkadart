PROJECT(rdkafka_bin)
cmake_minimum_required(VERSION 2.8.12)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(RDKAFKA_HEADERS
  src/consumer.h
  src/producer.h
  src/rdkafkadart.h
)

set(RDKAFKA_SOURCE
  src/consumer.cpp
  src/producer.cpp
  src/rdkafkadart.cpp
)

# Create RdkafkaDart lib
if(BUILD_SHARED_LIBS)
    message("Building as SHARED")
    add_library(RdkafkaDart SHARED ${RDKAFKA_SOURCE} ${RDKAFKA_HEADERS})
else()
    message("Building as STATIC")
    add_library(RdkafkaDart STATIC ${RDKAFKA_SOURCE} ${RDKAFKA_HEADERS})
endif()

target_include_directories(RdkafkaDart PUBLIC
    ${CONAN_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(RdkafkaDart
    ${CONAN_TARGETS})

include_directories(${CONAN_INCLUDE_DIRS}
    "src/")

# Enable unit testing
enable_testing()

if(BUILD_UNIT_TEST)
# Add unit tests
add_subdirectory(test_package)
endif(BUILD_UNIT_TEST)
