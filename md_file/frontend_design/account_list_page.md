# 账号列表页面设计文档

> 文档版本：v1.0  
> 创建日期：2026-05-17  
> 所属模块：前端页面

---

## 目录

- [1. 功能概述](#1-功能概述)
- [2. 页面结构](#2-页面结构)
- [3. API 接口设计](#3-api-接口设计)
- [4. 组件设计](#4-组件设计)
- [5. 状态管理](#5-状态管理)
- [6. 交互流程](#6-交互流程)
- [7. 数据模型](#7-数据模型)

---

## 1. 功能概述

### 1.1 功能定位

账号列表页面是游戏账号交易平台的核心展示页面，主要功能包括：

- ✅ 展示所有在售的游戏账号
- ✅ 支持按游戏名称、价格区间、区服筛选
- ✅ 展示账号详细信息（等级、英雄、皮肤、珍稀物品）
- ✅ 支持查看账号详情
- ✅ 支持直接购买账号

### 1.2 页面入口

| 入口位置 | 描述 |
|----------|------|
| 导航栏 | 点击"账号列表"链接 |
| 路由地址 | `/accounts` |

---

## 2. 页面结构

### 2.1 整体布局

```
┌─────────────────────────────────────────────────────────────────┐
│                        Header（导航栏）                        │
├─────────────────────────────────────────────────────────────────┤
│                    页面标题 + 描述                              │
├─────────────────────────────────────────────────────────────────┤
│                    筛选表单区域                                │
│  [游戏名称下拉] [最低价格输入] [最高价格输入] [区服下拉] [搜索]   │
├─────────────────────────────────────────────────────────────────┤
│                    账号卡片列表                                │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐           │
│  │ 卡片1   │ │ 卡片2   │ │ 卡片3   │ │ 卡片4   │ ...        │
│  └─────────┘ └─────────┘ └─────────┘ └─────────┘           │
├─────────────────────────────────────────────────────────────────┤
│                      账号详情弹窗                              │
│  ┌───────────────────────────────────────────────────────┐   │
│  │ 标题 | 价格 | 状态标签                                │   │
│  │ 游戏信息 | 账号属性                                 │   │
│  │ 账号描述 | 珍稀物品                                 │   │
│  │ [关闭] [立即购买]                                   │   │
│  └───────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

### 2.2 卡片结构

| 区域 | 内容 | 说明 |
|------|------|------|
| 封面图 | 游戏首字母 | 渐变色背景展示 |
| 标题区域 | 账号标题 + 状态标签 | 状态：在售/交易中/已售出 |
| 游戏信息 | 游戏名称 + 区服 | 图标标识 |
| 属性信息 | 等级、英雄数、皮肤数 | 图标+数值 |
| 珍稀物品 | 标签形式展示 | 最多显示3个，超出显示数量 |
| 价格区域 | 价格 + 购买按钮 | 价格红色突出显示 |

---

## 3. API 接口设计

### 3.1 接口列表

| API 路径 | HTTP 方法 | 功能描述 | 文件位置 |
|----------|----------|----------|----------|
| `/api/accounts` | GET | 获取账号列表（支持筛选） | [app.py](file:///C:/Users/haipe/Desktop/game_account_exchange/backend/api/app.py#L110) |
| `/api/accounts/{account_id}` | GET | 获取账号详情 | [app.py](file:///C:/Users/haipe/Desktop/game_account_exchange/backend/api/app.py#L158) |
| `/api/accounts/{account_id}/buy` | POST | 购买账号 | [app.py](file:///C:/Users/haipe/Desktop/game_account_exchange/backend/api/app.py#L187) |
| `/api/games_list` | GET | 获取游戏列表（筛选用） | [app.py](file:///C:/Users/haipe/Desktop/game_account_exchange/backend/api/app.py#L92) |
| `/api/servers` | GET | 获取区服列表（筛选用） | [app.py](file:///C:/Users/haipe/Desktop/game_account_exchange/backend/api/app.py#L98) |

### 3.2 接口详细设计

#### 3.2.1 GET /api/accounts

**请求参数**：

| 参数名 | 类型 | 必填 | 说明 |
|--------|------|------|------|
| game_name | string | 否 | 游戏名称 |
| target_min | int | 否 | 最低价格（分） |
| target_max | int | 否 | 最高价格（分） |
| server_area | string | 否 | 区服名称 |

**成功响应**（200）：

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
      "description": "string",
      "status": "int",
      "version": "int",
      "created_at": "datetime"
    }
  ]
}
```

**失败响应**（4xx/5xx）：

```json
{
  "success": false,
  "message": "string"
}
```

#### 3.2.2 GET /api/accounts/{account_id}

**请求参数**：

| 参数名 | 类型 | 必填 | 说明 |
|--------|------|------|------|
| account_id | string | 是 | 账号ID（路径参数） |

**成功响应**（200）：

```json
{
  "success": true,
  "data": {
    "id": "string",
    "game_name": "string",
    "server_area": "string",
    "title": "string",
    "price": "int",
    "account_level": "int",
    "hero_count": "int",
    "skin_count": "int",
    "rare_items": "string",
    "description": "string",
    "status": "int",
    "version": "int",
    "created_at": "datetime"
  }
}
```

#### 3.2.3 POST /api/accounts/{account_id}/buy

**请求参数**：

| 参数名 | 类型 | 必填 | 说明 |
|--------|------|------|------|
| account_id | string | 是 | 账号ID（路径参数） |

**成功响应**（200）：

```json
{
  "success": true,
  "message": "订单已创建，请通过智能助手完成购买流程",
  "data": {
    "account_id": "string"
  }
}
```

---

## 4. 组件设计

### 4.1 页面组件结构

```
AccountListPage.tsx
├── Header              # 导航头部
├── 筛选区域
│   ├── Select          # 游戏名称下拉
│   ├── Input           # 最低价格输入
│   ├── Input           # 最高价格输入
│   ├── Select          # 区服下拉
│   └── Button          # 搜索按钮
├── 账号列表
│   └── Card[]          # 账号卡片列表
│       ├── Card.Meta   # 卡片内容
│       └── Button      # 购买按钮
└── Modal               # 账号详情弹窗
    ├── 信息展示区
    └── 操作按钮区
```

### 4.2 组件文件位置

| 组件 | 文件路径 | 说明 |
|------|----------|------|
| AccountListPage | [pages/AccountListPage.tsx](file:///C:/Users/haipe/Desktop/game_account_exchange/frontend/src/pages/AccountListPage.tsx) | 主页面组件 |
| Header | [components/Header.tsx](file:///C:/Users/haipe/Desktop/game_account_exchange/frontend/src/components/Header.tsx) | 导航栏组件 |
| ApiClient | [services/api.ts](file:///C:/Users/haipe/Desktop/game_account_exchange/frontend/src/services/api.ts) | API 客户端 |

---

## 5. 状态管理

### 5.1 页面状态

```typescript
interface PageState {
  accounts: Account[];           // 账号列表数据
  loading: boolean;              // 加载状态
  error: string | null;          // 错误信息
  selectedAccount: Account | null; // 选中的账号（用于弹窗）
  isModalOpen: boolean;          // 弹窗显示状态
  searchParams: {                // 搜索参数
    game_name: string;
    min_price: string;
    max_price: string;
    server_area: string;
  };
  gameOptions: string[];         // 游戏选项列表
  serverOptions: string[];       // 区服选项列表
}
```

### 5.2 状态流转

```
页面初始化
    ↓
加载游戏选项和区服选项
    ↓
获取账号列表
    ↓
渲染账号卡片
    ↓
[用户点击卡片]
    ↓
打开详情弹窗
    ↓
[用户点击购买]
    ↓
调用购买API
    ↓
刷新账号列表
```

---

## 6. 交互流程

### 6.1 筛选流程

```
用户选择筛选条件
    ↓
点击搜索按钮
    ↓
构建查询参数
    ↓
调用 GET /api/accounts
    ↓
更新账号列表展示
```

### 6.2 查看详情流程

```
用户点击账号卡片
    ↓
设置 selectedAccount 状态
    ↓
打开详情弹窗
    ↓
渲染账号详细信息
    ↓
[用户点击关闭]
    ↓
关闭弹窗
```

### 6.3 购买流程

```
用户点击购买按钮
    ↓
调用 POST /api/accounts/{id}/buy
    ↓
显示购买结果提示
    ↓
刷新账号列表
    ↓
[如果购买成功]
    ↓
账号状态变为"交易中"或"已售出"
```

---

## 7. 数据模型

### 7.1 Account 接口

```typescript
export interface Account {
  id: string;           // 账号唯一ID
  game_name: string;    // 游戏名称
  server_area: string;  // 区服名称
  title: string;        // 账号标题
  price: number;        // 价格（单位：分）
  account_level: number; // 账号等级
  hero_count: number;   // 英雄数量
  skin_count: number;   // 皮肤数量
  rare_items: string;   // 珍稀物品（JSON数组格式）
  description: string;  // 账号描述
  status: number;       // 状态：0-在售，1-交易中，2-已售出
  version: number;      // 乐观锁版本号
  created_at: string;   // 创建时间
}
```

### 7.2 状态码说明

| 状态码 | 含义 | 显示标签 |
|--------|------|----------|
| 0 | 在售 | 绿色标签 |
| 1 | 交易中 | 橙色标签 |
| 2 | 已售出 | 灰色标签 |

---

## 附录

### A. 价格单位转换

前端显示时需要将分转换为元：

```typescript
const formatPrice = (price: number): string => {
  return (price / 100).toFixed(2);
};
```

### B. 珍稀物品解析

```typescript
const parseRareItems = (items: string): string[] => {
  try {
    return JSON.parse(items);
  } catch {
    return items.split(',').map(item => item.trim());
  }
};
```

### C. 文件清单

| 文件 | 路径 | 修改类型 |
|------|------|----------|
| AccountListPage.tsx | `frontend/src/pages/` | 新建 |
| types.ts | `frontend/src/` | 修改（添加 Account 接口） |
| api.ts | `frontend/src/services/` | 修改（添加 API 方法） |
| App.tsx | `frontend/src/` | 修改（添加路由） |
| Header.tsx | `frontend/src/components/` | 修改（添加导航链接） |
| app.py | `backend/api/` | 修改（添加 API 端点） |
