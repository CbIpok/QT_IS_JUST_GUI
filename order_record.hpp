#ifndef ORDER_RECORD_HPP
#define ORDER_RECORD_HPP

#include <string>

struct OrderRecord {
    std::string licenseNumber;
    std::string address;
    std::string cost;
    std::string date;
};

#endif // ORDER_RECORD_HPP
