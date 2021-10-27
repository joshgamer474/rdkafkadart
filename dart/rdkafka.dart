import 'dart:convert';
import 'dart:typed_data';

import 'lib/kafkaconsumer.dart';

void main() {
  final desctopics = [
    'S1H_description', 'S1W_description',
    'S1a_description', 'S2_description',
    'S2a_description', 'S3_description',
    'S3a_description', 'S4_description',
    'S4a_description', 'S5R_description',
    'S5a_description', 'S5l_description',
    'S6H_description', 'S6K_description',
    'S6a_description', 'S7D_description',
    'S7R_description', 'S8_description',
    'S8a_description', 'SM10_description',
    'SM10a_description', 'SM10b_description',
    'SM11_description', 'SM11a_description',
    'SM11b_description', 'SM12_description',
    'SM12a_description', 'SM5S_description',
    'SM5_plus_description', 'SM6_description',
    'SM6a_description', 'SM6b_description',
    'SM7a_description', 'SM7b_description',
    'SM8_description', 'SM8a_description',
    'SM8b_description', 'SM9_description',
    'SM9a_description'
  ];

  // Create consumer
  final KafkaConsumer consumer = KafkaConsumer("192.168.1.55:9092");
  print("Created consumer $consumer");

  // Get topics from Kafka
  final List<String> topics = consumer.get_topics_from_consumer();
  print("Found ${topics.length} topics: ${topics}");

  // Consume topics' messages
  Map<String, Map<int, Uint8List>>? msgs = consumer.consume(desctopics);
  // Print received messages
  if (msgs != null) {
    print("msgs len: ${msgs.length}");
    msgs.forEach((topic, msgmap) {
      msgmap.forEach((offset, data) {
        print("topic: $topic, offset: $offset, data: ${utf8.decode(data, allowMalformed: true)}");
      });
    });
  }

  // Destroy consumer
  consumer.destroy();
}