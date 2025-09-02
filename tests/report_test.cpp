#include <gtest/gtest.h>
#include "report.hpp"

TEST(ReportTest, SearchByLicense) {
    HashTable drivers(5);
    drivers.insert({"L1", "Alice", "BMW", 1});
    drivers.insert({"L2", "Bob", "Audi", 2});

    AVLTree orders; avl_init(&orders);
    avl_insert(&orders, {"L1", "Main", "100", "01 Jan 2023"}, 0);
    avl_insert(&orders, {"L1", "Second", "200", "15 Feb 2023"}, 1);
    avl_insert(&orders, {"L2", "Main", "150", "10 Mar 2023"}, 2);

    auto res = generateReport(drivers, orders, "L1", "", "", "", "");
    ASSERT_EQ(res.size(), 2u);
    EXPECT_EQ(res[0].address, "Main");
    EXPECT_EQ(res[1].address, "Second");
    avl_free(&orders);
}

TEST(ReportTest, Filters) {
    HashTable drivers(3);
    drivers.insert({"L1", "Alice", "BMW", 1});

    AVLTree orders; avl_init(&orders);
    avl_insert(&orders, {"L1", "Main", "100", "01 Jan 2023"}, 0);
    avl_insert(&orders, {"L1", "Second", "200", "10 Feb 2023"}, 1);

    auto res0 = generateReport(drivers, orders, "L1", "Audi", "", "", "");
    EXPECT_TRUE(res0.empty());

    auto res1 = generateReport(drivers, orders, "L1", "BMW", "Main", "", "");
    ASSERT_EQ(res1.size(), 1u);
    EXPECT_EQ(res1[0].address, "Main");

    auto res2 = generateReport(drivers, orders, "L1", "BMW", "", "01 Jan 2023", "31 Jan 2023");
    ASSERT_EQ(res2.size(), 1u);
    EXPECT_EQ(res2[0].date, "01 Jan 2023");
    avl_free(&orders);
}
