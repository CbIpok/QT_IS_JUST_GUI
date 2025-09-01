#ifndef DRIVER_RECORD_HPP
#define DRIVER_RECORD_HPP

#include <string>

struct DriverRecord {
    std::string licenseNumber; // unique driver license number (key)
    std::string fio;           // driver's full name
    std::string carBrand;      // vehicle brand
    int         originalLine;  // line in input file (or -1 if manual)
};

#endif // DRIVER_RECORD_HPP
