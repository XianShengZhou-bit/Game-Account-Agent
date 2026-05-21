"""数据库连接池模块"""

import os
from dotenv import load_dotenv
import mysql.connector
from mysql.connector import pooling

load_dotenv()

DB_CONFIG = {
    "host": os.getenv("DB_HOST"),
    "port": int(os.getenv("DB_PORT")),
    "user": os.getenv("DB_USER"),
    "password": os.getenv("DB_PASSWORD"),
    "database": os.getenv("DB_DATABASE"),
    "charset": "utf8mb4",
    "use_unicode": True,
}

_connection_pool = None


def get_connection_pool():
    """获取数据库连接池（延迟初始化）"""
    global _connection_pool
    if _connection_pool is None:
        _connection_pool = pooling.MySQLConnectionPool(
            pool_name="agent_pool",
            pool_size=5,
            **DB_CONFIG
        )
    return _connection_pool


def get_db_connection():
    """从连接池获取数据库连接"""
    return get_connection_pool().get_connection()