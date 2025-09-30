#include "data_integrator.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>
#include <iomanip>

namespace {

void trimCarriageReturn(std::string& value) {
    if (!value.empty() && value.back() == '\r') {
        value.pop_back();
    }
}

bool splitLine(const std::string& line, char delimiter, std::string* fields, std::size_t expectedCount) {
    std::size_t fieldIndex = 0;
    std::size_t start = 0;
    std::size_t length = line.size();
    for (std::size_t i = 0; i <= length; ++i) {
        bool isDelimiter = (i < length && line[i] == delimiter);
        bool isEnd = (i == length);
        if (!isDelimiter && !isEnd) {
            continue;
        }
        if (fieldIndex >= expectedCount) {
            return false;
        }
        fields[fieldIndex] = line.substr(start, i - start);
        ++fieldIndex;
        start = i + 1;
    }
    return fieldIndex == expectedCount;
}

bool parseDriverLine(const std::string& line, DriverRecord& out) {
    std::string parts[4];
    if (!splitLine(line, '|', parts, 4)) return false;
    for (std::size_t i = 0; i < 4; ++i) trimCarriageReturn(parts[i]);

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

    out.licenseNumber = parts[0];
    out.fio = parts[1];
    out.carBrand = parts[2];
    out.originalLine = originalLine;
    return true;
}

bool parseOrderLine(const std::string& line, OrderRecord& out) {
    std::string parts[4];
    if (!splitLine(line, '|', parts, 4)) return false;
    for (std::size_t i = 0; i < 4; ++i) trimCarriageReturn(parts[i]);

    if (parts[0].empty()) return false;

    out.licenseNumber = parts[0];
    out.address = parts[1];
    out.cost = parts[2];
    out.date = parts[3];
    return true;
}

void appendOrderNodeDetailed(const AVLNode*                     node,
                             const DoublyLinkedList<OrderRecord>& orders,
                             std::ostringstream&                 out,
                             const std::string&                  prefix,
                             bool                                isTail,
                             bool                                isRoot) {
    if (!node) {
        return;
    }

    out << prefix;
    if (!isRoot) {
        out << (isTail ? "`--" : "|--");
    }
    out << node->license << '\n';

    std::string childPrefix = prefix;
    if (!isRoot) {
        childPrefix += (isTail ? "    " : "|   ");
    }

    if (node->listIndices.empty()) {
        out << childPrefix << "• Заказов нет\n";
    }
    else {
        for (std::list<std::size_t>::const_iterator it = node->listIndices.begin();
             it != node->listIndices.end();
             ++it) {
            std::size_t listIndex = *it;
            out << childPrefix << "• ";
            if (listIndex < orders.size()) {
                const OrderRecord& order = orders.at(listIndex);
                out << order.address << " | " << order.cost << " | " << order.date;
            }
            else {
                out << "(недопустимый индекс " << listIndex << ")";
            }
            out << '\n';
        }
    }

    std::vector<const AVLNode*> children;
    if (node->left) {
        children.push_back(node->left);
    }
    if (node->right) {
        children.push_back(node->right);
    }

    for (std::size_t i = 0; i < children.size(); ++i) {
        bool childIsTail = (i + 1 == children.size());
        appendOrderNodeDetailed(children[i], orders, out, childPrefix, childIsTail, false);
    }
}

std::string normalizeDateKey(const std::string& value) {
    if (value.empty()) {
        return {};
    }

    std::tm parsed{};
    std::istringstream iso(value);
    iso >> std::get_time(&parsed, "%Y-%m-%d");
    if (!iso.fail()) {
        char buffer[32];
        std::snprintf(buffer,
                      sizeof(buffer),
                      "%04d%02d%02d",
                      parsed.tm_year + 1900,
                      parsed.tm_mon + 1,
                      parsed.tm_mday);
        return std::string(buffer);
    }

    std::string digits;
    digits.reserve(value.size());
    for (char ch : value) {
        if (std::isdigit(static_cast<unsigned char>(ch))) {
            digits.push_back(ch);
        }
    }
    if (digits.size() >= 8) {
        if (digits.size() > 8) {
            digits = digits.substr(digits.size() - 8);
        }
        return digits;
    }
    return value;
}

bool isDateWithinRange(const std::string& value,
                       const std::string& from,
                       const std::string& to) {
    if (from.empty() && to.empty()) {
        return true;
    }

    std::string valueKey = normalizeDateKey(value);
    std::string fromKey = normalizeDateKey(from);
    std::string toKey = normalizeDateKey(to);

    if (!from.empty()) {
        if (fromKey.empty()) {
            if (value < from) return false;
        }
        else {
            if (valueKey < fromKey) return false;
        }
    }

    if (!to.empty()) {
        if (toKey.empty()) {
            if (value > to) return false;
        }
        else {
            if (valueKey > toKey) return false;
        }
    }

    return true;
}

} // namespace

