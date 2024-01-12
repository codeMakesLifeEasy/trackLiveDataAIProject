
#include "logger.hpp"


// Definition of the logger variable
Logger logger("log_all.txt");

Logger::Logger(const std::string& logFileName) : currentLogLevel(INFO_LOG) {
    logStream.open(logFileName);
    if (!logStream.is_open()) {
        std::cerr << "Error opening log file: " << logFileName << std::endl;
    }
}

Logger::~Logger() {
    logStream.close();
}

template <typename T>
Logger& Logger::operator<<(const T& value) {
    logStream << value;
    std::cout << value; // Optionally, you can still print to cout if desired
    return *this;
}

Logger& Logger::operator<<(std::ostream& (*manipulator)(std::ostream&)) {
    logStream << manipulator;
    std::cout << manipulator; // Optionally, you can still print to cout if desired
    return *this;
}


Logger& Logger::logStringAndDouble(const std::string& str, double value) {
    logStream << str << value;
    std::cout << str << value; // Optionally, you can still print to cout if desired
    return *this;
}

void Logger::setLogLevel(LogLevel level) {
    currentLogLevel = level;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level <= currentLogLevel) {
        logStream << "[" << level << "] " << message << std::endl;
    }
}

void Logger::log(LogLevel level, const std::string& message, double value) {
    if (level <= currentLogLevel) {
        logStream << "[" << level << "] " << message << value <<std::endl;
    }
}

void Logger::log(LogLevel level, const std::string& message, int value) {
    if (level <= currentLogLevel) {
        logStream << "[" << level << "] " << message << value <<std::endl;
    }
}

void Logger::log(LogLevel level, const std::string& message, bool value) {
    if (level <= currentLogLevel) {
        logStream << "[" << level << "] " << message << value <<std::endl;
    }
}

void Logger::redirectCout() {
    std::cout.rdbuf(logStream.rdbuf());
}

