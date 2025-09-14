#pragma once
#include <string>
#include <vector>
#include <cstddef>

// Hash table that maps licenseNumber -> index in external storage.
class HashTableIndex {
public:
    explicit HashTableIndex(std::size_t initialSize = 8, double maxLoad = 0.75);

    bool insert(const std::string& license, std::size_t index);
    bool contains(const std::string& license) const;
    bool find_index(const std::string& license, std::size_t& outIndex) const;
    bool update_index(const std::string& license, std::size_t newIndex);
    bool remove(const std::string& license, std::size_t& removedIndex);

    void clear();
    std::size_t size() const { return count_; }

private:
    struct Cell {
        bool occupied{false};
        std::string key; // license
        std::size_t index{0};
    };

    std::vector<Cell> table_;
    std::size_t count_{0};
    double maxLoad_;

    std::size_t hashPrimary(const std::string& key) const;
    std::size_t hashStep(const std::string& key) const;
    void rehash(std::size_t newSize);
};