DataIntegrator::DataIntegrator(std::size_t driverTableInitialSize, double maxLoadFactor)
    : drivers_(),
      orders_(),
      driverTable_(std::max<std::size_t>(1u, driverTableInitialSize), maxLoadFactor),
      orderTree_{},
      driverTableReady_(false),
      orderTreeReady_(false),
      defaultDriverTableSize_(std::max<std::size_t>(1u, driverTableInitialSize)),
      driverTableMaxLoadFactor_(maxLoadFactor) {
    avl_init(&orderTree_);
}

bool DataIntegrator::createDriverTable(std::size_t initialSize) {
    std::size_t size = std::max<std::size_t>(1u, initialSize);
    driverTable_ = HashTable(size, driverTableMaxLoadFactor_);
    defaultDriverTableSize_ = size;
    drivers_.clear();
    driverTableReady_ = true;
    return true;
}

bool DataIntegrator::createOrderTree() {
    avl_free(&orderTree_);
    avl_init(&orderTree_);
    orders_.clear();
    orderTreeReady_ = true;
    return true;
}

void DataIntegrator::clearDriverTable() {
    drivers_.clear();
    driverTable_ = HashTable(defaultDriverTableSize_, driverTableMaxLoadFactor_);
    driverTableReady_ = false;
}

void DataIntegrator::clearOrderTree() {
    orders_.clear();
    avl_free(&orderTree_);
    avl_init(&orderTree_);
    orderTreeReady_ = false;
}

bool DataIntegrator::addDriver(const DriverRecord& record) {
    if (!driverTableReady_) return false;
    if (!validateDriverRecord(record)) return false;
    if (driverTable_.contains(record.licenseNumber)) return false;

    std::size_t index = drivers_.push_back(record);
    if (!driverTable_.insert(record.licenseNumber, index)) {
        DoublyLinkedList<DriverRecord>::SwapRemoveResult cleanup;
        drivers_.remove_by_index(index, cleanup);
        return false;
    }
    return true;
}

