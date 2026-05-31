# cpp 微服务架构设计

## 项目结构

```
game_account_exchange/
│
├── proto/                                    # .proto 协议定义文件
│   ├── order.proto                           # 订单服务接口定义
│
├── service_cpp/                              # C++ 微服务目录
│   ├── common/                              # 公共模块（所有微服务共享）
│   │   ├── config.hpp                       # 配置读取
│   │   ├── redis_client.hpp                 # Redis 基础客户端
│   │   ├── redis_lock.hpp                   # 分布式锁封装
│   │   ├── mysql_pool.hpp                   # MySQL 连接池
│   │   ├── logger.hpp                       # 日志
│   │   ├── error_code.hpp                   # 错误码定义
│   │   └── jwt.hpp                          # JWT 工具
│   │
│   ├── order/                               # 订单微服务
│   │   ├── order_service.hpp                # 业务逻辑实现
│   │   └── main.cpp                         # 服务入口
│   │
│   └── CMakeLists.txt                       # 顶层 CMake 配置
│
├── backend/
│   ├── service/                             # Python 调用 C++ 的客户端
│   │   ├── order_client.py                  # 订单服务 gRPC 客户端
│   │
│   └── agent/
│       └── tools/
│           ├── create_order_tool.py         # 创建订单工具
│
├── sql/                                     # 数据库脚本
│   └── schema.sql                           # 数据库表结构
│
└── .env                                    # 环境变量配置
```

---

## 各目录职责

| 目录 | 职责 | 说明 |
|------|------|------|
| `proto/` | 定义接口契约 | `.proto` 文件被 protoc 工具分别生成 Python 和 C++ 代码 |
| `service_cpp/common/` | 公共模块 | Redis、MySQL、Logger、Config，所有微服务共享 |
| `service_cpp/common/redis_lock.hpp` | 分布式锁 | 防止热门账号超卖 |
| `service_cpp/common/jwt.hpp` | JWT 工具 | Token 生成与验证 |
| `service_cpp/order/` | 订单微服务 | C++ 订单创建、支付链接生成、订单状态管理 |
| `backend/service/` | Python gRPC 客户端 | 封装 gRPC 调用，供 Agent 使用 |
| `backend/agent/tools/` | Agent 工具 | 调用 `xxx_client.py`，暴露给 LLM |
| `sql/` | 数据库脚本 | MySQL 表结构定义 |

---

## 数据流

### 订单创建流程

```
用户输入 → Python Agent (LLM 提取信息：account_id, buyer_id, phone, id_card)
                    ↓
    backend/agent/tools/create_order_tool.py
                    ↓ 调用 order_client.py
    backend/service/order_client.py
                    ↓ gRPC 调用 (Protobuf 二进制)
    service_cpp/order/src/main.cpp (cpp OrderService)
                    ↓
              MySQL (读取账号真实价格)
                    ↓
                 创建订单并落库
                    ↓ 
              生成 payment_url (格式: ${FRONTEND_URL}/payment?token=JWT)
                    ↓ 返回 payment_url
    backend/service/order_client.py
                    ↓ 返回
    backend/agent/tools/create_order_tool.py
                    ↓ 返回工具执行结果
              Python Agent (LLM 组织自然语言)
                    ↓ 自然语言响应 + payment_url
                    前端展示给用户
```

**完整链路**：LLM → 工具 → gRPC Client → cpp → MySQL → 生成支付链接 → 逐层返回 → LLM → 前端

**JWT Token 构造**：
- cpp 服务端从 MySQL 查询 `order_sn` 和真实 `price`
- 将 `order_sn`、`price`、`exp`（过期时间）写入 JWT Payload
- 使用 `SECRET_KEY` 对 Payload 签名，生成防篡改 Token
- 返回格式：`${FRONTEND_URL}/payment?token=eyJhbGciOiJIUzI1NiJ9...`

---

### 支付流程

```
前端 payment 页面 → 用户输入密码 → FastAPI (/api/payment/callback)
                    ↓
              gRPC 调用 → cpp OrderService (UpdateOrderStatus)
                    ↓
              cpp 更新订单状态 → 返回成功
                    ↓
              FastAPI → 前端渲染 "支付成功"
```

**完整链路**：前端 → FastAPI → gRPC → cpp → 逐层返回 → 前端

