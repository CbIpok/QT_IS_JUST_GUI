#ifndef RECORD_HPP
#define RECORD_HPP

#include <string>

struct Record {
    std::string licenseNumber; // unique driver license number (key)
    std::string fio;           // driver's full name
    std::string carBrand;      // vehicle brand
    int         originalLine;  // line in input file (or -1 if manual)
};

#endif // RECORD_HPP
