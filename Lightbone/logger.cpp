#include "pch.h"
#include <iostream>
#include <memory>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <string>

class LogConfigManager {
public:
    static LogConfigManager& getInstance() {
        static LogConfigManager instance;
        return instance;
    }

    // 创建基本文件日志记录器
    std::shared_ptr<spdlog::logger> create_basic_logger(const std::string& projectName, const std::string& logFileName) {
        std::string filePath = projectName + "_" + logFileName;
        try {
            return spdlog::basic_logger_mt(projectName + "_" + logFileName, filePath);
        }
        catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "Failed to create basic logger for " << projectName << ": " << ex.what() << std::endl;
            return nullptr;
        }
    }

    // 创建基本文件日志记录器
    std::shared_ptr<spdlog::logger> create_punish_logger(const std::string& projectName, const std::string& logFileName) {
        std::string filePath = projectName + "_" + logFileName;
        try {
            return spdlog::basic_logger_mt(projectName + "_" + logFileName, filePath);
        }
        catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "Failed to create basic logger for " << projectName << ": " << ex.what() << std::endl;
            return nullptr;
        }
    }

    // 创建按大小滚动的日志记录器
    std::shared_ptr<spdlog::logger> create_rotating_logger(const std::string& projectName, const std::string& logFileName,
        size_t maxFileSize, int maxFiles) {
        std::string filePath = projectName + "_" + logFileName;
        try {
            return spdlog::rotating_logger_mt(projectName + "_" + logFileName, filePath, maxFileSize, maxFiles);
        }
        catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "Failed to create rotating logger for " << projectName << ": " << ex.what() << std::endl;
            return nullptr;
        }
    }

    // 创建按时间滚动的日志记录器
    std::shared_ptr<spdlog::logger> create_daily_logger(const std::string& projectName, const std::string& logFileName,
        int hour, int minute) {
        std::string filePath = projectName + "_" + logFileName;
        try {
            return spdlog::daily_logger_mt(projectName + "_" + logFileName, filePath, hour, minute);
        }
        catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "Failed to create daily logger for " << projectName << ": " << ex.what() << std::endl;
            return nullptr;
        }
    }

private:
    LogConfigManager() = default;
    ~LogConfigManager() = default;
    LogConfigManager(const LogConfigManager&) = delete;
    LogConfigManager& operator=(const LogConfigManager&) = delete;
};