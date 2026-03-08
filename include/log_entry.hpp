#pragma once

#include <string>
#include <chrono>
#include <optional>
#include <sstream>
#include <regex>

enum class LogLevel { DEBUG, INFO, WARN, ERROR, FATAL };

struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    LogLevel level = LogLevel::INFO;
    std::string source;
    std::string message;

    static std::optional<LogEntry> parse(const std::string& line) {
        // Формат: [2024-01-15 10:30:45] [ERROR] [Database] Connection timeout
        std::regex pattern(R"(\[(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})\] \[(\w+)\] \[(\w+)\] (.+))");
        std::smatch matches;

        if (!std::regex_match(line, matches, pattern)) {
            return std::nullopt;
        }

        LogEntry entry;

        // Парсинг временной метки
        std::istringstream ss(matches[1].str());
        std::tm tm = {};
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        if (ss.fail()) {
            return std::nullopt;
        }
        entry.timestamp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

        // Парсинг уровня
        std::string level = matches[2].str();
        if (level == "DEBUG") entry.level = LogLevel::DEBUG;
        else if (level == "INFO") entry.level = LogLevel::INFO;
        else if (level == "WARN") entry.level = LogLevel::WARN;
        else if (level == "ERROR") entry.level = LogLevel::ERROR;
        else if (level == "FATAL") entry.level = LogLevel::FATAL;
        else return std::nullopt;

        entry.source = matches[3].str();
        entry.message = matches[4].str();

        return entry;
    }
};