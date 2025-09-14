#include <gtest/gtest.h>
#include "integrator.hpp"

TEST(IntegratorTest, HT_InsertFindRemove_CascadeToAVL) {
    std::cout << "Integrator smoke test entry\n";
    Integrator it; std::string err;
    OrderRecord o1{"LIC1","ADDR1","100","2024-01-01"};
    EXPECT_FALSE(it.avl_insert(o1, err));
    DriverRecord d1{"LIC1","FIO1","BMW",1};
    ASSERT_TRUE(it.ht_insert(d1, err));
    OrderRecord o2{"LIC1","ADDR2","200","2024-02-02"};
    ASSERT_TRUE(it.avl_insert(o1, err));
    ASSERT_TRUE(it.avl_insert(o2, err));

    // Cascade remove
    ASSERT_TRUE(it.ht_remove_by_license("LIC1"));
    // Should fail now
    ASSERT_FALSE(it.avl_insert(o2, err));
}

TEST(IntegratorTest, AVL_InsertRemove_SwapWithLast_IndicesUpdate) {
    Integrator it; std::string err;
    // Prepare drivers first
    ASSERT_TRUE(it.ht_insert(DriverRecord{"A","F","B",1}, err));
    ASSERT_TRUE(it.ht_insert(DriverRecord{"B","F","B",1}, err));
    ASSERT_TRUE(it.ht_insert(DriverRecord{"C","F","B",1}, err));

    // Insert AVL orders (drivers exist)
    OrderRecord a{"A","X","1","d"};
    OrderRecord b{"B","X","1","d"};
    OrderRecord c{"C","X","1","d"};
    ASSERT_TRUE(it.avl_insert(a, err));
    ASSERT_TRUE(it.avl_insert(b, err));
    ASSERT_TRUE(it.avl_insert(c, err));

    // Remove key {B,X}, should remove that node and compact storage
    ASSERT_TRUE(it.avl_remove_by_key(OrderKey{"B","X"}));

    // Find remaining keys
    std::vector<OrderRecord> out;
    ASSERT_TRUE(it.avl_find(OrderKey{"A","X"}, out));
    ASSERT_EQ(out.size(), 1u);
    ASSERT_TRUE(it.avl_find(OrderKey{"C","X"}, out));
}
