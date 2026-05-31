#!/usr/bin/env python3
"""
游戏账号交易平台API服务 - 提供REST API接口和Agent服务
"""

import os
import sys

# 获取项目根目录（backend目录的父目录）
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, project_root)

from typing import List, Dict, Any
from pydantic import BaseModel
from dotenv import load_dotenv
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
import mysql.connector
from pypinyin import lazy_pinyin

# 导入Agent
from agent import run_agent
load_dotenv()

app = FastAPI(title="游戏账号交易平台 API", version="2.0.0")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

STATIC_DIR = os.path.join(project_root, "static")
app.mount("/images", StaticFiles(directory=os.path.join(STATIC_DIR, "images")), name="images")

MYSQL_CONFIG = {
    "host": os.getenv("MYSQL_HOST"),
    "port": int(os.getenv("MYSQL_PORT")),
    "user": os.getenv("MYSQL_USER"),
    "password": os.getenv("MYSQL_PASSWORD"),
    "database": os.getenv("MYSQL_DATABASE"),
    "charset": "utf8mb4",
    "use_unicode": True,
}


def get_mysql_connection():
    """获取MySQL数据库连接"""
    conn = mysql.connector.connect(**MYSQL_CONFIG)
    conn.set_charset_collation('utf8mb4', 'utf8mb4_unicode_ci')
    return conn


def chinese_to_pinyin(name: str) -> str:
    """将中文名称转换为拼音，用下划线连接"""
    pinyin_list = lazy_pinyin(name)
    return '_'.join(pinyin_list)


# ============ Agent 接口 ============

class ChatRequest(BaseModel):
    """聊天请求"""
    message: str


class ChatResponse(BaseModel):
    """聊天响应"""
    success: bool
    response: str


@app.post("/api/chat", response_model=ChatResponse, tags=["Agent"])
def chat(request: ChatRequest):
    """智能聊天接口 - 使用Agent处理用户输入"""
    try:
        response = run_agent(request.message)
        return ChatResponse(
            success=True,
            response=response
        )
    except Exception as e:
        return ChatResponse(
            success=False,
            response=f"抱歉，服务暂时不可用：{str(e)}"
        )


# ============ 游戏接口 ============

@app.get("/api/games", tags=["游戏"])
def get_game_list():
    """获取所有游戏名称列表"""
    conn = None
    try:
        conn = get_mysql_connection()
        cursor = conn.cursor()

        cursor.execute("SELECT DISTINCT game_name FROM accounts ORDER BY game_name")
        games = cursor.fetchall()

        game_list = []
        icons = ['🎮', '⚔️', '🌟', '🎯', '🏆', '🔫', '⚡', '🌍', '🚀', '🏥', '👹', '🎭', '🥚', '💎', '🔥', '💀', '🎨', '🎪', '🐉', '🦋', '🌈', '⚙️', '🎵', '🎭', '🔮']

        for i, (game_name,) in enumerate(games):
            pinyin_name = chinese_to_pinyin(game_name)
            game_list.append({
                "id": i + 1,
                "name": game_name,
                "icon": icons[i % len(icons)],
                "image": f"/images/games/{pinyin_name}.png"
            })

        return {"success": True, "data": game_list}

    except Exception as e:
        return {"success": False, "message": str(e)}

    finally:
        if conn:
            conn.close()


def read_txt_file(file_path):
    """读取txt文件内容"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            lines = f.readlines()
            items = [line.strip() for line in lines if line.strip()]
        return {"success": True, "data": items}
    except Exception as e:
        return {"success": False, "message": str(e)}


@app.get("/api/games_list", tags=["数据文件"])
def get_games_list():
    """获取交易所支持的游戏列表（从games.txt读取）"""
    file_path = get_clear_and_add_dir() / "games.txt"
    return read_txt_file(file_path)


@app.get("/api/servers", tags=["数据文件"])
def get_servers_list():
    """获取游戏区服列表（从servers.txt读取）"""
    file_path = get_clear_and_add_dir() / "servers.txt"
    return read_txt_file(file_path)


@app.get("/api/rares", tags=["数据文件"])
def get_rares_list():
    """获取珍稀装备列表（从rares.txt读取）"""
    file_path = get_clear_and_add_dir() / "rares.txt"
    return read_txt_file(file_path)


@app.get("/api/accounts", tags=["账号"])
def get_accounts(
    game_name: str = None,
    target_min: int = None,
    target_max: int = None,
    server_area: str = None
):
    """获取账号列表，支持筛选条件"""
    conn = None
    try:
        conn = get_mysql_connection()
        cursor = conn.cursor(dictionary=True)

        query = """
            SELECT id, accounts_id, game_name, server_area, title, price,
                   account_level, hero_count, skin_count, rare_items,
                   status, version, created_at
            FROM accounts
            WHERE status = 0
        """
        params = []

        if game_name:
            query += " AND game_name = %s"
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

        query += " ORDER BY price DESC LIMIT 50"

        cursor.execute(query, params)
        accounts = cursor.fetchall()

        return {"success": True, "data": accounts}

    except Exception as e:
        return {"success": False, "message": str(e)}

    finally:
        if conn:
            conn.close()


@app.get("/api/accounts/{account_id}", tags=["账号"])
def get_account_detail(account_id: str):
    """获取账号详情"""
    conn = None
    try:
        conn = get_mysql_connection()
        cursor = conn.cursor(dictionary=True)

        cursor.execute("""
            SELECT id, accounts_id, game_name, server_area, title, price,
                   account_level, hero_count, skin_count, rare_items,
                   status, version, created_at
            FROM accounts
            WHERE accounts_id = %s
        """, (account_id,))

        account = cursor.fetchone()

        if account:
            return {"success": True, "data": account}
        else:
            return {"success": False, "message": "账号不存在"}

    except Exception as e:
        return {"success": False, "message": str(e)}

    finally:
        if conn:
            conn.close()


@app.post("/api/accounts/{account_id}/buy", tags=["账号"])
def buy_account(account_id: str):
    """购买账号（触发Agent流程）"""
    try:
        # 使用Agent处理购买请求
        message = f"我想购买账号 ID 为 {account_id} 的账号"
        response = run_agent(message)

        return {
            "success": True,
            "message": response,
            "data": {"account_id": account_id}
        }
    except Exception as e:
        return {
            "success": False,
            "message": f"购买失败：{str(e)}"
        }


@app.get("/health", tags=["健康检查"])
def health_check():
    """健康检查"""
    return {"status": "ok"}


if __name__ == "__main__":
    import uvicorn
    host = get_env("HOST")
    port = get_env_int("API_PORT")
    uvicorn.run(app, host=host, port=port)
