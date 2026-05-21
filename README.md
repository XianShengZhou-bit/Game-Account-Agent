# 🎮 游戏账号交易平台 - AI Agent 项目

> 基于 LangGraph ReAct 模式的智能游戏账号交易信息展示平台

## 📖 项目简介

本项目是一个展示 **LLMOps 工作流编排** 与 **LangGraph ReAct 模式** 实践能力的 Demo 项目。通过自然语言交互，用户可以搜索游戏账号、浏览商品信息，并与 AI Agent 进行智能对话。

### 核心特性

- 🤖 **ReAct 智能代理**：基于 LangGraph 实现 Think-Act-Observe 循环
- 🎯 **自然语言交互**：支持意图识别与多轮对话
- 🔍 **智能搜索**：多维度筛选游戏账号（价格、等级、皮肤数量等）
- 💳 **交易流程**：支持账号购买和订单创建
- 🌐 **前后端分离**：现代化 Web 架构，前端 React + Vite，后端 FastAPI + LangGraph

## 🏗️ 系统架构

```
┌─────────────────────────────────────────────────────────────┐
│                    前端 (React + TypeScript)                 │
│                      端口: 8082                              │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────┐   │
│  │   首页      │  │   搜索页   │  │   账号列表页   │   │
│  └─────────────┘  └─────────────┘  └─────────────────┘   │
└──────────────────────────────┬──────────────────────────────┘
                               │ HTTP REST API
┌──────────────────────────────▼──────────────────────────────┐
│                    API 服务层 (FastAPI)                        │
│                      端口: 8081                                │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────┐   │
│  │ /api/games  │  │ /api/chat   │  │  /api/accounts  │   │
│  └─────────────┘  └─────────────┘  └─────────────────┘   │
└──────────────────────────────┬──────────────────────────────┘
                               │ LangGraph Agent
┌──────────────────────────────▼──────────────────────────────┐
│                   AI Agent 层 (LangGraph)                      │
│                      端口: 8080                                │
│  ┌─────────────────────────────────────────────────────┐   │
│  │                   Main Graph                         │   │
│  │                                                       │   │
│  │   [Preprocess] → [ReAct Loop] → [Response Formatter] │   │
│  │                         │                             │   │
│  │                    ┌────┴────┐                       │   │
│  │                    │ Reasoner │ ← Think             │   │
│  │                    └────┬────┘                       │   │
│  │                         │                             │   │
│  │              [get_supported_games]                  │   │
│  │              [search_accounts]                      │   │
│  │              [create_order]                         │   │
│  │              [summarize_result]                      │   │
│  │              [extract_account_id]                   │   │
│  └─────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
                               │
                               ▼
                    ┌─────────────────┐
                    │     MySQL       │
                    │   Port: 3306   │
                    └─────────────────┘
```

## 🛠️ 技术栈

### 后端技术

| 技术 | 说明 | 版本 |
|------|------|------|
| **LangGraph** | AI Agent 工作流编排 | ≥1.0.0 |
| **FastAPI** | RESTful API 框架 | ≥0.100.0 |
| **Uvicorn** | ASGI 服务器 | ≥0.23.0 |
| **MySQL** | 关系型数据库 | 5.7+ |
| **python-dotenv** | 环境变量管理 | ≥1.0.0 |
| **OpenAI SDK** | LLM 接口调用 | ≥1.0.0 |

### 前端技术

| 技术 | 说明 | 版本 |
|------|------|------|
| **React** | UI 框架 | 19.1 |
| **TypeScript** | 类型系统 | 5.9 |
| **Vite** | 构建工具 | 6.4 |
| **Ant Design** | UI 组件库 | 6.0 |
| **Tailwind CSS** | 样式框架 | 3.4 |
| **React Router** | 路由管理 | 7.5 |

## 🚀 快速开始

### 环境要求

- Python 3.10+
- Node.js 18+
- MySQL 5.7+
- pnpm 或 npm

### 1. 配置环境变量

```bash
# 复制环境变量模板
cp .env.example .env

### 2. 初始化数据库

```bash
# 连接 MySQL，执行建表脚本
mysql -u root -p < clear_and_add/create_tables.sql

# 导入测试数据（可选）
mysql -u root -p game_account_exchange < clear_and_add/accounts_test_data.sql
```

### 3. 启动服务

```bash
# Windows 用户
./start.bat

# 或手动启动三个服务：

# 终端 1 - LangGraph 后端
cd backend
langgraph dev --port 8080 --no-browser

# 终端 2 - FastAPI 服务
cd backend
uvicorn api.app:app --host 0.0.0.0 --port 8081 --reload

