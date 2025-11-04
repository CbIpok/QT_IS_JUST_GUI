#include "date_utils.hpp"

#include <array>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace {

constexpr std::array<const char*, 12> MONTH_NAMES{{
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"}};

bool tryParseWithFormat(const std::string& text, const char* format, Date& out) {
    std::tm tm{};
    std::istringstream stream(text);
    stream >> std::get_time(&tm, format);
    if (stream.fail()) {
        return false;
    }
    int year = tm.tm_year + 1900;
    int month = tm.tm_mon + 1;
    int day = tm.tm_mday;
    if (month < 1 || month > 12 || day < 1) {
        return false;
    }
    out.year = year;
    out.month = monthFromNumber(month);
    out.day = day;
    out.valid = true;
    return true;
}

bool parseDigits(const std::string& text, Date& out) {
    std::string digits;
    digits.reserve(text.size());
    for (char ch : text) {
        if (std::isdigit(static_cast<unsigned char>(ch))) {
            digits.push_back(ch);
        }
    }
    if (digits.size() < 8) {
        return false;
    }
    int year = 0;
    int month = 0;
    int day = 0;
    for (int i = 0; i < 4; ++i) {
        year = year * 10 + (digits[i] - '0');
    }
    month = (digits[4] - '0') * 10 + (digits[5] - '0');
    day = (digits[6] - '0') * 10 + (digits[7] - '0');
    if (month < 1 || month > 12 || day < 1) {
        return false;
    }
    out.year = year;
    out.month = monthFromNumber(month);
    out.day = day;
    out.valid = true;
    return true;
}

bool parsePositiveInt(const std::string& text, int& value) {
    if (text.empty()) {
        return false;
    }
    int result = 0;
    for (char ch : text) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
        result = result * 10 + (ch - '0');
    }
    value = result;
    return true;
}

bool monthFromText(const std::string& token, Month& outMonth) {
    if (token.empty()) {
        return false;
    }
    std::string key;
    key.reserve(token.size());
    for (char ch : token) {
        if (!std::isalpha(static_cast<unsigned char>(ch))) {
            continue;
        }
        key.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
    }
    if (key.size() < 3) {
        return false;
    }
    key.resize(3);
    constexpr std::array<const char*, 12> MONTH_KEYS{{
        "jan", "feb", "mar", "apr", "may", "jun",
        "jul", "aug", "sep", "oct", "nov", "dec"}};
    for (std::size_t index = 0; index < MONTH_KEYS.size(); ++index) {
        if (key == MONTH_KEYS[index]) {
            outMonth = static_cast<Month>(index + 1);
            return true;
        }
    }
    return false;
}

bool parseDayMonthYear(const std::string& text, Date& out) {
    std::string normalized;
    normalized.reserve(text.size());
    for (char ch : text) {
        if (std::isalpha(static_cast<unsigned char>(ch))) {
            normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        } else if (std::isdigit(static_cast<unsigned char>(ch))) {
            normalized.push_back(ch);
        } else {
            normalized.push_back(' ');
        }
    }

    std::istringstream stream(normalized);
    std::string dayToken;
    std::string monthToken;
    std::string yearToken;
    if (!(stream >> dayToken >> monthToken >> yearToken)) {
        return false;
    }

    int day = 0;
    int year = 0;
    if (!parsePositiveInt(dayToken, day) || !parsePositiveInt(yearToken, year)) {
        return false;
    }
    if (day < 1) {
        return false;
    }

    Month month{};
    if (!monthFromText(monthToken, month)) {
        return false;
    }

    out.year = year;
    out.month = month;
    out.day = day;
    out.valid = true;
    return true;
}

} // namespace

Month monthFromNumber(int month) {
    if (month < 1) {
        month = 1;
    }
    if (month > 12) {
        month = 12;
    }
    return static_cast<Month>(month);
}

std::string monthShortName(Month month) {
    int index = static_cast<int>(month) - 1;
    if (index < 0 || index >= static_cast<int>(MONTH_NAMES.size())) {
        return "???";
    }
    return MONTH_NAMES[static_cast<std::size_t>(index)];
}

bool parseDate(const std::string& text, Date& out) {
    out = Date();
    if (text.empty()) {
        return false;
    }
    if (tryParseWithFormat(text, "%Y-%m-%d", out)) {
        return true;
    }
    if (tryParseWithFormat(text, "%Y-%b-%d", out)) {
        return true;
    }
    if (tryParseWithFormat(text, "%d %b %Y", out)) {
        return true;
    }
    if (tryParseWithFormat(text, "%d-%b-%Y", out)) {
        return true;
    }
    if (parseDayMonthYear(text, out)) {
        return true;
    }
    return parseDigits(text, out);
}

int compareDate(const Date& lhs, const Date& rhs) {
    if (!lhs.valid && !rhs.valid) {
        return 0;
    }
    if (!lhs.valid) {
        return -1;
    }
    if (!rhs.valid) {
        return 1;
    }
    if (lhs.year != rhs.year) {
        return lhs.year < rhs.year ? -1 : 1;
    }
    int lhsMonth = static_cast<int>(lhs.month);
    int rhsMonth = static_cast<int>(rhs.month);
    if (lhsMonth != rhsMonth) {
        return lhsMonth < rhsMonth ? -1 : 1;
    }
    if (lhs.day != rhs.day) {
        return lhs.day < rhs.day ? -1 : 1;
    }
    return 0;
}

std::string formatDateStorage(const Date& date) {
    if (!date.valid) {
        return {};
    }
    std::ostringstream out;
    out << std::setw(4) << std::setfill('0') << date.year << '-'
        << std::setw(2) << std::setfill('0') << static_cast<int>(date.month) << '-'
        << std::setw(2) << std::setfill('0') << date.day;
    return out.str();
}

std::string formatDateDisplay(const Date& date) {
    if (!date.valid) {
        return {};
    }
    std::ostringstream out;
    out << std::setw(4) << std::setfill('0') << date.year << '-'
        << monthShortName(date.month) << '-'
        << std::setw(2) << std::setfill('0') << date.day;
    return out.str();
}

std::string dateToKey(const Date& date) {
    if (!date.valid) {
        return {};
    }
    std::ostringstream out;
    out << std::setw(4) << std::setfill('0') << date.year
        << std::setw(2) << std::setfill('0') << static_cast<int>(date.month)
        << std::setw(2) << std::setfill('0') << date.day;
    return out.str();
}

bool operator==(const Date& lhs, const Date& rhs) {
    if (!lhs.valid && !rhs.valid) {
        return true;
    }
    return lhs.valid == rhs.valid && lhs.year == rhs.year && lhs.month == rhs.month && lhs.day == rhs.day;
}

bool operator!=(const Date& lhs, const Date& rhs) {
    return !(lhs == rhs);
}
