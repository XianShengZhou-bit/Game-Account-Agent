#include <iostream>
#include <string>
#include <cassert>
#include <thread>
#include <chrono>
#include "../../service/cpp/common/redis_client.hpp"
#include "../../service/cpp/common/config.hpp"
#include "../../service/cpp/common/logger.hpp"
#include "../../service/cpp/common/redis_lock.hpp"

class TestSuite {
private:
    int passed = 0;
    int failed = 0;

public:
    void report() {
        std::cout << "\n========================================" << std::endl;
        std::cout << "Test Results: " << passed << " passed, " << failed << " failed" << std::endl;
        std::cout << "========================================" << std::endl;
    }

    void reset() {
        passed = 0;
        failed = 0;
    }

    void assertTrue(bool condition, const std::string& testName) {
        if (condition) {
            passed++;
            std::cout << "[PASS] " << testName << std::endl;
        } else {
            failed++;
            std::cout << "[FAIL] " << testName << " - assertion failed" << std::endl;
        }
    }

    void assertFalse(bool condition, const std::string& testName) {
        assertTrue(!condition, testName);
    }

    template<typename T>
    void assertEqual(T actual, T expected, const std::string& testName) {
        if (actual == expected) {
            passed++;
            std::cout << "[PASS] " << testName << std::endl;
        } else {
            failed++;
            std::cout << "[FAIL] " << testName << " - Expected: " << expected << ", Got: " << actual << std::endl;
        }
    }

    void assertNotEmpty(const std::string& str, const std::string& testName) {
        assertTrue(!str.empty(), testName);
    }

    void assertEmpty(const std::string& str, const std::string& testName) {
        assertTrue(str.empty(), testName);
    }
};

TestSuite g_testSuite;

void testLoggerInit() {
    std::cout << "\n--- Testing Logger Initialization ---" << std::endl;
    logger::Logger::getInstance().init(true, "redis_test.log", 10 * 1024 * 1024, 5);
    g_testSuite.assertTrue(true, "Logger singleton getInstance");
    logger::Logger::getInstance().flush();
    std::cout << "[PASS] Logger flush" << std::endl;
}

void testConfigLoad() {
    std::cout << "\n--- Testing Config Loading ---" << std::endl;
    config::Config& config = const_cast<config::Config&>(config::Config::getInstance());
    bool loadResult = config.loadFromEnv(".env");
    g_testSuite.assertTrue(loadResult, "Config loadFromEnv");

    std::string host = config.getRedisHost();
    g_testSuite.assertEqual(host, std::string("localhost"), "Config getRedisHost");

    int port = config.getRedisPort();
    g_testSuite.assertEqual(port, 6379, "Config getRedisPort");
}

void testRedisConnect() {
    std::cout << "\n--- Testing Redis Connection ---" << std::endl;

    redis_client::RedisClient& client = redis_client::RedisClient::getInstance();
    bool connectResult = client.connect();
    g_testSuite.assertTrue(connectResult, "Redis connect");

    bool isConnected = client.isConnected();
    g_testSuite.assertTrue(isConnected, "Redis isConnected after connect");
}

void testRedisSetAndGet() {
    std::cout << "\n--- Testing Redis SET and GET ---" << std::endl;

    redis_client::RedisClient& client = redis_client::RedisClient::getInstance();

    std::string key = "test_key_" + std::to_string(time(nullptr));
    std::string value = "test_value_123";

    bool setResult = client.set(key, value);
    g_testSuite.assertTrue(setResult, "Redis set key-value");

    std::string getValue = client.get(key);
    g_testSuite.assertEqual(getValue, value, "Redis get returns correct value");

    client.del(key);
    std::string afterDel = client.get(key);
    g_testSuite.assertEmpty(afterDel, "Redis get after delete returns empty");
}

void testRedisDelIfMatch() {
    std::cout << "\n--- Testing Redis DEL IF MATCH (Lua Script) ---" << std::endl;

    redis_client::RedisClient& client = redis_client::RedisClient::getInstance();

    std::string key = "test_delifmatch_" + std::to_string(time(nullptr));
    std::string correctValue = "correct_value";
    std::string wrongValue = "wrong_value";

    client.set(key, correctValue);

    bool delWithCorrect = client.delIfMatch(key, correctValue);
    g_testSuite.assertTrue(delWithCorrect, "delIfMatch with correct value succeeds");

    bool existsAfterDel = client.exists(key);
    g_testSuite.assertFalse(existsAfterDel, "Key deleted after delIfMatch");

    client.set(key, correctValue);
    bool delWithWrong = client.delIfMatch(key, wrongValue);
    g_testSuite.assertFalse(delWithWrong, "delIfMatch with wrong value fails");

    bool stillExists = client.exists(key);
    g_testSuite.assertTrue(stillExists, "Key still exists after failed delIfMatch");

    client.del(key);
}

void testRedisLock() {
    std::cout << "\n--- Testing Redis Lock ---" << std::endl;

    redis_client::RedisClient::getInstance().connect();

    std::string key = "test_redis_lock_" + std::to_string(time(nullptr));

    redis_lock::RedisLock lock(key, 60);

    bool lockResult = lock.lock(5000, 100);
    g_testSuite.assertTrue(lockResult, "RedisLock lock() succeeds");

    g_testSuite.assertTrue(lock.isLocked(), "RedisLock isLocked() returns true");

    lock.unlock();

    g_testSuite.assertFalse(lock.isLocked(), "RedisLock isLocked() returns false after unlock");
}

