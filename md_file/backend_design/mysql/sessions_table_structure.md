### 核心业务表：`langgraph_sessions` (LangGraph 会话持久化表)

| 字段名               | 数据类型         | 约束                  | 默认值                | 描述                |
| :---------------- | :----------- | :------------------ | :----------------- | :---------------- |
| `id`              | BIGINT       | PK, AUTO\_INCREMENT | -                  | 记录主键ID            |
| `session_id`      | VARCHAR(128) | NOT NULL, UNIQUE    | -                  | 会话唯一标识（对应用户/对话线程） |
| `summary_content` | MEDIUMTEXT   | NOT NULL            | -                  | 汇总消息内容（每次更新时覆盖）   |
| `token_count`     | INT          | NOT NULL            | 0                  | 当前汇总消息的 token 数量  |
| `max_token_limit` | INT          | NOT NULL            | 6000               | 汇总消息最大 token 限制   |
| `message_count`   | INT          | NOT NULL            | 0                  | 累计处理的消息轮数         |
| `updated_at`      | TIMESTAMP    | NOT NULL            | CURRENT\_TIMESTAMP | 汇总消息最后更新时间        |

**索引设计 (Indexes)：**

1. `PRIMARY KEY (id)`：聚簇索引，加速主键定位。
2. `UNIQUE KEY uk_session_id (session_id)`：唯一索引，保证每个会话只有一条记录。
3. `KEY idx_session_updated (session_id, updated_at)`：加速会话更新时间查询。

***

**编码层面实现要点：**

1. 后端接收到 `humanmessage` 后，先通过业务逻辑处理得到 `AImessage`
2. 从数据库查询当前会话的 `summary_content`
3. 构造 LLM 调用请求：历史汇总 + humanmessage + AImessage
4. 在 LLM prompt 中明确限制输出不超过 6000 tokens
5. 验证 token 大小，不满足则重新调用 LLM，满足则覆盖更新 summary\_content。