bool DataIntegrator::removeDriver(const DriverRecord& record) {
    if (!driverTableReady_) return false;
    std::optional<std::size_t> driverIdxOpt = findDriverIndex(record);
    if (!driverIdxOpt.has_value()) return false;
    std::size_t driverIdx = driverIdxOpt.value();
    std::string licenseNumber = record.licenseNumber;

    if (orderTreeReady_) {
        for (std::size_t i = orders_.size(); i > 0; --i) {
        std::size_t orderIdx = i - 1;
        OrderRecord order = orders_.at(orderIdx);
        if (order.licenseNumber == licenseNumber) {
            removeOrderByIndex(order.licenseNumber, orderIdx);
        }
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

bool DataIntegrator::removeDriver(const std::string& licenseNumber) {
    std::optional<DriverRecord> stored = findDriver(licenseNumber);
    if (!stored.has_value()) return false;
    return removeDriver(stored.value());
}

bool DataIntegrator::updateDriver(const DriverRecord& current, const DriverRecord& updated) {
    if (!driverTableReady_) return false;
    if (!validateDriverRecord(updated)) return false;
    if (current.licenseNumber != updated.licenseNumber) return false;

    std::optional<std::size_t> driverIdxOpt = findDriverIndex(current);
    if (!driverIdxOpt.has_value()) return false;

    DriverRecord& stored = drivers_.at(driverIdxOpt.value());
    stored = updated;
    return true;
}

bool DataIntegrator::updateDriver(const std::string& licenseNumber, const DriverRecord& updated) {
    std::optional<DriverRecord> stored = findDriver(licenseNumber);
    if (!stored.has_value()) return false;
    return updateDriver(stored.value(), updated);
}


bool DataIntegrator::hasDriver(const std::string& licenseNumber) const {
    if (!driverTableReady_) return false;
    return driverTable_.contains(licenseNumber);
}

std::optional<DriverRecord> DataIntegrator::findDriver(const DriverRecord& record) const {
    if (!driverTableReady_) return std::nullopt;
    std::optional<std::size_t> driverIdxOpt = findDriverIndex(record);
    if (!driverIdxOpt.has_value()) return std::nullopt;
    return drivers_.at(driverIdxOpt.value());
}

std::optional<DriverRecord> DataIntegrator::findDriver(const std::string& licenseNumber) const {
    if (!driverTableReady_) return std::nullopt;
    std::optional<std::size_t> driverIdxOpt = findDriverIndex(licenseNumber);
    if (!driverIdxOpt.has_value()) return std::nullopt;
    return drivers_.at(driverIdxOpt.value());
}

bool DataIntegrator::addOrder(const OrderRecord& record) {
    if (!driverTableReady_ || !orderTreeReady_) return false;
    if (!validateOrderRecord(record)) return false;
    if (!driverTable_.contains(record.licenseNumber)) return false;

    std::size_t index = orders_.push_back(record);
    avl_insert(&orderTree_, record.licenseNumber, index);
    return true;
}

bool DataIntegrator::removeOrder(const OrderRecord& record) {
    if (!orderTreeReady_) return false;
    std::optional<std::size_t> idxOpt = findOrderIndex(record, &record);
    if (!idxOpt.has_value()) return false;
    removeOrderByIndex(record.licenseNumber, idxOpt.value());
    return true;
}

bool DataIntegrator::updateOrder(const OrderRecord& current, const OrderRecord& updated) {
    if (!orderTreeReady_ || !driverTableReady_) return false;
    std::optional<std::size_t> idxOpt = findOrderIndex(current, &current);
    if (!idxOpt.has_value()) return false;

    if (!validateOrderRecord(updated)) return false;
    if (!driverTable_.contains(updated.licenseNumber)) return false;

    std::size_t idx = idxOpt.value();
    bool licenseChanged = current.licenseNumber != updated.licenseNumber;

    if (licenseChanged) {
        avl_remove_index(&orderTree_, current.licenseNumber, idx);
        avl_insert(&orderTree_, updated.licenseNumber, idx);
    }

    orders_.at(idx) = updated;
    return true;
}

bool DataIntegrator::hasOrder(const OrderRecord& record) const {
    if (!orderTreeReady_) return false;
    std::optional<std::size_t> idxOpt = findOrderIndex(record, &record);
    return idxOpt.has_value();
}

std::vector<OrderRecord> DataIntegrator::ordersForDriver(const std::string& licenseNumber) const {
    if (!orderTreeReady_) return {};
    std::vector<OrderRecord> result;
    orders_.for_each([&](const OrderRecord& order, std::size_t) {
        if (order.licenseNumber == licenseNumber) {
            result.push_back(order);
        }
    });
    return result;
}

void DataIntegrator::clear() {
    clearDriverTable();
    clearOrderTree();
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

    DoublyLinkedList<DriverRecord> parsedDrivers;
    for (std::size_t i = 0; i < driverCount; ++i) {
        if (!std::getline(input, line)) return false;
        DriverRecord record{};
        if (!parseDriverLine(line, record)) return false;
        parsedDrivers.push_back(record);
    }

    long long orderCountRaw = 0;
    if (!(input >> header >> orderCountRaw)) return false;
    if (header != "orders" || orderCountRaw < 0) return false;
    std::size_t orderCount = static_cast<std::size_t>(orderCountRaw);
    std::getline(input, line);

    DoublyLinkedList<OrderRecord> parsedOrders;
    for (std::size_t i = 0; i < orderCount; ++i) {
        if (!std::getline(input, line)) return false;
        OrderRecord record{};
        if (!parseOrderLine(line, record)) return false;
        parsedOrders.push_back(record);
    }

    DataIntegrator temp(driverTable_.capacity(), driverTableMaxLoadFactor_);
    temp.createDriverTable(driverTable_.capacity());
    temp.createOrderTree();

    bool driversLoaded = true;
    parsedDrivers.for_each([&temp, &driversLoaded](const DriverRecord& driver, std::size_t) {
        if (!driversLoaded) {
            return;
        }
        if (!temp.addDriver(driver)) {
            driversLoaded = false;
        }
    });
    if (!driversLoaded) {
        return false;
    }

    bool ordersLoaded = true;
    parsedOrders.for_each([&temp, &ordersLoaded](const OrderRecord& order, std::size_t) {
        if (!ordersLoaded) {
            return;
        }
        if (!temp.addOrder(order)) {
            ordersLoaded = false;
        }
    });
    if (!ordersLoaded) {
        return false;
    }

    clear();
    drivers_ = std::move(temp.drivers_);
    orders_ = std::move(temp.orders_);
    driverTable_ = std::move(temp.driverTable_);
    orderTree_ = temp.orderTree_;
    driverTableReady_ = temp.driverTableReady_;
    orderTreeReady_ = temp.orderTreeReady_;
    defaultDriverTableSize_ = temp.defaultDriverTableSize_;
    driverTableMaxLoadFactor_ = temp.driverTableMaxLoadFactor_;
    temp.orderTree_.root = nullptr;
    temp.orderTreeReady_ = false;
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
    if (!driverTableReady_) {
        return {};
    }
    std::ostringstream out;
    out << "Водителей: " << drivers_.size()
        << " | Вместимость таблицы: " << driverTable_.capacity()
        << " | Записей: " << driverTable_.size() << '\n';
    out << "----------------------------------------\n";

    std::vector<HashTable::Entry> entries = driverTable_.entries();
    if (entries.empty()) {
        out << "(пусто)\n";
        return out.str();
    }

    for (const HashTable::Entry& entry : entries) {
        if (entry.index >= drivers_.size()) {
            continue;
        }
        const DriverRecord& driver = drivers_.at(entry.index);
        std::size_t        orderCount = 0;
        orders_.for_each([&](const OrderRecord& order, std::size_t) {
            if (order.licenseNumber == driver.licenseNumber) {
                ++orderCount;
            }
        });

        out << '[' << entry.slot << "] " << driver.licenseNumber
            << " | ФИО: " << driver.fio
            << " | Авто: " << driver.carBrand
            << " | Строка: " << driver.originalLine
            << " | Заказов: " << orderCount << '\n';
    }

    return out.str();
}

std::string DataIntegrator::orderTreeAsText() const {
    if (!orderTreeReady_) {
        return {};
    }
    if (!orderTree_.root) {
        std::ostringstream empty;
        empty << "Заказов нет\n";
        return empty.str();
    }

    std::ostringstream out;
    out << "Всего заказов: " << orders_.size() << '\n';
    out << "----------------------------------------\n";
    appendOrderNodeDetailed(orderTree_.root, orders_, out, "", true, true);
    return out.str();
}

bool DataIntegrator::saveStructures(const std::string& hashTablePath, const std::string& treePath) const {
    if (!hashTablePath.empty()) {
        std::ofstream hashOut(hashTablePath);
        if (!hashOut.is_open()) {
            return false;
        }
        std::string hashText = hashTableAsText();
        if (hashText.empty()) {
            hashOut << "Структура хеш-таблицы не создана\n";
        }
        else {
            hashOut << hashText;
        }
        if (!hashOut) {
            return false;
        }
    }

    if (!treePath.empty()) {
        std::ofstream treeOut(treePath);
        if (!treeOut.is_open()) {
            return false;
        }
        std::string treeText = orderTreeAsText();
        if (treeText.empty()) {
            treeOut << "Структура дерева заказов не создана\n";
        }
        else {
            treeOut << treeText;
        }
        if (!treeOut) {
            return false;
        }
    }

    return true;
}

std::optional<std::size_t> DataIntegrator::findDriverIndex(const std::string& licenseNumber) const {
    if (!driverTableReady_) return std::nullopt;
    std::size_t index = 0;
    int steps = 0;
    if (!driverTable_.search(licenseNumber, index, steps)) return std::nullopt;
    return index;
}

std::optional<std::size_t> DataIntegrator::findDriverIndex(const DriverRecord& record) const {
    std::optional<std::size_t> indexOpt = findDriverIndex(record.licenseNumber);
    if (!indexOpt.has_value()) return std::nullopt;
    const DriverRecord& stored = drivers_.at(indexOpt.value());
    if (stored == record) {
        return indexOpt;
    }
    return std::nullopt;
}

std::optional<std::size_t> DataIntegrator::findOrderIndex(const OrderRecord& key, const OrderRecord* match) const {
    if (!orderTreeReady_) return std::nullopt;
    const AVLNode* node = avl_search(&orderTree_, key.licenseNumber);
    if (!node) return std::nullopt;
    if (!match) {
        if (node->listIndices.empty()) return std::nullopt;
        return node->listIndices.front();
    }

    std::list<std::size_t>::const_iterator it = node->listIndices.begin();
    while (it != node->listIndices.end()) {
        std::size_t idx = *it;
        const OrderRecord& stored = orders_.at(idx);
        if (stored.licenseNumber == match->licenseNumber &&
            stored.address == match->address &&
            stored.cost == match->cost &&
            stored.date == match->date) {
            return idx;
        }
        ++it;
    }
    return std::nullopt;
}

void DataIntegrator::removeOrderByIndex(const std::string& licenseNumber, std::size_t index) {
    if (!orderTreeReady_) return;
    DoublyLinkedList<OrderRecord>::SwapRemoveResult result;
    if (!orders_.remove_by_index(index, result)) return;

    avl_remove_index(&orderTree_, licenseNumber, index);

    if (result.swapped && orders_.size() > index) {
        OrderRecord& movedOrder = orders_.at(index);
        avl_replace_index(&orderTree_, movedOrder.licenseNumber, result.swappedFromIndex, index);
    }
}

bool DataIntegrator::validateDriverRecord(const DriverRecord& record) const {
    if (record.licenseNumber.empty()) return false;
    if (record.fio.empty()) return false;
    if (record.carBrand.empty()) return false;
    if (record.originalLine < -1) return false;
    return true;
}

bool DataIntegrator::validateOrderRecord(const OrderRecord& record) const {
    if (record.licenseNumber.empty()) return false;
    if (record.address.empty()) return false;
    if (record.cost.empty()) return false;
    if (record.date.empty()) return false;
    return true;
}

std::vector<ReportEntry> DataIntegrator::generateReport(const std::string& licenseNumber,
                                                        const std::string& carBrand,
                                                        const std::string& address,
                                                        const std::string& dateFrom,
                                                        const std::string& dateTo) const {
    std::vector<ReportEntry> result;
    if (!driverTableReady_ || !orderTreeReady_) {
        return result;
    }
    if (licenseNumber.empty() || carBrand.empty() || address.empty()) {
        return result;
    }

    std::optional<DriverRecord> driverOpt = findDriver(licenseNumber);
    if (!driverOpt.has_value()) {
        return result;
    }
    const DriverRecord& driver = driverOpt.value();
    if (driver.carBrand != carBrand) {
        return result;
    }

    auto driverOrders = ordersForDriver(licenseNumber);
    for (const OrderRecord& order : driverOrders) {
        if (order.address != address) {
            continue;
        }
        if (!isDateWithinRange(order.date, dateFrom, dateTo)) {
            continue;
        }
        result.push_back(ReportEntry{driver.licenseNumber,
                                     driver.fio,
                                     driver.carBrand,
                                     order.address,
                                     order.cost,
                                     order.date});
    }

    return result;
}

std::string DataIntegrator::formatReport(const std::vector<ReportEntry>& entries) const {
    if (entries.empty()) {
        return "Совпадений не найдено.\n";
    }

    std::ostringstream out;
    out << "Номер лицензии | ФИО | Марка автомобиля | Адрес | Цена | Дата\n";
    out << "--------------------------------------------------------------------------\n";
    for (const ReportEntry& entry : entries) {
        out << entry.licenseNumber << " | "
            << entry.fio << " | "
            << entry.carBrand << " | "
            << entry.address << " | "
            << entry.cost << " | "
            << entry.date << '\n';
    }
    return out.str();
}



