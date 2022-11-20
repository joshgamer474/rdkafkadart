// Generated by `package:ffigen`.
// Updated by josh
import 'dart:ffi' as ffi;
import 'package:ffi/ffi.dart';

typedef cmsgcallback = ffi.Void Function(
    ffi.Pointer<ffi.Void> consumer,
    ffi.Pointer<Utf8> topic,
    ffi.Pointer<ffi.Uint8> data,
    ffi.Uint64 datalen,
    ffi.Int64 offset);

class RdkafkaLibrary {
  /// Holds the symbol lookup function.
  final ffi.Pointer<T> Function<T extends ffi.NativeType>(String symbolName)
      _lookup;

  /// The symbols are looked up in [dynamicLibrary].
  RdkafkaLibrary(ffi.DynamicLibrary dynamicLibrary)
      : _lookup = dynamicLibrary.lookup;

  /// The symbols are looked up with [lookup].
  RdkafkaLibrary.fromLookup(
      ffi.Pointer<T> Function<T extends ffi.NativeType>(String symbolName)
          lookup)
      : _lookup = lookup;

  /// set_logpath()
  void set_logpath(ffi.Pointer<Utf8> logpath) {
    return _set_logpath(
      logpath,
    );
  }
  late final _set_logpathPtr =
      _lookup<ffi.NativeFunction<ffi.Void Function(ffi.Pointer<Utf8>)>>(
          'set_logpath');
  late final _set_logpath =
      _set_logpathPtr.asFunction<void Function(ffi.Pointer<Utf8>)>();

  /// set_loglevel()
  void set_loglevel(ffi.Pointer<Utf8> logpath) {
    return _set_loglevel(
      logpath,
    );
  }
  late final _set_loglevelPtr =
      _lookup<ffi.NativeFunction<ffi.Void Function(ffi.Pointer<Utf8>)>>(
          'set_loglevel');
  late final _set_loglevel =
      _set_loglevelPtr.asFunction<void Function(ffi.Pointer<Utf8>)>();

  /// Kafka Consumer methods
  // create_consumer()
  ffi.Pointer<ffi.Void> create_consumer(
    ffi.Pointer<ffi.Int8> broker,
    ffi.Pointer<ffi.NativeFunction<cmsgcallback>> cmsgcallback,
    ffi.Pointer<ffi.Int64> start_offset,
  ) {
    return _create_consumer(
      broker,
      cmsgcallback,
      start_offset,
    );
  }
  late final _create_consumerPtr = _lookup<
          ffi.NativeFunction<
              ffi.Pointer<ffi.Void> Function(ffi.Pointer<ffi.Int8>,
                  ffi.Pointer<ffi.NativeFunction<cmsgcallback>>,
                  ffi.Pointer<ffi.Int64>)>>(
      'create_consumer');
  late final _create_consumer = _create_consumerPtr.asFunction<
      ffi.Pointer<ffi.Void> Function(ffi.Pointer<ffi.Int8>,
          ffi.Pointer<ffi.NativeFunction<cmsgcallback>>,
          ffi.Pointer<ffi.Int64>)>();

  // consume()
  void consume(
    ffi.Pointer<ffi.Void> consumer,
    ffi.Pointer<ffi.Pointer<ffi.Int8>> topics,
    int topicslen,
    int timeout_ms,
  ) {
    return _consume(
      consumer,
      topics,
      topicslen,
      timeout_ms,
    );
  }
  late final _consumePtr = _lookup<
      ffi.NativeFunction<
          ffi.Void Function(
              ffi.Pointer<ffi.Void>,
              ffi.Pointer<ffi.Pointer<ffi.Int8>>,
              ffi.Int32,
              ffi.Int32)>>('consume');
  late final _consume = _consumePtr.asFunction<
      void Function(ffi.Pointer<ffi.Void>, ffi.Pointer<ffi.Pointer<ffi.Int8>>,
          int, int)>();

