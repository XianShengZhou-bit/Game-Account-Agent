#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <map>
#include <fstream>
#include <mutex>
#include <optional>
#include "logger.hpp"

namespace config {
    class Config {
    public:
        static Config& getInstance() {
            LOG_INFO("[配置] 获取配置实例");
            static Config instance;
            LOG_INFO("[配置] 配置实例获取完成");
            return instance;
        }

        bool loadFromEnv(const std::string& filepath = "../../../.env") {
            LOG_INFO("[配置] 加载环境文件开始: {}", filepath);
            std::lock_guard<std::mutex> lock(_mutex);
            if (_loaded) {
                LOG_WARN("[配置] 环境文件已加载，无法重复加载");
                return false;
            }
            std::ifstream file(filepath);
            if (!file.is_open()) {
                LOG_ERROR("[配置] 无法打开环境文件: {}", filepath);
                return false;
            }

            std::string line;
            while (std::getline(file, line)) {
                line = trim(line);
                if (line.empty() || line[0] == '#') continue;

                size_t pos = line.find('=');
                if (pos != std::string::npos) {
                    std::string key = trim(line.substr(0, pos));
                    std::string value = trim(line.substr(pos + 1));
                    value = unquote(value);
                    _config[key] = value;
                }
            }

            _loaded = true;
            LOG_INFO("[配置] 环境文件加载完成，加载配置项数: {}", _config.size());
            return true;
        }

        bool has(const std::string& key) const {
            LOG_INFO("[配置] 检查配置项: {}", key);
            bool exists = _config.find(key) != _config.end();
            LOG_INFO("[配置] 配置项 {} 存在: {}", key, exists);
            return exists;
        }

        std::optional<std::string> get(const std::string& key) const {
            LOG_INFO("[配置] 获取配置项: {}", key);
            auto it = _config.find(key);
            if (it == _config.end()) {
                LOG_WARN("[配置] 缺少必需配置: {}", key);
                return {};
            }
            LOG_INFO("[配置] 获取配置项完成: {}", key);
            return it->second;
        }

        std::string getMySQLHost() const {
            LOG_INFO("[配置] 获取 MySQL 主机");
            auto val = get("MYSQL_HOST");
            std::string result = val.value_or("localhost");
            LOG_INFO("[配置] MySQL 主机: {}", result);
            return result;
        }

        int getMySQLPort() const {
            LOG_INFO("[配置] 获取 MySQL 端口");
            auto val = get("MYSQL_PORT");
            int result = val ? std::stoi(*val) : 3306;
            LOG_INFO("[配置] MySQL 端口: {}", result);
            return result;
        }

        std::string getMySQLUser() const {
            LOG_INFO("[配置] 获取 MySQL 用户名");
            auto val = get("MYSQL_USER");
            std::string result = val.value_or("root");
            LOG_INFO("[配置] MySQL 用户名: {}", result);
            return result;
        }

        std::string getMySQLPassword() const {
            LOG_INFO("[配置] 获取 MySQL 密码");
            auto val = get("MYSQL_PASSWORD");
            std::string result = val.value_or("");
            LOG_INFO("[配置] MySQL 密码: {}", result.empty() ? "(空)" : "(已设置)");
            return result;
        }

        std::string getMySQLDatabase() const {
            LOG_INFO("[配置] 获取 MySQL 数据库名");
            auto val = get("MYSQL_DATABASE");
            std::string result = val.value_or("game_account_exchange");
            LOG_INFO("[配置] MySQL 数据库名: {}", result);
            return result;
        }

        std::string getRedisHost() const {
            LOG_INFO("[配置] 获取 Redis 主机");
            auto val = get("REDIS_HOST");
            std::string result = val.value_or("localhost");
            LOG_INFO("[配置] Redis 主机: {}", result);
            return result;
        }

        int getRedisPort() const {
            LOG_INFO("[配置] 获取 Redis 端口");
            auto val = get("REDIS_PORT");
            int result = val ? std::stoi(*val) : 6379;
            LOG_INFO("[配置] Redis 端口: {}", result);
            return result;
        }

        int getRedisDb() const {
            LOG_INFO("[配置] 获取 Redis 数据库编号");
            auto val = get("REDIS_DB");
            int result = val ? std::stoi(*val) : 0;
            LOG_INFO("[配置] Redis 数据库编号: {}", result);
            return result;
        }

        std::string getRedisPassword() const {
            LOG_INFO("[配置] 获取 Redis 密码");
            auto val = get("REDIS_PASSWORD");
            std::string result = val.value_or("");
            LOG_INFO("[配置] Redis 密码: {}", result.empty() ? "(空)" : "(已设置)");
            return result;
        }

        int getSnowflakeMachineId() const {
            LOG_INFO("[配置] 获取 Snowflake 机器 ID");
            auto val = get("SNOWFLAKE_MACHINE_ID");
            int result = val ? std::stoi(*val) : 1;
            LOG_INFO("[配置] Snowflake 机器 ID: {}", result);
            return result;
        }

        int getSnowflakeDataCenterId() const {
            LOG_INFO("[配置] 获取 Snowflake 数据中心 ID");
            auto val = get("SNOWFLAKE_DATA_CENTER_ID");
            int result = val ? std::stoi(*val) : 1;
            LOG_INFO("[配置] Snowflake 数据中心 ID: {}", result);
            return result;
        }

        std::string getJwtAlgorithm() const {
            LOG_INFO("[配置] 获取 JWT 算法");
            auto val = get("JWT_ALGORITHM");
            std::string result = val.value_or("HS256");
            LOG_INFO("[配置] JWT 算法: {}", result);
            return result;
        }

        std::string getJwtSecret() const {
            LOG_INFO("[配置] 获取 JWT 密钥");
            auto val = get("JWT_SECRET");
            std::string result = val.value_or("");
            LOG_INFO("[配置] JWT 密钥: {}", result.empty() ? "(空)" : "(已设置)");
            return result;
        }

        bool isDebug() const {
            LOG_INFO("[配置] 获取调试模式");
            auto val = get("DEBUG");
            bool result = val && (*val == "true");
            LOG_INFO("[配置] 调试模式: {}", result ? "开启" : "关闭");
            return result;
        }

        int getOrderServicePort() const {
            LOG_INFO("[配置] 获取订单服务端口");
            auto val = get("ORDER_SERVICE_PORT");
            int result = val ? std::stoi(*val) : 9000;
            LOG_INFO("[配置] 订单服务端口: {}", result);
            return result;
        }

        std::string getOrderServiceIp() const {
            LOG_INFO("[配置] 获取订单服务 IP");
            auto val = get("ORDER_SERVICE_IP");
            std::string result = val.value_or("localhost");
            LOG_INFO("[配置] 订单服务 IP: {}", result);
            return result;
        }

    private:
        Config() = default;

        static std::string trim(const std::string& str) {
            size_t start = str.find_first_not_of(" \t\r\n");
            if (start == std::string::npos) return "";
            size_t end = str.find_last_not_of(" \t\r\n");
            return str.substr(start, end - start + 1);
        }

        static std::string unquote(const std::string& str) {
            if (str.size() >= 2) {
                if ((str.front() == '"' && str.back() == '"') ||
                    (str.front() == '\'' && str.back() == '\'')) {
                    return str.substr(1, str.size() - 2);
                }
            }
            return str;
        }

        std::map<std::string, std::string> _config;
        std::mutex _mutex;
        bool _loaded = false;
    };
}

#endif