#include <gtest/gtest.h>
#include "integrator.hpp"

TEST(IntegratorTest, RemoveDriverRemovesOrders) {
    Integrator integ;
    DriverRecord d{"TK-25-111111-2023", "Novikova Daria Sergeevna", "BMW", 0};
    OrderRecord o{"TK-25-111111-2023", "Street 1", "100", "01 jan 2025"};
    ASSERT_TRUE(integ.add_driver(d));
    ASSERT_TRUE(integ.add_order(o));
    ASSERT_TRUE(integ.order_exists(o));
    EXPECT_TRUE(integ.remove_driver(d.licenseNumber));
    EXPECT_FALSE(integ.driver_exists(d.licenseNumber));
    EXPECT_FALSE(integ.order_exists(o));
}

TEST(IntegratorTest, InsertOrderRequiresDriver) {
    Integrator integ;
    OrderRecord o{"TK-25-999999-2023", "Street 2", "200", "01 feb 2025"};
    EXPECT_FALSE(integ.add_order(o));
}
