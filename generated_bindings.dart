// AUTO GENERATED FILE, DO NOT EDIT.
//
// Generated by `package:ffigen`.
import 'dart:ffi' as ffi;

class NativeLibrary {
  /// Holds the symbol lookup function.
  final ffi.Pointer<T> Function<T extends ffi.NativeType>(String symbolName)
      _lookup;

  /// The symbols are looked up in [dynamicLibrary].
  NativeLibrary(ffi.DynamicLibrary dynamicLibrary)
      : _lookup = dynamicLibrary.lookup;

  /// The symbols are looked up with [lookup].
  NativeLibrary.fromLookup(
      ffi.Pointer<T> Function<T extends ffi.NativeType>(String symbolName)
          lookup)
      : _lookup = lookup;

  ffi.Pointer<ffi.Void> create_consumer(
    ffi.Pointer<ffi.Int8> broker,
    ffi.Pointer<ffi.Pointer<ffi.Int8>> topics,
    int topics_len,
  ) {
    return _create_consumer(
      broker,
      topics,
      topics_len,
    );
  }

  late final _create_consumerPtr = _lookup<
      ffi.NativeFunction<
          ffi.Pointer<ffi.Void> Function(
              ffi.Pointer<ffi.Int8>,
              ffi.Pointer<ffi.Pointer<ffi.Int8>>,
              ffi.Int32)>>('create_consumer');
  late final _create_consumer = _create_consumerPtr.asFunction<
      ffi.Pointer<ffi.Void> Function(
          ffi.Pointer<ffi.Int8>, ffi.Pointer<ffi.Pointer<ffi.Int8>>, int)>();

  void consume(
    ffi.Pointer<ffi.Void> consumer,
    int timeout_ms,
  ) {
    return _consume(
      consumer,
      timeout_ms,
    );
  }

  late final _consumePtr = _lookup<
      ffi.NativeFunction<
          ffi.Void Function(ffi.Pointer<ffi.Void>, ffi.Int32)>>('consume');
  late final _consume =
      _consumePtr.asFunction<void Function(ffi.Pointer<ffi.Void>, int)>();

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

  ffi.Pointer<ffi.Pointer<ffi.Int8>> get_topics_from_consumer(
    ffi.Pointer<ffi.Void> consumer,
  ) {
    return _get_topics_from_consumer(
      consumer,
    );
  }

  late final _get_topics_from_consumerPtr = _lookup<
      ffi.NativeFunction<
          ffi.Pointer<ffi.Pointer<ffi.Int8>> Function(
              ffi.Pointer<ffi.Void>)>>('get_topics_from_consumer');
  late final _get_topics_from_consumer =
      _get_topics_from_consumerPtr.asFunction<
          ffi.Pointer<ffi.Pointer<ffi.Int8>> Function(ffi.Pointer<ffi.Void>)>();
}