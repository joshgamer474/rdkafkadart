import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:dart/dart.dart';

void main() {
  const MethodChannel channel = MethodChannel('dart');

  TestWidgetsFlutterBinding.ensureInitialized();

  setUp(() {
    channel.setMockMethodCallHandler((MethodCall methodCall) async {
      return '42';
    });
  });

  tearDown(() {
    channel.setMockMethodCallHandler(null);
  });

  test('getPlatformVersion', () async {
    expect(await Dart.platformVersion, '42');
  });
}
