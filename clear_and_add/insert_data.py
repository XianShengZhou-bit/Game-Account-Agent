import pymysql

sql_file_path = r'C:\Users\haipe\Desktop\game_account_exchange\clear_and_add\accounts_test_data.sql'

db_config = {
    'host': 'localhost',
    'port': 3306,
    'user': 'root',
    'password': 'xianzun123$',
    'database': 'game_account_exchange',
    'charset': 'utf8mb4'
}

try:
    connection = pymysql.connect(**db_config)
    cursor = connection.cursor()

    print("Connected to database successfully!")

    with open(sql_file_path, 'r', encoding='utf-8') as file:
        sql_content = file.read()

    sql_commands = sql_content.split(';')

    total_commands = 0
    for i, command in enumerate(sql_commands):
        command = command.strip()
        if command and not command.startswith('USE'):
            if command.startswith('INSERT'):
                cursor.execute(command)
                total_commands += 1
                if total_commands % 100 == 0:
                    print(f"Inserted {total_commands} records...")

    connection.commit()
    print(f"\nSuccessfully inserted {total_commands} records!")

    cursor.execute("SELECT COUNT(*) FROM accounts")
    count = cursor.fetchone()[0]
    print(f"Total records in accounts table: {count}")

    cursor.close()
    connection.close()
    print("Database connection closed.")

except Exception as e:
    print(f"Error: {e}")
