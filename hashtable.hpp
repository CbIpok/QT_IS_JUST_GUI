#ifndef HASHTABLE_HPP
#define HASHTABLE_HPP

#include <cstddef>
#include <string>

#include "dynamic_array.hpp"

struct Cell {
    bool        occupied;
    std::string key;
    std::size_t index;
    Cell();
};

class HashTable {
public:
    struct Entry {
        std::size_t slot;
        std::string key;
        std::size_t index;
    };

    explicit HashTable(std::size_t initialSize, double maxLoad = 0.75);
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;
    HashTable(HashTable&& other) noexcept;
    HashTable& operator=(HashTable&& other) noexcept;
    ~HashTable();

    bool insert(const std::string& key, std::size_t listIndex);
    bool remove(const std::string& key, std::size_t& removedIndex);
    bool search(const std::string& key, std::size_t& outIndex, int& steps) const;
    bool update_index(const std::string& key, std::size_t newIndex);

    bool contains(const std::string& key) const;

    void clear();
    std::string toString() const;
    DynamicArray<Entry> entries() const;

    std::size_t capacity() const { return m_size; }
    std::size_t size() const { return m_count; }

private:
    std::size_t m_size;
    std::size_t m_count;
    std::size_t m_initialSize;
    double      m_maxLoadFactor;
    double      m_minLoadFactor;
    Cell*       table;

    std::size_t hashPrimary(const std::string& key) const;
    std::size_t hashSecondary(std::size_t base, const std::string& key, std::size_t iteration) const;
    void        rehash(std::size_t newSize);
    std::size_t find_slot(const std::string& key, int* steps = nullptr) const;
};

#endif // HASHTABLE_HPP
