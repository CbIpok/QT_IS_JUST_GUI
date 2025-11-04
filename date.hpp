#pragma once
#include <string>
#include <sstream>

enum class Month : int {
    Jan = 1, Feb, Mar, Apr, May, Jun,
    Jul, Aug, Sep, Oct, Nov, Dec
};

struct Date {
    int day = 0;
    Month month = Month::Jan;
    int year = 0;

    Date() = default;
    Date(int d, Month m, int y) : day(d), month(m), year(y) {}

    // Преобразование в строку "5 Nov 2025"
    std::string toString() const {
        static const char* MONTH_NAMES[12] = {
            "Jan","Feb","Mar","Apr","May","Jun",
            "Jul","Aug","Sep","Oct","Nov","Dec"
        };
        int index = static_cast<int>(month) - 1;
        const char* mName = (index >= 0 && index < 12) ? MONTH_NAMES[index] : "???";

        std::ostringstream out;
        out << day << ' ' << mName << ' ' << year;
        return out.str();
    }

    // Целое число для сравнения в дереве: YYYYMMDD
    int toInt() const {
        return year * 10000 + static_cast<int>(month) * 100 + day;
    }
};

// Вспомогательные функции, если вдруг пригодятся
inline Month monthFromNumber(int m) {
    if (m < 1) m = 1;
    if (m > 12) m = 12;
    return static_cast<Month>(m);
}

inline const char* monthShortName(Month m) {
    static const char* MONTH_NAMES[12] = {
        "Jan","Feb","Mar","Apr","May","Jun",
        "Jul","Aug","Sep","Oct","Nov","Dec"
    };
    int i = static_cast<int>(m) - 1;
    return (i >= 0 && i < 12) ? MONTH_NAMES[i] : "???";
}
