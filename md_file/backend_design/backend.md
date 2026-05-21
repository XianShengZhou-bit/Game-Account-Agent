# 游戏账号交易平台 - Agent 架构设计文档

> 本文档描述基于 ReAct (Reasoning + Acting) 模式的智能游戏账号交易平台 Agent 架构。

## 目录

- [1. 概述](#1-概述)
- [2. 架构设计](#2-架构设计)
- [3. 工具模块](#3-工具模块)
- [4. 工作流程](#4-工作流程)
- [5. 状态管理](#5-状态管理)
- [6. 技术实现](#6-技术实现)

---

## 1. 概述

### 1.1 设计目标

构建一个基于 ReAct 模式的智能 Agent 系统，实现以下功能流程：

```
前端智能体捕获用户请求
    ↓
LLM分析用户需求
    ↓
LLM自主调用各类工具完成处理
    ↓
结果通过前端智能体返回给用户
```

### 1.2 核心优势

- **自主决策**：LLM 根据用户输入自主判断需要调用哪些工具
- **模块化设计**：工具职责单一，便于维护和扩展
- **可观测性**：清晰的工具调用链路，便于调试和监控
- **灵活性**：支持动态组合工具，适应复杂业务场景

---

## 2. 架构设计

### 2.1 整体架构图

```
┌─────────────────────────────────────────────────────────────┐
│                        前端 (React)                          │
│                      端口: 8082                              │
│  ┌─────────────────────────────────────────────────────┐   │
│  │              Frontend Agent                          │   │
│  │  - 捕获用户请求                                      │   │
│  │  - 渲染 Agent 响应                                  │   │
│  │  - 管理对话上下文                                   │   │
│  └─────────────────────────────────────────────────────┘   │
└──────────────────────────────┬──────────────────────────────┘
                               │ HTTP REST API
┌──────────────────────────────▼──────────────────────────────┐
│                     Agent 调度层 (LangGraph)                │
│                      端口: 8080                              │
│  ┌─────────────────────────────────────────────────────┐   │
│  │                  Main Graph                          │   │
│  │  ┌───────────┐  ┌────────────┐  ┌──────────────┐  │   │
│  │  │ Request   │→ │   Reason   │→ │ Tool         │  │   │
│  │  │ Handler   │  │   (LLM)    │  │ Executor     │  │   │
│  │  └───────────┘  └────────────┘  └──────────────┘  │   │
│  └─────────────────────────────────────────────────────┘   │
│                           │                                  │
│                    工具调用层                                │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────────┐    │
│  │Database │ │Business │ │Response │ │   LLM       │    │
│  │ Tools   │ │ Tools   │ │ Tools   │ │  Tools      │    │
│  └─────────┘ └─────────┘ └─────────┘ └─────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 核心组件

| 组件 | 职责 | 说明 |
|------|------|------|
| **Frontend Agent** | 前端交互层 | 捕获用户请求，渲染 Agent 响应，管理对话上下文 |
| **Reasoner (LLM)** | 推理决策中心 | 分析用户需求，决定调用哪些工具并执行调用 |
| **Tool Layer** | 工具层 | 包含所有可调用的工具模块 |

---

## 3. 工具模块

### 3.1 工具分类总览

```
工具层 (Tool Layer)
├── 数据库工具 (Database Tools)
│   ├── create_accounts(硬编码预定义sql)
│   ├── search_accounts(硬编码预定义sql)
│   ├── update_account(硬编码预定义sql)
│   └── create_order(硬编码预定义sql)
│
└── LLM 工具 (LLM Tools)
    ├── summarize_result
    ├── extract_account_id(硬编码预定义sql)
    └── generate_payment_link
```

### 3.2 数据库工具 (Database Tools)

#### 3.2.1 create_accounts

**功能**：创建游戏账号

**参数**：

```json
{
  "game_name": "string",           // 游戏名称，必选
  "price": "int",                  // 价格（单位：分），必选
  "rare_items_contains": ["string"], // 稀有物品关键词列表，必选
  "server_area": "string",         // 游戏区服，必选
  "title": "string",               // 账号描述，必选
  "skin_count": "int",             // 皮肤数量，必选
  "account_level": "int",          // 账号等级，必选
  "hero_count": "int"              // 英雄数量，必选
}
```

**返回值**：

```json
{
  "success": "bool",
  "message": "string"
}
```

**账号状态**：

- `0`: 在售
- `1`: 交易中
- `2`: 已售出

---

#### 3.2.2 search_accounts

**功能**：搜索游戏账号

**参数**：

```json
{
  "account_id": "string",           // 账号 ID，optional
  "game_name": "string",           // 游戏名称，optional
  "target_min": "int",              // 最低价格（单位：分），optional
  "target_max": "int",              // 最高价格（单位：分），optional
  "rare_items_contains": ["string"], // 稀有物品关键词列表，optional
  "server_area": "string",          // 游戏区服，optional
  "skin_count": "int",             // 皮肤数量下限，optional
  "account_level": "int",          // 账号等级下限，optional
  "hero_count": "int",              // 英雄数量下限，optional
  "number": "int"                    // 返回账号条数，默认 20
}
```

**返回值**：

```json
{
  "success": true,
  "data": [
    {
      "id": "string",
      "game_name": "string",
      "server_area": "string",
      "title": "string",
      "price": "int",
      "account_level": "int",
      "hero_count": "int",
      "skin_count": "int",
      "rare_items": "string",
      "status": "int",
      "version": "int",
      "created_at": "datetime"
    }
  ],
  "count": "int"
}
```

**SQL 生成规则**：

- 只生成 SELECT 查询语句
- 必须包含 `WHERE status = 0`（只查询在售账号）
- 按价格降序排列
- 使用参数化查询防止 SQL 注入

---

#### 3.2.3 update_account

**功能**：更新游戏账号信息

**参数**：

```json
{
  "account_id": "string",           // 账号 ID，必选
  "game_name": "string",           // 游戏名称，optional
  "price": "int",                  // 价格（单位：分），optional
  "rare_items_contains": ["string"], // 稀有物品关键词列表，optional
  "server_area": "string",         // 游戏区服，optional
  "title": "string",               // 账号描述，optional
  "skin_count": "int",             // 皮肤数量，optional
  "account_level": "int",          // 账号等级，optional
  "hero_count": "int",             // 英雄数量，optional
  "status": "int"                  // 账号状态，optional
}
```

**返回值**：

```json
{
  "success": "bool",
  "message": "string"
}
```

**账号状态**：

- `0`: 在售
- `1`: 交易中
- `2`: 已售出

---

#### 3.2.4 create_order

**功能**：创建游戏订单

**参数**：

```json
{
  "account_id": "string",           // 账号 ID，必选
  "buyer_id": "int",               // 买家 ID，必选
  "price": "int"                   // 交易金额（分），必选
}
```

**返回值**：

```json
{
  "success": "bool",
  "order_sn": "string",
  "message": "string"
}
```

---

### 3.3 LLM 工具 (LLM Tools)

#### 3.3.1 summarize_result

**功能**：总结sql操作执行结果为礼貌得体的自然语言

**参数**：

```json
{
  "query_result": [
    {
      "id": "string",             // 账号 ID, optional
      "game_name": "string",      // 游戏名称, optional
      "server_area": "string",    // 游戏区服, optional
      "title": "string",          // 账号描述, optional
      "price": "int",             // 价格（单位：分），optional
      "account_level": "int",     // 账号等级, optional
      "hero_count": "int",        // 英雄数量, optional
      "skin_count": "int",        // 皮肤数量, optional
      "rare_items": "string",    // 稀有物品关键词, optional
      "success": "bool",          // 是否成功, optional
      "message": "string"          // 操作结果, optional
    }
  ]
}
```

**返回值**：

```json
{
  "response": "string"  // 格式化的自然语言响应
}
```

**格式化规则**：

- 价格单位从分转换为元，保留 2 位小数
- 账号信息以清晰的卡片格式展示
- 禁止使用 `#`、`*` 等特殊字符

---

#### 3.3.2 extract_account_id

**功能**：从用户输入中提取账号 ID

**参数**：

```json
{
  "user_input": "string"  // 用户输入，包含账号 ID
}
```

**返回值**：

```json
{
  "account_id": "string"  // 提取到的账号 ID
}
```

**提取规则**：

- 从用户输入中提取账号 ID，支持直接输入账号 ID 或包含账号 ID 的文本
- 账号 ID 长度必是 16 位
- 账号 ID 不能包含空格、特殊字符

---

#### 3.3.3 generate_payment_link

**功能**：生成支付链接

**参数**：

```json
{
  "order_sn": "string"  // 订单号，必选
}
```

**返回值**：

```json
{
  "payment_link": "string"  // 支付链接
}
```

---

## 4. 工作流程

### 4.1 整体流程

```
用户请求
    ↓
┌─────────────────────────────────────────┐
│           Frontend Agent                │
│  1. 捕获用户输入                         │
│  2. 维护对话上下文                       │
│  3. 渲染 Agent 响应                     │
└─────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────┐
│           Agent 调度层                  │
│                                         │
│  ┌───────────────┐                      │
│  │ Request       │                      │
│  │ Handler       │                      │
│  │ (接收用户输入) │                      │
│  └───────┬───────┘                      │
│          ↓                              │
│  ┌───────────────┐                      │
│  │   Reasoner    │                      │
│  │   (LLM 推理)   │ ← 分析用户需求       │
│  │               │ ← 决定调用哪些工具    │
│  └───────┬───────┘                      │
│          ↓                              │
│  ┌───────────────┐                      │
│  │  Tool         │ ← 执行工具           │
│  │  Executor     │ ← 返回工具结果       │
│  └───────┬───────┘                      │
│          ↓                              │
│  ┌───────────────┐                      │
│  │   Response    │ ← LLM 组织响应       │
│  │   Formatter   │ ← 返回最终结果       │
│  └───────────────┘                      │
└─────────────────────────────────────────┘
    ↓
用户响应
```

### 4.2 ReAct 模式流程

```
┌─────────────────────────────────────────┐
│         ReAct Loop                      │
│                                         │
│  Loop:                                  │
│    1. Think (LLM)                       │
│       - 分析当前状态                     │
│       - 决定下一步行动                   │
│       - 选择要调用的工具                 │
│                                         │
│    2. Act (Tool Executor)               │
│       - 调用选定的工具                   │
│       - 获取工具执行结果                 │
│                                         │
│    3. Observe                           │
│       - 分析工具返回结果                 │
│       - 决定是否继续循环                 │
│                                         │
│    4. 如果需要更多工具，重复 1-3        │
│    5. 如果完成，生成最终响应             │
└─────────────────────────────────────────┘
```

### 4.3 工具调用示例

#### 场景 1：搜索游戏账号

```
用户：我想买一个王者荣耀的账号，价格在 500-1000 元之间

Agent (Think):
  用户想要搜索游戏账号，需要：
  1. 直接调用 search_accounts 工具（参数从用户输入中提取）
  2. 调用 summarize_result 工具生成自然语言响应

Agent (Act):
  → search_accounts(game_name="王者荣耀", target_min=50000, target_max=100000)
  ← {success: true, data: [{id: "2a1e803f431d4dd2", price: 68000, ...}], count: 1}

  → summarize_result(query_result=[{...}])
  ← "为您找到以下符合条件的账号..."

用户响应:
  为您找到以下符合条件的王者荣耀账号：
  1. 【王者】- 尊贵铂金 - 价格：680.00元 - 账号 2a1e803f431d4dd2
     ...
```

#### 场景 2：购买游戏账号

```
用户：我想购买账号 ID 为 2a1e803f431d4dd2 的账号

Agent (Think):
  用户想要购买指定账号，需要：
  1. 调用 search_accounts 验证账号存在且可售
  2. 调用 update_account 锁定账号（status=1）
  3. 调用 create_order 创建订单
  4. 调用 generate_payment_link 生成支付链接

Agent (Act):
  → search_accounts(account_id="2a1e803f431d4dd2")
  ← {success: true, data: [{id: "2a1e803f431d4dd2", price: 68000, status: 0, ...}]}

  → update_account(account_id="2a1e803f431d4dd2", status=1)
  ← {success: true, message: "账号状态已更新"}

  → create_order(account_id="2a1e803f431d4dd2", buyer_id=12345, price=68000)
  ← {success: true, order_sn: "ORD20260517901234567890", message: "订单已创建"}

  → generate_payment_link(order_sn="ORD20260517901234567890")
  ← {payment_link: "https://example.com/pay/ORD20260517901234567890"}

用户响应:
  订单已创建，订单号：ORD20260517901234567890，价格：680.00元
  请点击以下链接完成支付：
  https://example.com/pay/ORD20260517901234567890
```

#### 场景 3：普通聊天

```
用户：今天天气真好

Agent (Think):
  用户只是在闲聊，不需要调用任何数据库或业务工具，
  直接使用 LLM 进行对话即可

Agent (Act):
  无需调用工具，直接生成响应

用户响应:
  是的，天气很好呢！很高兴能为您服务。有什么关于游戏账号的问题可以随时问我。
```

---

## 5. 状态管理

### 5.1 主图状态 (MainState)

```python
@dataclass
class MainState:
    """主图状态定义"""
    user_input: str = ""              # 用户输入
    conversation_history: List[Dict] = []  # 对话历史
    current_action: str = ""           # 当前执行的动作
    tool_calls: List[Dict] = []       # 工具调用历史
    tool_results: List[Any] = []       # 工具执行结果
    response: str = ""                # 返回给用户的响应
```

### 5.2 ReAct 状态 (ReActState)

```python
@dataclass
class ReActState:
    """ReAct 循环状态"""
    user_input: str = ""              # 用户输入
    thought: str = ""                 # LLM 思考过程
    action: str = ""                  # 当前要执行的动作
    action_input: Dict = {}           # 动作输入参数
    observation: str = ""             # 工具执行结果
    response: str = ""                # 最终响应
    step_count: int = 0               # 循环步数
    max_steps: int = 10               # 最大循环步数
```

---

## 6. 技术实现

### 6.1 LLM 配置

- **API**: 从配置文件读取
- **Base URL**: 从配置文件读取
- **模型列表**: 从配置文件读取
- **超时时间**: 60 秒
- **重试策略**: 循环尝试多个模型，超时重试次数为0，若全部超时则从第一个LLM重新开始尝试调用，直到成功或总循环次数超过最大总循环次数。

### 6.2 工具调用机制

使用 LangGraph 的 `toolcalling` 特性实现：

```python
# 定义可调用的工具
tools = [
    create_accounts,
    search_accounts,
    create_order,
    update_account,
    extract_account_id,
    generate_payment_link,
    summarize_result,
]

# LLM 调用工具
llm_with_tools = llm.bind_tools(tools)
```

### 6.3 数据库连接

- **连接池管理**: 使用 mysql-connector-python 的连接池
- **连接池大小**: 5

### 6.4 账号锁定机制

- **乐观锁**: 使用 version 字段实现
- **状态流转**:
  - `0`: 在售
  - `1`: 交易中
  - `2`: 已售出

---

## 附录

### A. 数据库表结构

参见 `md_file/backend_design/mysql/` 目录下的表结构文档。
