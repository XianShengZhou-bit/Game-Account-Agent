import random
import uuid
import json
from datetime import datetime, timedelta

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

def generate_insert_statements(count=1000):
    statements = []
    start_date = datetime(2000, 1, 1, 0, 0, 0)
    end_date = datetime(2026, 12, 31, 23, 59, 59)
    total_seconds = int((end_date - start_date).total_seconds())

    price_segments = [
        (1, 100, 0.05),
        (100, 1000, 0.15),
        (1000, 10000, 0.30),
        (10000, 100000, 0.25),
        (100000, 1000000, 0.15),
        (1000000, 10000000, 0.10)
    ]

    def generate_price():
        rand = random.random()
        cumulative = 0
        for min_price, max_price, probability in price_segments:
            cumulative += probability
            if rand <= cumulative:
                return random.randint(min_price, max_price)
        return random.randint(1, 100)

    for i in range(1, count + 1):
        accounts_id = str(uuid.uuid4()).replace('-', '')[:16].upper()
        game_name = random.choice(games)
        server_area = random.choice(servers)
        title = random.choice(titles)
        price = generate_price()
        account_level = random.randint(1, 100)
        hero_count = random.randint(1, 200)
        skin_count = random.randint(0, 500)

        rare_count = random.randint(0, 5)
        selected_rares = random.sample(rares, rare_count) if rare_count > 0 else []
        rare_items_json = json.dumps(selected_rares, ensure_ascii=False)

        status_choices = [0, 0, 0, 0, 0, 0, 0, 0, 1, 2]
        status = random.choice(status_choices)

        version = 0

        random_seconds = random.randint(0, total_seconds)
        created_at = start_date + timedelta(seconds=random_seconds)
        created_at_str = created_at.strftime('%Y-%m-%d %H:%M:%S')

        statement = f"INSERT INTO accounts (accounts_id, game_name, server_area, title, price, account_level, hero_count, skin_count, rare_items, status, version, created_at) VALUES ('{accounts_id}', '{game_name}', '{server_area}', '{title}', {price}, {account_level}, {hero_count}, {skin_count}, '{rare_items_json}', {status}, {version}, '{created_at_str}');"

        statements.append(statement)

    return statements

sql_content = "USE game_account_exchange;\n\n"

statements = generate_insert_statements(1000)
sql_content += "\n".join(statements)

with open('C:\\Users\\haipe\\Desktop\\game_account_exchange\\clear_and_add\\accounts_test_data.sql', 'w', encoding='utf-8') as f:
    f.write(sql_content)

print(f"Generated 1000 INSERT statements")
print(f"File saved to: C:\\Users\\haipe\\Desktop\\game_account_exchange\\clear_and_add\\accounts_test_data.sql")