  // destroy_consumer()
  void destroy_consumer(
    ffi.Pointer<ffi.Void> consumer,
  ) {
    return _destroy_consumer(
      consumer,
    );
  }
  late final _destroy_consumerPtr =
      _lookup<ffi.NativeFunction<ffi.Void Function(ffi.Pointer<ffi.Void>)>>(
          'destroy_consumer');
  late final _destroy_consumer =
      _destroy_consumerPtr.asFunction<void Function(ffi.Pointer<ffi.Void>)>();

  // get_topics_from_consumer()
  ffi.Pointer<Utf8> get_topics_from_consumer(ffi.Pointer<ffi.Void> consumer) {
    return _get_topics_from_consumer(consumer);
  }
  late final _get_topics_from_consumerPtr = _lookup<
      ffi.NativeFunction<
          ffi.Pointer<Utf8> Function(
              ffi.Pointer<ffi.Void>)>>('get_topics_from_consumer');
  late final _get_topics_from_consumer = _get_topics_from_consumerPtr
      .asFunction<ffi.Pointer<Utf8> Function(ffi.Pointer<ffi.Void>)>();

  // ack()
  void ack(
    ffi.Pointer<ffi.Void> consumer,
  ) {
    return _ack(
      consumer,
    );
  }
  late final _ackPtr =
      _lookup<ffi.NativeFunction<ffi.Void Function(ffi.Pointer<ffi.Void>)>>(
          'ack');
  late final _ack =
      _ackPtr.asFunction<void Function(ffi.Pointer<ffi.Void>)>();

  // ack_all()
  void ack_all(
    ffi.Pointer<ffi.Void> consumer,
  ) {
    return _ack_all(
      consumer,
    );
  }
  late final _ack_allPtr =
      _lookup<ffi.NativeFunction<ffi.Void Function(ffi.Pointer<ffi.Void>)>>(
          'ack_all');
  late final _ack_all =
      _ack_allPtr.asFunction<void Function(ffi.Pointer<ffi.Void>)>();


  /// Kafka Producer methods
  // create_producer()
  ffi.Pointer<ffi.Void> create_producer(
    ffi.Pointer<ffi.Int8> broker
  ) {
    return _create_producer(
      broker,
    );
  }
  late final _create_producerPtr = _lookup<
          ffi.NativeFunction<
              ffi.Pointer<ffi.Void> Function(ffi.Pointer<ffi.Int8>)>>(
      'create_producer');
  late final _create_producer = _create_producerPtr.asFunction<
      ffi.Pointer<ffi.Void> Function(ffi.Pointer<ffi.Int8>)>();

  // produce()
  void produce(
    ffi.Pointer<ffi.Void> producer,
    ffi.Pointer<Utf8> topic,
    ffi.Pointer<ffi.Uint8> data,
    int datalen,
  ) {
    return _produce(
      producer,
      topic,
      data,
      datalen,
    );
  }
  late final _producePtr = _lookup<
      ffi.NativeFunction<
          ffi.Void Function(
              ffi.Pointer<ffi.Void>,
              ffi.Pointer<Utf8>,
              ffi.Pointer<ffi.Uint8>,
              ffi.Uint64)>>('produce');
  late final _produce = _producePtr.asFunction<
      void Function(ffi.Pointer<ffi.Void>,
        ffi.Pointer<Utf8>,
        ffi.Pointer<ffi.Uint8>,
        int)>();

  // destroy_producer()
  void destroy_producer(
    ffi.Pointer<ffi.Void> producer,
  ) {
    return _destroy_producer(
      producer,
    );
  }
  late final _destroy_producerPtr =
      _lookup<ffi.NativeFunction<ffi.Void Function(ffi.Pointer<ffi.Void>)>>(
          'destroy_producer');
  late final _destroy_producer =
      _destroy_producerPtr.asFunction<void Function(ffi.Pointer<ffi.Void>)>();
}
