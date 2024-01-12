#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <librdkafka/rdkafka.h>
#include <errno.h>
#include <unistd.h>

static int running = 1;

static void sigterm(int sig) {
    running = 0;
}

static void msg_process(rd_kafka_message_t *message) {
    if (message->err) {
        fprintf(stderr, "%% Error: %s\n", rd_kafka_message_errstr(message));
    } else {
        //printf("Received message (%zd bytes, offset %" PRId64 ", partition %" PRId32 ")\n",
               //message->len, message->offset, message->partition);
        printf("%.*s\n", (int)message->len, (char *)message->payload);
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

    while (running) {
        rd_kafka_message_t *rkmessage = rd_kafka_consumer_poll(rk, 500);
        if (rkmessage) {
            msg_process(rkmessage);
            rd_kafka_message_destroy(rkmessage);
            usleep(700000);
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
