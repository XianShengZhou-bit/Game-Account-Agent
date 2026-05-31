"""数据库工具模块 - 提供数据库操作工具"""

import json
import os
import sys
from typing import Any, Dict, List, Optional

# 添加项目根目录到路径
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))

from agent.common.db import get_mysql_connection
from agent.common.logging_config import setup_logger

logger = setup_logger(__name__)


def create_accounts(
    game_name: str,
    price: int,
    rare_items_contains: List[str],
    server_area: str,
    title: str,
    skin_count: int,
    account_level: int,
    hero_count: int
) -> Dict[str, Any]:
    """创建游戏账号

    Args:
        game_name: 游戏名称
        price: 价格（单位：分）
        rare_items_contains: 稀有物品关键词列表
        server_area: 游戏区服
        title: 账号描述
        skin_count: 皮肤数量
        account_level: 账号等级
        hero_count: 英雄数量

    Returns:
        包含success和message的字典
    """
    logger.info(f"创建游戏账号: game_name={game_name}, price={price}")

    conn = None
    cursor = None
    try:
        conn = get_mysql_connection()
        cursor = conn.cursor()

        # 将稀有物品列表转换为JSON
        rare_items_json = json.dumps(rare_items_contains, ensure_ascii=False)

        # 生成16位UUID作为accounts_id
        import uuid
        accounts_id = uuid.uuid4().hex[:16].upper()

        sql = """
            INSERT INTO accounts (accounts_id, game_name, server_area, title, price,
                                 account_level, hero_count, skin_count, rare_items,
                                 status, version)
            VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, 0, 0)
        """

        cursor.execute(sql, (
            accounts_id, game_name, server_area, title, price,
            account_level, hero_count, skin_count, rare_items_json
        ))
        conn.commit()

        logger.info(f"账号创建成功: accounts_id={accounts_id}")
        return {
            "success": True,
            "message": f"账号创建成功，账号ID：{accounts_id}"
        }

    except Exception as e:
        logger.error(f"创建账号失败: {e}")
        if conn:
            conn.rollback()
        return {
            "success": False,
            "message": f"创建账号失败：{str(e)}"
        }

    finally:
        if cursor:
            cursor.close()
        if conn:
            conn.close()


def search_accounts(
    account_id: Optional[str] = None,
    game_name: Optional[str] = None,
    target_min: Optional[int] = None,
    target_max: Optional[int] = None,
    rare_items_contains: Optional[List[str]] = None,
    server_area: Optional[str] = None,
    skin_count: Optional[int] = None,
    account_level: Optional[int] = None,
    hero_count: Optional[int] = None,
    number: int = 20
) -> Dict[str, Any]:
    """搜索游戏账号

    Args:
        account_id: 账号ID（accounts_id），optional
        game_name: 游戏名称，optional
        target_min: 最低价格（单位：分），optional
        target_max: 最高价格（单位：分），optional
        rare_items_contains: 稀有物品关键词列表，optional
        server_area: 游戏区服，optional
        skin_count: 皮肤数量下限，optional
        account_level: 账号等级下限，optional
        hero_count: 英雄数量下限，optional
        number: 返回账号条数，默认20

    Returns:
        包含success、data和count的字典
    """
    logger.info(f"搜索账号: game_name={game_name}")

    conn = None
    cursor = None
    try:
        conn = get_mysql_connection()
        cursor = conn.cursor(dictionary=True)

        # 构建查询语句
        query = """
            SELECT id, accounts_id, game_name, server_area, title, price,
                   account_level, hero_count, skin_count, rare_items,
                   status, version, created_at
            FROM accounts
            WHERE status = 0
        """
        params = []

        # 如果指定了account_id（账号编号），添加到查询条件
        if account_id:
            query += " AND accounts_id = %s"
            params.append(account_id)

        if game_name:
            query += " AND game_name = %s"
            logger.info(f"查询游戏名称: {game_name}")
            params.append(game_name)

        if target_min is not None:
            query += " AND price >= %s"
            params.append(target_min)

        if target_max is not None:
            query += " AND price <= %s"
            params.append(target_max)

        if server_area:
            query += " AND server_area = %s"
            params.append(server_area)

        if skin_count is not None and skin_count > 0:
            query += " AND skin_count >= %s"
            params.append(skin_count)

        if account_level is not None and account_level > 0:
            query += " AND account_level >= %s"
            params.append(account_level)

        if hero_count is not None and hero_count > 0:
            query += " AND hero_count >= %s"
            params.append(hero_count)

        if rare_items_contains:
            for item in rare_items_contains:
                query += " AND JSON_CONTAINS(rare_items, %s)"
                params.append(json.dumps(item))

        query += " ORDER BY price DESC"

        query += " LIMIT %s"
        params.append(number)

        cursor.execute(query, params)
        logger.info(f"执行查询: {cursor.statement}")
        results = cursor.fetchall()

        # 转换稀有物品JSON为列表，并将价格从分转换为元
        for row in results:
            if row['rare_items']:
                try:
                    row['rare_items'] = json.loads(row['rare_items'])
                except:
                    row['rare_items'] = []
            # 将价格从分转换为元（保留两位小数）
            if 'price' in row and row['price'] is not None:
                row['price'] = round(row['price'] / 100, 2)

        logger.info(f"查询到 {len(results)} 条结果")
        return {
            "success": True,
            "data": results,
            "count": len(results)
        }

    except Exception as e:
        logger.error(f"搜索账号失败: {e}")
        return {
            "success": False,
            "data": [],
            "count": 0,
            "message": f"搜索失败：{str(e)}"
        }

    finally:
        if cursor:
            cursor.close()
        if conn:
            conn.close()


