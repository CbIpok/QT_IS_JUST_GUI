#pragma once

#include <optional>
#include <string>
#include <vector>

#include "DoublyLinkedList.hpp"
#include "avl_tree.h"
#include "driver_record.hpp"
#include "hashtable.hpp"
#include "order_record.hpp"

class DataIntegrator {
public:
    explicit DataIntegrator(std::size_t driverTableInitialSize = 32, double maxLoadFactor = 0.75);

    bool addDriver(const DriverRecord& record);
    bool removeDriver(const std::string& licenseNumber);
    bool updateDriver(const std::string& licenseNumber, const DriverRecord& updated);
    bool hasDriver(const std::string& licenseNumber) const;
    std::optional<DriverRecord> findDriver(const std::string& licenseNumber) const;

    bool addOrder(const OrderRecord& record);
    bool removeOrder(const OrderRecord& record);
    bool updateOrder(const OrderRecord& current, const OrderRecord& updated);
    bool hasOrder(const OrderRecord& record) const;
    std::vector<OrderRecord> ordersForDriver(const std::string& licenseNumber) const;

    std::size_t driverCount() const { return drivers_.size(); }
    std::size_t orderCount() const { return orders_.size(); }

    void clear();

    bool loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path) const;
    std::string hashTableAsText() const;
    std::string orderTreeAsText() const;
    bool saveStructures(const std::string& hashTablePath, const std::string& treePath) const;

private:
    DoublyLinkedList<DriverRecord> drivers_;
    DoublyLinkedList<OrderRecord>  orders_;
    HashTable                      driverTable_;
    AVLTree                        orderTree_;

    std::optional<std::size_t> findDriverIndex(const std::string& licenseNumber) const;
    std::optional<std::size_t> findOrderIndex(const OrderRecord& key, const OrderRecord* match) const;
    void removeOrderByIndex(const OrderRecord& key, std::size_t index);
};
