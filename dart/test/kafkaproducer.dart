import 'dart:typed_data';
import 'package:flutter/material.dart';

import 'package:rdkafka_dart/kafkaproducer.dart' as rdk;

class KafkaProducer {
  final String server;
  final int port;

  late rdk.KafkaProducer _producer;

  // Private constructor
  KafkaProducer._create(this.server, this.port) {
    // Create KafkaProducers, load rdkafkadart library
    _producer = rdk.KafkaProducer('$server:$port');
  }

  // Async constructor due to flutter path_provider requiring to be used post-constructed
  static Future<KafkaProducer> create(String server, int port) async {
    // Create KafkaProducer object
    KafkaProducer ret = KafkaProducer._create(server, port);
    // Create producer, wait until they're created
    await ret._producer.create_producer();
    return ret;
  }

  Future<void> produce(String topic, Uint8List rawbytes) async {
    // Produce to server
    _producer.produce(topic, rawbytes);
  }
}