#pragma once
#include <string>
#include <vector>
#include "driver_record.hpp"
#include "order_record.hpp"
#include "record_store.hpp"
#include "hashtable_idx.hpp"
#include "avl_tree_idx.hpp"

// Integrator orchestrates operations across HashTableIndex and AVLTreeIndex,
// storing actual records in two RecordStore instances. Structures keep indices only.
class Integrator {
public:
    // HashTable operations
    bool ht_insert(const DriverRecord& rec, std::string& err);
    bool ht_remove_by_license(const std::string& license);
    bool ht_find(const std::string& license, DriverRecord& out) const;

    // AVLTree operations
    bool avl_insert(const OrderRecord& rec, std::string& err);
    bool avl_remove_by_key(const OrderKey& key);
    bool avl_find(const OrderKey& key, std::vector<OrderRecord>& out) const;

    // Helpers for tests
    std::size_t ht_size() const { return ht_.size(); }

private:
    // storages
    RecordStore<DriverRecord> drivers_;
    RecordStore<OrderRecord> orders_;
    // indices
    HashTableIndex ht_;
    AVLTreeIndex   avl_;
};

