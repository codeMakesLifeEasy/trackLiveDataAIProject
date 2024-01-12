// messageHandler.hpp

#pragma once

#include <string>
#include <vector>
#include <librdkafka/rdkafka.h>

void handler_process_batch(const std::vector<std::string>& batch);
void kafka_consumer(rd_kafka_t* rk, rd_kafka_topic_partition_list_t* topics);
void message_processor();
void sigterm(int sig);
