#include <gtest/gtest.h>
#include "data_integrator.hpp"

namespace {

class IntegratorAddsDriverAndOrder : public ::testing::Test {
protected:
    void TestBody() override;
};

class IntegratorRejectsOrderWithoutDriver : public ::testing::Test {
protected:
    void TestBody() override;
};

class IntegratorCascadesDriverRemoval : public ::testing::Test {
protected:
    void TestBody() override;
};

class IntegratorMaintainsIndicesAfterOrderRemoval : public ::testing::Test {
protected:
    void TestBody() override;
};

class IntegratorUpdatesOrderKey : public ::testing::Test {
protected:
    void TestBody() override;
};

class IntegratorUpdateDriverKeepsData : public ::testing::Test {
protected:
    void TestBody() override;
};

void IntegratorAddsDriverAndOrder::TestBody() {
    DataIntegrator integrator;
    DriverRecord driver{"TK-25-111111-2023", "Novikova Daria", "BMW", 10};
    OrderRecord order{driver.licenseNumber, "Ul. Lesnaya", "300 r.", "02 jan 2025"};

    EXPECT_TRUE(integrator.addDriver(driver));
    EXPECT_TRUE(integrator.hasDriver(driver.licenseNumber));
    EXPECT_EQ(integrator.driverCount(), 1u);

    EXPECT_TRUE(integrator.addOrder(order));
    EXPECT_EQ(integrator.orderCount(), 1u);

    auto storedDriver = integrator.findDriver(driver.licenseNumber);
    ASSERT_TRUE(storedDriver.has_value());
    EXPECT_EQ(storedDriver->fio, driver.fio);

    auto orders = integrator.ordersForDriver(driver.licenseNumber);
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].address, order.address);
}

void IntegratorRejectsOrderWithoutDriver::TestBody() {
    DataIntegrator integrator;
    OrderRecord order{"TK-25-333333-2025", "Ul. Mira", "500 r.", "05 feb 2025"};
    EXPECT_FALSE(integrator.addOrder(order));
    EXPECT_EQ(integrator.orderCount(), 0u);
}

void IntegratorCascadesDriverRemoval::TestBody() {
    DataIntegrator integrator;
    DriverRecord driver{"TK-25-444444-2025", "Melnikov Igor", "Audi", 5};
    OrderRecord o1{driver.licenseNumber, "Ul. Mira", "400 r.", "10 feb 2025"};
    OrderRecord o2{driver.licenseNumber, "Ul. Lenina", "600 r.", "12 feb 2025"};

    ASSERT_TRUE(integrator.addDriver(driver));
    ASSERT_TRUE(integrator.addOrder(o1));
    ASSERT_TRUE(integrator.addOrder(o2));
    EXPECT_EQ(integrator.orderCount(), 2u);

    EXPECT_TRUE(integrator.removeDriver(driver.licenseNumber));
    EXPECT_FALSE(integrator.hasDriver(driver.licenseNumber));
    EXPECT_EQ(integrator.orderCount(), 0u);
    EXPECT_FALSE(integrator.hasOrder(o1));
    EXPECT_FALSE(integrator.hasOrder(o2));
}

void IntegratorMaintainsIndicesAfterOrderRemoval::TestBody() {
    DataIntegrator integrator;
    DriverRecord driver{"TK-25-555555-2025", "Sokolov Petr", "VW", 0};
    OrderRecord o1{driver.licenseNumber, "Street 1", "100 r.", "01 jan 2025"};
    OrderRecord o2{driver.licenseNumber, "Street 2", "200 r.", "02 jan 2025"};
    OrderRecord o3{driver.licenseNumber, "Street 3", "300 r.", "03 jan 2025"};

    ASSERT_TRUE(integrator.addDriver(driver));
    ASSERT_TRUE(integrator.addOrder(o1));
    ASSERT_TRUE(integrator.addOrder(o2));
    ASSERT_TRUE(integrator.addOrder(o3));

    EXPECT_TRUE(integrator.removeOrder(o2));
    EXPECT_FALSE(integrator.hasOrder(o2));
    auto orders = integrator.ordersForDriver(driver.licenseNumber);
    ASSERT_EQ(orders.size(), 2u);
    EXPECT_EQ(orders[0].address, o1.address);
    EXPECT_EQ(orders[1].address, o3.address);
}

void IntegratorUpdatesOrderKey::TestBody() {
    DataIntegrator integrator;
    DriverRecord driver{"TK-25-666666-2025", "Alexeeva Olga", "Kia", 0};
    OrderRecord original{driver.licenseNumber, "Old Street", "150 r.", "01 mar 2025"};
    OrderRecord updated{driver.licenseNumber, "New Street", "155 r.", "02 mar 2025"};

    ASSERT_TRUE(integrator.addDriver(driver));
    ASSERT_TRUE(integrator.addOrder(original));

    EXPECT_TRUE(integrator.updateOrder(original, updated));
    EXPECT_FALSE(integrator.hasOrder(original));
    EXPECT_TRUE(integrator.hasOrder(updated));
    auto orders = integrator.ordersForDriver(driver.licenseNumber);
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].address, updated.address);
    EXPECT_EQ(orders[0].cost, updated.cost);
}

void IntegratorUpdateDriverKeepsData::TestBody() {
    DataIntegrator integrator;
    DriverRecord driver{"TK-25-777777-2025", "Smirnov Ilya", "Ford", 0};
    ASSERT_TRUE(integrator.addDriver(driver));

    DriverRecord updated = driver;
    updated.carBrand = "Tesla";
    EXPECT_TRUE(integrator.updateDriver(driver.licenseNumber, updated));
    auto stored = integrator.findDriver(driver.licenseNumber);
    ASSERT_TRUE(stored.has_value());
    EXPECT_EQ(stored->carBrand, "Tesla");
}

} // namespace

void RegisterIntegratorTests() {
    ::testing::RegisterTest("DataIntegratorTest", "IntegratorAddsDriverAndOrder", nullptr, nullptr, __FILE__, __LINE__,
        []() -> ::testing::Test* { return new IntegratorAddsDriverAndOrder; });
    ::testing::RegisterTest("DataIntegratorTest", "IntegratorRejectsOrderWithoutDriver", nullptr, nullptr, __FILE__, __LINE__,
        []() -> ::testing::Test* { return new IntegratorRejectsOrderWithoutDriver; });
    ::testing::RegisterTest("DataIntegratorTest", "IntegratorCascadesDriverRemoval", nullptr, nullptr, __FILE__, __LINE__,
        []() -> ::testing::Test* { return new IntegratorCascadesDriverRemoval; });
    ::testing::RegisterTest("DataIntegratorTest", "IntegratorMaintainsIndicesAfterOrderRemoval", nullptr, nullptr, __FILE__, __LINE__,
        []() -> ::testing::Test* { return new IntegratorMaintainsIndicesAfterOrderRemoval; });
    ::testing::RegisterTest("DataIntegratorTest", "IntegratorUpdatesOrderKey", nullptr, nullptr, __FILE__, __LINE__,
        []() -> ::testing::Test* { return new IntegratorUpdatesOrderKey; });
    ::testing::RegisterTest("DataIntegratorTest", "IntegratorUpdateDriverKeepsData", nullptr, nullptr, __FILE__, __LINE__,
        []() -> ::testing::Test* { return new IntegratorUpdateDriverKeepsData; });
}
