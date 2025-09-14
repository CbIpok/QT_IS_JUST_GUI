#ifndef HASHTABLE_HPP
#define HASHTABLE_HPP

#include <string>
#include <ostream>

struct Cell {
    bool occupied;
    std::string key; // license number
    size_t index;    // position in linked list
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

    bool insert(const std::string& key, size_t index);
    bool get_index(const std::string& key, size_t& index, int& steps) const;
    bool remove(const std::string& key, size_t& index);
    bool update_index(const std::string& key, size_t newIndex);

    void clear();
    void print(std::ostream& out) const;

private:
    size_t m_size, m_count, m_initialSize;
    double m_maxLoadFactor, m_minLoadFactor;
    Cell* table;

    size_t hashPrimary(const std::string& key) const;
    size_t hashSecondary(size_t base, const std::string& key, size_t iteration) const;
    void   rehash(size_t newSize);
};

#endif // HASHTABLE_HPP
