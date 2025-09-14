#pragma once
#include "hashtable.hpp"
#include "avl_tree.h"
#include "DoublyLinkedList.hpp"
#include "driver_record.hpp"
#include "order_record.hpp"
#include <string>

class Integrator {
public:
    explicit Integrator(size_t hashSize = 16);
    ~Integrator();

    bool add_driver(const DriverRecord& rec);
    bool remove_driver(const std::string& licenseNumber);
    bool add_order(const OrderRecord& order);

    bool driver_exists(const std::string& licenseNumber) const;
    bool order_exists(const OrderRecord& order) const;

private:
    DoublyLinkedList<DriverRecord> drivers_;
    DoublyLinkedList<OrderRecord> orders_;
    HashTable hash_;
    AVLTree tree_;
};
