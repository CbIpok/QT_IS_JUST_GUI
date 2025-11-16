#ifndef ORDER_RECORD_HPP
#define ORDER_RECORD_HPP

#include <string>

#include "date.hpp"
#include "string_utils.hpp"

struct OrderRecord {
    std::string licenseNumber;
    std::string address;
    double      cost;
    Date        date;

    std::string toString() const {
        return licenseNumber + "|" + address + "|" + string_utils::formatCost(cost) + "|" + date.storageString();
    }
};

inline bool operator==(const OrderRecord& lhs, const OrderRecord& rhs) {
    return lhs.licenseNumber == rhs.licenseNumber && lhs.address == rhs.address &&
           lhs.cost == rhs.cost && lhs.date == rhs.date;
}

inline bool operator!=(const OrderRecord& lhs, const OrderRecord& rhs) {
    return !(lhs == rhs);
}

#endif // ORDER_RECORD_HPP
