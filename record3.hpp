#ifndef RECORD_HPP
#define RECORD_HPP

#include <string>

struct Record {
    std::string fio;              // full name (key part #1)
    int         applicationNumber;// (key part #2)
    std::string street;           // payload
    long long   phoneNumber;      // payload, now 64-bit
    int         originalLine;     // line in input file (or -1 if manual)
};

#endif // RECORD_HPP
