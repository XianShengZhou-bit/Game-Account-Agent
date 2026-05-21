import random
import json

games = [
    "三界竞锋", "幻域奇旅", "绝境突围", "纪元战盟", "铁血突袭",
    "沧澜劫影", "方寸秘境", "星途穹轨", "黎明前哨", "玄灵秘录",
    "诡域秘闻", "萌趣乐园", "星境先锋", "雷霆竞技", "异界勇士",
    "异世幻境", "星焰少女", "极速星途", "玄境特攻", "亘古环域",
    "灵兽幻域", "永恒战魂", "火线突击", "逐光秘境", "星界幻塔",
    "忍界风云", "魔法遗迹", "森屿物语", "江湖剑韵", "星弈联盟",
    "荒野突围", "零界秘境", "沧蓝契约", "群雄逐域", "狂兽乱斗",
    "幻世仙途", "潮声纪元", "宿命星舟", "纪元神域", "寒江梦录",
    "彩涂乱斗", "圣域奇谭", "霓虹都市", "星穹瞳影", "破晓战姬",
    "战地号令", "天穹守卫", "泰拉神域", "苍穹谕言", "寰宇崛起",
    "浮生江湖"
]

rares = [
    "龙狙", "火麒麟", "青龙偃月刀", "朱雀羽扇", "白虎护符",
    "玄武甲胄", "金色传说", "史诗神器", "传说皮肤", "限定坐骑",
    "稀有宠物", "绝版时装", "神秘宝箱", "远古遗物", "至尊称号",
    "灭世魔剑", "幽冥神爪", "破晓神弓", "暗影披风", "圣光战甲",
    "冰霜之怒", "雷霆之锤", "烈焰法杖", "虚空宝珠", "时间沙漏",
    "噬魂剑", "追魂枪", "斩妖刀", "破天斧", "摄魂铃",
    "迷魂笛", "噬魂幡", "缚妖索", "镇妖塔", "地狱业火珠",
    "天堂圣光剑", "幽冥鬼火莲", "星辰碎心石", "凤凰涅槃羽", "麒麟踏云靴",
    "青龙逆鳞甲", "朱雀焚天扇", "玄武镇海盾", "白虎裂地爪", "幽冥黄泉之刃",
    "时空裂隙之眼", "星辰陨落之泪", "凤凰浴火之羽", "魔龙噬魂之牙", "天使救赎之翼",
    "恶魔堕落之角", "精灵祝福之环", "亡灵诅咒之骨", "神明赐福之冠"
]

servers = [
    "雾里偷闲", "云端漫步", "星河璀璨", "月影婆娑", "清风徐来",
    "烟雨朦胧", "星辰大海", "繁花似锦", "月光倾城", "世外桃源",
    "九霄云外", "蓬莱仙境", "瑶池圣地", "玲珑秘境", "飘渺仙山",
    "逍遥谷", "百花谷", "清风峡", "明月湾", "碧水潭",
    "紫竹林", "青云峰", "落霞岭", "断魂崖", "绝情谷",
    "断肠崖", "冰火岛", "幽灵岛", "桃花岛", "恶人谷"
]

titles = [
    "极品账号出售", "高等级成品号", "皮肤多多超值号", "退游诚心出", "肝帝心血号",
    "欧皇附体号", "全英雄解锁", "稀有道具齐全", "养老号便宜出", "潜力号低价甩",
    "土豪专属号", "神级账号出售", "满皮肤账号", "绝版道具号", "氪金大佬号",
    "肝王之王号", "竞技场霸主号", "排行榜前几名", "全图鉴账号", "限定皮肤多多",
    "史诗英雄齐全", "传说品质账号", "万氪大佬号", "满星英雄号", "稀有坐骑号",
    "绝版宠物号", "极品神器号", "无敌战神号", "巅峰王者号", "神话级账号"
]