---

### gRPC + Protobuf 优势

| 特性 | 说明 |
|------|------|
| **传输格式** | Protobuf 二进制序列化，比 JSON 小 3-10 倍 |
| **解析速度** | 比 JSON 快 20-100 倍 |
| **协议** | HTTP/2，多路复用，头部压缩 |
| **代码生成** | 自动生成 Python/cpp 强类型代码 |

---

## proto/ 内容

```
proto/
└── order.proto       # 订单服务 protobuf 定义
```

---

## service/order/

| 微服务 | 端口 | 依赖 | 说明 |
|--------|------|------|------|
| `order/` | `${ORDER_SERVICE_PORT}` | MySQL, 支付网关 | 订单创建、状态管理 |

---

## 端口分配

| 服务 | 端口 | 说明 |
|------|------|------|
| cpp OrderService | `${ORDER_SERVICE_PORT}` | 订单微服务 |

> **注意**: 所有端口号统一从 `.env` 配置文件读取

---

## 前端支付页面

### 路由设计

| 路由 | 说明 |
|------|------|
| `/payment` | 支付回调接口，接收 URL 参数 |

### URL 格式（安全设计）

```
# JWT 方案
${FRONTEND_URL}/payment?token={JWT Token}
```

**JWT Token 包含**：
- `order_sn`：订单号
- `price`：真实金额（防篡改）
- `exp`：过期时间（防重放）
- `signature`：签名（防伪造）

### 示例

```
# JWT Token 示例（Base64 可解码，签名不可伪造）
http://localhost:8082/payment?token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJvcmRlcl9zbiI6IjIwMjYwNTIyMTIzNDU2Nzg5Ii4uLn0.abc123signature
```

### 页面功能

1. 解析 URL 参数中的 JWT Token
2. 解码 Token 获取 `order_sn` 和 `price`（只读展示，不可修改）
3. 展示订单号和金额给用户
4. 6位密码输入框（任意数字）
5. 点击确认 → POST `/api/payment/callback` → FastAPI → gRPC → cpp 更新状态 → 返回成功 → 前端渲染成功

**前端安全原则**：前端仅展示 Token 中的数据，所有关键金额以 MySQL 为准。

### 调用链路

```
前端 payment 页面
    ↓ POST /api/payment/callback { token, password }
    ↓
FastAPI (8081)
    ↓ gRPC UpdateOrderStatus (token)
    ↓
cpp OrderService (8083)
    ↓ ① JWT 验证签名（防篡改）
    ↓ ② 从 MySQL 读取真实 price（以数据库为准）
    ↓ ③ 执行支付，更新订单状态
    ↓
返回成功 → FastAPI → 前端
```

**cpp 验证逻辑**：
1. 验证 JWT Token 签名一致性（防篡改）
2. 从 MySQL 查询订单真实价格
3. 使用 MySQL 的价格执行扣款（忽略 Token 中的 price）

---

## JWT 安全设计

### Token 结构

```
token = Header.Payload.Signature

Header:       {"alg": "HS256", "typ": "JWT"}
Payload:      {"order_sn": "xxx", "price": 100, "exp": 1747...}
Signature:    HMAC-SHA256(Header.Payload, SECRET_KEY)
```

### 安全性原理

| 攻击方式 | 有 JWT 签名的保护 | 原因 |
|----------|-------------------|------|
| **篡改 price** | ❌ 无法通过验证 | 篡改后签名不匹配 |
| **篡改 order_sn** | ❌ 无法通过验证 | 签名验证失败 |
| **伪造 token** | ❌ 无法生成有效签名 | 不知道 SECRET_KEY |
| **重放攻击** | ✅ exp 过期时间限制 | Token 有时效性 |
| **重复支付** | ✅ 订单状态检查 | MySQL 记录状态 |

### SECRET_KEY 管理

| 存储方式 | 安全性 | 说明 |
|----------|--------|------|
| 写在代码里 | ❌ 不推荐 | 泄露到 Git 即失效 |
| 写在 .env 文件 | ✅ 推荐 | 仅本地配置，不提交 Git |
| 环境变量注入 | ✅ 推荐 | K8s/Docker 常见方式 |

```bash
# .env 示例
JWT_SECRET=your_secure_256bit_secret_key_here
```

