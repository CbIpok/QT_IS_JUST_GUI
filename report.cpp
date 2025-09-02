#include "report.hpp"
#include <sstream>
#include <tuple>
#include <algorithm>
#include <cctype>

static int monthFromStr(const std::string& m) {
    static const char* months[] = {"jan","feb","mar","apr","may","jun","jul","aug","sep","oct","nov","dec"};
    std::string lower; lower.resize(m.size());
    std::transform(m.begin(), m.end(), lower.begin(), [](unsigned char c){ return std::tolower(c); });
    for (int i = 0; i < 12; ++i) {
        if (lower == months[i]) return i + 1;
    }
    return -1;
}

bool parseDate(const std::string& str, Date& out) {
    std::istringstream is(str);
    std::string monthStr;
    if (!(is >> out.day >> monthStr >> out.year)) return false;
    out.month = monthFromStr(monthStr);
    return out.month != -1;
}

static std::tuple<int,int,int> toTuple(const Date& d) {
    return std::make_tuple(d.year, d.month, d.day);
}

bool dateInRange(const std::string& date, const std::string& start, const std::string& end) {
    Date d;
    if (!parseDate(date, d)) return false;
    if (!start.empty()) {
        Date s; if (!parseDate(start, s)) return false;
        if (toTuple(s) > toTuple(d)) return false;
    }
    if (!end.empty()) {
        Date e; if (!parseDate(end, e)) return false;
        if (toTuple(e) < toTuple(d)) return false;
    }
    return true;
}

std::vector<OrderRecord> generateReport(const HashTable& drivers, const AVLTree& orders,
                                        const std::string& licenseNumber,
                                        const std::string& carBrandFilter,
                                        const std::string& addressFilter,
                                        const std::string& startDate,
                                        const std::string& endDate) {
    std::vector<OrderRecord> result;
    DriverRecord driver;
    if (!drivers.getByLicense(licenseNumber, driver)) {
        return result;
    }
    if (!carBrandFilter.empty() && driver.carBrand != carBrandFilter) {
        return result;
    }
    auto nodes = avl_inorder_nodes(&orders);
    for (auto node : nodes) {
        const OrderRecord& ord = node->key;
        if (ord.licenseNumber != licenseNumber) continue;
        if (!addressFilter.empty() && ord.address != addressFilter) continue;
        if (!dateInRange(ord.date, startDate, endDate)) continue;
        result.push_back(ord);
    }
    return result;
}
