#pragma once

#include <string>

enum class Month {
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

struct Date {
    int   year;
    Month month;
    int   day;
    bool  valid;

    Date() : year(0), month(Month::January), day(0), valid(false) {}
};

bool parseDate(const std::string& text, Date& out);
int  compareDate(const Date& lhs, const Date& rhs);
std::string formatDateStorage(const Date& date);
std::string formatDateDisplay(const Date& date);
std::string dateToKey(const Date& date);

bool operator==(const Date& lhs, const Date& rhs);
bool operator!=(const Date& lhs, const Date& rhs);

Month monthFromNumber(int month);
std::string monthShortName(Month month);
