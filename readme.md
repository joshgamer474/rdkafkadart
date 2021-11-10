# RdkafkaDart

[![Build Status](https://app.travis-ci.com/joshgamer474/rdkafkadart.svg?branch=master)](https://travis-ci.org/joshgamer474/rdkafkadart)

A functional Apache Kafka Dart library that utilizes edenhill's C/C++ [librdkafka library](https://github.com/edenhill/librdkafka) v1.8.2.

* Note: ```dart/``` contains the Dart library

## Progress

The RdkafkaDart library is currently able to connect to and read a given Kafka server's metadata, consume topics, and produce to PLAINTEXT Kafka servers.

The RdkafkaDart C++ library currently compiles for all major platforms (Windows, Linux, MacOS, Android, iOS) using the C++ package manager Conan.io.

It is **required** that you build the RdkafkaDart C++ library and then provide the built C++ library files to your Flutter project that uses the RdkafkaDart Flutter library in order for the Dart library to function as expected.

## Issues

* Only PLAINTEXT Kafka servers are supported at this time
* The Consumer currently only supports consuming a topic completely from start to end. More topic consumption options (e.g. consume from beginning, consume from end, consume x msgs, etc) will be made available in future library updates.

# Features

* High-level producer, including Idempotent and Transactional producers
* High-level balanced KafkaConsumer (requires broker >= 0.9)
* Broker version support: >=0.8
* Guaranteed API stability for C & C++ APIs (ABI safety guaranteed for C)
* Portable: runs on Linux, MacOS X, Windows, Android, iOS, ...

# Adding RdkafkaDart to your Flutter project
Add the following to your Flutter project's ```/pubspec.yaml``` to import the RdkafkaDart Flutter library:

```
rdkafka_dart:
  # Uncomment path: if using local cloned rdkafkadart
  #path: ../rdkafkadart
  git:
    url: https://github.com/joshgamer474/rdkafkadart.git
    path: dart
```

# Usage in Dart

* Note: RdkafkaDart requires additional C++ build steps before you can successfully use RdkafkaDart in your Flutter project.
  * See the **Build C++ RdkafkaDart** section.

## Kafka Consuming

### Creating a Kafka Consumer
``` dart
import 'package:rdkafka_dart/kafkaconsumer.dart' as rdk;

rdk.KafkaConsumer _consumer = rdk.KafkaConsumer('localhost:9092');
// Must always call create_consumer() after construction in order to initialize the C++ Kafka consumer
await _consumer.create_consumer();
```

### Getting all topics on a Kafka server
``` dart
final List<String> topics = _consumer.get_topics();
```

### Consuming topics synchronously (*messages are consumed asynchronous but the callback is made to be synchronous)
``` dart
final List<String> topics = ['a', 'b', 'c'];

// Consume topics a, b, and c w/ synchronous consume() callback
Map<String, Map<int, Uint8List>>? msgs =
  _consumer.consume(topics, timeout_ms: 1000);

// Hard-copy received msgs data from UintList8 to String
Map<String, Map<int, String>> decoded_msgs = Map<String, Map<int, String>>();
if (msgs != null) {
  msgs.forEach((topic, msgmap) {
    msgmap.forEach((offset, data) {
      // Decode and hard-copy Uint8List data to String
      final String value = String.fromCharCodes(data);
      // Store decoded string data into new map
      if (decoded_msgs.containsKey(topic) && decoded_msgs[topic] != null) {
        // Append existing map
        decoded_msgs[topic]![offset] = value;
      } else {
        decoded_msgs[topic] = Map<int, String>();
        decoded_msgs[topic]![offset] = value;
      }
      // Acknowledge message has been received and its C memory can be now freed
      _consumer.ack(topic, offset);
    });
  });
  // Free all ack'd messages C memory as our data
  // is now copied to the dart layer
  _consumer.cleanup_acked_msgs();
}
// Do something with decoded_msgs
```

### Destroying/Freeing the C++ Kafka Consumer
``` dart
// Must always call destroy() when you're done with/destructing the Kafka Consumer
// to ensure that C memory is freed
_consumer.destroy();
```

## Kafka Producing

### Creating a Kafka Producer
``` dart
import 'package:rdkafka_dart/kafkaproducer.dart' as rdk;

rdk.KafkaProducer _producer = rdk.KafkaProducer('localhost:9092');
// Must always call create() after construction in order to initialize the C++ Kafka producer
await _producer.create();
```

### Producing to a topic
``` dart
final String topic = "t";
Uint8List rawbytes = [0, 1, 2, 3, 4, 5];
_producer.produce(topic, rawbytes);
```

# Build C++ RdkafkaDart

## Prerequisites

* [CMake 3.x](https://cmake.org/download/)
* [Python 3.x](https://www.python.org/downloads/)
* pip3
* Conan.io via pip
* A compiler of your choice (Visual Studio, gcc, clang, etc)

## Prerequisite Commands
Install and configure Conan.io via pip:

```pip3 install conan --user```

```conan remote remove conan-center```

```conan remote add conan-center https://center.conan.io```

## Building C++ RdkafkaDart for development (Windows, Linux, MacOS)

```git clone https://github.com/joshgamer474/rdkafkadart.git```

```cd rdkafkadart```

```conan install . -if=build --build=outdated -s compiler.cppstd=17```

```conan build . -bf=build```

## Cross-building C++ RdkafkaDart for Android
```git clone https://github.com/joshgamer474/rdkafkadart.git```

```cd rdkafkadart```

```sudo chmod 777 buildandroidlibs.sh```

Here I suggest modifying the last line of ```buildandroidlibs.sh``` to automatically copy the newly built library files to your desired Flutter project's ```android/app``` folder:

```cp -r apkg/lib ../{YOUR_FLUTTER_PROJ_PATH}/android/app/conan_deploy```

 and then running

```./buildandroidlibs.sh```.

## Modify android/app/build.gradle
In order for Android to pull in the newly built libRdkafkaDart.so files, you must add the following to your ```android/app/build.gradle```.
```gradle
android {
    defaultConfig {
        ndk {
            // Specifies the ABI configurations of your native
            // libraries Gradle should build and package with your APK
            abiFilters 'x86', 'x86_64', 'armeabi-v7a', 'arm64-v8a'
        }
    }
    sourceSets {
        main.jniLibs.srcDirs = ['conan_deploy/lib']
    }
}
```
After these changes Flutter should successfully copy libRdkafkaDart.so into your Android appbundle at build time.

## Cross-building C++ RdkafkaDart for iOS
* Note: Must be ran on MacOS.

```git clone https://github.com/theodelrieu/conan-darwin-toolchain.git```

```cd conan-darwin-toolchain && conan export . theodelrieu/stable && cd ..```

```git clone https://github.com/joshgamer474/rdkafkadart.git```

```cd rdkafkadart```

```sudo chmod 777 buildioslibs.sh```

Here I suggest modifying the last line of ```buildioslibs.sh``` to automatically copy the newly built library files to your Flutter project's ```ios/Runner/Frameworks``` folder:

```cp -r apkg/lib/ ../{YOUR_FLUTTER_PROJ_PATH}/ios/Runner/Frameworks```

and then finally running

```./buildioslibs.sh```

## Modify ios/Runner.xcworkspace
* Note: Must be ran on MacOS.
* Note: All modern iOS devices are armv8. You only need to build a x86_64 library if you're using an iOS simulator.

In order for iOS to pull in the newly built libRdkafkaDart.dylib file, follow the instructions below to modify your ```ios/Runner.xcworkspace``` project file. More information on instructions is listed [here](https://flutter.dev/docs/development/platform-integration/c-interop).

1. Open ios/Runner.xcworkspace into Xcode
2. Click Runner under **TARGETS** and go to the **Build Phases** tab.
    * Drag libRdkafkaDart.dylib into **Link Binary With Libraries**, set status to *Optional*.
    * Drag libRdkafkaDart.dylib into the **Copy Bundle Resources** list.
    * Drag libRdkafkaDart.dylib into **Embed Libraries**, check *Code Sign on Copy*.

3. Click Runner under **TARGETS** and go to the **General tab**.
    * Drag libRdkafkaDart.dylib into the **Frameworks, Libararies and Embedded Content** list.
    * Select *Embed & Sign*.

4. Click Runner and go to the **Build Settings** tab.
    * In the Search Paths section configure the Library Search Paths to include the path where libRdkafkaDart.dylib is located, e.g.:
        ```
        $(inherited)
        $(PROJECT_DIR)/Runner/Frameworks/x86
        $(PROJECT_DIR)/Runner/Frameworks/x86_64
        $(PROJECT_DIR)/Runner/Frameworks/armeabi-v7a
        $(PROJECT_DIR)/Runner/Frameworks/arm64-v8a
        ```

After these changes Xcode should then successfully pull in libRdkafkaDart.dylib into your Flutter ios project.

# TODO

## Implement some Core features
- [x] Windows build support
- [x] Linux build support
- [x] Android build support
- [x] iOS build support
- [x] MacOS build support
- [x] Support both asynchronous and synchronous Kafka topic consuming
- [x] Implement C++ message receive callback
- [x] Implement C message receive callback (will be called back only on the main thread to support Android and iOS)
- [ ] Expose configuration for consumers and producers
- [ ] Hook up SSL support for consumers and producers

## Organization
- [x] Implement Conan.io package for this repo
- [x] Include CMake building
- [x] Include CMake cross-building profiles for Android and iOS


## Testing

### Core feature testing and correctness
- [x] Make unit tests for the C++ library
- [x] Make unit tests for the C exported library functions
- [ ] Make unit tests for the Flutter RdkafkaDart library
- [ ] Add encryption unit tests
