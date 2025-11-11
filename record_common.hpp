#pragma once

#include <string>

struct Record {
    std::string licenseNumber;   // unique driver identifier
    std::string fio;             // full name
    std::string carBrand;        // car manufacturer name
    std::string pickupAddress;   // pickup location
    int         costKopecks;     // cost stored in kopecks
    std::string date;            // request date in format DD Mon YYYY
    int         originalLine;    // source line number (or -1 if manual)
};

