#include <gtest/gtest.h>
#include "../data_integrator.hpp"

TEST(Integrator, InsertAndRemove) {
    DataIntegrator integrator;
    Record rec{"Ivanov I I", 1, "Street", 12345, 1};
    ASSERT_TRUE(integrator.insertRecord(rec));

    size_t idx; int steps;
    EXPECT_TRUE(integrator.hashtable.search(rec.fio, rec.applicationNumber, idx, steps));
    PersonKey pk{rec.fio, static_cast<int>(rec.phoneNumber)};
    EXPECT_NE(integrator.avlTree.search(pk), nullptr);

    EXPECT_TRUE(integrator.removeByHash(rec));
    EXPECT_EQ(integrator.avlTree.search(pk), nullptr);
}

TEST(Integrator, TreeInsertConflict) {
    DataIntegrator integrator;
    Record rec{"Ivanov I I", 1, "Street", 12345, 1};
    ASSERT_TRUE(integrator.insertRecord(rec));

    PersonKey pk{rec.fio, static_cast<int>(rec.phoneNumber)};
    EXPECT_FALSE(integrator.insertTreeOnly(pk, 2));
}

TEST(Integrator, SeparateInsertAndAssemble) {
    DataIntegrator integrator;
    Record rec{"Petrov P P", 2, "Lane", 54321, 5};
    ASSERT_TRUE(integrator.insertHashOnly(rec));
    PersonKey pk{rec.fio, static_cast<int>(rec.phoneNumber)};
    ASSERT_TRUE(integrator.insertTreeOnly(pk, rec.originalLine));

    Record out;
    EXPECT_TRUE(integrator.getRecord(rec.fio, rec.applicationNumber, out));
    EXPECT_EQ(out.street, rec.street);
}

TEST(Integrator, SeparateRemovals) {
    DataIntegrator integrator;
    Record rec{"Sidorov S S", 3, "Ave", 11111, 7};
    ASSERT_TRUE(integrator.insertRecord(rec));
    PersonKey pk{rec.fio, static_cast<int>(rec.phoneNumber)};

    EXPECT_TRUE(integrator.removeTreeOnly(pk));
    size_t idx; int steps;
    EXPECT_TRUE(integrator.hashtable.search(rec.fio, rec.applicationNumber, idx, steps));
    Record out;
    EXPECT_FALSE(integrator.getRecord(rec.fio, rec.applicationNumber, out));

    EXPECT_TRUE(integrator.removeHashOnly(rec));
    EXPECT_FALSE(integrator.hashtable.search(rec.fio, rec.applicationNumber, idx, steps));
}
