

#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <iostream>
#include <sstream>

enum LogLevel {
    ERROR_LOG,
    WARNING_LOG,
    INFO_LOG
};

class Logger {
public:
    Logger(const std::string& logFileName);
    ~Logger();

    template <typename T>
    Logger& operator<<(const T& value);

    Logger& operator<<(std::ostream& (*manipulator)(std::ostream&));

    Logger& logStringAndDouble(const std::string& str, double value);

    void setLogLevel(LogLevel level);
    void log(LogLevel level, const std::string& message);

    void log(LogLevel level, const std::string& message, double value);

    void log(LogLevel level, const std::string& message, int value);

    void log(LogLevel level, const std::string& message, bool value);

    void redirectCout();

private:
    std::ofstream logStream;
    LogLevel currentLogLevel;
};

// Declare logger variable
extern Logger logger;

#endif // LOGGER_H