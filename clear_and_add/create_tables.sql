CREATE DATABASE IF NOT EXISTS game_account_exchange CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

USE game_account_exchange;

DROP TABLE IF EXISTS accounts;

CREATE TABLE accounts (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    accounts_id VARCHAR(64) NOT NULL UNIQUE,
    game_name VARCHAR(64) NOT NULL,
    server_area VARCHAR(64) NOT NULL,
    title VARCHAR(128) NOT NULL,
    price INT NOT NULL,
    account_level INT NOT NULL DEFAULT 0,
    hero_count INT NOT NULL DEFAULT 0,
    skin_count INT NOT NULL DEFAULT 0,
    rare_items JSON NOT NULL DEFAULT ('[]'),
    status TINYINT NOT NULL DEFAULT 0,
    version INT NOT NULL DEFAULT 0,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,

    INDEX idx_game_server_price_skin_level_hero (game_name, server_area, price, skin_count, account_level, hero_count),
    INDEX idx_game_price (game_name, price),
    INDEX idx_accounts_id (accounts_id),
    INDEX idx_status (status)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DROP TABLE IF EXISTS orders;

CREATE TABLE orders (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    order_sn VARCHAR(64) NOT NULL UNIQUE,
    account_id BIGINT NOT NULL,
    buyer_id BIGINT NOT NULL,
    price INT NOT NULL,
    buyer_phone VARCHAR(20) NOT NULL,
    buyer_id_card VARCHAR(32) NOT NULL,
    status TINYINT NOT NULL DEFAULT 0,
    create_time DATETIME NOT NULL,
    mutex BOOL NOT NULL DEFAULT FALSE,

    INDEX idx_order_sn (order_sn),
    INDEX idx_account_id (account_id),
    INDEX idx_buyer_id (buyer_id),
    INDEX idx_buyer_phone (buyer_phone),
    INDEX idx_buyer_id_card (buyer_id_card),
    INDEX idx_status (status)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

SELECT 'Tables created successfully!' AS result;
SHOW TABLES;
