from confluent_kafka import Consumer, KafkaError

# Configure the consumer
conf = {
    'bootstrap.servers': 'localhost:9092',
    'group.id': 'my_consumer_group',  # Choose a unique consumer group id
    'auto.offset.reset': 'earliest',  # Start consuming from the beginning of the topic
}

consumer = Consumer(conf)

# Subscribe to the Kafka topic
consumer.subscribe(['quickstart-events'])

try:
    # Start an infinite loop to continuously consume messages
    while True:
        msg = consumer.poll(2000)  # 1000ms timeout

        if msg is None:
            continue
        if msg.error():
            if msg.error().code() == KafkaError._PARTITION_EOF:
                # End of partition event, not an error
                print(f"Reached end of partition {msg.partition()} for topic {msg.topic()}")
            else:
                print(f"Error: {msg.error()}")
        else:
            # Process the received message
            print(f"Received message: {msg.value().decode('utf-8')}")

except KeyboardInterrupt:
    pass
finally:
    # Close the consumer
    consumer.close()