def get_supported_games() -> Dict[str, Any]:
    """获取平台当前支持的游戏列表

    Returns:
        包含success、data（游戏名称列表）和count的字典
    """
    logger.info("获取支持的游戏列表")

    conn = None
    cursor = None
    try:
        conn = get_mysql_connection()
        cursor = conn.cursor()

        cursor.execute(
            "SELECT DISTINCT game_name FROM accounts WHERE status = 0 ORDER BY game_name"
        )
        results = cursor.fetchall()

        games = [row[0] for row in results]

        logger.info(f"获取到 {len(games)} 个支持的游戏")
        return {
            "success": True,
            "data": games,
            "count": len(games)
        }

    except Exception as e:
        logger.error(f"获取游戏列表失败: {e}")
        return {
            "success": False,
            "data": [],
            "count": 0,
            "message": f"获取游戏列表失败：{str(e)}"
        }

    finally:
        if cursor:
            cursor.close()
        if conn:
            conn.close()


def update_account(
    account_id: str,
    game_name: Optional[str] = None,
    price: Optional[int] = None,
    rare_items_contains: Optional[List[str]] = None,
    server_area: Optional[str] = None,
    title: Optional[str] = None,
    skin_count: Optional[int] = None,
    account_level: Optional[int] = None,
    hero_count: Optional[int] = None,
    status: Optional[int] = None
) -> Dict[str, Any]:
    """更新游戏账号信息

    Args:
        account_id: 账号ID（accounts_id），必选
        game_name: 游戏名称，optional
        price: 价格（单位：分），optional
        rare_items_contains: 稀有物品关键词列表，optional
        server_area: 游戏区服，optional
        title: 账号描述，optional
        skin_count: 皮肤数量，optional
        account_level: 账号等级，optional
        hero_count: 英雄数量，optional
        status: 账号状态，optional

    Returns:
        包含success和message的字典
    """
    logger.info(f"更新账号: account_id={account_id}")

    conn = None
    cursor = None
    try:
        conn = get_mysql_connection()
        cursor = conn.cursor()

        # 构建更新语句
        updates = []
        params = []

        if game_name is not None:
            updates.append("game_name = %s")
            params.append(game_name)

        if price is not None:
            updates.append("price = %s")
            params.append(price)

        if rare_items_contains is not None:
            updates.append("rare_items = %s")
            params.append(json.dumps(rare_items_contains, ensure_ascii=False))

        if server_area is not None:
            updates.append("server_area = %s")
            params.append(server_area)

        if title is not None:
            updates.append("title = %s")
            params.append(title)

        if skin_count is not None:
            updates.append("skin_count = %s")
            params.append(skin_count)

        if account_level is not None:
            updates.append("account_level = %s")
            params.append(account_level)

        if hero_count is not None:
            updates.append("hero_count = %s")
            params.append(hero_count)

        if status is not None:
            updates.append("status = %s")
            params.append(status)

        if not updates:
            return {
                "success": False,
                "message": "没有需要更新的字段"
            }

        # 使用accounts_id更新
        sql = f"UPDATE accounts SET {', '.join(updates)} WHERE accounts_id = %s"
        params.append(account_id)

        cursor.execute(sql, params)
        conn.commit()

        if cursor.rowcount > 0:
            logger.info(f"账号更新成功: account_id={account_id}")
            return {
                "success": True,
                "message": "账号信息更新成功"
            }
        else:
            logger.warning(f"账号不存在或无需更新: account_id={account_id}")
            return {
                "success": False,
                "message": "账号不存在或无需更新"
            }

    except Exception as e:
        logger.error(f"更新账号失败: {e}")
        if conn:
            conn.rollback()
        return {
            "success": False,
            "message": f"更新账号失败：{str(e)}"
        }

    finally:
        if cursor:
            cursor.close()
        if conn:
            conn.close()


def create_order(
    account_id: str,
    buyer_id: int,
    price: int,
    buyer_phone: str = "",
    buyer_id_card: str = ""
) -> Dict[str, Any]:
    """创建游戏订单

    Args:
        account_id: 账号ID（accounts_id），必选
        buyer_id: 买家ID，必选
        price: 交易金额（分），必选
        buyer_phone: 买家手机号，optional
        buyer_id_card: 买家身份证号，optional

    Returns:
        包含success、order_sn和message的字典
    """
    logger.info(f"创建订单: account_id={account_id}, buyer_id={buyer_id}, price={price}")

    conn = None
    cursor = None
    try:
        conn = get_mysql_connection()
        cursor = conn.cursor(dictionary=True)

        # 先查询账号是否存在且状态为在售
        cursor.execute(
            "SELECT id, status, version FROM accounts WHERE accounts_id = %s",
            (account_id,)
        )
        account = cursor.fetchone()

        if not account:
            logger.warning(f"账号不存在: account_id={account_id}")
            return {
                "success": False,
                "message": "账号不存在"
            }

        if account['status'] != 0:
            logger.warning(f"账号状态不是待售: account_id={account_id}, status={account['status']}")
            return {
                "success": False,
                "message": "账号状态不是待售"
            }
    

    finally:
        if cursor:
            cursor.close()
        if conn:
            conn.close()
