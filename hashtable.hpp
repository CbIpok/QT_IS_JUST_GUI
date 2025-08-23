#ifndef HASHTABLE_HPP
#define HASHTABLE_HPP

#include <string>
#include <ostream>
#include "record3.hpp"

struct Cell {
    bool     occupied;
    Record   data;
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

    bool insert(const Record& rec);
    bool remove(const Record& rec);
    bool search(const std::string& fio, int applicationNumber,
        size_t& out_index, int& steps) const;

    void clear();
    void print(std::ostream& out) const;
    void saveToFile(const std::string& filename) const;
    int  getOriginalLine(size_t index) const;

private:
    size_t m_size, m_count, m_initialSize;
    double m_maxLoadFactor, m_minLoadFactor;
    Cell* table;

    std::string makeKey(const std::string& fio, int applicationNumber) const;
    size_t      hashPrimary(const std::string& key) const;
    size_t      hashSecondary(size_t base, const std::string& key, size_t iteration) const;
    void        rehash(size_t newSize);
};

#endif // HASHTABLE_HPP
