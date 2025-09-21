#ifndef ORDER_RECORD_HPP
#define ORDER_RECORD_HPP

#include <string>

struct OrderRecord {
    std::string licenseNumber;
    std::string address;
    std::string cost;
    std::string date;
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
