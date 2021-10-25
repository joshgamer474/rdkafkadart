import 'dart:ffi' as ffi;
import 'dart:io';
import 'package:path/path.dart' as path;
import 'package:ffi/ffi.dart';

import 'generated_bindings.dart';

typedef flutffi_native_create_consumer = ffi.Pointer<ffi.Void> Function(ffi.Pointer<ffi.Int8> broker, ffi.Pointer<ffi.Pointer<ffi.Int8>> topics, ffi.Int32 topics_len);
typedef CreateConsumer = ffi.Pointer<ffi.Void> Function(ffi.Pointer<ffi.Int8> broker, ffi.Pointer<ffi.Pointer<ffi.Int8>> topics, int topics_len);

typedef flutffi_native_consume = ffi.Void Function(ffi.Pointer<ffi.Void> consumer, ffi.Int32 timeout_ms);
typedef Consume = void Function(ffi.Pointer<ffi.Void>, int timeout_ms);

typedef flutffi_native_destroy_consumer = ffi.Void Function(ffi.Pointer<ffi.Void> consumer);
typedef DestroyConsumer = void Function(ffi.Pointer<ffi.Void> consumer);

typedef flutffi_native_get_topics_from_consumer = ffi.Pointer<ffi.Pointer<ffi.Int8>> Function(ffi.Pointer<ffi.Void> consumer);
typedef GetTopicsFromConsumer = ffi.Pointer<ffi.Pointer<ffi.Int8>> Function(ffi.Pointer<ffi.Void>);

String getLibraryPath() {
  /// Find library path
  var libraryPath = path.join(Directory.current.path, 'build_release', 'lib', 'libRdkafkaDart.so');
  if (Platform.isMacOS) {
    libraryPath = path.join(Directory.current.path, 'build_release', 'bin', 'Rdkafka.dylib');
  } else if (Platform.isWindows) {
    libraryPath = path.join(Directory.current.path, 'build_release', 'bin', 'Rdkafka.dll');
  }
  print("libraryPath: $libraryPath");
  return libraryPath;
}

ffi.DynamicLibrary loadLibrary() {
  /// Load library and return
  return ffi.DynamicLibrary.open(getLibraryPath());
}

class KafkaConsumer {
  // Memory pointer to class
  ffi.Pointer<ffi.Void>? _native_instance;
  // Loaded functions
  late CreateConsumer createconsumer;
  late Consume consume;
  late DestroyConsumer destroyconsumer;
  late GetTopicsFromConsumer gettopicsfromconsumer;

  KafkaConsumer(String broker, List<String> topics) {
    /// Load library
    final dylib = loadLibrary();

    /// Link dart functions to c++ library
    createconsumer = dylib
      .lookup<ffi.NativeFunction<flutffi_native_create_consumer>>('create_consumer')
      .asFunction();

    consume = dylib
      .lookup<ffi.NativeFunction<flutffi_native_consume>>('consume')
      .asFunction();

    destroyconsumer = dylib
      .lookup<ffi.NativeFunction<flutffi_native_destroy_consumer>>('destroy_consumer')
      .asFunction();

    gettopicsfromconsumer = dylib
      .lookup<ffi.NativeFunction<flutffi_native_get_topics_from_consumer>>('get_topics_from_consumer')
      .asFunction();

    /// Initialize KafkaConsumer instance
    ffi.Allocator alloc;
    ffi.Pointer<ffi.Int8> brokerp = broker.toNativeUtf8().cast<ffi.Int8>();
    final ffi.Pointer<ffi.Pointer<ffi.Int8>> topicp = calloc(topics.length);
    for (var i = 0; i < topics.length; i++) {
      topicp[i] = topics[i].toNativeUtf8().cast<ffi.Int8>();
    }
    _native_instance = createconsumer(brokerp, topicp, topics.length);

    // Free memory temporarily used
    calloc.free(topicp);
  }
}

void main() {
  //final KafkaConsumer consumer = KafkaConsumer("192.168.1.55:9092", ["SM11b"]);
  NativeLibrary nativelib = NativeLibrary(loadLibrary());


  final List<String> topics = ['SM11b'];
  ffi.Allocator alloc;
  ffi.Pointer<ffi.Int8> brokerp = "192.168.1.55:9092".toNativeUtf8().cast<ffi.Int8>();
  final ffi.Pointer<ffi.Pointer<ffi.Int8>> topicp = calloc(topics.length);
  for (var i = 0; i < topics.length; i++) {
    topicp[i] = topics[i].toNativeUtf8().cast<ffi.Int8>();
  }

  // Create consumer
  ffi.Pointer<ffi.Void> consumer = nativelib.create_consumer(brokerp, topicp, topics.length);
  print("Created consumer $consumer");

  // Get topics from consumer
  ffi.Pointer<ffi.Pointer<ffi.Int8>> topicsconsumed = nativelib.get_topics_from_consumer(consumer);
  print("Consumed topics: $topicsconsumed");
  List<String> topicsconsumedstr = [];
  for (var i = 0; i < topicsconsumed.length; i++) {
    topicsconsumedstr.add(utf8.encode(topicsconsumed.elementAt(i)));
  }

  // Consume topics from consumer
  nativelib.consume(consumer, 100);

  // Let consumer consume
  sleep(Duration(seconds: 3));

  // Destroy consumer
  print("Destroying consumer $consumer");
  nativelib.destroy_consumer(consumer);
}