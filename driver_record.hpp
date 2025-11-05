#ifndef DRIVER_RECORD_HPP
#define DRIVER_RECORD_HPP

#include <string>

struct DriverRecord {
    std::string licenseNumber; // unique driver license number (key)
    std::string fio;           // driver's full name
    std::string carBrand;      // vehicle brand
};

inline bool operator==(const DriverRecord& lhs, const DriverRecord& rhs) {
    return lhs.licenseNumber == rhs.licenseNumber &&
           lhs.fio == rhs.fio &&
           lhs.carBrand == rhs.carBrand;
}

inline bool operator!=(const DriverRecord& lhs, const DriverRecord& rhs) {
    return !(lhs == rhs);
}

#endif // DRIVER_RECORD_HPP
