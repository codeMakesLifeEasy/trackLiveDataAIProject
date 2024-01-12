#include <iostream>
#include <string>
#include <unordered_map>
#include <random>
#include <chrono>
#include <thread>
//#include "processData.hpp"
#include "occupiedStatusDetection.hpp"
#include "loadStatus.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <signal.h>
#include <librdkafka/rdkafka.h>
#include <errno.h>
#include <unistd.h>

static int running = 1;

static void sigterm(int sig) {
    running = 0;
}

static void msg_process(rd_kafka_message_t *message, std::string &jsonData) {
    //usleep(1000000);
    if (message->err) {
        fprintf(stderr, "%% Error: %s\n", rd_kafka_message_errstr(message));
    } else {
        // Convert the payload to a RapidJSON value
        rapidjson::Document document;
        document.Parse(static_cast<const char *>(message->payload));

        if (document.HasParseError()) {
            std::cerr << "Error parsing Kafka message payload as JSON." << std::endl;
            return;
        }

        // Access JSON fields and create jsonData string
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
        jsonData = buffer.GetString();

       
           //printf("%s\n", jsonData.c_str());
            
            // Update the last processed time
           // lastProcessedTime = currentTime;
        //}
    }
}

void basic_consume_loop(rd_kafka_t *rk, rd_kafka_topic_partition_list_t *topics) {
    rd_kafka_resp_err_t err;

    if ((err = rd_kafka_subscribe(rk, topics))) {
        fprintf(stderr, "%% Failed to start consuming topics: %s\n", rd_kafka_err2str(err));
        exit(1);
    }

    signal(SIGINT, sigterm);
    signal(SIGTERM, sigterm);
    //std::chrono::steady_clock::time_point lastProcessedTime = std::chrono::steady_clock::now();

    while (running) {
        rd_kafka_message_t *rkmessage = rd_kafka_consumer_poll(rk, 500);
        if (rkmessage) {
            // Process Kafka message and convert payload to JSON
             std::string jsonData;
             msg_process(rkmessage, jsonData);

            // Process jsonData with your existing logic
            BoundingBox roi = {0, 0, 1000, 620};
            BoundingBox truckBoundingBox = {0, 0, 0, 0};
            TruckOccupationChecker truckOccupationChecker(roi, truckBoundingBox);
            truckOccupationChecker.processContinuousData(jsonData);
            //usleep(9000);
            while (rd_kafka_consumer_poll(rk, 0) != nullptr) {
            // Discard additional messages
            }
            usleep(00);
            rd_kafka_message_destroy(rkmessage);
            
        }
    }

    err = rd_kafka_consumer_close(rk);
    if (err) {
        fprintf(stderr, "%% Failed to close consumer: %s\n", rd_kafka_err2str(err));
    } else {
        fprintf(stderr, "%% Consumer closed\n");
    }
}

int main() {
    rd_kafka_t *rk;
    rd_kafka_conf_t *conf;
    rd_kafka_topic_partition_list_t *topics;

    const char *brokers = "localhost:9092";
    const char *group_id = "sentics_consumer_group";

    // Create Kafka configuration
    conf = rd_kafka_conf_new();

    // Set broker list
    if (rd_kafka_conf_set(conf, "bootstrap.servers", brokers, NULL, 0) != RD_KAFKA_CONF_OK) {
        fprintf(stderr, "%% Failed to set bootstrap.servers\n");
        exit(1);
    }

    // Set group id
    if (rd_kafka_conf_set(conf, "group.id", group_id, NULL, 0) != RD_KAFKA_CONF_OK) {
        fprintf(stderr, "%% Failed to set group.id\n");
        exit(1);
    }

    // Create Kafka instance
    rk = rd_kafka_new(RD_KAFKA_CONSUMER, conf, NULL, 0);
    if (!rk) {
        fprintf(stderr, "%% Failed to create new consumer: %s\n", strerror(errno));
        exit(1);
    }

    // Create topics list
    topics = rd_kafka_topic_partition_list_new(1);
    rd_kafka_topic_partition_list_add(topics, "quickstart-events", -1);

    // Start consuming messages
    basic_consume_loop(rk, topics);

    // Cleanup
    rd_kafka_topic_partition_list_destroy(topics);
    rd_kafka_destroy(rk);

    return 0;
}
