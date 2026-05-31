#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/pattern_formatter.h>
#include <mutex>
#include <string>

namespace logger {

    class Logger {
    public:
        static Logger& getInstance() {
            static Logger instance;
            return instance;
        }

        bool init(bool debug, const std::string& log_file = "logs.log",
                size_t max_file_size = 10 * 1024 * 1024,
                size_t max_files = 5) {
            std::lock_guard<std::mutex> lock(_mutex);

            if (_logger) {
                std::cerr << "[日志] 日志已初始化，无法重复初始化" << std::endl;
                return false;
            }
            spdlog::sink_ptr sink;
            _debug = debug;

            if (_debug) {
                sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            } else {
                sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                    log_file, max_file_size, max_files);
            }

            sink->set_level(spdlog::level::trace);
            sink->set_pattern("%Y-%m-%d %H:%M:%S | %s:%# | %-5l | %v");

            _logger = std::make_shared<spdlog::logger>("order_service", sink);
            _logger->set_level(_debug ? spdlog::level::debug : spdlog::level::info);
            _logger->flush_on(spdlog::level::debug);
            return true;
        }

        template<typename... Args>
        void debug(const char* file, int line, const char* fmt, Args... args) {
            if (!_logger) {
                std::cerr << "[日志] 日志未初始化，无法调试日志" << std::endl;
                return;
            }
            _logger->debug(fmt, args...);
        }

        template<typename... Args>
        void info(const char* file, int line, const char* fmt, Args... args) {
            if (!_logger) {
                std::cerr << "[日志] 日志未初始化，无法信息日志" << std::endl;
                return;
            }
            _logger->info(fmt, args...);
        }

        template<typename... Args>
        void warn(const char* file, int line, const char* fmt, Args... args) {
            if (!_logger) {
                std::cerr << "[日志] 日志未初始化，无法警告日志" << std::endl;
                return;
            }
            _logger->warn(fmt, args...);
        }

        template<typename... Args>
        void error(const char* file, int line, const char* fmt, Args... args) {
            if (!_logger) {
                std::cerr << "[日志] 日志未初始化，无法错误日志" << std::endl;
                return;
            }
            _logger->error(fmt, args...);
        }

        void flush() {
            if (!_logger) {
                std::cerr << "[日志] 日志未初始化，无法刷新日志" << std::endl;
                return;
            }
            _logger->flush();
        }

    private:
        Logger() = default;
        ~Logger() = default;
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        std::shared_ptr<spdlog::logger> _logger;
        std::mutex _mutex;
        bool _debug = false;
    };
}

#define LOG_DEBUG(...) logger::Logger::getInstance().debug(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...) logger::Logger::getInstance().info(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...) logger::Logger::getInstance().warn(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) logger::Logger::getInstance().error(__FILE__, __LINE__, __VA_ARGS__)

#endif