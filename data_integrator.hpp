#pragma once

#include <optional>
#include <string>
#include <vector>

#include "DoublyLinkedList.hpp"
#include "avl_tree.h"
#include "driver_record.hpp"
#include "hashtable.hpp"
#include "order_record.hpp"

struct ReportEntry {
    std::string licenseNumber;
    std::string fio;
    std::string carBrand;
    std::string address;
    std::string cost;
    std::string date;
};

class DataIntegrator {
public:
    explicit DataIntegrator(std::size_t driverTableInitialSize = 32, double maxLoadFactor = 0.75);

    bool createDriverTable(std::size_t initialSize);
    bool createOrderTree();
    void clearDriverTable();
    void clearOrderTree();
    bool hasDriverTable() const { return driverTableReady_; }
    bool hasOrderTree() const { return orderTreeReady_; }

    bool addDriver(const DriverRecord& record);
    bool removeDriver(const DriverRecord& record);
    bool removeDriver(const std::string& licenseNumber);
    bool updateDriver(const DriverRecord& current, const DriverRecord& updated);
    bool updateDriver(const std::string& licenseNumber, const DriverRecord& updated);
    bool hasDriver(const std::string& licenseNumber) const;
    std::optional<DriverRecord> findDriver(const DriverRecord& record) const;
    std::optional<DriverRecord> findDriver(const std::string& licenseNumber) const;

    bool addOrder(const OrderRecord& record);
    bool removeOrder(const OrderRecord& record);
    bool updateOrder(const OrderRecord& current, const OrderRecord& updated);
    bool hasOrder(const OrderRecord& record) const;
    std::vector<OrderRecord> ordersForDriver(const std::string& licenseNumber) const;

    std::size_t driverCount() const { return drivers_.size(); }
    std::size_t orderCount() const { return orders_.size(); }
    std::size_t driverTableCapacity() const { return driverTable_.capacity(); }

    void setNextDriverTableSize(std::size_t size);

    void clear();

    bool loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path) const;
    std::string hashTableAsText() const;
    std::string orderTreeAsText() const;
    bool saveStructures(const std::string& hashTablePath, const std::string& treePath) const;

    std::vector<ReportEntry> generateReport(const std::string& licenseNumber,
                                            const std::string& carBrand,
                                            const std::string& address,
                                            const std::string& dateFrom,
                                            const std::string& dateTo) const;
    std::string formatReport(const std::vector<ReportEntry>& entries) const;

private:
    DoublyLinkedList<DriverRecord> drivers_;
    DoublyLinkedList<OrderRecord>  orders_;
    HashTable                      driverTable_;
    AVLTree                        orderTree_;
    bool                           driverTableReady_;
    bool                           orderTreeReady_;
    std::size_t                    defaultDriverTableSize_;
    double                         driverTableMaxLoadFactor_;
    std::optional<std::size_t>     pendingDriverTableSize_;

    std::optional<std::size_t> findDriverIndex(const std::string& licenseNumber) const;
    std::optional<std::size_t> findDriverIndex(const DriverRecord& record) const;
    std::optional<std::size_t> findOrderIndex(const OrderRecord& key, const OrderRecord* match) const;
    void removeOrderByIndex(const std::string& licenseNumber, std::size_t index);
    bool validateDriverRecord(const DriverRecord& record) const;
    bool validateOrderRecord(const OrderRecord& record) const;
};
