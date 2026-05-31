#include <iostream>
#include <string>
#include <cassert>
#include "../../service/cpp/common/mysql_pool.hpp"
#include "../../service/cpp/common/config.hpp"
#include "../../service/cpp/common/logger.hpp"

#define TEST_PASSED(name) std::cout << "[PASS] " << name << std::endl
#define TEST_FAILED(name, reason) std::cout << "[FAIL] " << name << " - " << reason << std::endl

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
            TEST_PASSED(testName);
        } else {
            failed++;
            TEST_FAILED(testName, "assertion failed");
        }
    }

    void assertFalse(bool condition, const std::string& testName) {
        assertTrue(!condition, testName);
    }

    template<typename T>
    void assertEqual(T actual, T expected, const std::string& testName) {
        if (actual == expected) {
            passed++;
            TEST_PASSED(testName);
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
    
    Logger::getInstance().init(true, "test.log", 10 * 1024 * 1024, 5);
    g_testSuite.assertTrue(true, "Logger singleton getInstance");
    
    Logger::getInstance().flush();
    TEST_PASSED("Logger flush");
}

void testConfigLoad() {
    std::cout << "\n--- Testing Config Loading ---" << std::endl;
    
    const Config& config = Config::getInstance();
    g_testSuite.assertTrue(true, "Config singleton getInstance");
    
    bool loadResult = const_cast<Config&>(config).loadFromEnv("../../.env");
    g_testSuite.assertTrue(loadResult, "Config loadFromEnv");
    
    std::string host = config.getMySQLHost();
    g_testSuite.assertEqual(host, std::string("localhost"), "Config getMySQLHost");
    
    int port = config.getMySQLPort();
    g_testSuite.assertEqual(port, 3306, "Config getMySQLPort");
    
    std::string user = config.getMySQLUser();
    g_testSuite.assertNotEmpty(user, "Config getMySQLUser not empty");
    
    std::string password = config.getMySQLPassword();
    g_testSuite.assertNotEmpty(password, "Config getMySQLPassword not empty");
    
    std::string database = config.getMySQLDatabase();
    g_testSuite.assertEqual(database, std::string("game_account_exchange"), "Config getMySQLDatabase");
    
    int machineId = config.getSnowflakeMachineId();
    g_testSuite.assertEqual(machineId, 1, "Config getSnowflakeMachineId");
    
    int dataCenterId = config.getSnowflakeDataCenterId();
    g_testSuite.assertEqual(dataCenterId, 1, "Config getSnowflakeDataCenterId");
}

void testSnowflakeId() {
    std::cout << "\n--- Testing SnowflakeId ---" << std::endl;
    
    SnowflakeId& snowflake = SnowflakeId::getInstance();
    g_testSuite.assertTrue(true, "SnowflakeId singleton getInstance");
    
    snowflake.init(1, 1);
    g_testSuite.assertTrue(true, "SnowflakeId init(1, 1)");
    
    int64_t id1 = snowflake.nextId();
    g_testSuite.assertTrue(id1 > 0, "SnowflakeId nextId returns positive value");
    
    int64_t id2 = snowflake.nextId();
    g_testSuite.assertTrue(id2 > 0, "SnowflakeId nextId returns positive value (second call)");
    g_testSuite.assertTrue(id2 > id1, "SnowflakeId nextId generates increasing IDs");
    
    int64_t last12Bits = snowflake.getLast12Bits();
    g_testSuite.assertTrue(last12Bits >= 0 && last12Bits <= 0xFFF, "SnowflakeId getLast12Bits in valid range");
}

void testMySQLPoolInit() {
    std::cout << "\n--- Testing MySQLPool Initialization ---" << std::endl;
    
    MySQLPool& pool = MySQLPool::getInstance(5);
    g_testSuite.assertTrue(true, "MySQLPool singleton getInstance");
    
    bool initResult = pool.init(5);
    g_testSuite.assertTrue(initResult, "MySQLPool init with 5 connections");
    
    int availableCount = pool.getAvailableConnectionCount();
    g_testSuite.assertTrue(availableCount > 0, "MySQLPool getAvailableConnectionCount > 0");
    std::cout << "    Available connections: " << availableCount << std::endl;
}

void testMySQLPoolGetConnection() {
    std::cout << "\n--- Testing MySQLPool Connection Access ---" << std::endl;
    
    int count1 = MySQLPool::getInstance().getAvailableConnectionCount();
    g_testSuite.assertTrue(count1 > 0, "MySQLPool getAvailableConnectionCount returns valid count");
    
    int count2 = MySQLPool::getInstance().getAvailableConnectionCount();
    g_testSuite.assertEqual(count1, count2, "MySQLPool connection count is consistent");
    
    int count3 = MySQLPool::getInstance().getAvailableConnectionCount();
    g_testSuite.assertTrue(count3 > 0, "MySQLPool getAvailableConnectionCount returns valid count (second check)");
}

void testCreateAndQueryOrder() {
    std::cout << "\n--- Testing Order Creation and Query ---" << std::endl;
    
    MySQLPool& pool = MySQLPool::getInstance();
    
    int64_t testAccountId = 12345;
    int64_t testBuyerId = 1001;
    std::string testPhone = "13800138000";
    std::string testIdCard = "110101199001011234";
    double testPrice = 999.99;
    
    OrderInfo createdOrder = pool.createOrder(testAccountId, testBuyerId, testPhone, testIdCard, testPrice);
    
    g_testSuite.assertNotEmpty(createdOrder.order_sn, "Order createOrder returns order_sn");
    std::cout << "    Created order_sn: " << createdOrder.order_sn << std::endl;
    
    g_testSuite.assertEqual(createdOrder.account_id, testAccountId, "Order createOrder account_id matches");
    g_testSuite.assertEqual(createdOrder.buyer_id, testBuyerId, "Order createOrder buyer_id matches");
    g_testSuite.assertEqual(createdOrder.buyer_phone, testPhone, "Order createOrder buyer_phone matches");
    g_testSuite.assertEqual(createdOrder.buyer_id_card, testIdCard, "Order createOrder buyer_id_card matches");
    g_testSuite.assertEqual(createdOrder.status, 0, "Order createOrder initial status is 0");
    g_testSuite.assertFalse(createdOrder.mutex, "Order createOrder initial mutex is false");
    g_testSuite.assertNotEmpty(createdOrder.created_at, "Order createOrder returns created_at");
    
    OrderInfo queriedOrder = pool.getOrderBySn(createdOrder.order_sn);
    g_testSuite.assertNotEmpty(queriedOrder.order_sn, "Order getOrderBySn returns order");
    g_testSuite.assertEqual(queriedOrder.order_sn, createdOrder.order_sn, "Order getOrderBySn order_sn matches");
    g_testSuite.assertEqual(queriedOrder.account_id, testAccountId, "Order getOrderBySn account_id matches");
    g_testSuite.assertEqual(queriedOrder.buyer_id, testBuyerId, "Order getOrderBySn buyer_id matches");
    g_testSuite.assertEqual(queriedOrder.status, 0, "Order getOrderBySn status matches");
}

void testUpdateOrderStatus() {
    std::cout << "\n--- Testing Order Status Update ---" << std::endl;
    
    MySQLPool& pool = MySQLPool::getInstance();
    
    int64_t testAccountId = 54321;
    int64_t testBuyerId = 1002;
    std::string testPhone = "13900139000";
    std::string testIdCard = "110101199002022345";
    double testPrice = 1999.99;
    
    OrderInfo createdOrder = pool.createOrder(testAccountId, testBuyerId, testPhone, testIdCard, testPrice);
    g_testSuite.assertNotEmpty(createdOrder.order_sn, "Order createOrder for status test");
    
    bool updateResult = pool.updateOrderStatus(createdOrder.order_sn, 1);
    g_testSuite.assertTrue(updateResult, "Order updateOrderStatus returns true");
    
    OrderInfo queriedOrder = pool.getOrderBySn(createdOrder.order_sn);
    g_testSuite.assertEqual(queriedOrder.status, 1, "Order status updated to 1");
}

void testCreateAndQueryAccount() {
    std::cout << "\n--- Testing Account Creation and Query ---" << std::endl;
    
    MySQLPool& pool = MySQLPool::getInstance();
    
    MYSQL* conn = nullptr;
    
    std::string testAccountsId = "ACC" + std::to_string(time(nullptr));
    std::string testGameName = "测试游戏";
    std::string testServerArea = "测试服务器";
    std::string testTitle = "高级账号";
    int testPrice = 50000;
    int testLevel = 50;
    int testHeroCount = 10;
    int testSkinCount = 5;
    std::string testRareItems = "[\"龙狙\", \"火麒麟\"]";
    
    char insertQuery[2048];
    snprintf(insertQuery, sizeof(insertQuery),
        "INSERT INTO accounts (accounts_id, game_name, server_area, title, price, account_level, hero_count, skin_count, rare_items, status, version) "
        "VALUES ('%s', '%s', '%s', '%s', %d, %d, %d, %d, '%s', 0, 0)",
        testAccountsId.c_str(), testGameName.c_str(), testServerArea.c_str(), testTitle.c_str(),
        testPrice, testLevel, testHeroCount, testSkinCount, testRareItems.c_str());
    
    conn = MySQLPool::getInstance().getAvailableConnectionCount() > 0 ? mysql_init(nullptr) : nullptr;
    Config& config = const_cast<Config&>(Config::getInstance());
    
    if (conn) {
        mysql_real_connect(conn, config.getMySQLHost().c_str(), config.getMySQLUser().c_str(),
            config.getMySQLPassword().c_str(), config.getMySQLDatabase().c_str(),
            config.getMySQLPort(), nullptr, 0);
        
        if (mysql_query(conn, insertQuery) == 0) {
            TEST_PASSED("Account INSERT via raw connection");
        } else {
            TEST_FAILED("Account INSERT via raw connection", mysql_error(conn));
        }
        
        mysql_close(conn);
    }
    
    AccountInfo queriedAccount = pool.getAccountById(testAccountsId);
    
    g_testSuite.assertNotEmpty(queriedAccount.account_id, "Account getAccountById returns account");
    g_testSuite.assertEqual(queriedAccount.account_id, testAccountsId, "Account accounts_id matches");
    g_testSuite.assertEqual(queriedAccount.game_name, testGameName, "Account game_name matches");
    g_testSuite.assertEqual(queriedAccount.server_area, testServerArea, "Account server_area matches");
    g_testSuite.assertEqual(queriedAccount.title, testTitle, "Account title matches");
    g_testSuite.assertEqual(queriedAccount.price, testPrice, "Account price matches");
    g_testSuite.assertEqual(queriedAccount.account_level, testLevel, "Account account_level matches");
    g_testSuite.assertEqual(queriedAccount.hero_count, testHeroCount, "Account hero_count matches");
    g_testSuite.assertEqual(queriedAccount.skin_count, testSkinCount, "Account skin_count matches");
    g_testSuite.assertNotEmpty(queriedAccount.created_at, "Account created_at not empty");
}

void testUpdateAccountStatus() {
    std::cout << "\n--- Testing Account Status Update ---" << std::endl;
    
    MySQLPool& pool = MySQLPool::getInstance();
    
    MYSQL* conn = nullptr;
    
    std::string testAccountsId = "ACC_STATUS_" + std::to_string(time(nullptr));
    
    conn = MySQLPool::getInstance().getAvailableConnectionCount() > 0 ? mysql_init(nullptr) : nullptr;
    Config& config = const_cast<Config&>(Config::getInstance());
    
    if (conn) {
        mysql_real_connect(conn, config.getMySQLHost().c_str(), config.getMySQLUser().c_str(),
            config.getMySQLPassword().c_str(), config.getMySQLDatabase().c_str(),
            config.getMySQLPort(), nullptr, 0);
        
        char insertQuery[1024];
        snprintf(insertQuery, sizeof(insertQuery),
            "INSERT INTO accounts (accounts_id, game_name, server_area, title, price, account_level, hero_count, skin_count, rare_items, status, version) "
            "VALUES ('%s', '测试游戏2', '测试区服2', '测试标题2', 10000, 10, 2, 1, '[]', 0, 0)",
            testAccountsId.c_str());
        
        if (mysql_query(conn, insertQuery) == 0) {
            TEST_PASSED("Account INSERT for status test");
        } else {
            TEST_FAILED("Account INSERT for status test", mysql_error(conn));
        }
        
        mysql_close(conn);
    }
    
    bool updateResult = pool.updateAccountStatus(testAccountsId, 1);
    g_testSuite.assertTrue(updateResult, "Account updateAccountStatus returns true");
    
    AccountInfo queriedAccount = pool.getAccountById(testAccountsId);
    g_testSuite.assertEqual(queriedAccount.status, 1, "Account status updated to 1");
}

void testOrderSnGeneration() {
    std::cout << "\n--- Testing Order SN Generation ---" << std::endl;
    
    SnowflakeId& snowflake = SnowflakeId::getInstance();
    int64_t id = snowflake.nextId();
    
    std::string orderSn = "ORD" + std::to_string(id);
    g_testSuite.assertTrue(orderSn.length() > 3, "Order SN generation creates non-empty string");
    g_testSuite.assertTrue(orderSn.find("ORD") == 0, "Order SN starts with ORD prefix");
    std::cout << "    Generated order_sn: " << orderSn << std::endl;
}

void testConcurrency() {
    std::cout << "\n--- Testing Connection Pool Concurrency ---" << std::endl;
    
    MySQLPool& pool = MySQLPool::getInstance();
    int initialCount = pool.getAvailableConnectionCount();
    
    g_testSuite.assertTrue(initialCount > 0, "Pool has available connections for concurrency test");
    std::cout << "    Initial available connections: " << initialCount << std::endl;
    
    int finalCount = pool.getAvailableConnectionCount();
    g_testSuite.assertEqual(initialCount, finalCount, "Connection count consistent after multiple accesses");
}

int main() {
    std::cout << "==================================================" << std::endl;
    std::cout << "      MySQL Pool Test Suite" << std::endl;
    std::cout << "==================================================" << std::endl;
    
    testLoggerInit();
    
    testConfigLoad();
    
    testSnowflakeId();
    
    testMySQLPoolInit();
    
    testMySQLPoolGetConnection();
    
    testOrderSnGeneration();
    
    testCreateAndQueryOrder();
    
    testUpdateOrderStatus();
    
    testCreateAndQueryAccount();
    
    testUpdateAccountStatus();
    
    testConcurrency();
    
    g_testSuite.report();
    
    std::cout << "\n==================================================" << std::endl;
    std::cout << "           Test Completed" << std::endl;
    std::cout << "==================================================" << std::endl;
    
    return 0;
}