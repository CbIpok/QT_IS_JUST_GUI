#include "hashtable_idx.hpp"
#include <functional>

static std::size_t next_prime(std::size_t n) {
    auto is_prime = [](std::size_t x){ if (x<2) return false; for (std::size_t i=2;i*i<=x;++i) if (x%i==0) return false; return true; };
    while (!is_prime(n)) ++n; return n;
}

HashTableIndex::HashTableIndex(std::size_t initialSize, double maxLoad)
    : table_(next_prime(initialSize)), maxLoad_(maxLoad) {}

std::size_t HashTableIndex::hashPrimary(const std::string& key) const {
    // simple FNV-1a like
    std::size_t h = 1469598103934665603ull;
    for (unsigned char c : key) { h ^= c; h *= 1099511628211ull; }
    return h % table_.size();
}

std::size_t HashTableIndex::hashStep(const std::string& key) const {
    std::size_t h = 2166136261u;
    for (unsigned char c : key) { h ^= c; h *= 16777619u; }
    // ensure odd step < size
    std::size_t step = (h % (table_.size()-1)) + 1; if ((step & 1)==0) ++step; return step;
}

void HashTableIndex::rehash(std::size_t newSize) {
    std::vector<Cell> old = std::move(table_);
    table_.assign(next_prime(newSize), Cell{});
    count_ = 0;
    for (const auto& c : old) if (c.occupied) insert(c.key, c.index);
}

bool HashTableIndex::insert(const std::string& license, std::size_t index) {
    if ((double)(count_ + 1) / (double)table_.size() > maxLoad_) rehash(table_.size() * 2);
    std::size_t i = hashPrimary(license);
    std::size_t step = hashStep(license);
    while (true) {
        if (!table_[i].occupied) {
            table_[i].occupied = true;
            table_[i].key = license;
            table_[i].index = index;
            ++count_;
            return true;
        }
        if (table_[i].key == license) {
            // duplicate not allowed
            return false;
        }
        i = (i + step) % table_.size();
    }
}

bool HashTableIndex::contains(const std::string& license) const {
    std::size_t dummy; return find_index(license, dummy);
}

bool HashTableIndex::find_index(const std::string& license, std::size_t& outIndex) const {
    std::size_t i = hashPrimary(license);
    std::size_t step = hashStep(license);
    std::size_t probes = 0;
    while (probes < table_.size()) {
        const auto& c = table_[i];
        if (!c.occupied) return false;
        if (c.key == license) { outIndex = c.index; return true; }
        i = (i + step) % table_.size();
        ++probes;
    }
    return false;
}

bool HashTableIndex::update_index(const std::string& license, std::size_t newIndex) {
    std::size_t i = hashPrimary(license);
    std::size_t step = hashStep(license);
    std::size_t probes = 0;
    while (probes < table_.size()) {
        auto& c = table_[i];
        if (!c.occupied) return false;
        if (c.key == license) { c.index = newIndex; return true; }
        i = (i + step) % table_.size();
        ++probes;
    }
    return false;
}

bool HashTableIndex::remove(const std::string& license, std::size_t& removedIndex) {
    std::size_t i = hashPrimary(license);
    std::size_t step = hashStep(license);
    std::size_t probes = 0;
    while (probes < table_.size()) {
        auto& c = table_[i];
        if (!c.occupied) return false;
        if (c.key == license) {
            removedIndex = c.index;
            // mark as removed by reinserting following cluster
            c.occupied = false;
            c.key.clear();
            --count_;
            // reinsert downstream cluster entries
            std::size_t j = (i + step) % table_.size();
            while (table_[j].occupied) {
                auto entry = table_[j];
                table_[j].occupied = false;
                --count_;
                insert(entry.key, entry.index);
                j = (j + step) % table_.size();
            }
            return true;
        }
        i = (i + step) % table_.size();
        ++probes;
    }
    return false;
}

void HashTableIndex::clear() {
    table_.assign(table_.size(), Cell{});
    count_ = 0;
}

