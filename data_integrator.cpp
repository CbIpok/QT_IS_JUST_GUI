#include "data_integrator.hpp"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

void trimCarriageReturn(std::string& value) {
    if (!value.empty() && value.back() == '\r') {
        value.pop_back();
    }
}

std::vector<std::string> splitLine(const std::string& line, char delimiter) {
    std::vector<std::string> result;
    std::string current;
    for (char ch : line) {
        if (ch == delimiter) {
            result.push_back(current);
            current.clear();
        }
        else {
            current.push_back(ch);
        }
    }
    result.push_back(current);
    return result;
}

bool parseDriverLine(const std::string& line, DriverRecord& out) {
    auto parts = splitLine(line, '|');
    if (parts.size() != 4) return false;
    for (auto& part : parts) trimCarriageReturn(part);

    std::size_t consumed = 0;
    int originalLine = 0;
    try {
        originalLine = std::stoi(parts[3], &consumed);
    }
    catch (...) {
        return false;
    }
    if (consumed != parts[3].size()) return false;

    if (parts[0].empty()) return false;

    out.licenseNumber = std::move(parts[0]);
    out.fio = std::move(parts[1]);
    out.carBrand = std::move(parts[2]);
    out.originalLine = originalLine;
    return true;
}

bool parseOrderLine(const std::string& line, OrderRecord& out) {
    auto parts = splitLine(line, '|');
    if (parts.size() != 4) return false;
    for (auto& part : parts) trimCarriageReturn(part);

    if (parts[0].empty()) return false;

    out.licenseNumber = std::move(parts[0]);
    out.address = std::move(parts[1]);
    out.cost = std::move(parts[2]);
    out.date = std::move(parts[3]);
    return true;
}

} // namespace

DataIntegrator::DataIntegrator(std::size_t driverTableInitialSize, double maxLoadFactor)
    : drivers_(),
      orders_(),
      driverTable_(driverTableInitialSize, maxLoadFactor),
      orderTree_{} {
    avl_init(&orderTree_);
}

bool DataIntegrator::addDriver(const DriverRecord& record) {
    if (driverTable_.contains(record.licenseNumber)) return false;

    std::size_t index = drivers_.push_back(record);
    if (!driverTable_.insert(record.licenseNumber, index)) {
        DoublyLinkedList<DriverRecord>::SwapRemoveResult cleanup;
        drivers_.remove_by_index(index, cleanup);
        return false;
    }
    return true;
}

bool DataIntegrator::removeDriver(const std::string& licenseNumber) {
    auto driverIdxOpt = findDriverIndex(licenseNumber);
    if (!driverIdxOpt) return false;
    std::size_t driverIdx = *driverIdxOpt;

    for (std::size_t i = orders_.size(); i > 0; --i) {
        std::size_t orderIdx = i - 1;
        OrderRecord order = orders_.at(orderIdx);
        if (order.licenseNumber == licenseNumber) {
            removeOrderByIndex(order, orderIdx);
        }
    }

    DoublyLinkedList<DriverRecord>::SwapRemoveResult result;
    drivers_.remove_by_index(driverIdx, result);

    std::size_t removedIndex = 0;
    driverTable_.remove(licenseNumber, removedIndex);

    if (result.swapped && drivers_.size() > driverIdx) {
        DriverRecord& movedDriver = drivers_.at(driverIdx);
        driverTable_.update_index(movedDriver.licenseNumber, driverIdx);
    }
    return true;
}

bool DataIntegrator::updateDriver(const std::string& licenseNumber, const DriverRecord& updated) {
    auto driverIdxOpt = findDriverIndex(licenseNumber);
    if (!driverIdxOpt) return false;
    if (updated.licenseNumber != licenseNumber) return false;

    DriverRecord& stored = drivers_.at(*driverIdxOpt);
    stored = updated;
    return true;
}


bool DataIntegrator::hasDriver(const std::string& licenseNumber) const {
    return driverTable_.contains(licenseNumber);
}

std::optional<DriverRecord> DataIntegrator::findDriver(const std::string& licenseNumber) const {
    auto driverIdxOpt = findDriverIndex(licenseNumber);
    if (!driverIdxOpt) return std::nullopt;
    return drivers_.at(*driverIdxOpt);
}

bool DataIntegrator::addOrder(const OrderRecord& record) {
    if (!driverTable_.contains(record.licenseNumber)) return false;

    std::size_t index = orders_.push_back(record);
    avl_insert(&orderTree_, record, index);
    return true;
}

bool DataIntegrator::removeOrder(const OrderRecord& record) {
    auto idxOpt = findOrderIndex(record, &record);
    if (!idxOpt) return false;
    removeOrderByIndex(record, *idxOpt);
    return true;
}

bool DataIntegrator::updateOrder(const OrderRecord& current, const OrderRecord& updated) {
    auto idxOpt = findOrderIndex(current, &current);
    if (!idxOpt) return false;

    if (!driverTable_.contains(updated.licenseNumber)) return false;

    std::size_t idx = *idxOpt;
    bool keyChanged = current.licenseNumber != updated.licenseNumber ||
                      current.address != updated.address;

    if (keyChanged) {
        avl_remove_index(&orderTree_, current, idx);
        avl_insert(&orderTree_, updated, idx);
    }

    orders_.at(idx) = updated;
    return true;
}

bool DataIntegrator::hasOrder(const OrderRecord& record) const {
    return findOrderIndex(record, &record).has_value();
}

