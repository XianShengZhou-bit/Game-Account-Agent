# 前端与 LangGraph 交互文档

## 概述

前端通过 LangGraph SDK 与后端 Agent 进行通信，实现 AI 对话功能。简化设计，不使用 session/cookie。

---

## 1. SDK 配置与初始化

### 安装依赖
```bash
npm install @langchain/langgraph-sdk
```

### 初始化客户端
```typescript
import { LangGraphClient } from '@langchain/langgraph-sdk';

const client = new LangGraphClient({
  apiUrl: 'http://localhost:8181',
  apiKey: ''
});
```

---

## 2. 消息发送与接收

### 发送用户消息
```typescript
const response = await client.sessions.sendMessage({
  sessionId: 'default',
  message: {
    type: 'human',
    content: '我想搜索游戏账号'
  }
});
```

---

## 3. 消息格式规范（前后端一致）

### 前端 → 后端消息格式
```json
{
  "type": "human",
  "content": "用户输入的文本内容"
}
```

### 后端 → 前端响应格式

**类型1：AI回复消息**
```json
{
  "type": "ai",
  "content": "AI的回复内容"
}
```

**类型2：中断消息（收集手机号/身份证）**
```json
{
  "type": "interrupt",
  "content": "请提供您的手机号和身份证号",
  "required_fields": ["phone", "id_card"]
}
```

**类型3：订单完成消息**
```json
{
  "type": "order_completed",
  "content": "购买成功！",
  "order_info": {
    "order_sn": "20240101120000001",
    "account_id": 123,
    "buyer_phone": "13800138000",
    "amount": 10000
  }
}
```

**类型4：错误消息**
```json
{
  "type": "error",
  "code": "ACCOUNT_NOT_FOUND",
  "message": "账号不存在",
  "detail": "未找到ID为123的游戏账号",
  "retryable": false,
  "timestamp": "2024-01-01T12:00:00Z"
}
```

### 消息字段说明

| 消息类型 | 字段 | 类型 | 说明 |
|---------|------|------|------|
| human | type | string | 固定为 "human" |
| human | content | string | 用户输入文本 |
| ai | type | string | 固定为 "ai" |
| ai | content | string | AI回复内容 |
| interrupt | type | string | 固定为 "interrupt" |
| interrupt | content | string | 提示信息 |
| interrupt | required_fields | array | 需要收集的字段名列表 |
| order_completed | type | string | 固定为 "order_completed" |
| order_completed | content | string | 提示信息 |
| order_completed | order_info | object | 订单详情 |
| order_info | order_sn | string | 订单号 |
| order_info | account_id | number | 账号ID |
| order_info | buyer_phone | string | 买家手机号 |
| order_info | amount | number | 金额（单位：分） |

---

## 4. 中断处理（收集用户信息）

```typescript
if (response.type === 'interrupt') {
  const userData = {
    phone: '13800138000',
    id_card: '110101199001011234'
  };
  
  const continueResponse = await client.sessions.sendMessage({
    sessionId: 'default',
    message: {
      type: 'human',
      content: JSON.stringify(userData)
    }
  });
}
```

---

## 5. 订单完成处理

```typescript
if (response.type === 'order_completed') {
  const orderInfo = response.order_info;
  console.log('订单号:', orderInfo.order_sn);
  console.log('账号ID:', orderInfo.account_id);
  console.log('手机号:', orderInfo.buyer_phone);
  console.log('金额:', orderInfo.amount);
}
```

---

## 6. 错误处理机制（企业级标准）

### 6.1 错误类型定义

| 错误类别 | 说明 | 示例场景 |
|---------|------|---------|
| **网络错误** | 客户端与服务端通信失败 | 网络断开、超时、DNS解析失败 |
| **认证错误** | 用户身份验证失败 | API Key无效、Token过期 |
| **业务错误** | 业务逻辑校验失败 | 账号不存在、余额不足 |
| **参数错误** | 请求参数校验失败 | 手机号格式错误、缺少必填字段 |
| **系统错误** | 服务端内部错误 | 数据库连接失败、程序异常 |
| **限流错误** | 请求频率超过限制 | 短时间内多次请求 |

### 6.2 错误码规范

| 错误码 | HTTP状态码 | 错误类型 | 说明 | 是否可重试 |
|-------|-----------|---------|------|-----------|
| NETWORK_ERROR | 503 | 网络错误 | 网络连接失败 | 是 |
| TIMEOUT_ERROR | 408 | 网络错误 | 请求超时 | 是 |
| AUTH_ERROR | 401 | 认证错误 | 认证失败 | 否 |
| API_KEY_INVALID | 401 | 认证错误 | API Key无效 | 否 |
| ACCOUNT_NOT_FOUND | 404 | 业务错误 | 账号不存在 | 否 |
| ACCOUNT_LOCKED | 400 | 业务错误 | 账号已被锁定 | 否 |
| ACCOUNT_SOLD_OUT | 400 | 业务错误 | 账号已售出 | 否 |
| INSUFFICIENT_BALANCE | 400 | 业务错误 | 余额不足 | 否 |
| ORDER_CREATE_FAILED | 500 | 业务错误 | 订单创建失败 | 是 |
| VALIDATION_ERROR | 400 | 参数错误 | 参数校验失败 | 否 |
| PHONE_INVALID | 400 | 参数错误 | 手机号格式错误 | 否 |
| ID_CARD_INVALID | 400 | 参数错误 | 身份证格式错误 | 否 |
| SYSTEM_ERROR | 500 | 系统错误 | 服务器内部错误 | 是 |
| DATABASE_ERROR | 500 | 系统错误 | 数据库操作失败 | 是 |
| RATE_LIMIT_EXCEEDED | 429 | 限流错误 | 请求过于频繁 | 是 |

