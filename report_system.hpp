#ifndef REPORT_SYSTEM_HPP
#define REPORT_SYSTEM_HPP

#include <string>
#include "hashtable.hpp"
#include "avl_tree.h"
#include "DoublyLinkedList.hpp"

struct Date {
    int day;
    int month;
    int year;
};

class ReportSystem {
public:
    ReportSystem();
    ~ReportSystem();

    bool loadDrivers(const std::string& filename);
    bool loadOrders(const std::string& filename);

    const DriverRecord* findDriver(const std::string& license) const;
    DoublyLinkedList<OrderRecord> ordersByLicense(const std::string& license) const;

    DoublyLinkedList<OrderRecord> filter(const std::string& carBrand,
                                         const std::string& address,
                                         const Date& start,
                                         const Date& end) const;

    bool exportToFile(const std::string& filename,
                      const DoublyLinkedList<OrderRecord>& records) const;

private:
    HashTable drivers;
    AVLTree   orders;
    DoublyLinkedList<DriverRecord> driverList;
    DoublyLinkedList<OrderRecord>  orderList;

    static bool parseDriverLine(const std::string& line, DriverRecord& rec);
    static bool parseOrderLine(const std::string& line, OrderRecord& rec);
    static bool parseDate(const std::string& str, Date& out);
    static bool inRange(const std::string& dateStr,
                        const Date& start,
                        const Date& end);

    void collectByLicense(AVLNode* node, const std::string& license,
                          DoublyLinkedList<OrderRecord>& out) const;
    void collectFilter(AVLNode* node, const std::string& carBrand,
                       const std::string& address,
                       const Date& start, const Date& end,
                       DoublyLinkedList<OrderRecord>& out) const;
    static int monthFromString(const std::string& m);
};

#endif // REPORT_SYSTEM_HPP