# 终端 3 - 前端开发服务器
cd frontend
pnpm install
pnpm dev --host 0.0.0.0 --port 8082
```

### 4. 访问服务

- 前端页面：http://localhost:8082
- FastAPI 文档：http://localhost:8081/docs
- LangGraph Studio：http://localhost:8080

## 📁 项目结构

```
game_account_exchange/
├── backend/
│   ├── agent/                    # AI Agent 核心代码
│   │   ├── main.py              # 主图定义 (Main Graph + ReAct Loop)
│   │   ├── state.py             # 状态定义 (MainState, ReActState)
│   │   ├── common/
│   │   │   ├── db.py            # 数据库连接池
│   │   │   └── logging_config.py # 日志配置
│   │   └── tools/
│   │       ├── database_tools.py # 数据库操作工具
│   │       └── llm_tools.py     # LLM 调用工具
│   └── api/
│       └── app.py               # FastAPI 服务入口
├── frontend/
│   ├── src/
│   │   ├── App.tsx              # 应用主组件
│   │   ├── main.tsx             # 入口文件
│   │   ├── pages/               # 页面组件
│   │   │   ├── HomePage.tsx     # 首页
│   │   │   ├── SearchPage.tsx   # 搜索页
│   │   │   └── AccountListPage.tsx # 账号列表页
│   │   ├── components/          # 通用组件
│   │   │   ├── Header.tsx       # 顶部导航
│   │   │   ├── GameCard.tsx     # 游戏卡片
│   │   │   ├── GameGrid.tsx     # 游戏网格
│   │   │   ├── SearchBar.tsx    # 搜索栏
│   │   │   └── Chatbot.tsx      # AI 聊天机器人
│   │   ├── services/
│   │   │   └── api.ts           # API 客户端
│   │   └── types.ts             # 类型定义
│   └── package.json
├── clear_and_add/               # 数据管理工具
│   ├── create_tables.sql        # 数据库建表脚本
│   ├── accounts_test_data.sql   # 测试数据
│   ├── games.txt                # 支持的游戏列表
│   ├── servers.txt              # 服务器列表
│   └── rares.txt                # 珍稀装备列表
├── langgraph.json               # LangGraph 配置文件
├── .env.example                 # 环境变量模板
├── .gitignore                   # Git 忽略规则
├── pyproject.toml               # Python 项目配置
└── README.md                    # 项目说明文档
```

## 🎯 ReAct Agent 工作流程

```
用户输入 → Preprocess → ReAct Loop (Think-Act-Observe)
                               │
                               ├── Think (Reasoner)
                               │   LLM 分析当前状态，选择工具
                               │
                               ├── Act (Tool Executor)
                               │   执行选定的工具函数
                               │
                               └── Observe (Should Continue)
                                   判断是否继续或结束
```

### 可用工具函数

| 工具函数 | 功能 |
|---------|------|
| `get_supported_games` | 获取平台支持的游戏列表 |
| `search_accounts` | 多条件搜索游戏账号 |
| `create_accounts` | 创建新的游戏账号 |
| `create_order` | 创建购买订单 |
| `update_account` | 更新账号状态 |
| `extract_account_id` | 从文本中提取账号ID |
| `generate_payment_link` | 生成支付链接 |
| `summarize_result` | 将查询结果总结为自然语言 |

## 📋 核心功能演示

### 1. 智能聊天机器人

用户可以通过自然语言与 AI Agent 对话：

```
用户：我想买一个王者荣耀的账号
Agent：好的，我来帮您搜索王者荣耀的账号信息...

用户：有没有价格低于1000元的？
Agent：根据筛选条件，我为您找到了以下账号...
```

### 2. 游戏账号浏览

- 📋 支持的游戏列表（实时从数据库获取）
- 🖥️ 服务器分区信息
- 💎 珍稀装备展示
- 💰 价格筛选（最低/最高价格）

### 3. 多维度搜索

```
search_accounts 参数说明：
- game_name: 游戏名称
- target_min: 最低价格
- target_max: 最高价格
- server_area: 服务器区域
- skin_count: 皮肤数量
- account_level: 账号等级
- hero_count: 英雄数量
- rare_items_contains: 稀有物品包含
```

## ⚙️ 环境变量说明

```env
# LLM 配置
LLM_API_KEY=your_api_key_here
LLM_BASE_URL=https://api.openai.com/v1
LLM_MODELS=["gpt-4o", "gpt-4o-mini"]
LLM_TIMEOUT=60

# 数据库配置
DB_HOST=localhost
DB_PORT=3306
DB_USER=root
DB_PASSWORD=your_password_here
DB_DATABASE=game_account_exchange

# 服务端口
HOST=localhost
BACKEND_PORT=8080
API_PORT=8081
FRONTEND_PORT=8082

# 雪花ID配置
SNOWFLAKE_MACHINE_ID=1
SNOWFLAKE_DATA_CENTER_ID=1
```

## 🔧 开发指南

### 添加新的工具函数

1. 在 `backend/agent/tools/` 下创建新工具文件
2. 定义工具函数和参数 schema
3. 在 `main.py` 的 `reasoner` 函数中注册工具
4. 在 `tool_executor` 中添加执行逻辑

### 扩展前端页面

1. 在 `frontend/src/pages/` 创建新页面组件
2. 在 `App.tsx` 中添加路由配置
3. 在 `api.ts` 中添加对应的 API 调用方法

## 📄 许可证

MIT License

---

⭐ 如果这个项目对你有帮助，请给我一个 Star！