void testRedisLockTimeout() {
    std::cout << "\n--- Testing Redis Lock Timeout ---" << std::endl;

    redis_client::RedisClient::getInstance().connect();

    std::string key = "test_lock_timeout_" + std::to_string(time(nullptr));

    redis_lock::RedisLock lock1(key, 60);
    bool lock1Result = lock1.lock(5000, 100);
    g_testSuite.assertTrue(lock1Result, "First lock succeeds");

    redis_lock::RedisLock lock2(key, 60);
    bool lock2Result = lock2.lock(100, 50);
    g_testSuite.assertFalse(lock2Result, "Second lock with timeout fails");

    lock1.unlock();
}

void testRedisScopedLock() {
    std::cout << "\n--- Testing Redis ScopedLock ---" << std::endl;

    redis_client::RedisClient::getInstance().connect();

    std::string key = "test_scoped_lock_" + std::to_string(time(nullptr));

    {
        redis_lock::ScopedLock lock(key, 60, 5000, 100);
        g_testSuite.assertTrue(lock.isAcquired(), "ScopedLock acquired");
    }

    std::string value = redis_client::RedisClient::getInstance().get(key);
    g_testSuite.assertEmpty(value, "Key released after ScopedLock destroyed");
}

void testRedisExists() {
    std::cout << "\n--- Testing Redis EXISTS ---" << std::endl;

    redis_client::RedisClient& client = redis_client::RedisClient::getInstance();
    std::string key = "test_exists_" + std::to_string(time(nullptr));

    bool existsBefore = client.exists(key);
    g_testSuite.assertFalse(existsBefore, "Key does not exist before SET");

    client.set(key, "value");
    bool existsAfter = client.exists(key);
    g_testSuite.assertTrue(existsAfter, "Key exists after SET");

    client.del(key);
    bool existsAfterDel = client.exists(key);
    g_testSuite.assertFalse(existsAfterDel, "Key does not exist after DEL");
}

void testRedisExpire() {
    std::cout << "\n--- Testing Redis EXPIRE ---" << std::endl;

    redis_client::RedisClient& client = redis_client::RedisClient::getInstance();
    std::string key = "test_expire_" + std::to_string(time(nullptr));

    client.set(key, "value");
    bool setResult = client.exists(key);
    g_testSuite.assertTrue(setResult, "Key set successfully");

    bool expireResult = client.expire(key, 1);
    g_testSuite.assertTrue(expireResult, "EXPIRE command succeeds");

    bool existsBefore = client.exists(key);
    g_testSuite.assertTrue(existsBefore, "Key still exists before TTL");

    std::this_thread::sleep_for(std::chrono::seconds(2));
    bool existsAfterTTL = client.exists(key);
    g_testSuite.assertFalse(existsAfterTTL, "Key expired after TTL");

    client.del(key);
}

void testRedisSetNX() {
    std::cout << "\n--- Testing Redis SETNX ---" << std::endl;

    redis_client::RedisClient& client = redis_client::RedisClient::getInstance();
    std::string key = "test_setnx_" + std::to_string(time(nullptr));
    std::string value = "test_value";

    bool setNX1 = client.setNX(key, value);
    g_testSuite.assertTrue(setNX1, "First SETNX succeeds when key does not exist");

    bool setNX2 = client.setNX(key, "another_value");
    g_testSuite.assertFalse(setNX2, "Second SETNX fails when key already exists");

    std::string getValue = client.get(key);
    g_testSuite.assertEqual(getValue, value, "Value unchanged after failed SETNX");

    client.del(key);
}

void testRedisSetWithExpire() {
    std::cout << "\n--- Testing Redis SET with expiration ---" << std::endl;

    redis_client::RedisClient& client = redis_client::RedisClient::getInstance();
    std::string key = "test_set_expire_" + std::to_string(time(nullptr));

    bool setResult = client.set(key, "value", 1);
    g_testSuite.assertTrue(setResult, "SET with expire succeeds");

    bool existsBefore = client.exists(key);
    g_testSuite.assertTrue(existsBefore, "Key exists before TTL");

    std::this_thread::sleep_for(std::chrono::seconds(2));
    bool existsAfterTTL = client.exists(key);
    g_testSuite.assertFalse(existsAfterTTL, "Key expired after TTL");
}

int main() {
    std::cout << "==================================================" << std::endl;
    std::cout << "      Redis Client Test Suite" << std::endl;
    std::cout << "==================================================" << std::endl;

    testLoggerInit();

    testConfigLoad();

    testRedisConnect();

    testRedisSetAndGet();

    testRedisDelIfMatch();

    testRedisLock();

    testRedisLockTimeout();

    testRedisScopedLock();

    testRedisExists();

    testRedisExpire();

    testRedisSetNX();

    testRedisSetWithExpire();

    g_testSuite.report();

    std::cout << "\n==================================================" << std::endl;
    std::cout << "           Test Completed" << std::endl;
    std::cout << "==================================================" << std::endl;

    return 0;
}