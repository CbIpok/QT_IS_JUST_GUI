#ifndef REPORT_HPP
#define REPORT_HPP

#include <string>
#include <vector>
#include "hashtable.hpp"
#include "avl_tree.h"

struct Date {
    int day;
    int month;
    int year;
};

bool parseDate(const std::string& str, Date& out);
bool dateInRange(const std::string& date, const std::string& start, const std::string& end);

std::vector<OrderRecord> generateReport(const HashTable& drivers, const AVLTree& orders,
                                        const std::string& licenseNumber,
                                        const std::string& carBrandFilter,
                                        const std::string& addressFilter,
                                        const std::string& startDate,
                                        const std::string& endDate);

#endif // REPORT_HPP
