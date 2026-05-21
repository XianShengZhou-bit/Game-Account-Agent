### 核心业务表：`users` (平台用户表)

| 字段名 | 数据类型 | 约束 | 默认值 | 描述 |
| :--- | :--- | :--- | :--- | :--- |
| `id` | BIGINT | PK, AUTO_INCREMENT | - | 用户主键ID |
| `username` | VARCHAR(64) | UNIQUE, NOT NULL | - | 登录账号/昵称 |
| `password_hash`| VARCHAR(128) | NOT NULL | - | 加密后的密码 (切忌明文) |
| `phone` | VARCHAR(20) | UNIQUE, NULL | - | 绑定手机号 |
| `id_card` | VARCHAR(20) | UNIQUE, NULL | - | 实名认证身份证号 |
| `created_at` | TIMESTAMP | NOT NULL | CURRENT_TIMESTAMP | 注册时间 |

**设计建议：**
在本项目中，**用户系统的认证模块做的是极简处理**。