#pragma once
#include <string>

struct Record {
    std::string lastName;
    std::string firstName;
    std::string patronymic;
    std::string street;
    long        phoneNumber;
    int         applicationNumber;
    int         originalLine;
};
