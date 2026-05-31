#ifndef REDIS_LOCK_HPP
#define REDIS_LOCK_HPP

#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <memory>
#include <random>
#include "redis_client.hpp"
#include "logger.hpp"

namespace redis_lock {
    class RedisLock {
    public:
        RedisLock(const std::string& key, int ttl_seconds = 60)
            : _key(key), _ttlSeconds(ttl_seconds), _locked(false) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(100000, 999999);
            _lockValue = std::to_string(dis(gen)) + "_" + 
                        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        }

        ~RedisLock() {
            if (_locked) {
                unlock();
            }
        }

        bool lock(int timeout_ms = 10000, int retry_interval_ms = 100) {
            auto start_time = std::chrono::steady_clock::now();
            
            while (true) {
                if (redis_client::RedisClient::getInstance().setNX(_key, _lockValue, _ttlSeconds)) {
                    _locked = true;
                    LOG_INFO("分布式锁获取成功: {}", _key);
                    return true;
                }

                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start_time).count();
                if (elapsed >= timeout_ms) {
                    LOG_WARN("分布式锁获取超时: {}", _key);
                    return false;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms));
            }
        }

        void unlock() {
            if (!_locked) return;

            bool released = redis_client::RedisClient::getInstance().delIfMatch(_key, _lockValue);
            if (released) {
                LOG_INFO("分布式锁释放成功: {}", _key);
            } else {
                LOG_WARN("分布式锁释放失败(值不匹配或键不存在): {}", _key);
            }
            _locked = false;
        }

        bool isLocked() const { return _locked; }
    private:
        std::string _key;
        std::string _lockValue;
        int _ttlSeconds;
        std::atomic<bool> _locked;
    };

    class ScopedLock {
    public:
        explicit ScopedLock(const std::string& key, int ttl_seconds = 60,
                            int timeout_ms = 10000, int retry_interval_ms = 100)
            : _lock(std::make_unique<RedisLock>(key, ttl_seconds)) {
            _acquired = _lock->lock(timeout_ms, retry_interval_ms);
        }

        ~ScopedLock() {
            if (_acquired) {
                _lock->unlock();
            }
        }

        bool isAcquired() const { return _acquired; }

    private:
        std::unique_ptr<RedisLock> _lock;
        bool _acquired;
    };
}

#endif