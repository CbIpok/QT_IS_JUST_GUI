#pragma once

#include <iomanip>
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

        Date parsed{};
        if (parseIso(trimmed, parsed) || parseText(trimmed, parsed)) {
            out = parsed;
            return true;
        }
        return false;
    }

    std::string displayString() const {
        std::ostringstream out;
        out << std::setfill('0') << std::setw(2) << day << ' ' << monthName(month) << ' '
            << std::setw(4) << year;
        return out.str();
    }

    std::string storageString() const {
        std::ostringstream out;
        out << std::setfill('0') << std::setw(4) << year << '-' << std::setw(2)
            << static_cast<int>(month) << '-' << std::setw(2) << day;
        return out.str();
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
        if (day < 1 || day > 31) return false;
        if (year < 0) return false;
        return true;
    }

private:
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

    static bool parseIso(const std::string& text, Date& out) {
        std::size_t firstDash = text.find('-');
        if (firstDash == std::string::npos) return false;
        std::size_t secondDash = text.find('-', firstDash + 1);
        if (secondDash == std::string::npos) return false;

        std::string yearPart = text.substr(0, firstDash);
        std::string monthPart = text.substr(firstDash + 1, secondDash - firstDash - 1);
        std::string dayPart = text.substr(secondDash + 1);

        int yearValue = 0;
        int monthValue = 0;
        int dayValue = 0;
        if (!string_utils::parseInteger(yearPart, yearValue)) return false;
        if (!string_utils::parseInteger(monthPart, monthValue)) return false;
        if (!string_utils::parseInteger(dayPart, dayValue)) return false;

        if (monthValue < 1 || monthValue > 12) return false;
        if (dayValue < 1 || dayValue > 31) return false;

        out = Date(dayValue, static_cast<Month>(monthValue), yearValue);
        return true;
    }

    static bool parseText(const std::string& text, Date& out) {
        std::istringstream input(text);
        std::string      dayPart;
        std::string      monthPart;
        std::string      yearPart;
        if (!(input >> dayPart >> monthPart >> yearPart)) {
            return false;
        }

        int dayValue = 0;
        int yearValue = 0;
        if (!string_utils::parseInteger(dayPart, dayValue)) return false;
        if (dayValue < 1 || dayValue > 31) return false;
        if (!string_utils::parseInteger(yearPart, yearValue)) return false;

        Month monthValue;
        if (!parseMonth(monthPart, monthValue)) return false;

        out = Date(dayValue, monthValue, yearValue);
        return true;
    }

    static bool parseMonth(const std::string& text, Month& month) {
        static const char* MONTH_NAMES[12] = {
            "jan", "feb", "mar", "apr", "may", "jun",
            "jul", "aug", "sep", "oct", "nov", "dec"};
        std::string lower = string_utils::toLower(text);
        for (int i = 0; i < 12; ++i) {
            if (lower == MONTH_NAMES[i]) {
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
