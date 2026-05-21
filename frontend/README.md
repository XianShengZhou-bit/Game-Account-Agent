# 游戏账号交易平台 - 前端

这是游戏账号交易平台的前端项目。

## 技术栈

- **框架**: React 19 + TypeScript
- **构建工具**: Vite
- **UI 组件库**: Ant Design
- **样式**: Tailwind CSS
- **路由**: React Router

## 项目结构

```
frontend/
├── src/
│   ├── components/          # 组件
│   │   ├── Chatbot.tsx      # 智能客服聊天组件
│   │   ├── GameCard.tsx    # 游戏卡片组件
│   │   ├── GameGrid.tsx    # 游戏网格组件
│   │   ├── Header.tsx      # 顶部导航栏组件
│   │   └── SearchBar.tsx   # 搜索栏组件
│   ├── pages/             # 页面
│   │   ├── HomePage.tsx  # 首页
│   │   └── SearchPage.tsx # 搜索结果页
│   ├── services/          # 服务
│   │   └── api.ts        # API 客户端
│   ├── types.ts         # 类型定义
│   ├── App.tsx          # 应用入口组件
│   ├── main.tsx         # 应用入口
│   └── index.css        # 全局样式
├── package.json
├── tsconfig.json
├── vite.config.ts
├── tailwind.config.js
└── README.md
```

## 快速开始

### 1. 安装依赖

```bash
cd frontend
npm install
```

### 2. 启动开发服务器

```bash
npm run dev
```

开发服务器将在 `http://localhost:5173` 启动。

### 3. 构建生产版本

```bash
npm run build
```

### 4. 预览生产构建

```bash
npm run preview
```

## 功能特性

- 🌐 **游戏展示：以网格形式展示热门游戏
- 🔍 **搜索功能：顶部搜索栏（待完善
- 🤖 **智能客服：右下角悬浮聊天机器人，与后端 LangGraph 智能体交互
- 📱 **响应式设计：适配不同屏幕尺寸

## 后端集成

前端通过 API 客户端与后端 LangGraph 服务通信：

- API 基础地址：`http://localhost:8181`
- 后端需要确保后端服务正常运行才能使用智能客服功能

## 开发说明

- 智能客服聊天组件位于 `src/components/Chatbot.tsx`
- API 客户端位于 `src/services/api.ts`
- 如需修改 API 地址，请更新 `src/services/api.ts` 中的 baseUrl

## 下一步

- 完善搜索结果页面
- 添加账户详情页面
- 实现更多游戏数据
- 优化用户认证系统

---

**注意**：确保后端服务在 `http://localhost:8181` 正常运行，否则智能客服功能可能无法正常工作。
