
#ifndef LOGGER_CONFIG_H
#define LOGGER_CONFIG_H

#include "logger.hpp"

// Declare logger variable

// Configure logger when included
struct LoggerConfigurator {
    LoggerConfigurator() {
        logger.redirectCout();
        logger.setLogLevel(INFO_LOG);
    }
};

extern Logger logger;

// Define logger variable in logger.cpp
//Logger logger("log_all.txt");

static LoggerConfigurator loggerConfig; // Static instance ensures configuration on program startup

#endif // LOGGER_CONFIG_H