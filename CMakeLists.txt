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
    add_compile_definitions(RdkafkaDart_EXPORTS)
else()
    message("Building as STATIC")
    add_library(RdkafkaDart STATIC ${RDKAFKA_SOURCE} ${RDKAFKA_HEADERS})
endif()

if(BUILD_ANDROID)
    set(ANDROID_STL c++_shared)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fexceptions -frtti")
endif()

target_include_directories(RdkafkaDart PUBLIC
    ${CONAN_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(RdkafkaDart
    ${CONAN_TARGETS})

include_directories(${CONAN_INCLUDE_DIRS}
    "src/")

################################
# Copy ndk used libc++_shared.so to lib
################################
if(CMAKE_NM AND ANDROID_ABI)
  get_filename_component(NDKPARENT_DIR ${CMAKE_NM} DIRECTORY)
  get_filename_component(NDKPARENT_DIR ${NDKPARENT_DIR} DIRECTORY)
  set(NDKLIBPATH "${NDKPARENT_DIR}/sysroot/usr/lib")
  message("NDKLIBPATH ${NDKLIBPATH}")
  # x86
  if(${ANDROID_ABI} STREQUAL x86)
  set(USENDKLIBPATH ${NDKLIBPATH}/i686-linux-android)
  endif()
  # x86_64
  if(${ANDROID_ABI} STREQUAL x86_64)
  set(USENDKLIBPATH ${NDKLIBPATH}/x86_64-linux-android)
  endif()
  # armv7
  if(${ANDROID_ABI} STREQUAL armeabi-v7a)
    set(USENDKLIBPATH ${NDKLIBPATH}/arm-linux-androideabi)
  endif()
  # armv8
  if(${ANDROID_ABI} STREQUAL arm64-v8a)
    set(USENDKLIBPATH ${NDKLIBPATH}/aarch64-linux-android)
  endif()
  # Copy libc++_shared.so to build/lib
  if (USENDKLIBPATH)
    message("Copying ${USENDKLIBPATH}/libc++_shared.so to ${CMAKE_BINARY_DIR}/lib")
    file(COPY ${USENDKLIBPATH}/libc++_shared.so
        DESTINATION ${CMAKE_BINARY_DIR}/lib)
  endif()
endif()

# Enable unit testing
enable_testing()

if(BUILD_UNIT_TEST AND NOT BUILD_ANDROID)
# Add unit tests
add_subdirectory(test_package)
endif(BUILD_UNIT_TEST)
