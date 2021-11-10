import 'dart:typed_data';
import 'package:rdkafka_dart/kafkaconsumer.dart' as rdk;

class KafkaConsumer {
  final String server;
  final int port;
  late rdk.KafkaConsumer _consumer;
  Set<String> _topics = Set<String>();

  // Private constructor
  KafkaConsumer._create(this.server, this.port) {
    // Create kafkaconsumers, load rdkafkadart library
    _consumer = rdk.KafkaConsumer('$server:$port');
  }

  // Async constructor due to flutter path_provider requiring to be used post-constructed
  static Future<KafkaConsumer> create(String server, int port) async {
    KafkaConsumer ret = KafkaConsumer._create(server, port);
    // Create consumer, wait until it is created
    await ret._consumer.create_consumer();
    // Get topics from Kafka server
    ret._topics = ret.get_topics();
    return ret;
  }

  Set<String> get_topics() {
    Set<String> ret = _consumer.get_topics_from_consumer().toSet();
    // Remove unwanted topic __consumer_offsets
    ret.remove('__consumer_offsets');
    return ret;
  }

  Map<String, Map<int, String>> consume_topics_raw(List<String> topics) {
    Map<String, Map<int, String>> ret = Map<String, Map<int, String>>();
    if (topics.isEmpty) {
      return ret;
    }

    // Consume messages from topics
    Map<String, Map<int, Uint8List>>? msgs =
        _consumer.consume(topics, timeout_ms: 1000);
    if (msgs == null) {
      return ret;
    }
    msgs.forEach((topic, msgmap) {
      msgmap.forEach((offset, data) {
        final String value = String.fromCharCodes(data);
        if (ret.containsKey(topic) && ret[topic] != null) {
          // Append existing map
          ret[topic]![offset] = value;
        } else {
          ret[topic] = Map<int, String>();
          ret[topic]![offset] = value;
        }
        _consumer.ack(topic, offset);
      });
    });
    _consumer.cleanup_acked_msgs();
    return ret;
  }
}
