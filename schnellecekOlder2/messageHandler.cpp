// messageHandler.cpp

#include <iostream>
#include <thread>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <signal.h>
#include <librdkafka/rdkafka.h>
#include "occupiedStatusDetection.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "messageHandler.hpp"

std::queue<std::string> messageQueue;
std::mutex queueMutex;
std::condition_variable queueCondition;

const size_t BATCH_SIZE = 6;

static int running = 1;

static void msg_process(const std::string& jsonData) {
    BoundingBox roi = {0, 0, 1000, 620};
    BoundingBox truckBoundingBox = {0, 0, 0, 0};
    TruckOccupationChecker truckOccupationChecker(roi, truckBoundingBox);
    truckOccupationChecker.processContinuousData(jsonData);
}

void handler_process_batch(const std::vector<std::string>& batch) {
    for (const std::string& jsonData : batch) {
        msg_process(jsonData);
    }
    //std::this_thread::sleep_for(std::chrono::milliseconds(700));
}

void kafka_consumer(rd_kafka_t* rk, rd_kafka_topic_partition_list_t* topics) {
    rd_kafka_resp_err_t err;

    if ((err = rd_kafka_subscribe(rk, topics))) {
        fprintf(stderr, "%% Failed to start consuming topics: %s\n", rd_kafka_err2str(err));
        exit(1);
    }

    signal(SIGINT, sigterm);
    signal(SIGTERM, sigterm);

    while (running) {
        rd_kafka_message_t* rkmessage = rd_kafka_consumer_poll(rk, 500);
        if (rkmessage) {
            std::string jsonData(static_cast<const char*>(rkmessage->payload), rkmessage->len);

            {
                std::lock_guard<std::mutex> lock(queueMutex);
                messageQueue.push(jsonData);
            }
            queueCondition.notify_one();

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
        queueCondition.wait(lock, [&] { return !messageQueue.empty(); });

        std::vector<std::string> batch;
        while (!messageQueue.empty() && batch.size() < BATCH_SIZE) {
            batch.push_back(messageQueue.front());
            messageQueue.pop();
        }

        lock.unlock();

        if (!batch.empty()) {
            handler_process_batch(batch);
        }
    }
}

void sigterm(int sig) {
    running = 0;
    queueCondition.notify_all();
}
