#ifndef MYSQL_POOL_HPP
#define MYSQL_POOL_HPP

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <mysql.h>
#include "config.hpp"
#include "logger.hpp"

namespace mysql_pool {

    struct OrderInfo {
        std::string order_sn;
        int64_t account_id;
        int64_t buyer_id;
        std::string buyer_phone;
        std::string buyer_id_card;
        double price;
        int status;
        bool mutex;
        std::string created_at;
    };

    struct AccountInfo {
        std::string account_id;
        std::string game_name;
        std::string server_area;
        std::string title;
        int price;
        int account_level;
        int hero_count;
        int skin_count;
        std::string rare_items;
        int status;
        int version;
        std::string created_at;
    };

    class SnowflakeId 
    {
    public:
        static SnowflakeId& getInstance() {
            static SnowflakeId instance;
            return instance;
        }

        void init(int64_t machine_id, int64_t data_center_id) {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_initialized) return;
            _machine_id = machine_id;
            _data_center_id = data_center_id;
            _initialized = true;
        }

        int64_t nextId() {
            std::lock_guard<std::mutex> lock(_mutex);

            int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count() - _epoch;

            if (now < _last_timestamp) {
                now = _last_timestamp;
            }

            if (now == _last_timestamp) {
                _sequence = (_sequence + 1) & _max_sequence;
                if (_sequence == 0) {
                    now = waitNextMillis();
                }
            } else {
                _sequence = 0;
            }

            _last_timestamp = now;

            return ((now << 22) |
                    (_data_center_id << 17) |
                    (_machine_id << 12) |
                    (_sequence));
        }

        int64_t getLast12Bits() {
            return nextId() & 0xFFF;
        }

    private:
        SnowflakeId() : _initialized(false), _machine_id(0), _data_center_id(0),
                    _sequence(0), _last_timestamp(0) {}

        int64_t waitNextMillis() {
            int64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count() - _epoch;
            while (timestamp <= _last_timestamp) {
                timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count() - _epoch;
            }
            return timestamp;
        }

        static constexpr int64_t _epoch = 1704067200000L;
        static constexpr int64_t _max_sequence = 0xFFF;

