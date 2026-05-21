### 核心业务表：`orders` (交易订单表)

| 字段名          | 数据类型     | 约束               | 默认值 | 索引    | 描述                      |
| :-------------- | :----------- | :----------------- | :----- | :------ | :------------------------ |
| `id`            | BIGINT       | PK, AUTO_INCREMENT | -      | PRIMARY | 订单内部主键              |
| `order_sn`      | VARCHAR(64)  | UNIQUE, NOT NULL   | -      | UNIQUE  | 订单号（雪花算法生成）    |
| `account_id`    | BIGINT       | NOT NULL           | -      | INDEX   | 关联游戏账号ID            |
| `buyer_id`      | BIGINT       | NOT NULL           | -      | INDEX   | 购买者用户ID              |
| `price`         | INT          | NOT NULL           | -      | -       | 交易金额(分)|
| `buyer_phone`   | VARCHAR(20)  | NOT NULL           | -      | INDEX   | 买家预留手机号            |
| `buyer_id_card` | VARCHAR(32)  | NOT NULL           | -      | INDEX   | 买家预留身份证号          |
| `status`        | TINYINT      | NOT NULL           | 0      | INDEX   | 订单状态: 0-刚下单, 1-支付中, 2-已完成支付 |
| `create_time`   | DATETIME     | NOT NULL           | -      | INDEX   | 订单创建时间              |
| `mutex`         | BOOL         | NOT NULL           | false  | INDEX   | 写锁标识，false无人在写，true有人在写 |

**索引设计：**
* **`order_sn`**：UNIQUE 索引，保证订单号唯一性，支持按订单号快速查询
* **`account_id`**：普通索引，支持按游戏账号ID查询其所有交易记录
* **`buyer_id`**：普通索引，支持按用户ID查询其所有订单
* **`buyer_phone`**：普通索引，支持按手机号查询订单
* **`buyer_id_card`**：普通索引，支持按身份证号查询订单
* **`buyer_id_card`**：普通索引，支持按身份证号查询订单

**格式：**
* **`order_sn`**：订单号格式ORD{yyyyMMdd}{雪花ID后12位}，例如：ORD20260517901234567890