stats = {
    "total_records": 1000,
    "game_distribution": {},
    "server_distribution": {},
    "title_distribution": {},
    "price_range": {"min": None, "max": None, "avg": 0},
    "account_level_range": {"min": None, "max": None, "avg": 0},
    "hero_count_range": {"min": None, "max": None, "avg": 0},
    "skin_count_range": {"min": None, "max": None, "avg": 0},
    "rare_items_distribution": {0: 0, 1: 0, 2: 0, 3: 0, 4: 0, 5: 0},
    "status_distribution": {0: 0, 1: 0, 2: 0},
    "rare_items_popularity": {}
}

total_price = 0
total_level = 0
total_hero = 0
total_skin = 0

for i in range(1000):
    game = random.choice(games)
    server = random.choice(servers)
    title = random.choice(titles)
    price = random.randint(1000, 100000)
    account_level = random.randint(1, 100)
    hero_count = random.randint(1, 200)
    skin_count = random.randint(0, 500)

    rare_count = random.randint(0, 5)
    selected_rares = random.sample(rares, rare_count) if rare_count > 0 else []

    status_choices = [0, 0, 0, 0, 0, 0, 0, 0, 1, 2]
    status = random.choice(status_choices)

    stats["game_distribution"][game] = stats["game_distribution"].get(game, 0) + 1
    stats["server_distribution"][server] = stats["server_distribution"].get(server, 0) + 1
    stats["title_distribution"][title] = stats["title_distribution"].get(title, 0) + 1

    if stats["price_range"]["min"] is None or price < stats["price_range"]["min"]:
        stats["price_range"]["min"] = price
    if stats["price_range"]["max"] is None or price > stats["price_range"]["max"]:
        stats["price_range"]["max"] = price
    total_price += price

    if stats["account_level_range"]["min"] is None or account_level < stats["account_level_range"]["min"]:
        stats["account_level_range"]["min"] = account_level
    if stats["account_level_range"]["max"] is None or account_level > stats["account_level_range"]["max"]:
        stats["account_level_range"]["max"] = account_level
    total_level += account_level

    if stats["hero_count_range"]["min"] is None or hero_count < stats["hero_count_range"]["min"]:
        stats["hero_count_range"]["min"] = hero_count
    if stats["hero_count_range"]["max"] is None or hero_count > stats["hero_count_range"]["max"]:
        stats["hero_count_range"]["max"] = hero_count
    total_hero += hero_count

    if stats["skin_count_range"]["min"] is None or skin_count < stats["skin_count_range"]["min"]:
        stats["skin_count_range"]["min"] = skin_count
    if stats["skin_count_range"]["max"] is None or skin_count > stats["skin_count_range"]["max"]:
        stats["skin_count_range"]["max"] = skin_count
    total_skin += skin_count

    stats["rare_items_distribution"][rare_count] += 1
    stats["status_distribution"][status] += 1

    for rare in selected_rares:
        stats["rare_items_popularity"][rare] = stats["rare_items_popularity"].get(rare, 0) + 1

stats["price_range"]["avg"] = total_price / 1000
stats["account_level_range"]["avg"] = total_level / 1000
stats["hero_count_range"]["avg"] = total_hero / 1000
stats["skin_count_range"]["avg"] = total_skin / 1000

print("=" * 80)
print(" " * 25 + "测试数据统计摘要")
print("=" * 80)
print(f"\n总记录数: {stats['total_records']} 条\n")

print("-" * 80)
print("1. 游戏分布 (随机抽样)")
print("-" * 80)
sorted_games = sorted(stats["game_distribution"].items(), key=lambda x: x[1], reverse=True)[:10]
for game, count in sorted_games:
    percentage = (count / 1000) * 100
    bar = "█" * int(percentage / 2)
    print(f"{game:12s}: {bar:25s} {count:4d} ({percentage:5.2f}%)")

print("\n" + "-" * 80)
print("2. 区服分布 (随机抽样)")
print("-" * 80)
sorted_servers = sorted(stats["server_distribution"].items(), key=lambda x: x[1], reverse=True)[:10]
for server, count in sorted_servers:
    percentage = (count / 1000) * 100
    bar = "█" * int(percentage / 2)
    print(f"{server:12s}: {bar:25s} {count:4d} ({percentage:5.2f}%)")

