#include <gtest/gtest.h>
#include "DoublyLinkedList.hpp"
#include <sstream>

TEST(DoublyLinkedListTest, PushAndContains) {
    DoublyLinkedList<int> list;
    list.push_back(1);
    list.push_front(0);
    list.push_back(2);
    EXPECT_EQ(list.length(), 3);
    EXPECT_TRUE(list.contains(0));
    EXPECT_TRUE(list.contains(1));
    EXPECT_TRUE(list.contains(2));
}

TEST(DoublyLinkedListTest, RemoveReversePrint) {
    DoublyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    list.remove_all(2);
    EXPECT_EQ(list.length(), 2);
    EXPECT_FALSE(list.contains(2));
    std::stringstream ss;
    list.print(ss);
    EXPECT_EQ(ss.str(), "1 3 \n");
    list.reverse();
    std::stringstream ss2;
    list.print(ss2);
    EXPECT_EQ(ss2.str(), "3 1 \n");
}

TEST(DoublyLinkedListTest, RemoveBeforeValue) {
    DoublyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    list.remove_before_value(2);
    std::stringstream ss;
    list.print(ss);
    EXPECT_EQ(ss.str(), "2 3 \n");
}

TEST(DoublyLinkedListTest, RemoveByIndexSwap) {
    DoublyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    DoublyLinkedList<int>::SwapRemoveResult result;
    EXPECT_TRUE(list.remove_by_index(0, result));
    EXPECT_TRUE(result.swapped);
    EXPECT_EQ(result.removedValue, 1);
    EXPECT_EQ(list.length(), 2);
    EXPECT_EQ(list.at(0), 3);
    EXPECT_EQ(list.at(1), 2);
}
