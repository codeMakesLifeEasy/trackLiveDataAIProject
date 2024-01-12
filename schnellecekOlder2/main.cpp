
#include <iostream>
#include <thread>
#include <string>
#include <librdkafka/rdkafka.h>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "messageHandler.hpp"


int main() {
    rd_kafka_t* rk;
    rd_kafka_conf_t* conf;
    rd_kafka_topic_partition_list_t* topics;

    const char* brokers = "localhost:9092";
    const char* group_id = "sentics_consumer_group";

    conf = rd_kafka_conf_new();

    if (rd_kafka_conf_set(conf, "bootstrap.servers", brokers, NULL, 0) != RD_KAFKA_CONF_OK) {
        fprintf(stderr, "%% Failed to set bootstrap.servers\n");
        exit(1);
    }

    if (rd_kafka_conf_set(conf, "group.id", group_id, NULL, 0) != RD_KAFKA_CONF_OK) {
        fprintf(stderr, "%% Failed to set group.id\n");
        exit(1);
    }

    rk = rd_kafka_new(RD_KAFKA_CONSUMER, conf, NULL, 0);
    if (!rk) {
        fprintf(stderr, "%% Failed to create a new consumer: %s\n", strerror(errno));
        exit(1);
    }

    topics = rd_kafka_topic_partition_list_new(1);
    rd_kafka_topic_partition_list_add(topics, "quickstart-events", -1);

    std::thread consumerThread(kafka_consumer, rk, topics);
    std::thread processorThread(message_processor);

    consumerThread.join();
    processorThread.join();

    rd_kafka_topic_partition_list_destroy(topics);
    rd_kafka_destroy(rk);

    return 0;
}
