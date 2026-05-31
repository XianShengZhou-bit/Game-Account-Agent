import os
import sys
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'venv', 'Lib', 'site-packages'))

import mysql.connector
from dotenv import load_dotenv

def main():
    # 加载环境变量
    load_dotenv(os.path.join(os.path.dirname(__file__), '..', '.env'))
    
    # 获取数据库配置
    host = os.getenv('MYSQL_HOST', 'localhost')
    port = int(os.getenv('MYSQL_PORT', 3306))
    user = os.getenv('MYSQL_USER', 'root')
    password = os.getenv('MYSQL_PASSWORD', '')
    database = os.getenv('MYSQL_DATABASE', 'game_account_exchange')
    
    try:
        # 连接数据库
        conn = mysql.connector.connect(
            host=host,
            port=port,
            user=user,
            password=password,
            database=database
        )
        cursor = conn.cursor()
        
        # 清空 orders 表
        cursor.execute("TRUNCATE TABLE orders")
        print(f"✓ orders 表已清空")
        
        # 清空 accounts 表
        cursor.execute("TRUNCATE TABLE accounts")
        print(f"✓ accounts 表已清空")
        
        conn.commit()
        print("\n✓ 所有表已成功清空！")
        
    except mysql.connector.Error as err:
        print(f"✗ 数据库错误: {err}")
        sys.exit(1)
    finally:
        if conn.is_connected():
            cursor.close()
            conn.close()

if __name__ == "__main__":
    print("=== 清空数据库表 ===")
    print("注意：此操作将永久删除所有测试数据！")
    confirm = input("确认继续？(y/N): ").strip().lower()
    if confirm == 'y':
        main()
    else:
        print("操作已取消")
