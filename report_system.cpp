#include "report_system.hpp"
#include <fstream>
#include <sstream>

ReportSystem::ReportSystem()
    : drivers(16) {
    avl_init(&orders);
}

ReportSystem::~ReportSystem() {
    avl_free(&orders);
}

bool ReportSystem::parseDriverLine(const std::string& line, DriverRecord& rec) {
    std::stringstream ss(line);
    std::string license, fio, brand;
    if (!std::getline(ss, license, ';')) return false;
    if (!std::getline(ss, fio, ';')) return false;
    if (!std::getline(ss, brand, '\n')) return false;
    rec.licenseNumber = license;
    rec.fio = fio;
    rec.carBrand = brand;
    rec.originalLine = -1;
    return true;
}

bool ReportSystem::parseOrderLine(const std::string& line, OrderRecord& rec) {
    std::stringstream ss(line);
    if (!std::getline(ss, rec.licenseNumber, ';')) return false;
    if (!std::getline(ss, rec.address, ';')) return false;
    if (!std::getline(ss, rec.cost, ';')) return false;
    if (!std::getline(ss, rec.date)) return false;
    return true;
}

bool ReportSystem::parseDate(const std::string& str, Date& out) {
    std::stringstream ss(str);
    std::string dayStr, monStr, yearStr;
    if (!(ss >> dayStr >> monStr >> yearStr)) return false;
    int mon = monthFromString(monStr);
    if (mon == 0) return false;
    out.day = std::stoi(dayStr);
    out.month = mon;
    out.year = std::stoi(yearStr);
    return true;
}

static bool lessDate(const Date& a, const Date& b) {
    if (a.year != b.year) return a.year < b.year;
    if (a.month != b.month) return a.month < b.month;
    return a.day < b.day;
}

bool ReportSystem::inRange(const std::string& dateStr,
                           const Date& start,
                           const Date& end) {
    Date d;
    if (!parseDate(dateStr, d)) return false;
    if (lessDate(d, start)) return false;
    if (lessDate(end, d)) return false;
    return true;
}

bool ReportSystem::loadDrivers(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) return false;
    driverList.clear();
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        DriverRecord rec;
        if (parseDriverLine(line, rec)) {
            driverList.push_back(rec);
        }
    }
    size_t tableSize = driverList.size() * 2;
    if (tableSize < 1) tableSize = 1;
    drivers = HashTable(tableSize);
    for (auto it = driverList.begin(); it != driverList.end(); ++it) {
        drivers.insert(*it);
    }
    return true;
}

bool ReportSystem::loadOrders(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) return false;
    orderList.clear();
    std::string line;
    int lineNum = 0;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        OrderRecord rec;
        if (parseOrderLine(line, rec)) {
            orderList.push_back(rec);
            avl_insert(&orders, rec, ++lineNum);
        }
    }
    return true;
}

const DriverRecord* ReportSystem::findDriver(const std::string& license) const {
    return drivers.find(license);
}

DoublyLinkedList<OrderRecord> ReportSystem::ordersByLicense(const std::string& license) const {
    DoublyLinkedList<OrderRecord> res;
    collectByLicense(orders.root, license, res);
    return res;
}

DoublyLinkedList<OrderRecord> ReportSystem::filter(const std::string& carBrand,
                                                    const std::string& address,
                                                    const Date& start,
                                                    const Date& end) const {
    DoublyLinkedList<OrderRecord> res;
    collectFilter(orders.root, carBrand, address, start, end, res);
    return res;
}

bool ReportSystem::exportToFile(const std::string& filename,
                                const DoublyLinkedList<OrderRecord>& records) const {
    std::ofstream out(filename);
    if (!out) return false;
    for (auto it = records.begin(); it != records.end(); ++it) {
        const OrderRecord& r = *it;
        out << r.licenseNumber << ';' << r.address << ';'
            << r.cost << ';' << r.date << '\n';
    }
    return true;
}

void ReportSystem::collectByLicense(AVLNode* node, const std::string& license,
                                    DoublyLinkedList<OrderRecord>& out) const {
    if (!node) return;
    collectByLicense(node->left, license, out);
    if (node->key.licenseNumber == license) {
        out.push_back(node->key);
    }
    collectByLicense(node->right, license, out);
}

void ReportSystem::collectFilter(AVLNode* node, const std::string& carBrand,
                                 const std::string& address,
                                 const Date& start, const Date& end,
                                 DoublyLinkedList<OrderRecord>& out) const {
    if (!node) return;
    collectFilter(node->left, carBrand, address, start, end, out);
    const OrderRecord& ord = node->key;
    const DriverRecord* drv = findDriver(ord.licenseNumber);
    if (drv) {
        bool brandOk = carBrand.empty() || drv->carBrand == carBrand;
        bool addrOk  = address.empty() || ord.address == address;
        if (brandOk && addrOk && inRange(ord.date, start, end)) {
            out.push_back(ord);
        }
    }
    collectFilter(node->right, carBrand, address, start, end, out);
}

int ReportSystem::monthFromString(const std::string& m) {
    if (m == "jan") return 1;
    if (m == "feb") return 2;
    if (m == "mar") return 3;
    if (m == "apr") return 4;
    if (m == "may") return 5;
    if (m == "jun") return 6;
    if (m == "jul") return 7;
    if (m == "aug") return 8;
    if (m == "sep") return 9;
    if (m == "oct") return 10;
    if (m == "nov") return 11;
    if (m == "dec") return 12;
    return 0;
}

