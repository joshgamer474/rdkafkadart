import 'dart:convert';
import 'dart:ffi' as ffi;
import 'dart:io';
import 'dart:typed_data';
import 'package:path/path.dart' as path;
import 'package:ffi/ffi.dart';
import 'package:path_provider/path_provider.dart';

import 'package:rdkafka_dart/rdkafkalibrary.dart';
import 'package:rdkafka_dart/util/util.dart';

class KafkaProducer {
  // Rdkafka native library class
  late RdkafkaLibrary _nativelib;
  // Memory pointer to producer class
  ffi.Pointer<ffi.Void>? _native_instance;

  final String broker;

  KafkaProducer(String broker)
    : broker = broker {
    // Load rdkafka library
    _nativelib = RdkafkaLibrary(loadLibrary());
  }

  Future<void> create_producer() async {
    // Get app folder we can write logs to
    final Directory dir = await getTemporaryDirectory();
    final String temppath = dir.path;
    final String loglevel = "trace";

    // Convert temppath and loglevel to dart.ffi c pointer objects
    ffi.Pointer<Utf8> temppathptr = temppath.toNativeUtf8().cast<Utf8>();
    ffi.Pointer<Utf8> temploglevelptr = loglevel.toNativeUtf8().cast<Utf8>();
    // Set log path for lib
    print("Using path for logs: $temppath");
    _nativelib.set_logpath(temppathptr);
    _nativelib.set_loglevel(temploglevelptr);

    // Convert parameters to dart.ffi c pointer objects
    ffi.Pointer<ffi.Int8> brokerp = broker.toNativeUtf8().cast<ffi.Int8>();

    // Initialize Kafka Producer instance
    _native_instance = _nativelib.create_producer(brokerp);
  }

  /// Produce data to topic
  void produce(String topic, Uint8List data) {
      // Copy parameters into dart.ffi c pointer objects
      ffi.Pointer<Utf8> topicptr = topic.toNativeUtf8().cast<Utf8>();
      ffi.Pointer<ffi.Uint8> dataptr = malloc.allocate<ffi.Uint8>(data.length);
      for (int i = 0; i < data.length; i++) {
        dataptr[i] = data[i];
      }
      // Produce data to topic
      _nativelib.produce(_native_instance!, topicptr, dataptr, data.length);
      malloc.free(dataptr);
  }

  /// Destroys the created Kafka Producer
  void destroy() {
    if (_native_instance != null) {
      _nativelib.destroy_producer(_native_instance!);
      _native_instance = null;
    }
  }
}
