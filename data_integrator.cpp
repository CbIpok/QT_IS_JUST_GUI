#include "data_integrator.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

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
    Date parsed{};
    if (!Date::parse(parts[3], parsed)) {
        return false;
    }
    out.date = parsed;
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
        node->listIndices.for_each([&](std::size_t listIndex, std::size_t) {
            out << childPrefix << "• ";
            if (listIndex < orders.size()) {
                const OrderRecord& order = orders.at(listIndex);
                out << order.address << " | " << order.cost << " | " << order.date.displayString();
            }
            else {
                out << "(недопустимый индекс " << listIndex << ")";
            }
            out << '\n';
        });
    }

    if (node->left) {
        appendOrderNodeDetailed(node->left, orders, out, childPrefix, node->right == nullptr, false);
    }
    if (node->right) {
        appendOrderNodeDetailed(node->right, orders, out, childPrefix, true, false);
    }
}

void appendDateNodeDetailed(const AVLNode*                     node,
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

    std::string label = node->license;
    if (label.size() == 8) {
        std::string iso = label.substr(0, 4) + "-" + label.substr(4, 2) + "-" + label.substr(6, 2);
        Date parsed{};
        if (Date::parse(iso, parsed)) {
            label = parsed.displayString();
        }
    }
    out << label << '\n';

    std::string childPrefix = prefix;
    if (!isRoot) {
        childPrefix += (isTail ? "    " : "|   ");
    }

    if (node->listIndices.empty()) {
        out << childPrefix << "• Заказов нет\n";
    }
    else {
        node->listIndices.for_each([&](std::size_t listIndex, std::size_t) {
            out << childPrefix << "• ";
            if (listIndex < orders.size()) {
                const OrderRecord& order = orders.at(listIndex);
                out << order.licenseNumber << " | " << order.address << " | " << order.cost;
            }
            else {
                out << "(недопустимый индекс " << listIndex << ")";
            }
            out << '\n';
        });
    }

    if (node->left) {
        appendDateNodeDetailed(node->left, orders, out, childPrefix, node->right == nullptr, false);
    }
    if (node->right) {
        appendDateNodeDetailed(node->right, orders, out, childPrefix, true, false);
    }
}

} // namespace

