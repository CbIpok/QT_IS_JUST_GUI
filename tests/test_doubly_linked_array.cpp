#include <gtest/gtest.h>
#include <sstream>
#include "../doubly_linked_array.hpp"

TEST(DoublyLinkedArray, BasicOperations) {
    DoublyLinkedArray<int> list;
    int a = list.push_back(1);
    int b = list.push_back(2);
    int c = list.push_back(3);

    EXPECT_EQ(list.at(a)->value, 1);
    EXPECT_EQ(list.at(b)->prev, a);
    EXPECT_EQ(list.at(c)->prev, b);

    EXPECT_TRUE(list.remove(b));
    EXPECT_EQ(list.at(b), nullptr);

    std::ostringstream oss;
    list.print(oss);
    EXPECT_EQ(oss.str(), "1 <-> 3");
}