print("\n" + "-" * 80)
print("3. 标题分布 (随机抽样)")
print("-" * 80)
sorted_titles = sorted(stats["title_distribution"].items(), key=lambda x: x[1], reverse=True)[:10]
for title, count in sorted_titles:
    percentage = (count / 1000) * 100
    bar = "█" * int(percentage / 2)
    print(f"{title:12s}: {bar:25s} {count:4d} ({percentage:5.2f}%)")

print("\n" + "-" * 80)
print("4. 价格分布 (单位: 分)")
print("-" * 80)
print(f"  最小值: {stats['price_range']['min']:6d} 分 (约 {stats['price_range']['min']/100:.2f} 元)")
print(f"  最大值: {stats['price_range']['max']:6d} 分 (约 {stats['price_range']['max']/100:.2f} 元)")
print(f"  平均值: {stats['price_range']['avg']:6.2f} 分 (约 {stats['price_range']['avg']/100:.2f} 元)")

print("\n" + "-" * 80)
print("5. 账号等级分布")
print("-" * 80)
print(f"  最小值: {stats['account_level_range']['min']:3d} 级")
print(f"  最大值: {stats['account_level_range']['max']:3d} 级")
print(f"  平均值: {stats['account_level_range']['avg']:6.2f} 级")

print("\n" + "-" * 80)
print("6. 英雄/角色数量分布")
print("-" * 80)
print(f"  最小值: {stats['hero_count_range']['min']:3d} 个")
print(f"  最大值: {stats['hero_count_range']['max']:3d} 个")
print(f"  平均值: {stats['hero_count_range']['avg']:6.2f} 个")

print("\n" + "-" * 80)
print("7. 皮肤数量分布")
print("-" * 80)
print(f"  最小值: {stats['skin_count_range']['min']:3d} 个")
print(f"  最大值: {stats['skin_count_range']['max']:3d} 个")
print(f"  平均值: {stats['skin_count_range']['avg']:6.2f} 个")

print("\n" + "-" * 80)
print("8. 珍稀道具数量分布")
print("-" * 80)
for count, num in stats["rare_items_distribution"].items():
    percentage = (num / 1000) * 100
    bar = "█" * int(percentage / 2)
    print(f"  {count}个道具: {bar:25s} {num:4d} 条 ({percentage:5.2f}%)")

print("\n" + "-" * 80)
print("9. 账号状态分布")
print("-" * 80)
status_names = {0: "在售", 1: "交易中", 2: "已售出"}
for status, count in stats["status_distribution"].items():
    percentage = (count / 1000) * 100
    bar = "█" * int(percentage / 2)
    print(f"  {status_names[status]:6s} (status={status}): {bar:25s} {count:4d} ({percentage:5.2f}%)")

print("\n" + "-" * 80)
print("10. 珍稀道具热门排行榜 (TOP 15)")
print("-" * 80)
sorted_rares = sorted(stats["rare_items_popularity"].items(), key=lambda x: x[1], reverse=True)[:15]
for rare, count in sorted_rares:
    percentage = (count / 1000) * 100
    bar = "█" * int(percentage / 2)
    print(f"  {rare:12s}: {bar:25s} {count:4d} 次 ({percentage:5.2f}%)")

print("\n" + "=" * 80)
print("数据生成规则:")
print("=" * 80)
print("  • accounts_id: 16位UUID (大写字母和数字)")
print("  • game_name: 从51个游戏中随机选择")
print("  • server_area: 从30个区服中随机选择")
print("  • title: 从30个标题中随机选择")
print("  • price: 1000-100000分 (约10-1000元)")
print("  • account_level: 1-100级")
print("  • hero_count: 1-200个")
print("  • skin_count: 0-500个")
print("  • rare_items: 0-5个珍稀道具 (JSON数组)")
print("  • status: 0-在售(80%), 1-交易中(10%), 2-已售出(10%)")
print("  • version: 初始为0")
print("  • created_at: 2025年5月1日-31日随机时间")
print("=" * 80)
