import random
import datetime

surnames = [
    "Иванов", "Петров", "Сидоров", "Кузнецов", "Смирнов",
    "Попов", "Васильев", "Новиков", "Фёдоров", "Михайлов"
]

first_names = [
    "Алексей", "Дмитрий", "Максим", "Сергей", "Андрей",
    "Иван", "Михаил", "Николай", "Евгений", "Владимир"
]

patronymics = [
    "Иванович", "Петрович", "Сергеевич", "Андреевич",
    "Михайлович", "Николаевич", "Васильевич"
]

car_brands = ["Lada", "Toyota", "Hyundai", "Kia", "Renault", "Skoda", "Audi", "Bmw", "Mercedes", "Volvo", "Лада", "Газ"]

streets = [
    "Ленина", "Гагарина", "Победы", "Мира", "Советская",
    "Центральная", "Парковая", "Школьная", "Садовая", "Набережная"
]

months = ["Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"]


def generate_license(existing):
    while True:
        code = "LIC" + f"{random.randint(0, 999999):06d}"
        if code not in existing:
            existing.add(code)
            return code


def generate_full_name():
    surname = random.choice(surnames)
    first_name = random.choice(first_names)
    patronymic = random.choice(patronymics)
    return f"{surname} {first_name} {patronymic}"


def generate_address():
    street = random.choice(streets)
    house_number = random.randint(1, 200)
    building = random.choice(["", f" корпус {random.randint(1, 5)}"])
    address = f"Улица {street} {house_number}{building}"
    return " ".join(word.capitalize() if word else word for word in address.split())


def generate_cost():
    rubles = random.randint(100, 5000)
    kopecks = random.randint(0, 99)
    return f"{rubles},{kopecks:02d}"


def generate_date():
    start_date = datetime.date(2023, 1, 1)
    end_date = datetime.date(2024, 12, 31)
    delta_days = (end_date - start_date).days
    day = start_date + datetime.timedelta(days=random.randint(0, delta_days))
    return f"{day.day:02d} {months[day.month - 1]} {day.year}"


def main():
    try:
        n = int(input("Enter the number of records: "))
        if n < 1:
            raise ValueError
    except ValueError:
        print("Invalid input. Please enter a positive integer.")
        return

    licenses = set()
    with open("input.txt", "w", encoding="utf-8") as file:
        for i in range(1, n + 1):
            license_number = generate_license(licenses)
            full_name = generate_full_name()
            brand = random.choice(car_brands)
            address = generate_address()
            cost = generate_cost()
            date = generate_date()
            line = f"{license_number};{full_name};{brand};{address};{cost};{date}\n"
            file.write(line)

    print(f"File 'input.txt' successfully created with {n} records.")


if __name__ == "__main__":
    main()

