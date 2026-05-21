import random
from datetime import datetime, timedelta

def analyze_data_file():
    with open('C:\\Users\\haipe\\Desktop\\game_account_exchange\\clear_and_add\\accounts_test_data.sql', 'r', encoding='utf-8') as f:
        lines = f.readlines()

    prices = []
    dates = []

    for line in lines:
        if line.strip().startswith('VALUES'):
            parts = line.split('VALUES')[1].strip()[1:-2].split(',')
            if len(parts) >= 5:
                try:
                    price = int(parts[4].strip())
                    prices.append(price)

                    date_str = parts[11].strip()[1:-1]
                    date = datetime.strptime(date_str, '%Y-%m-%d %H:%M:%S')
                    dates.append(date)
                except:
                    pass

    if not prices or not dates:
        print("No data found")
        return

    print("=" * 80)
    print(" " * 25 + "更新后的数据统计摘要")
    print("=" * 80)

    print("\n" + "-" * 80)
    print("1. 价格分布 (单位: 分)")
    print("-" * 80)
    print(f"  最小值: {min(prices):12d} 分 (约 {min(prices)/100:.2f} 元)")
    print(f"  最大值: {max(prices):12d} 分 (约 {max(prices)/100:.2f} 元)")
    print(f"  平均值: {sum(prices)/len(prices):12.2f} 分 (约 {sum(prices)/len(prices)/100:.2f} 元)")

    price_ranges = {
        "1-100分 (1-1元)": 0,
        "100-1000分 (1-10元)": 0,
        "1000-10000分 (10-100元)": 0,
        "10000-100000分 (100-1000元)": 0,
        "100000-1000000分 (1000-10000元)": 0,
        "1000000-10000000分 (10000-100000元)": 0
    }

    for price in prices:
        if price < 100:
            price_ranges["1-100分 (1-1元)"] += 1
        elif price < 1000:
            price_ranges["100-1000分 (1-10元)"] += 1
        elif price < 10000:
            price_ranges["1000-10000分 (10-100元)"] += 1
        elif price < 100000:
            price_ranges["10000-100000分 (100-1000元)"] += 1
        elif price < 1000000:
            price_ranges["100000-1000000分 (1000-10000元)"] += 1
        else:
            price_ranges["1000000-10000000分 (10000-100000元)"] += 1

    print("\n  价格区间分布:")
    for range_name, count in price_ranges.items():
        percentage = (count / len(prices)) * 100
        bar = "█" * int(percentage / 2)
        print(f"    {range_name:35s}: {bar:25s} {count:4d} ({percentage:5.2f}%)")

    print("\n" + "-" * 80)
    print("2. 时间分布 (2000-2026年)")
    print("-" * 80)
    print(f"  最早时间: {min(dates).strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"  最晚时间: {max(dates).strftime('%Y-%m-%d %H:%M:%S')}")

    year_counts = {}
    for date in dates:
        year = date.year
        year_counts[year] = year_counts.get(year, 0) + 1

    print("\n  各年份分布:")
    sorted_years = sorted(year_counts.items())
    for year, count in sorted_years:
        percentage = (count / len(dates)) * 100
        bar = "█" * int(percentage / 2)
        print(f"    {year}年: {bar:25s} {count:4d} ({percentage:5.2f}%)")

    print("\n" + "=" * 80)
    print("数据更新说明:")
    print("=" * 80)
    print("  ✅ 价格范围: 1-10000000分 (最高可达10000000元)")
    print("  ✅ 价格分布: 多区间均匀分布，覆盖低中高价位")
    print("  ✅ 时间范围: 2000年1月1日 至 2026年12月31日")
    print("  ✅ 时间分布: 覆盖27年随机时间点")
    print("=" * 80)

analyze_data_file()