DataIntegrator::DataIntegrator(std::size_t driverTableInitialSize, double maxLoadFactor)
    : drivers_(),
      orders_(),
      driverTable_(std::max<std::size_t>(1u, driverTableInitialSize), maxLoadFactor),
      orderTree_{},
      orderDateTree_{},
      driverTableReady_(false),
      orderTreeReady_(false),
      defaultDriverTableSize_(std::max<std::size_t>(1u, driverTableInitialSize)),
      driverTableMaxLoadFactor_(maxLoadFactor) {
    avl_init(&orderTree_);
    avl_init(&orderDateTree_);
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
    avl_free(&orderDateTree_);
    avl_init(&orderDateTree_);
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
    avl_free(&orderDateTree_);
    avl_init(&orderDateTree_);
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

std::optional<DriverRecord> DataIntegrator::driverAt(std::size_t index) const {
    if (index >= drivers_.size()) {
        return std::nullopt;
    }
    return drivers_.at(index);
}

std::optional<OrderRecord> DataIntegrator::orderAt(std::size_t index) const {
    if (index >= orders_.size()) {
        return std::nullopt;
    }
    return orders_.at(index);
}

bool DataIntegrator::addOrder(const OrderRecord& record) {
    if (!driverTableReady_ || !orderTreeReady_) return false;
    if (!validateOrderRecord(record)) return false;
    if (!driverTable_.contains(record.licenseNumber)) return false;

    std::size_t index = orders_.push_back(record);
    avl_insert(&orderTree_, record.licenseNumber, index);
    avl_insert(&orderDateTree_, record.date.key(), index);
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

    bool dateChanged = current.date != updated.date;

    if (licenseChanged) {
        avl_remove_index(&orderTree_, current.licenseNumber, idx);
        avl_insert(&orderTree_, updated.licenseNumber, idx);
    }
    if (dateChanged) {
        avl_remove_index(&orderDateTree_, current.date.key(), idx);
        avl_insert(&orderDateTree_, updated.date.key(), idx);
    }

    orders_.at(idx) = updated;
    return true;
}

bool DataIntegrator::hasOrder(const OrderRecord& record) const {
    if (!orderTreeReady_) return false;
    std::optional<std::size_t> idxOpt = findOrderIndex(record, &record);
    return idxOpt.has_value();
}

DoublyLinkedList<OrderRecord> DataIntegrator::ordersForDriver(const std::string& licenseNumber) const {
    DoublyLinkedList<OrderRecord> result;
    if (!orderTreeReady_) return result;
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

bool DataIntegrator::loadFromFile(const std::string& path, std::size_t initialDriverTableSize) {
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

    std::size_t tableCapacity = driverTable_.capacity();
    if (initialDriverTableSize > 0) {
        tableCapacity = initialDriverTableSize;
    }

    DataIntegrator temp(tableCapacity, driverTableMaxLoadFactor_);
    temp.createDriverTable(tableCapacity);
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
    orderDateTree_ = temp.orderDateTree_;
    driverTableReady_ = temp.driverTableReady_;
    orderTreeReady_ = temp.orderTreeReady_;
    defaultDriverTableSize_ = temp.defaultDriverTableSize_;
    driverTableMaxLoadFactor_ = temp.driverTableMaxLoadFactor_;
    temp.orderTree_.root = nullptr;
    temp.orderDateTree_.root = nullptr;
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
        output << order.licenseNumber << '|' << order.address << '|' << order.cost << '|' << order.date.storageString() << '\n';
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

    DoublyLinkedList<HashTable::Entry> entries = driverTable_.entries();
    if (entries.empty()) {
        out << "(пусто)\n";
        return out.str();
    }

    entries.for_each([&](const HashTable::Entry& entry, std::size_t) {
        if (entry.index >= drivers_.size()) {
            return;
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
    });

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

std::string DataIntegrator::orderDateTreeAsText() const {
    if (!orderTreeReady_) {
        return {};
    }
    if (!orderDateTree_.root) {
        std::ostringstream empty;
        empty << "Дерево дат пусто\n";
        return empty.str();
    }

    std::ostringstream out;
    out << "Всего заказов: " << orders_.size() << '\n';
    out << "----------------------------------------\n";
    appendDateNodeDetailed(orderDateTree_.root, orders_, out, "", true, true);
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

    std::optional<std::size_t> foundIndex;
    node->listIndices.for_each_while([&](std::size_t idx, std::size_t) {
        const OrderRecord& stored = orders_.at(idx);
        if (stored.licenseNumber == match->licenseNumber &&
            stored.address == match->address &&
            stored.cost == match->cost &&
            stored.date == match->date) {
            foundIndex = idx;
            return false;
        }
        return true;
    });
    if (!foundIndex.has_value()) {
        return std::nullopt;
    }
    return foundIndex;
}

void DataIntegrator::removeOrderByIndex(const std::string& licenseNumber, std::size_t index) {
    if (!orderTreeReady_) return;
    DoublyLinkedList<OrderRecord>::SwapRemoveResult result;
    if (!orders_.remove_by_index(index, result)) return;

    avl_remove_index(&orderTree_, licenseNumber, index);
    avl_remove_index(&orderDateTree_, result.removedValue.date.key(), index);

    if (result.swapped && orders_.size() > index) {
        OrderRecord& movedOrder = orders_.at(index);
        avl_replace_index(&orderTree_, movedOrder.licenseNumber, result.swappedFromIndex, index);
        avl_replace_index(&orderDateTree_, movedOrder.date.key(), result.swappedFromIndex, index);
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
    if (!record.date.isValid()) return false;
    return true;
}

void DataIntegrator::collectOrdersInDateRange(const std::string& fromKey,
                                              const std::string& toKey,
                                              bool               hasFrom,
                                              bool               hasTo,
                                              DoublyLinkedList<std::size_t>& indices) const {
    collectOrdersInDateRange(orderDateTree_.root, fromKey, toKey, hasFrom, hasTo, indices);
}

void DataIntegrator::collectOrdersInDateRange(const AVLNode* node,
                                              const std::string& fromKey,
                                              const std::string& toKey,
                                              bool               hasFrom,
                                              bool               hasTo,
                                              DoublyLinkedList<std::size_t>& indices) const {
    if (!node) {
        return;
    }

    if (hasFrom && node->license < fromKey) {
        collectOrdersInDateRange(node->right, fromKey, toKey, hasFrom, hasTo, indices);
        return;
    }
    if (hasTo && node->license > toKey) {
        collectOrdersInDateRange(node->left, fromKey, toKey, hasFrom, hasTo, indices);
        return;
    }

    collectOrdersInDateRange(node->left, fromKey, toKey, hasFrom, hasTo, indices);
    node->listIndices.for_each([&](std::size_t idx, std::size_t) {
        indices.push_back(idx);
    });
    collectOrdersInDateRange(node->right, fromKey, toKey, hasFrom, hasTo, indices);
}

DoublyLinkedList<ReportEntry> DataIntegrator::generateReport(const std::string& licenseNumber,
                                                             const std::string& carBrand,
                                                             const std::string& address,
                                                             const std::string& dateFrom,
                                                             const std::string& dateTo) const {
    DoublyLinkedList<ReportEntry> result;
    if (!driverTableReady_ || !orderTreeReady_) {
        return result;
    }

    Date fromDate{};
    Date toDate{};
    bool      hasFrom = Date::parse(dateFrom, fromDate);
    bool      hasTo = Date::parse(dateTo, toDate);
    std::string fromKey = hasFrom ? fromDate.key() : std::string();
    std::string toKey = hasTo ? toDate.key() : std::string();

    DoublyLinkedList<std::size_t> candidateIndices;
    if ((hasFrom || hasTo) && orderDateTree_.root) {
        collectOrdersInDateRange(fromKey, toKey, hasFrom, hasTo, candidateIndices);
    }
    else {
        orders_.for_each([&](const OrderRecord&, std::size_t idx) {
            candidateIndices.push_back(idx);
        });
    }

    std::optional<DriverRecord> specificDriver;
    if (!licenseNumber.empty()) {
        specificDriver = findDriver(licenseNumber);
        if (!specificDriver.has_value()) {
            return result;
        }
    }

    candidateIndices.for_each([&](std::size_t index, std::size_t) {
        if (index >= orders_.size()) {
            return;
        }
        const OrderRecord& order = orders_.at(index);

        if (!licenseNumber.empty() && order.licenseNumber != licenseNumber) {
            return;
        }

        std::optional<DriverRecord> driverOpt;
        if (specificDriver.has_value() && order.licenseNumber == specificDriver->licenseNumber) {
            driverOpt = specificDriver;
        }
        else {
            driverOpt = findDriver(order.licenseNumber);
        }
        if (!driverOpt.has_value()) {
            return;
        }
        const DriverRecord& driver = driverOpt.value();

        if (!carBrand.empty() && driver.carBrand != carBrand) {
            return;
        }
        if (!address.empty() && order.address != address) {
            return;
        }
        if (hasFrom && order.date.key() < fromKey) {
            return;
        }
        if (hasTo && order.date.key() > toKey) {
            return;
        }

        result.push_back(ReportEntry{driver.licenseNumber,
                                     driver.fio,
                                     driver.carBrand,
                                     order.address,
                                     order.cost,
                                     order.date.displayString()});
    });

    return result;
}

std::string DataIntegrator::formatReport(const DoublyLinkedList<ReportEntry>& entries) const {
    if (entries.empty()) {
        return "Совпадений не найдено.\n";
    }

    std::ostringstream out;
    out << "Номер лицензии | ФИО | Марка автомобиля | Адрес | Цена | Дата\n";
    out << "--------------------------------------------------------------------------\n";
    entries.for_each([&](const ReportEntry& entry, std::size_t) {
        out << entry.licenseNumber << " | "
            << entry.fio << " | "
            << entry.carBrand << " | "
            << entry.address << " | "
            << entry.cost << " | "
            << entry.date << '\n';
    });
    return out.str();
}



