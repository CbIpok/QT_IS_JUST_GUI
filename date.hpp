#pragma once

#include <ctime>
#include <iomanip>
#include <regex>
#include <sstream>
#include <string>

#include "string_utils.hpp"

enum class Month : int {
    Jan = 1,
    Feb,
    Mar,
    Apr,
    May,
    Jun,
    Jul,
    Aug,
    Sep,
    Oct,
    Nov,
    Dec
};

struct Date {
    int   day = 0;
    Month month = Month::Jan;
    int   year = 0;

    Date() = default;
    Date(int d, Month m, int y) : day(d), month(m), year(y) {}

    static bool parse(const std::string& text, Date& out) {
        std::string trimmed = string_utils::trim(text);
        if (trimmed.empty()) {
            return false;
        }

        static const std::regex pattern(
            R"(^(0[1-9]|[12][0-9]|3[01]) (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) ([0-9]{4})$)");
        std::smatch match;
        if (!std::regex_match(trimmed, match, pattern)) {
            return false;
        }

        int dayValue = std::stoi(match[1].str());
        int yearValue = std::stoi(match[3].str());
        Month monthValue;
        if (!parseMonth(match[2].str(), monthValue)) {
            return false;
        }

        out = Date(dayValue, monthValue, yearValue);
        return out.isValid();
    }

    std::string displayString() const {
        std::ostringstream out;
        out << std::setfill('0') << std::setw(2) << day << ' ' << monthName(month) << ' '
            << std::setw(4) << year;
        return out.str();
    }

    std::string storageString() const {
        return displayString();
    }

    std::string key() const {
        std::ostringstream out;
        out << std::setfill('0') << std::setw(4) << year << std::setw(2)
            << static_cast<int>(month) << std::setw(2) << day;
        return out.str();
    }

    std::string toString() const { return displayString(); }

    bool isValid() const {
        int monthValue = static_cast<int>(month);
        if (monthValue < 1 || monthValue > 12) return false;
        if (year < 0) return false;

        int maxDay = maxDaysInMonth(month, year);
        if (day < 1 || day > maxDay) return false;

        Date today = currentSystemDate();
        if (year > today.year) return false;
        if (year == today.year) {
            int todayMonthValue = static_cast<int>(today.month);
            if (monthValue > todayMonthValue) return false;
            if (monthValue == todayMonthValue && day > today.day) return false;
        }

        return true;
    }

private:
    static bool isLeapYear(int year) {
        if (year % 400 == 0) return true;
        if (year % 100 == 0) return false;
        return year % 4 == 0;
    }

    static int maxDaysInMonth(Month month, int year) {
        static const int DAYS_IN_MONTH[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        int index = static_cast<int>(month) - 1;
        if (index < 0 || index >= 12) {
            return 31;
        }

        if (month == Month::Feb && isLeapYear(year)) {
            return 29;
        }

        return DAYS_IN_MONTH[index];
    }

    static Date currentSystemDate() {
        std::time_t now = std::time(nullptr);
        std::tm* local = std::localtime(&now);
        if (!local) {
            return Date();
        }
        int currentYear = local->tm_year + 1900;
        Month currentMonth = static_cast<Month>(local->tm_mon + 1);
        int currentDay = local->tm_mday;
        return Date(currentDay, currentMonth, currentYear);
    }

    static const char* monthName(Month month) {
        static const char* MONTH_NAMES[12] = {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun",
            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        int index = static_cast<int>(month) - 1;
        if (index < 0 || index >= 12) {
            return "???";
        }
        return MONTH_NAMES[index];
    }

    static bool parseMonth(const std::string& text, Month& month) {
        static const char* MONTH_NAMES[12] = {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun",
            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        for (int i = 0; i < 12; ++i) {
            if (text == MONTH_NAMES[i]) {
                month = static_cast<Month>(i + 1);
                return true;
            }
        }
        return false;
    }
};

inline bool operator==(const Date& lhs, const Date& rhs) {
    return lhs.day == rhs.day && lhs.month == rhs.month && lhs.year == rhs.year;
}
inline bool operator!=(const Date& lhs, const Date& rhs) {
    return !(lhs == rhs);
}
