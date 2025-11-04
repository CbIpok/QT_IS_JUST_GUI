#ifndef ORDER_RECORD_HPP
#define ORDER_RECORD_HPP

#include <cctype>
#include <cstdio>
#include <sstream>
#include <string>
#include <utility>

enum class Month : int {
    January = 1,
    February,
    March,
    April,
    May,
    June,
    July,
    August,
    September,
    October,
    November,
    December
};

inline Month MonthFromNumber(int value) {
    if (value < 1) value = 1;
    if (value > 12) value = 12;
    return static_cast<Month>(value);
}

inline const char* MonthToDisplay(Month month) {
    switch (month) {
        case Month::January: return "Jan";
        case Month::February: return "Feb";
        case Month::March: return "Mar";
        case Month::April: return "Apr";
        case Month::May: return "May";
        case Month::June: return "Jun";
        case Month::July: return "Jul";
        case Month::August: return "Aug";
        case Month::September: return "Sep";
        case Month::October: return "Oct";
        case Month::November: return "Nov";
        case Month::December: return "Dec";
        default: return "???";
    }
}

inline int MonthToNumber(Month month) {
    return static_cast<int>(month);
}

struct OrderDate {
    int   year;
    Month month;
    int   day;
    bool  valid;

    OrderDate() : year(0), month(Month::January), day(0), valid(false) {}
    OrderDate(int y, Month m, int d) : year(y), month(m), day(d), valid(true) {}

    bool isValid() const { return valid; }

    std::string storageString() const {
        if (!valid) return {};
        char buffer[16];
        std::snprintf(buffer,
                      sizeof(buffer),
                      "%04d-%02d-%02d",
                      year,
                      MonthToNumber(month),
                      day);
        return std::string(buffer);
    }

    std::string displayString() const {
        if (!valid) return {};
        char buffer[32];
        std::snprintf(buffer,
                      sizeof(buffer),
                      "%02d %s %04d",
                      day,
                      MonthToDisplay(month),
                      year);
        return std::string(buffer);
    }

    std::string key() const {
        if (!valid) return {};
        char buffer[16];
        std::snprintf(buffer,
                      sizeof(buffer),
                      "%04d%02d%02d",
                      year,
                      MonthToNumber(month),
                      day);
        return std::string(buffer);
    }

    static bool parse(const std::string& text, OrderDate& out) {
        OrderDate parsed;
        std::string trimmed = text;
        while (!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.front()))) {
            trimmed.erase(trimmed.begin());
        }
        while (!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.back()))) {
            trimmed.pop_back();
        }
        if (trimmed.empty()) {
            out = OrderDate();
            out.valid = false;
            return false;
        }

        int y = 0;
        int m = 0;
        int d = 0;
        char dash1 = 0;
        char dash2 = 0;
        std::istringstream iso(trimmed);
        if ((iso >> y >> dash1 >> m >> dash2 >> d) && dash1 == '-' && dash2 == '-') {
            if (y >= 0 && d >= 1 && d <= 99 && m >= 1 && m <= 12) {
                parsed.year = y;
                parsed.month = MonthFromNumber(m);
                parsed.day = d;
                parsed.valid = true;
                out = parsed;
                return true;
            }
        }

        std::istringstream words(trimmed);
        std::string part1, part2, part3;
        if (words >> part1 >> part2 >> part3) {
            try {
                d = std::stoi(part1);
                y = std::stoi(part3);
            }
            catch (...) {
                out = OrderDate();
                out.valid = false;
                return false;
            }
            if (d < 1 || d > 99) {
                out = OrderDate();
                out.valid = false;
                return false;
            }

            for (char& ch : part2) {
                ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
            }

            const char* monthNames[] = {"jan","feb","mar","apr","may","jun","jul","aug","sep","oct","nov","dec"};
            bool found = false;
            for (int i = 0; i < 12; ++i) {
                if (part2 == monthNames[i]) {
                    parsed.month = static_cast<Month>(i + 1);
                    found = true;
                    break;
                }
            }
            if (!found) {
                out = OrderDate();
                out.valid = false;
                return false;
            }
            parsed.year = y;
            parsed.day = d;
            parsed.valid = true;
            out = parsed;
            return true;
        }

        out = OrderDate();
        out.valid = false;
        return false;
    }
};

inline bool operator==(const OrderDate& lhs, const OrderDate& rhs) {
    if (!lhs.valid && !rhs.valid) return true;
    if (lhs.valid != rhs.valid) return false;
    return lhs.year == rhs.year && lhs.month == rhs.month && lhs.day == rhs.day;
}

inline bool operator!=(const OrderDate& lhs, const OrderDate& rhs) {
    return !(lhs == rhs);
}

struct OrderRecord {
    std::string licenseNumber;
    std::string address;
    std::string cost;
    OrderDate   date;
};

inline bool operator==(const OrderRecord& lhs, const OrderRecord& rhs) {
    return lhs.licenseNumber == rhs.licenseNumber &&
           lhs.address == rhs.address &&
           lhs.cost == rhs.cost &&
           lhs.date == rhs.date;
}

inline bool operator!=(const OrderRecord& lhs, const OrderRecord& rhs) {
    return !(lhs == rhs);
}

#endif // ORDER_RECORD_HPP
