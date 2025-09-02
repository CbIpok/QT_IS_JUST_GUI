#ifndef HASHTABLE_HPP
#define HASHTABLE_HPP

#include <string>
#include <ostream>
#include "driver_record.hpp"

struct Cell {
    bool     occupied;
    DriverRecord   data;
    Cell();
};

class HashTable {
public:
    explicit HashTable(size_t initialSize, double maxLoad = 0.75);
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;
    HashTable(HashTable&& other) noexcept;
    HashTable& operator=(HashTable&& other) noexcept;
    ~HashTable();

    bool insert(const DriverRecord& rec);
    bool remove(const DriverRecord& rec);
    bool search(const std::string& licenseNumber,
        size_t& out_index, int& steps) const;

    // Returns pointer to record by license number or nullptr if not found
    const DriverRecord* find(const std::string& licenseNumber) const;

    void clear();
    void print(std::ostream& out) const;
    void saveToFile(const std::string& filename) const;
    int  getOriginalLine(size_t index) const;

private:
    size_t m_size, m_count, m_initialSize;
    double m_maxLoadFactor, m_minLoadFactor;
    Cell* table;

    std::string makeKey(const std::string& licenseNumber) const;
    size_t      hashPrimary(const std::string& key) const;
    size_t      hashSecondary(size_t base, const std::string& key, size_t iteration) const;
    void        rehash(size_t newSize);
};

#endif // HASHTABLE_HPP