std::vector<OrderRecord> DataIntegrator::ordersForDriver(const std::string& licenseNumber) const {
    std::vector<OrderRecord> result;
    orders_.for_each([&](const OrderRecord& order, std::size_t) {
        if (order.licenseNumber == licenseNumber) {
            result.push_back(order);
        }
    });
    return result;
}

void DataIntegrator::clear() {
    drivers_.clear();
    orders_.clear();
    driverTable_.clear();
    avl_free(&orderTree_);
    avl_init(&orderTree_);
}

bool DataIntegrator::loadFromFile(const std::string& path) {
    std::ifstream input(path);
    if (!input.is_open()) return false;

    std::string header;
    long long driverCountRaw = 0;
    if (!(input >> header >> driverCountRaw)) return false;
    if (header != "drivers" || driverCountRaw < 0) return false;
    std::size_t driverCount = static_cast<std::size_t>(driverCountRaw);

    std::string line;
    std::getline(input, line); // consume the rest of the header line

    std::vector<DriverRecord> parsedDrivers;
    parsedDrivers.reserve(driverCount);
    std::set<std::string> licenseNumbers;

    for (std::size_t i = 0; i < driverCount; ++i) {
        if (!std::getline(input, line)) return false;
        DriverRecord record{};
        if (!parseDriverLine(line, record)) return false;
        if (!licenseNumbers.insert(record.licenseNumber).second) return false;
        parsedDrivers.push_back(std::move(record));
    }

    long long orderCountRaw = 0;
    if (!(input >> header >> orderCountRaw)) return false;
    if (header != "orders" || orderCountRaw < 0) return false;
    std::size_t orderCount = static_cast<std::size_t>(orderCountRaw);
    std::getline(input, line);

    std::vector<OrderRecord> parsedOrders;
    parsedOrders.reserve(orderCount);

    for (std::size_t i = 0; i < orderCount; ++i) {
        if (!std::getline(input, line)) return false;
        OrderRecord record{};
        if (!parseOrderLine(line, record)) return false;
        if (licenseNumbers.find(record.licenseNumber) == licenseNumbers.end()) return false;
        parsedOrders.push_back(std::move(record));
    }

    clear();

    for (const auto& driver : parsedDrivers) {
        if (!addDriver(driver)) {
            clear();
            return false;
        }
    }

    for (const auto& order : parsedOrders) {
        if (!addOrder(order)) {
            clear();
            return false;
        }
    }

    return true;
}

bool DataIntegrator::saveToFile(const std::string& path) const {
    std::ofstream output(path);
    if (!output.is_open()) return false;

    output << "drivers " << drivers_.size() << '\n';
    drivers_.for_each([&](const DriverRecord& driver, std::size_t) {
        output << driver.licenseNumber << '|' << driver.fio << '|' << driver.carBrand << '|' << driver.originalLine << '\n';
    });

    output << "orders " << orders_.size() << '\n';
    orders_.for_each([&](const OrderRecord& order, std::size_t) {
        output << order.licenseNumber << '|' << order.address << '|' << order.cost << '|' << order.date << '\n';
    });

    output.flush();
    return static_cast<bool>(output);
}

std::string DataIntegrator::hashTableAsText() const {
    return driverTable_.toString();
}

std::string DataIntegrator::orderTreeAsText() const {
    return avl_tree_to_string(&orderTree_);
}

bool DataIntegrator::saveStructures(const std::string& hashTablePath, const std::string& treePath) const {
    if (!hashTablePath.empty()) {
        std::ofstream hashOut(hashTablePath);
        if (!hashOut.is_open()) {
            return false;
        }
        hashOut << hashTableAsText();
        if (!hashOut) {
            return false;
        }
    }

    if (!treePath.empty()) {
        std::ofstream treeOut(treePath);
        if (!treeOut.is_open()) {
            return false;
        }
        treeOut << orderTreeAsText();
        if (!treeOut) {
            return false;
        }
    }

    return true;
}

std::optional<std::size_t> DataIntegrator::findDriverIndex(const std::string& licenseNumber) const {
    std::size_t index = 0;
    int steps = 0;
    if (!driverTable_.search(licenseNumber, index, steps)) return std::nullopt;
    return index;
}

std::optional<std::size_t> DataIntegrator::findOrderIndex(const OrderRecord& key, const OrderRecord* match) const {
    const AVLNode* node = avl_search(&orderTree_, key);
    if (!node) return std::nullopt;
    if (!match) {
        if (node->listIndices.empty()) return std::nullopt;
        return node->listIndices.front();
    }

    for (std::size_t idx : node->listIndices) {
        const OrderRecord& stored = orders_.at(idx);
        if (stored.licenseNumber == match->licenseNumber &&
            stored.address == match->address &&
            stored.cost == match->cost &&
            stored.date == match->date) {
            return idx;
        }
    }
    return std::nullopt;
}

void DataIntegrator::removeOrderByIndex(const OrderRecord& key, std::size_t index) {
    DoublyLinkedList<OrderRecord>::SwapRemoveResult result;
    if (!orders_.remove_by_index(index, result)) return;

    avl_remove_index(&orderTree_, key, index);

    if (result.swapped && orders_.size() > index) {
        OrderRecord& movedOrder = orders_.at(index);
        avl_replace_index(&orderTree_, movedOrder, result.swappedFromIndex, index);
    }
}



