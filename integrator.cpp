#include "integrator.hpp"

// HashTable ops
bool Integrator::ht_insert(const DriverRecord& rec, std::string& err) {
    if (ht_.contains(rec.licenseNumber)) { err = "duplicate license"; return false; }
    // Insert into storage first
    std::size_t idx = drivers_.add(rec);
    if (!ht_.insert(rec.licenseNumber, idx)) {
        // rollback
        drivers_.remove_at(idx);
        err = "hashtable insert failed";
        return false;
    }
    return true;
}

bool Integrator::ht_remove_by_license(const std::string& license) {
    std::size_t idx;
    if (!ht_.remove(license, idx)) return false;

    // Cascade delete in AVL by same license
    auto keys = avl_.collect_keys_with_license(license);
    for (const auto& key : keys) {
        std::vector<std::size_t> removedIndices;
        if (avl_.remove_key(key, removedIndices)) {
            // remove each order record index from orders_ store
            for (std::size_t oi : removedIndices) {
                auto res = orders_.remove_at(oi);
                if (res.moved) {
                    // moved order record; update its index in avl using its key
                    const OrderRecord& moved = res.moved_element;
                    avl_.update_index_for_key(OrderKey{ moved.licenseNumber, moved.address }, res.moved_from, res.moved_to);
                }
            }
        }
    }

    // Finally remove driver record from storage (swap-with-last)
    auto r = drivers_.remove_at(idx);
    if (r.moved) {
        const DriverRecord& moved = r.moved_element;
        // Update hashtable index for moved driver
        ht_.update_index(moved.licenseNumber, r.moved_to);
    }
    return true;
}

bool Integrator::ht_find(const std::string& license, DriverRecord& out) const {
    std::size_t idx; if (!ht_.find_index(license, idx)) return false; out = drivers_.get(idx); return true;
}

// AVL ops
bool Integrator::avl_insert(const OrderRecord& rec, std::string& err) {
    // Orders can be inserted only if there is an HT entry with same license
    if (!ht_.contains(rec.licenseNumber)) { err = "no driver with such license in HT"; return false; }
    std::size_t idx = orders_.add(rec);
    avl_.insert(OrderKey{ rec.licenseNumber, rec.address }, idx);
    return true;
}

bool Integrator::avl_remove_by_key(const OrderKey& key) {
    std::vector<std::size_t> removed;
    if (!avl_.remove_key(key, removed)) return false;
    for (std::size_t oi : removed) {
        auto r = orders_.remove_at(oi);
        if (r.moved) {
            const OrderRecord& moved = r.moved_element;
            avl_.update_index_for_key(OrderKey{ moved.licenseNumber, moved.address }, r.moved_from, r.moved_to);
        }
    }
    return true;
}

bool Integrator::avl_find(const OrderKey& key, std::vector<OrderRecord>& out) const {
    std::vector<std::size_t> idxs; if (!avl_.find_indices(key, idxs)) return false; out.clear(); out.reserve(idxs.size()); for (auto i : idxs) out.push_back(orders_.get(i)); return true;
}