        bool _initialized;
        int64_t _machine_id;
        int64_t _data_center_id;
        int64_t _sequence;
        int64_t _last_timestamp;
        std::mutex _mutex;
    };

    class MySQLPool {
    public:
        static MySQLPool& getInstance(int pool_size = 10) {
            LOG_INFO("[数据库连接池] 获取实例");
            static MySQLPool instance;
            return instance;
        }

        bool init(int pool_size = 10) {
            LOG_INFO("[数据库连接池] 初始化");
            std::lock_guard<std::mutex> lock(_mutex);
            if (_pool_size > 0) {
                LOG_ERROR("[数据库连接池] 已初始化，无法重复初始化");
                return false;
            }

            config::Config& config = const_cast<config::Config&>(config::Config::getInstance());
            int64_t machine_id = config.getSnowflakeMachineId();
            int64_t data_center_id = config.getSnowflakeDataCenterId();
            SnowflakeId::getInstance().init(machine_id, data_center_id);

            if (pool_size <= 0) pool_size = 10;
            _pool_size = pool_size;
            
            std::string host = config.getMySQLHost();
            int port = config.getMySQLPort();
            std::string user = config.getMySQLUser();
            std::string password = config.getMySQLPassword();
            std::string database = config.getMySQLDatabase();

            for (int i = 0; i < pool_size; ++i) {
                MYSQL* conn = mysql_init(nullptr);
                if (!conn) {
                    LOG_ERROR("[数据库连接池] MySQL连接 {} 初始化失败", i);
                    continue;
                }

                int reconnect = 1;
                mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);

                if (!mysql_real_connect(conn, host.c_str(), user.c_str(), 
                    password.c_str(), database.c_str(), port, nullptr, 0)) {
                    LOG_ERROR("[数据库连接池] MySQL连接 {} 失败: {}", i, mysql_error(conn));
                    mysql_close(conn);
                    continue;
                }

                _pool.push(conn);
                LOG_INFO("[数据库连接池] MySQL连接 {} 创建成功", i);
            }

            if (_pool.empty()) {
                LOG_ERROR("[数据库连接池] MySQL连接池创建失败");
                return false;
            }

            LOG_INFO("[数据库连接池] 已初始化，共 {} 个连接", _pool.size());
            return true;
        }
        
        OrderInfo createOrder(int64_t account_id, int64_t buyer_id,
                            const std::string& buyer_phone, const std::string& buyer_id_card,
                            double price) {
            LOG_INFO("[订单服务] 创建订单开始，account_id: {}, buyer_id: {}, price: {}", account_id, buyer_id, price);
            OrderInfo order;
            MYSQL* conn = getConnection();
            if (!conn) {
                LOG_ERROR("[订单服务] 获取MySQL连接失败");
                return order;
            }

            try {
                auto now = std::chrono::system_clock::now();
                auto time_t = std::chrono::system_clock::to_time_t(now);
                char timestamp[32];
                strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&time_t));

                std::string order_sn = generateOrderSn();
                int price_in_cents = static_cast<int>(price * 100);

                char query[1024];
                snprintf(query, sizeof(query),
                    "INSERT INTO orders (order_sn, account_id, buyer_id, buyer_phone, buyer_id_card, price, status, create_time, mutex) "
                    "VALUES ('%s', %ld, %ld, '%s', '%s', %d, 0, '%s', 0)",
                    order_sn.c_str(), account_id, buyer_id, 
                    buyer_phone.c_str(), buyer_id_card.c_str(), price_in_cents, timestamp);

                if (mysql_query(conn, query) != 0) {
                    LOG_ERROR("[订单服务] 创建订单失败: {}", mysql_error(conn));
                    returnConnection(conn);
                    return order;
                }

                order.order_sn = order_sn;
                order.account_id = account_id;
                order.buyer_id = buyer_id;
                order.buyer_phone = buyer_phone;
                order.buyer_id_card = buyer_id_card;
                order.price = price;
                order.status = 0;
                order.mutex = false;
                order.created_at = timestamp;

                LOG_INFO("[订单服务] 创建订单完成: {}", order_sn);
            } catch (const std::exception& e) {
                LOG_ERROR("[订单服务] 创建订单失败: {}", e.what());
            }
            returnConnection(conn);
            return order;
        }

        bool updateOrderStatus(const std::string& order_sn, int status) {
            LOG_INFO("[订单服务] 更新订单状态开始，order_sn: {}, status: {}", order_sn, status);
            MYSQL* conn = getConnection();
            if (!conn) {
                LOG_ERROR("[订单服务] 获取MySQL连接失败");
                return false;
            }

            try {
                auto now = std::chrono::system_clock::now();
                auto time_t = std::chrono::system_clock::to_time_t(now);
                char timestamp[32];
                strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&time_t));

                char query[512];
                snprintf(query, sizeof(query),
                    "UPDATE orders SET status = %d, create_time = '%s' WHERE order_sn = '%s'",
                    status, timestamp, order_sn.c_str());

                if (mysql_query(conn, query) != 0) {
                    LOG_ERROR("[订单服务] 更新订单状态失败: {}", mysql_error(conn));
                    returnConnection(conn);
                    return false;
                }

                unsigned long long affected = mysql_affected_rows(conn);
                
                LOG_INFO("[订单服务] 更新订单状态完成，order_sn: {}, status: {}, 影响行数: {}", order_sn, status, affected);
                returnConnection(conn);
                return affected > 0;
            } catch (const std::exception& e) {
                LOG_ERROR("[订单服务] 更新订单状态失败: {}", e.what());
                returnConnection(conn);
                return false;
            }
        }

        OrderInfo getOrderBySn(const std::string& order_sn) {
            LOG_INFO("[订单服务] 查询订单开始，order_sn: {}", order_sn);
            OrderInfo order;
            MYSQL* conn = getConnection();
            if (!conn) {
                LOG_ERROR("[订单服务] 获取MySQL连接失败");
                return order;
            }

            try {
                char query[512];
                snprintf(query, sizeof(query),
                    "SELECT order_sn, account_id, buyer_id, buyer_phone, buyer_id_card, price, status, create_time, mutex "
                    "FROM orders WHERE order_sn = '%s'", order_sn.c_str());

                if (mysql_query(conn, query) != 0) {
                    LOG_ERROR("[订单服务] 查询订单失败: {}", mysql_error(conn));
                    returnConnection(conn);
                    return order;
                }

                MYSQL_RES* result = mysql_store_result(conn);
                if (!result) {
                    LOG_ERROR("[订单服务] 获取查询结果失败: {}", mysql_error(conn));
                    returnConnection(conn);
                    return order;
                }

                MYSQL_ROW row = mysql_fetch_row(result);
                if (row) {
                    order.order_sn = row[0] ? row[0] : "";
                    order.account_id = row[1] ? atoll(row[1]) : 0;
                    order.buyer_id = row[2] ? atoll(row[2]) : 0;
                    order.buyer_phone = row[3] ? row[3] : "";
                    order.buyer_id_card = row[4] ? row[4] : "";
                    int price_in_cents = row[5] ? atoi(row[5]) : 0;
                    order.price = price_in_cents / 100.0;
                    order.status = row[6] ? atoi(row[6]) : 0;
                    order.mutex = row[7] ? (atoi(row[7]) != 0) : false;
                    order.created_at = row[8] ? row[8] : "";
                }
                mysql_free_result(result);

                LOG_INFO("[订单服务] 查询订单完成，order_sn: {}, 找到: {}", order_sn, !order.order_sn.empty());
            } catch (const std::exception& e) {
                LOG_ERROR("[订单服务] 查询订单失败: {}", e.what());
            }
            returnConnection(conn);
            return order;
        }

        AccountInfo getAccountById(const std::string& account_id) {
            LOG_INFO("[账号服务] 查询账号开始，account_id: {}", account_id);
            AccountInfo account;
            MYSQL* conn = getConnection();
            if (!conn) {
                LOG_ERROR("[账号服务] 获取MySQL连接失败");
                return account;
            }

            try {
                char query[512];
                snprintf(query, sizeof(query),
                    "SELECT accounts_id, game_name, server_area, title, price, account_level, hero_count, skin_count, rare_items, status, version, created_at "
                    "FROM accounts WHERE accounts_id = '%s'", account_id.c_str());

                if (mysql_query(conn, query) != 0) {
                    LOG_ERROR("[账号服务] 查询账号失败: {}", mysql_error(conn));
                    returnConnection(conn);
                    return account;
                }

                MYSQL_RES* result = mysql_store_result(conn);
                if (!result) {
                    LOG_ERROR("[账号服务] 获取查询结果失败: {}", mysql_error(conn));
                    returnConnection(conn);
                    return account;
                }

                MYSQL_ROW row = mysql_fetch_row(result);
                if (row) {
                    account.account_id = row[0] ? row[0] : "";
                    account.game_name = row[1] ? row[1] : "";
                    account.server_area = row[2] ? row[2] : "";
                    account.title = row[3] ? row[3] : "";
                    account.price = row[4] ? atoi(row[4]) : 0;
                    account.account_level = row[5] ? atoi(row[5]) : 0;
                    account.hero_count = row[6] ? atoi(row[6]) : 0;
                    account.skin_count = row[7] ? atoi(row[7]) : 0;
                    account.rare_items = row[8] ? row[8] : "";
                    account.status = row[9] ? atoi(row[9]) : 0;
                    account.version = row[10] ? atoi(row[10]) : 0;
                    account.created_at = row[11] ? row[11] : "";
                }
                mysql_free_result(result);

                LOG_INFO("[账号服务] 查询账号完成，account_id: {}, 找到: {}", account_id, !account.account_id.empty());
            } catch (const std::exception& e) {
                LOG_ERROR("[账号服务] 查询账号失败: {}", e.what());
            }
            returnConnection(conn);
            return account;
        }

        bool updateAccountStatus(const std::string& account_id, int status) {
            LOG_INFO("[账号服务] 更新账号状态开始，account_id: {}, status: {}", account_id, status);
            MYSQL* conn = getConnection();
            if (!conn) {
                LOG_ERROR("[账号服务] 获取MySQL连接失败");
                return false;
            }

            try {
                char query[512];
                snprintf(query, sizeof(query),
                    "UPDATE accounts SET status = %d WHERE accounts_id = '%s'",
                    status, account_id.c_str());

                if (mysql_query(conn, query) != 0) {
                    LOG_ERROR("[账号服务] 更新账号状态失败: {}", mysql_error(conn));
                    returnConnection(conn);
                    return false;
                }

                unsigned long long affected = mysql_affected_rows(conn);
                
                LOG_INFO("[账号服务] 更新账号状态完成，account_id: {}, status: {}, 影响行数: {}", account_id, status, affected);
                returnConnection(conn);
                return affected > 0;
            } catch (const std::exception& e) {
                LOG_ERROR("[账号服务] 更新账号状态失败: {}", e.what());
                returnConnection(conn);
                return false;
            }
        }

        int getAvailableConnectionCount() const {
            return static_cast<int>(_pool.size());
        }

    private:
        MySQLPool() = default;

        ~MySQLPool() { destroy(); }

        MySQLPool(const MySQLPool&) = delete;
        MySQLPool& operator=(const MySQLPool&) = delete;

        void destroy() {
            std::lock_guard<std::mutex> lock(_mutex);
            while (!_pool.empty()) {
                MYSQL* conn = _pool.front();
                _pool.pop();
                if (conn) {
                    mysql_close(conn);
                }
            }
            LOG_INFO("[数据库连接池] 已销毁");
        }

        MYSQL* getConnection() {
            std::unique_lock<std::mutex> lock(_mutex);
            _cv.wait(lock, [this] { return !_pool.empty(); });
            MYSQL* conn = _pool.front();
            _pool.pop();
            return conn;
        }

        void returnConnection(MYSQL* conn) {
            if (!conn) return;
            std::lock_guard<std::mutex> lock(_mutex);
            _pool.push(conn);
            _cv.notify_one();
        }

        std::string generateOrderSn() {
            int64_t id = SnowflakeId::getInstance().nextId();
            time_t now = time(nullptr);
            struct tm* tm_info = localtime(&now);
            char buffer[32];
            strftime(buffer, sizeof(buffer), "%Y%m%d", tm_info);
            std::string prefix = "ORD" + std::string(buffer);
            std::string suffix = std::to_string(id);
            if (suffix.length() > 12) {
                suffix = suffix.substr(suffix.length() - 12);
            }
            return prefix + suffix;
        }

        std::mutex _mutex;
        std::condition_variable _cv;
        std::queue<MYSQL*> _pool;
        int _pool_size = 0;
    };
}

#endif