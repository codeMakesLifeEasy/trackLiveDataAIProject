#include <iostream>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <signal.h>
#include <librdkafka/rdkafka.h>
#include "occupiedStatusDetection.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

std::queue<std::string> messageQueue;
std::mutex queueMutex;
std::condition_variable queueCondition;

static int running = 1;

static void sigterm(int sig) {
    running = 0;
    queueCondition.notify_all();  // Notify waiting threads to exit
}

static void msg_process(const std::string &jsonData) {
    // Process jsonData with your existing logic
    BoundingBox roi = {0, 0, 1000, 620};
    BoundingBox truckBoundingBox = {0, 0, 0, 0};
    TruckOccupationChecker truckOccupationChecker(roi, truckBoundingBox);
    truckOccupationChecker.processContinuousData(jsonData);
}

void kafka_consumer(rd_kafka_t *rk, rd_kafka_topic_partition_list_t *topics) {
    rd_kafka_resp_err_t err;

    if ((err = rd_kafka_subscribe(rk, topics))) {
        fprintf(stderr, "%% Failed to start consuming topics: %s\n", rd_kafka_err2str(err));
        exit(1);
    }

    signal(SIGINT, sigterm);
    signal(SIGTERM, sigterm);

    while (running) {
        rd_kafka_message_t *rkmessage = rd_kafka_consumer_poll(rk, 500);
        if (rkmessage) {
            // Process Kafka message and convert payload to JSON
            std::string jsonData(static_cast<const char *>(rkmessage->payload), rkmessage->len);
            //std::this_thread::sleep_for(std::chrono::milliseconds(100));
            // Push jsonData to the message queue
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                messageQueue.push(jsonData);
            }
            queueCondition.notify_one();  // Notify waiting thread to process the message

            while (rd_kafka_consumer_poll(rk, 0) != nullptr) {
                // Discard additional messages
            }

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

void message_processor() {
    while (running) {
        std::unique_lock<std::mutex> lock(queueMutex);
        queueCondition.wait(lock, [] { return !messageQueue.empty(); });

        // Pop message from the queue
        std::string jsonData = messageQueue.front();
        messageQueue.pop();

        lock.unlock();

        // Process the message
        msg_process(jsonData);
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
        fprintf(stderr, "%% Failed to create a new consumer: %s\n", strerror(errno));
        exit(1);
    }

    // Create topics list
    topics = rd_kafka_topic_partition_list_new(1);
    rd_kafka_topic_partition_list_add(topics, "quickstart-events", -1);

    // Start Kafka consumer and message processor threads
    std::thread consumerThread(kafka_consumer, rk, topics);
    std::thread processorThread(message_processor);

    // Wait for threads to finish
    consumerThread.join();
    processorThread.join();

    // Cleanup
    rd_kafka_topic_partition_list_destroy(topics);
    rd_kafka_destroy(rk);

    return 0;
}
