import 'dart:convert';
import 'dart:ffi' as ffi;
import 'dart:io';
import 'dart:typed_data';
import 'package:path/path.dart' as path;
import 'package:ffi/ffi.dart';

import 'lib/rdkafkalibrary.dart';

typedef cmsgcallback = ffi.Void Function(ffi.Pointer<ffi.Void> consumer, ffi.Pointer<Utf8> topic,
  ffi.Pointer<ffi.Uint8> data, ffi.Uint64 datalen);

/// Returns rdkafka library path
String getLibraryPath() {
  var libraryPath = path.join(Directory.current.parent.path, 'build_release', 'lib', 'libRdkafkaDart.so');
  if (Platform.isMacOS) {
    libraryPath = path.join(Directory.current.parent.path, 'build_release', 'bin', 'Rdkafka.dylib');
  } else if (Platform.isWindows) {
    libraryPath = path.join(Directory.current.parent.path, 'build_release', 'bin', 'RdkafkaDart.dll');
  }
  print("libraryPath: $libraryPath");
  return libraryPath;
}

/// Load library and return
ffi.DynamicLibrary loadLibrary() {
  return ffi.DynamicLibrary.open(getLibraryPath());
}

class KafkaConsumer {
  // Rdkafka native library class
  late RdkafkaLibrary _nativelib;
  // Memory pointer to class
  ffi.Pointer<ffi.Void>? _native_instance;
  // Map to hold consumed messages in
  static Map<ffi.Pointer<ffi.Void>, Map<String, Map<int, Uint8List>>> _consumed_msgs =
    Map<ffi.Pointer<ffi.Void>, Map<String, Map<int, Uint8List>>>();

  KafkaConsumer(String broker, List<String> topics) {
    // Load rdkafka library
    _nativelib = RdkafkaLibrary(loadLibrary());

    // Convert parameters to dart.ffi vars
    ffi.Pointer<ffi.Int8> brokerp = broker.toNativeUtf8().cast<ffi.Int8>();
    final ffi.Pointer<ffi.Pointer<ffi.Int8>> topicsp = calloc(topics.length);
    for (var i = 0; i < topics.length; i++) {
      topicsp[i] = topics[i].toNativeUtf8().cast<ffi.Int8>();
    }

    // Initialize Kafka Consumer instance
    _native_instance = _nativelib.create_consumer(
        brokerp,
        topicsp,
        topics.length,
        ffi.Pointer.fromFunction<cmsgcallback>(cmsg_callback));

    // Free memory for temporary ffi pointer
    calloc.free(topicsp);
  }

  /// RdKafka C message receive callback
  static void cmsg_callback(ffi.Pointer<ffi.Void> consumer,
    ffi.Pointer<Utf8> topic, ffi.Pointer<ffi.Uint8> data, int datalen) {
    final String topicstr = topic.toDartString();
    final Uint8List datalist = data.asTypedList(datalen);
    final String datastr = utf8.decode(datalist);
    if (!_consumed_msgs.containsKey(consumer)) {
      _consumed_msgs[consumer] = Map<String, Map<int, Uint8List>>();
    }
    if (!_consumed_msgs[consumer]!.containsKey(topicstr)) {
      _consumed_msgs[consumer]![topicstr] = Map<int, Uint8List>();
    }
    final int len = _consumed_msgs[consumer]![topicstr]!.length;
    // Store received message to be accessed later from non-static method
    _consumed_msgs[consumer]![topicstr]![len] = datalist;
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
  Map<String, Map<int, Uint8List>>? consume({int timeout_ms = 100}) {
    if (_native_instance != null) {
      _nativelib.consume(_native_instance!, timeout_ms);
    }

    if (_consumed_msgs.containsKey(_native_instance)) {
      return _consumed_msgs[_native_instance]!;
    }
  }

  /// Destroys the created Kafka Consumer
  void destroy() {
    if (_native_instance != null) {
      _nativelib.destroy_consumer(_native_instance!);
    }
  }
}

void main() {
  // Create consumer
  final KafkaConsumer consumer = KafkaConsumer("192.168.1.55:9092", ["SM11b", "SM11b_description"]);
  print("Created consumer $consumer");

  // Get topics from Kafka
  final List<String> topics = consumer.get_topics_from_consumer();
  print("Found ${topics.length} topics: ${topics}");

  // Consume topics' messages
  Map<String, Map<int, Uint8List>>? msgs = consumer.consume();
  // Print received messages
  if (msgs != null) {
    print("msgs len: ${msgs.length}");
    msgs.forEach((topic, msgmap) {
      msgmap.forEach((key, data) {
        print("topic: $topic, key: $key, data: ${utf8.decode(data, allowMalformed: true)}");
      });
    });
  }

  // Destroy consumer
  consumer.destroy();
}