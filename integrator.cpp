#include "integrator.hpp"
#include <vector>

Integrator::Integrator(size_t hashSize) : hash_(hashSize) {
    avl_init(&tree_);
}

Integrator::~Integrator() {
    avl_free(&tree_);
}

bool Integrator::add_driver(const DriverRecord& rec) {
    size_t existing; int steps;
    if (hash_.get_index(rec.licenseNumber, existing, steps)) return false;
    size_t idx = drivers_.push_back_index(rec);
    return hash_.insert(rec.licenseNumber, idx);
}

bool Integrator::remove_driver(const std::string& licenseNumber) {
    size_t driverIdx;
    if (!hash_.remove(licenseNumber, driverIdx)) return false;

    DriverRecord movedDriver; size_t from;
    drivers_.remove_at(driverIdx, movedDriver, from);
    if (from != driverIdx) {
        hash_.update_index(movedDriver.licenseNumber, driverIdx);
    }

    // remove orders with same license number
    auto nodes = avl_inorder_nodes(&tree_);
    for (auto node : nodes) {
        if (node->key.licenseNumber == licenseNumber) {
            std::vector<int> indices = node->lineNumbers;
            for (int idx : indices) {
                OrderRecord movedOrder; size_t movedFrom;
                orders_.remove_at(idx, movedOrder, movedFrom);
                if (movedFrom != static_cast<size_t>(idx)) {
                    avl_update_index(&tree_, movedOrder, static_cast<int>(movedFrom), idx);
                }
            }
            avl_remove(&tree_, node->key);
        }
    }
    return true;
}

bool Integrator::add_order(const OrderRecord& order) {
    size_t driverIdx; int steps;
    if (!hash_.get_index(order.licenseNumber, driverIdx, steps)) {
        return false; // driver not found
    }
    size_t idx = orders_.push_back_index(order);
    avl_insert(&tree_, order, static_cast<int>(idx));
    return true;
}

bool Integrator::driver_exists(const std::string& licenseNumber) const {
    size_t idx; int steps;
    return hash_.get_index(licenseNumber, idx, steps);
}

bool Integrator::order_exists(const OrderRecord& order) const {
    return avl_search(const_cast<AVLTree*>(&tree_), order) != nullptr;
}
