### 核心业务表：`accounts` (游戏账号商品表)

| 字段名             | 数据类型         | 约束                     | 默认值                | 描述                          |
| :-------------- | :----------- | :--------------------- | :----------------- | :-------------------------- |
| `id`            | BIGINT       | PK, AUTO\_INCREMENT    | -                  | 账号内部主键ID (自带聚簇索引)   |
| `accounts_id`   | VARCHAR(64)  | UNIQUE, NOT NULL       | 16位uuid            | 游戏账号编号                 |
| `game_name`     | VARCHAR(64)  | NOT NULL               | -                  | 所属游戏名称 (如: 失重人间游记、晚风碎月事务所)  |
| `server_area`   | VARCHAR(64)  | NOT NULL               | -                  | 区服名称 (如: 雾里偷闲、单机游戏) |
| `title`         | VARCHAR(128) | NOT NULL               | -                  | 卖家描述                        |
| `price`         | INT          | NOT NULL               | -                  | 价格(单位:分，避免精度丢失)             |
| `account_level` | INT          | NOT NULL               | 0                  | 账号等级                        |
| `hero_count`    | INT          | NOT NULL               | 0                  | 英雄/角色数量                     |
| `skin_count`    | INT          | NOT NULL               | 0                  | 皮肤数量                        |
| `rare_items`    | JSON         | NOT NULL               | `'[]'`             | 珍稀道具列表 (如: ["龙狙", "火麒麟"])  |
| `status`        | TINYINT      | NOT NULL               | 0                  | 状态: 0-在售, 1-交易中, 2-已售出      |
| `version`       | INT          | NOT NULL               | 0                  | 乐观锁版本号 (防并发超卖与买卖双方同时修改账号状态) |
| `created_at`    | TIMESTAMP    | NOT NULL               | CURRENT\_TIMESTAMP | 上架时间                        |

**索引设计 (Indexes)：**

1. `PRIMARY KEY (id)`：聚簇索引，加速主键定位。
2. `KEY idx_game_server_price_skin_level_hero (game_name, server_area, price, skin_count, account_level, hero_count)`：联合索引，支持多维度组合查询，覆盖游戏名、区服、价格、皮肤数量、账号等级、英雄数量的联合筛选场景。
3. `KEY idx_game_price (game_name, price)`：联合索引，加速按游戏名+价格区间的查询场景（如：查找某游戏下特定价格范围的账号）。
4. `KEY idx_game_rare_items (game_name, (CAST(rare_items AS CHAR(32) ARRAY)))`：联合多值索引(MySQL 8.0+特有)，加速按游戏名+珍稀道具的组合查询（如：查找某游戏下包含特定珍稀道具的账号）。

