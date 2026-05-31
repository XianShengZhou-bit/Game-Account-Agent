"""数据库连接池模块"""

import os
from dotenv import load_dotenv
import mysql.connector
from mysql.connector import pooling

load_dotenv()

MYSQL_CONFIG = {
    "host": os.getenv("MYSQL_HOST"),
    "port": int(os.getenv("MYSQL_PORT")),
    "user": os.getenv("MYSQL_USER"),
    "password": os.getenv("MYSQL_PASSWORD"),
    "database": os.getenv("MYSQL_DATABASE"),
    "charset": "utf8mb4",
    "use_unicode": True,
}

_connection_pool = None


def get_mysql_connection_pool():
    """获取MySQL数据库连接池（延迟初始化）"""
    global _connection_pool
    if _connection_pool is None:
        _connection_pool = pooling.MySQLConnectionPool(
            pool_name="agent_pool",
            pool_size=5,
            **MYSQL_CONFIG
        )
    return _connection_pool


def get_mysql_connection():
    """从连接池获取MySQL数据库连接"""
    return get_mysql_connection_pool().get_connection()
