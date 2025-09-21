#include <gtest/gtest.h>
#include "DoublyLinkedList.hpp"
#include <sstream>
#include <cstdio>

namespace {

class DoublyLinkedList_PushAndContains : public ::testing::Test {
protected:
    void TestBody() override;
};

class DoublyLinkedList_RemoveReversePrint : public ::testing::Test {
protected:
    void TestBody() override;
};

class DoublyLinkedList_RemoveBeforeValue : public ::testing::Test {
protected:
    void TestBody() override;
};

class DoublyLinkedList_RemoveByIndexSwap : public ::testing::Test {
protected:
    void TestBody() override;
};

void DoublyLinkedList_PushAndContains::TestBody() {
    DoublyLinkedList<int> list;
    list.push_back(1);
    list.push_front(0);
    list.push_back(2);
    EXPECT_EQ(list.length(), 3);
    EXPECT_TRUE(list.contains(0));
    EXPECT_TRUE(list.contains(1));
    EXPECT_TRUE(list.contains(2));
}

void DoublyLinkedList_RemoveReversePrint::TestBody() {
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

void DoublyLinkedList_RemoveBeforeValue::TestBody() {
    DoublyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    list.remove_before_value(2);
    std::stringstream ss;
    list.print(ss);
    EXPECT_EQ(ss.str(), "2 3 \n");
}

void DoublyLinkedList_RemoveByIndexSwap::TestBody() {
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

} // namespace

void RegisterDoublyLinkedListTests() {
    std::printf("Registering DLL tests\n");
    ::testing::RegisterTest("DoublyLinkedListTest", "PushAndContains", nullptr, nullptr, __FILE__, __LINE__,
        []() -> ::testing::Test* { return new DoublyLinkedList_PushAndContains; });
    ::testing::RegisterTest("DoublyLinkedListTest", "RemoveReversePrint", nullptr, nullptr, __FILE__, __LINE__,
        []() -> ::testing::Test* { return new DoublyLinkedList_RemoveReversePrint; });
    ::testing::RegisterTest("DoublyLinkedListTest", "RemoveBeforeValue", nullptr, nullptr, __FILE__, __LINE__,
        []() -> ::testing::Test* { return new DoublyLinkedList_RemoveBeforeValue; });
    ::testing::RegisterTest("DoublyLinkedListTest", "RemoveByIndexSwap", nullptr, nullptr, __FILE__, __LINE__,
        []() -> ::testing::Test* { return new DoublyLinkedList_RemoveByIndexSwap; });
}