### 6.3 错误响应格式（后端返回）

```json
{
  "type": "error",
  "code": "ACCOUNT_NOT_FOUND",
  "message": "账号不存在",
  "detail": "未找到ID为123的游戏账号，请检查账号ID是否正确",
  "retryable": false,
  "timestamp": "2024-01-01T12:00:00Z",
  "request_id": "req-abc123",
  "suggestion": "请确认账号ID是否正确，或尝试搜索其他账号"
}
```

**字段说明：**

| 字段 | 类型 | 说明 |
|-----|------|------|
| type | string | 固定为 "error" |
| code | string | 错误码 |
| message | string | 错误简短描述 |
| detail | string | 错误详细信息 |
| retryable | boolean | 是否可重试 |
| timestamp | string | 错误发生时间（ISO 8601格式） |
| request_id | string | 请求唯一标识（用于日志追踪） |
| suggestion | string | 处理建议 |

### 6.4 前端错误处理方案

#### 错误处理流程图
```
请求发送 → 成功 → 处理响应
        ↓ 失败
    捕获错误
        ↓
    判断错误类型
        ↓
    ├─ 网络错误 → 提示用户检查网络，提供重试按钮
    ├─ 认证错误 → 提示用户重新登录
    ├─ 参数错误 → 显示具体字段错误信息
    ├─ 业务错误 → 显示业务提示信息
    └─ 系统错误 → 提示用户稍后重试，记录日志
```

#### 统一错误处理组件

```typescript
import { useState, useEffect } from 'react';

interface ErrorState {
  code: string;
  message: string;
  detail: string;
  retryable: boolean;
}

const ErrorHandler: React.FC<{ error: ErrorState; onRetry?: () => void }> = ({ error, onRetry }) => {
  const getErrorMessage = () => {
    switch (error.code) {
      case 'NETWORK_ERROR':
        return '网络连接失败，请检查网络设置';
      case 'TIMEOUT_ERROR':
        return '请求超时，请稍后重试';
      case 'AUTH_ERROR':
        return '登录状态已失效，请重新登录';
      case 'ACCOUNT_NOT_FOUND':
        return '账号不存在，请检查账号ID';
      case 'ACCOUNT_LOCKED':
        return '账号已被锁定，无法购买';
      case 'VALIDATION_ERROR':
        return error.detail || '输入参数有误';
      case 'RATE_LIMIT_EXCEEDED':
        return '请求过于频繁，请稍后再试';
      case 'SYSTEM_ERROR':
        return '服务器繁忙，请稍后重试';
      default:
        return error.message || '发生未知错误';
    }
  };

  return (
    <div className="error-container">
      <div className="error-icon">⚠️</div>
      <div className="error-title">{getErrorMessage()}</div>
      {error.detail && <div className="error-detail">{error.detail}</div>}
      {error.retryable && onRetry && (
        <button onClick={onRetry} className="retry-button">
          重试
        </button>
      )}
    </div>
  );
};
```

#### 请求封装（带重试机制）

```typescript
class ApiClient {
  private client: LangGraphClient;
  private maxRetries: number = 3;

  constructor() {
    this.client = new LangGraphClient({ apiUrl: 'http://localhost:8181' });
  }

  async sendMessage(content: string, retries: number = 0): Promise<any> {
    try {
      const response = await this.client.sessions.sendMessage({
        sessionId: 'default',
        message: { type: 'human', content }
      });

      if (response.type === 'error') {
        if (response.retryable && retries < this.maxRetries) {
          await this.delay(Math.pow(2, retries) * 1000);
          return this.sendMessage(content, retries + 1);
        }
        throw new Error(response.message);
      }

      return response;
    } catch (error) {
      if (this.isNetworkError(error) && retries < this.maxRetries) {
        await this.delay(Math.pow(2, retries) * 1000);
        return this.sendMessage(content, retries + 1);
      }
      throw error;
    }
  }

  private isNetworkError(error: any): boolean {
    const networkErrors = ['NETWORK_ERROR', 'TIMEOUT_ERROR', 'DATABASE_ERROR'];
    return networkErrors.includes(error.code);
  }

  private delay(ms: number): Promise<void> {
    return new Promise(resolve => setTimeout(resolve, ms));
  }
}
```

#### 错误日志上报

```typescript
const reportError = (error: ErrorState) => {
  const errorReport = {
    ...error,
    user_agent: navigator.userAgent,
    timestamp: new Date().toISOString(),
    url: window.location.href
  };

  fetch('/api/error-report', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(errorReport)
  }).catch(() => {});
};
```

---

## 7. 完整流程示例

```typescript
const apiClient = new ApiClient();

try {
  const response = await apiClient.sendMessage('购买账号ID 123');

  if (response.type === 'interrupt') {
    const userData = { phone: '13800138000', id_card: '110101199001011234' };
    const finalResponse = await apiClient.sendMessage(JSON.stringify(userData));
    
    if (finalResponse.type === 'order_completed') {
      alert('购买成功！订单号: ' + finalResponse.order_info.order_sn);
    }
  }
} catch (error) {
  console.error('请求失败:', error);
  reportError(error);
  
  if (error.code === 'ACCOUNT_NOT_FOUND') {
    alert('账号不存在，请重新输入');
  } else if (error.code === 'NETWORK_ERROR') {
    alert('网络连接失败，请检查网络');
  } else {
    alert('操作失败，请稍后重试');
  }
}
```
