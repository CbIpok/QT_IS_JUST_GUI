#ifndef DATA_INTEGRATOR_HPP
#define DATA_INTEGRATOR_HPP

#include "hashtable.hpp"
#include "avl_tree.h"
#include "doubly_linked_array.hpp"

class DataIntegrator {
public:
    DataIntegrator(size_t hashSize = 4)
        : hashStorage(),
          treeStorage(),
          hashtable(hashSize, hashStorage),
          avlTree(treeStorage) {}

    // Insert record into both structures.  Returns false on duplicate.
    bool insertRecord(const Record& rec) {
        if (!hashtable.insert(rec)) return false;
        PersonKey pk{rec.fio, static_cast<int>(rec.phoneNumber)};
        if (avlTree.search(pk)) {
            hashtable.remove(rec);
            return false;
        }
        avlTree.insert(pk, rec.originalLine);
        return true;
    }

    // Remove record from hash table and corresponding entry from tree
    bool removeByHash(const Record& rec) {
        bool ok = hashtable.remove(rec);
        if (ok) {
            PersonKey pk{rec.fio, static_cast<int>(rec.phoneNumber)};
            avlTree.remove(pk);
        }
        return ok;
    }

    // Insert only into hash table
    bool insertHashOnly(const Record& rec) {
        return hashtable.insert(rec);
    }

    // Remove only from hash table
    bool removeHashOnly(const Record& rec) {
        return hashtable.remove(rec);
    }

    // Insert into tree only if not present in hash table
    bool insertTreeOnly(const PersonKey& pk, int line) {
        size_t idx; int steps;
        if (hashtable.search(pk.fullName, pk.phoneNumber, idx, steps)) {
            return false; // conflicting key in hash table
        }
        if (avlTree.search(pk)) return false;
        avlTree.insert(pk, line);
        return true;
    }

    // Remove only from tree
    bool removeTreeOnly(const PersonKey& pk) {
        return avlTree.remove(pk);
    }

    // Retrieve record if present in both structures
    bool getRecord(const std::string& fio, int applicationNumber, Record& out) const {
        if (!hashtable.get(fio, applicationNumber, out)) return false;
        PersonKey pk{out.fio, static_cast<int>(out.phoneNumber)};
        return avlTree.search(pk) != nullptr;
    }

    DoublyLinkedArray<Record>     hashStorage;
    DoublyLinkedArray<PersonKey>  treeStorage;
    HashTable hashtable;
    AVLTree   avlTree;
};

#endif // DATA_INTEGRATOR_HPP
