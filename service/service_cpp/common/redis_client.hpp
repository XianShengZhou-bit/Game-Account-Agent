#ifndef REDIS_CLIENT_HPP
#define REDIS_CLIENT_HPP

#include <string>
#include <vector>
#include <memory>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include "config.hpp"
#include "logger.hpp"

namespace redis_client {
    class RedisClient {
    public:
        static RedisClient& getInstance() {
            static RedisClient instance;
            return instance;
        }

        bool connect() {
            const config::Config& config = config::Config::getInstance();

            _context = redisConnect(config.getRedisHost().c_str(), config.getRedisPort());
            if (_context == nullptr || _context->err) {
                if (_context) {
                    LOG_ERROR("Redis连接错误: {}", _context->errstr);
                    redisFree(_context);
                    _context = nullptr;
                } else {
                    LOG_ERROR("Redis连接错误: 无法分配redis上下文");
                }
                return false;
            }

            std::string password = config.getRedisPassword();
            if (!password.empty()) {
                redisReply* reply = (redisReply*)redisCommand(_context, "AUTH %s", password.c_str());
                if (reply == nullptr || _context->err) {
                    LOG_ERROR("Redis AUTH认证失败");
                    if (reply) freeReplyObject(reply);
                    return false;
                }
                freeReplyObject(reply);
            }

            redisReply* reply = (redisReply*)redisCommand(_context, "SELECT %d", config.getRedisDb());
            if (reply == nullptr || _context->err) {
                LOG_ERROR("Redis SELECT db失败");
                if (reply) freeReplyObject(reply);
                return false;
            }
            freeReplyObject(reply);

            LOG_INFO("Redis成功连接到 {}:{}", config.getRedisHost(), config.getRedisPort());
            return true;
        }

        void disconnect() {
            if (_context) {
                redisFree(_context);
                _context = nullptr;
                LOG_INFO("Redis已断开连接");
            }
        }

        bool isConnected() const {
            return _context != nullptr && _context->err == 0;
        }

        std::string get(const std::string& key) {
            if (!isConnected()) return "";

            redisReply* reply = (redisReply*)redisCommand(_context, "GET %s", key.c_str());
            if (reply == nullptr) {
                LOG_ERROR("Redis GET命令失败, 键: {}", key);
                return "";
            }

            std::string result;
            if (reply->type == REDIS_REPLY_STRING) {
                result = std::string(reply->str, reply->len);
            }
            freeReplyObject(reply);
            return result;
        }

        bool set(const std::string& key, const std::string& value, int expire_seconds = 0) {
            if (!isConnected()) return false;

            redisReply* reply = nullptr;
            if (expire_seconds > 0) {
                reply = (redisReply*)redisCommand(_context, "SETEX %s %d %s",
                    key.c_str(), expire_seconds, value.c_str());
            } else {
                reply = (redisReply*)redisCommand(_context, "SET %s %s",
                    key.c_str(), value.c_str());
            }

            if (reply == nullptr || _context->err) {
                LOG_ERROR("Redis SET命令失败, 键: {}", key);
                if (reply) freeReplyObject(reply);
                return false;
            }

            freeReplyObject(reply);
            return true;
        }

        bool del(const std::string& key) {
            if (!isConnected()) return false;

            redisReply* reply = (redisReply*)redisCommand(_context, "DEL %s", key.c_str());
            if (reply == nullptr || _context->err) {
                LOG_ERROR("Redis DEL命令失败, 键: {}", key);
                if (reply) freeReplyObject(reply);
                return false;
            }

            freeReplyObject(reply);
            return true;
        }

        bool exists(const std::string& key) {
            if (!isConnected()) return false;

            redisReply* reply = (redisReply*)redisCommand(_context, "EXISTS %s", key.c_str());
            if (reply == nullptr || _context->err) {
                LOG_ERROR("Redis EXISTS命令失败, 键: {}", key);
                if (reply) freeReplyObject(reply);
                return false;
            }

            bool exists = (reply->integer > 0);
            freeReplyObject(reply);
            return exists;
        }

        bool expire(const std::string& key, int seconds) {
            if (!isConnected()) return false;

            redisReply* reply = (redisReply*)redisCommand(_context, "EXPIRE %s %d",
                key.c_str(), seconds);
            if (reply == nullptr || _context->err) {
                LOG_ERROR("Redis EXPIRE命令失败, 键: {}", key);
                if (reply) freeReplyObject(reply);
                return false;
            }

            freeReplyObject(reply);
            return true;
        }

        bool setNX(const std::string& key, const std::string& value, int expire_seconds = 0) {
            if (!isConnected()) return false;

            redisReply* reply = nullptr;
            if (expire_seconds > 0) {
                reply = (redisReply*)redisCommand(_context, "SET %s %s NX EX %d",
                    key.c_str(), value.c_str(), expire_seconds);
            } else {
                reply = (redisReply*)redisCommand(_context, "SET %s %s NX",
                    key.c_str(), value.c_str());
            }

            if (reply == nullptr) {
                LOG_ERROR("Redis SET NX命令失败, 键: {}", key);
                return false;
            }

            bool success = (reply->type == REDIS_REPLY_STATUS &&
                        std::string(reply->str, reply->len) == "OK");
            freeReplyObject(reply);
            return success;
        }

        bool delIfMatch(const std::string& key, const std::string& expected_value) {
            if (!isConnected()) return false;

            std::string script = "if redis.call('get', KEYS[1]) == ARGV[1] then "
                                "    return redis.call('del', KEYS[1]) "
                                "else "
                                "    return 0 "
                                "end";

            redisReply* reply = (redisReply*)redisCommand(_context,
                "EVAL %s 1 %s %s",
                script.c_str(), key.c_str(), expected_value.c_str());

            if (reply == nullptr || _context->err) {
                LOG_ERROR("Redis DEL IF MATCH命令失败, 键: {}", key);
                if (reply) freeReplyObject(reply);
                return false;
            }

            bool success = (reply->integer == 1);
            freeReplyObject(reply);
            return success;
        }

    private:
        RedisClient() : _context(nullptr) {}
        ~RedisClient() { disconnect(); }

        RedisClient(const RedisClient&) = delete;
        RedisClient& operator=(const RedisClient&) = delete;

        redisContext* _context;
    };
}

#endif