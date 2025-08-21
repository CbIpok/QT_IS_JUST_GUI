import random

# Sample lists of Russian surnames, first names, and patronymics
surnames = [
    "Ivanov", "Petrov", "Sidorov", "Kuznetsov", "Smirnov",
    "Popov", "Vasilyev", "Novikov", "Fyodorov", "Mikhailov"
]

first_names = [
    "Alexander", "Dmitry", "Maxim", "Sergey", "Andrey",
    "Ivan", "Mikhail", "Nikolay", "Evgeny", "Vladimir"
]

patronymics = [
    "Ivanovich", "Petrovich", "Sergeyevich", "Andreevich",
    "Mikhailovich", "Nikolaevich", "Vasilyevich"
]

cities = ["Moscow", "Saint Petersburg", "Novosibirsk", "Yekaterinburg", "Kazan"]
streets = ["Lenin", "Pushkin", "Gagarin", "Mir", "Soviet"]

def generate_full_name():
    # Составляем фамилию из трёх случайных без пробелов, имя и отчество
    surname = "".join(random.sample(surnames, 3))
    first_name = random.choice(first_names)
    patronymic = "".join(random.sample(patronymics, 2))
    return f"{surname} {first_name} {patronymic}"

def generate_address():
    city = random.choice(cities)
    street = random.choice(streets)
    house_number = random.randint(1, 200)
    return f"{city}, {street} St., No. {house_number}"

def generate_phone_number():
    return str(random.randint(1_000_000_000, 9_999_999_999))

def generate_request_id():
    # Генерация случайного номера заявки (можно ограничить диапазон)
    return random.randint(1, 100000)  # Здесь можно выбрать диапазон для request_id

def main():
    try:
        n = int(input("Enter the number of records: "))
        if n < 1:
            raise ValueError
    except ValueError:
        print("Invalid input. Please enter a positive integer.")
        return

    with open("input.txt", "w", encoding="utf-8") as file:
        for i in range(1, n + 1):
            full_name = generate_full_name()
            address = generate_address()
            phone = generate_phone_number()
            request_id = generate_request_id()  # Генерируем случайный номер заявки
            # Записываем четыре поля, разделённые точкой-с-запятой
            line = f"{full_name};{address};{phone};{request_id}\n"
            file.write(line)

    print(f"File 'input.txt' successfully created with {n} records.")

if __name__ == "__main__":
    main()