### 验证流程（cpp 服务端）

```cpp
bool verify_payment_token(const std::string& token) {
    // ① JWT 自验证（防篡改 + 防伪造）
    auto payload = jwt_verify(token, SECRET_KEY);
    if (!payload.valid) {
        return false;
    }

    // ② 检查是否过期
    if (payload.exp < now()) {
        return false;
    }

    // ③ 从 MySQL 读取真实价格（以数据库为准）
    auto order = mysql.query_order(payload.order_sn);
    auto real_price = order.price;

    // ④ 使用真实价格执行支付
    charge_user(order.buyer_id, real_price);

    return true;
}
```

### 存储

| 数据类型 | 存储位置 | 说明 |
|----------|---------|------|
| JWT 签名验证 | ❌ 不需要 | 签名数学自验证 |

---

## 环境变量配置

### 项目根目录中.env的配置信息

```bash
# JWT 安全配置
JWT_SECRET=

# MySQL 配置
MYSQL_HOST=
MYSQL_PORT=
MYSQL_USER=
MYSQL_PASSWORD=
MYSQL_DATABASE=

# Redis 配置
REDIS_HOST=
REDIS_PORT=
REDIS_DB=
REDIS_PASSWORD=

# 服务端口
ORDER_SERVICE_PORT=

# 前端
FRONTEND_PORT=
FRONTEND_URL=

# FastAPI
API_PORT=
```

---

## Redis 分布式锁设计

### Key 前缀规范

```
格式：{业务线}:{数据类型}:{资源类型}:{标识符}
示例：exchange:lock:account:ACC001
```

### Key 设计表

| 用途 | Key 格式 | TTL | 说明 |
|------|----------|-----|------|
| 账号分布式锁 | `exchange:lock:account:{account_id}` | 60秒 | 下单防超卖 |
| 订单分布式锁 | `exchange:lock:order:{order_sn}` | 60秒 | 支付防重复 |
| 验证码 | `exchange:captcha:{phone}` | 5分钟 | 短信验证码 |

## 错误码体系

### 业务错误码 (宏定义(ERROR_CODE))

| 错误码 | gRPC 状态码 | HTTP 状态码 | 说明 | 用户提示 |
|--------|-------------|-------------|------|----------|
| `SUCCESS` | `OK` | 200 | 成功 | - |
| `ACCOUNT_LOCKED` | `RESOURCE_EXHAUSTED` | 429 | 账号正被操作 | 账号正在被其他买家操作，请稍后重试 |
| `ACCOUNT_SOLD` | `ALREADY_EXISTS` | 409 | 账号已售出 | 该账号已售出 |
| `INVALID_TOKEN` | `UNAUTHENTICATED` | 401 | Token 无效 | 支付链接无效，请重新下单 |
| `TOKEN_EXPIRED` | `UNAUTHENTICATED` | 401 | Token 过期 | 支付链接已过期，请重新下单 |
| `ORDER_NOT_FOUND` | `NOT_FOUND` | 404 | 订单不存在 | 订单不存在 |
| `ORDER_PAID` | `ALREADY_EXISTS` | 409 | 订单已支付 | 该订单已支付完成 |
| `BALANCE_INSUFFICIENT` | `FAILED_PRECONDITION` | 400 | 余额不足 | 余额不足，请充值 |
| `INTERNAL_ERROR` | `INTERNAL` | 500 | 服务器内部错误 | 系统繁忙，请稍后重试 |
| `DB_ERROR` | `INTERNAL` | 500 | 数据库错误 | 系统繁忙，请稍后重试 |
| `REDIS_ERROR` | `UNAVAILABLE` | 503 | Redis 不可用 | 服务暂不可用，请稍后重试 |

### gRPC 错误响应格式

```proto
message ErrorResponse {
    ERROR_CODE error_code = 1;      // 错误码
    int32 http_status_code = 2;      // HTTP 状态码（前端直接使用此字段）
    string user_message = 3;         // 用户提示（直接展示给用户）
    string error_message = 4;        // 错误信息（内部调试用）
    int64 timestamp = 5;            // 时间戳
}
```

前端根据 `http_status_code` 设置 HTTP 状态码，根据 `user_message` 展示用户提示，错误响应闭环。

---

```