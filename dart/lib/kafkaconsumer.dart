import 'dart:convert';
import 'dart:ffi' as ffi;
import 'dart:io';
import 'dart:typed_data';
import 'package:path/path.dart' as path;
import 'package:ffi/ffi.dart';

import 'package:rdkafka_dart/rdkafkalibrary.dart';
import 'package:rdkafka_dart/util/util.dart';

class KafkaConsumer {
  // Rdkafka native library class
  late RdkafkaLibrary _nativelib;
  // Memory pointer to class
  ffi.Pointer<ffi.Void>? _native_instance;
  // Map to hold consumed messages in
  static Map<ffi.Pointer<ffi.Void>, Map<String, Map<int, Uint8List>>> _consumed_msgs =
    Map<ffi.Pointer<ffi.Void>, Map<String, Map<int, Uint8List>>>();
  // Map to hold marked for deletion messages
  Map<String, List<int>> _ackd_msgs = Map<String, List<int>>();

  KafkaConsumer(String broker) {
    init(broker);
  }

  void init(String broker) {
    // Load rdkafka library
    _nativelib = RdkafkaLibrary(loadLibrary());

    // Convert parameters to dart.ffi vars
    ffi.Pointer<ffi.Int8> brokerp = broker.toNativeUtf8().cast<ffi.Int8>();

    // Initialize Kafka Consumer instance
    _native_instance = _nativelib.create_consumer(
        brokerp,
        ffi.Pointer.fromFunction<cmsgcallback>(cmsg_callback));
  }

  /// RdKafka C message receive callback
  static void cmsg_callback(ffi.Pointer<ffi.Void> consumer,
    ffi.Pointer<Utf8> topic,
    ffi.Pointer<ffi.Uint8> data,
    int datalen, 
    int offset) {
    final String topicstr = topic.toDartString();
    final Uint8List datalist = data.asTypedList(datalen);
    final String datastr = utf8.decode(datalist);
    // Ensure _consumed_msgs is properly initialized
    if (!_consumed_msgs.containsKey(consumer)) {
      _consumed_msgs[consumer] = Map<String, Map<int, Uint8List>>();
    }
    if (!_consumed_msgs[consumer]!.containsKey(topicstr)) {
      _consumed_msgs[consumer]![topicstr] = Map<int, Uint8List>();
    }
    // Store received message to be accessed later from non-static method
    _consumed_msgs[consumer]![topicstr]![offset] = datalist;
   // print("cmsg_callback() topic: $topicstr, datalen: ${datalen}, data: ${datastr}");
  }

  /// Returns a List<String> containing the Kafka server's topics
  List<String> get_topics_from_consumer() {
    if (_native_instance == null) {
      return [];
    }
    // Get topics from consumer
    ffi.Pointer<Utf8> topicsconsumed = _nativelib.get_topics_from_consumer(_native_instance!);
    final String topicsdatastr = topicsconsumed.toDartString();
    return topicsdatastr.split(',');
  }

  /// Consume all topics from Kafka
  Map<String, Map<int, Uint8List>>? consume(List<String> topics, {int timeout_ms = 100}) {
    if (_native_instance != null) {
      // Convert parameter topics to dart.ffi var
      final ffi.Pointer<ffi.Pointer<ffi.Int8>> topicsp = calloc(topics.length);
      for (var i = 0; i < topics.length; i++) {
        topicsp[i] = topics[i].toNativeUtf8().cast<ffi.Int8>();
      }
      _nativelib.consume(_native_instance!, topicsp, topics.length, timeout_ms);
      // Free memory for temporary ffi pointer
      calloc.free(topicsp);
    }

    if (_consumed_msgs.containsKey(_native_instance)) {
      return _consumed_msgs[_native_instance]!;
    }
  }

  /// Users should call this method when acknowledging consumption of message
  /// ACK removes message from _consumed_msgs map
  void ack(String topic, int offset) {
    if (_consumed_msgs.containsKey(_native_instance)) {
      if (_consumed_msgs[_native_instance]!.containsKey(topic) &&
        _consumed_msgs[_native_instance]![topic]!.containsKey(offset)) {
          if (!_ackd_msgs.containsKey(topic)) {
            _ackd_msgs[topic] = [];
          }
          _ackd_msgs[topic]!.add(offset);
        }
    }
  }

  void cleanup_acked_msgs() {
    if (_native_instance == null) {
      return;
    }
    if (_consumed_msgs.containsKey(_native_instance)) {
      _ackd_msgs.forEach((topic, offsetlist) {
        if (_consumed_msgs[_native_instance]!.containsKey(topic)) {
          offsetlist.forEach((offset) {
            if (_consumed_msgs[_native_instance]![topic]!.containsKey(offset)) {
              _consumed_msgs[_native_instance]![topic]!.remove(offset);
            }
          });
          // Check if topic is now empty, remove its key if so
          if (_consumed_msgs[_native_instance]![topic]!.isEmpty) {
            _consumed_msgs[_native_instance]!.remove(topic);
          }
        }
      });
      // Check if map for _native_instance is empty now
      if (_consumed_msgs[_native_instance]!.isEmpty) {
        _consumed_msgs.remove(_native_instance);
      }
    }
  }

  /// Destroys the created Kafka Consumer
  void destroy() {
    if (_native_instance != null) {
      if (_consumed_msgs.containsKey(_native_instance)) {
        _consumed_msgs.remove(_native_instance);
      }
      _nativelib.destroy_consumer(_native_instance!);
      _native_instance = null;
    }
  }
}