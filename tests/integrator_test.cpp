#include <algorithm>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <system_error>
#include <string>

#include "data_integrator.hpp"

namespace {

std::filesystem::path ConfigPath(const std::string& name) {
    static const std::filesystem::path base = std::filesystem::path(__FILE__).parent_path() / "data" / "integrator";
    return base / name;
}

std::filesystem::path TempFilePathForCurrentTest() {
    const auto* info = ::testing::UnitTest::GetInstance()->current_test_info();
    std::filesystem::path tempDir = std::filesystem::temp_directory_path();
    std::string fileName = "cursedarch_";
    if (info && info->name()) {
        fileName += info->name();
    }
    else {
        fileName += "temp";
    }
    fileName += ".cfg";
    return tempDir / fileName;
}

void RemoveIfExists(const std::filesystem::path& path) {
    std::error_code ec;
    std::filesystem::remove(path, ec);
}

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

class IntegratorLoadsConfigBasic : public ::testing::Test {
protected:
    void TestBody() override;
};

class IntegratorLoadsConfigMultiple : public ::testing::Test {
protected:
    void TestBody() override;
};

class IntegratorLoadsConfigNoOrders : public ::testing::Test {
protected:
    void TestBody() override;
};

class IntegratorSavesToFile : public ::testing::Test {
protected:
    void TestBody() override;
};

class IntegratorSavesAndReloadsModifications : public ::testing::Test {
protected:
    void TestBody() override;
};

class IntegratorRejectsInvalidConfigFile : public ::testing::Test {
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


void IntegratorLoadsConfigBasic::TestBody() {
    DataIntegrator integrator;
    auto path = ConfigPath("valid_basic.cfg");
    ASSERT_TRUE(integrator.loadFromFile(path.string()));

    EXPECT_EQ(integrator.driverCount(), 1u);
    EXPECT_EQ(integrator.orderCount(), 1u);

    auto driver = integrator.findDriver("TK-100");
    ASSERT_TRUE(driver.has_value());
    EXPECT_EQ(driver->fio, "Ivanov Petr");
    EXPECT_EQ(driver->carBrand, "Toyota");
    EXPECT_EQ(driver->originalLine, 5);

    auto orders = integrator.ordersForDriver("TK-100");
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].address, "Lenina 1");
    EXPECT_EQ(orders[0].cost, "2500");
    EXPECT_EQ(orders[0].date, "2024-12-01");
}

void IntegratorLoadsConfigMultiple::TestBody() {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_multiple.cfg").string()));

    EXPECT_EQ(integrator.driverCount(), 2u);
    EXPECT_EQ(integrator.orderCount(), 3u);

    auto driver200 = integrator.findDriver("TK-200");
    ASSERT_TRUE(driver200.has_value());
    EXPECT_EQ(driver200->fio, "Sidorov Ivan");
    EXPECT_EQ(driver200->carBrand, "Lada");

    auto driver201 = integrator.findDriver("TK-201");
    ASSERT_TRUE(driver201.has_value());
    EXPECT_EQ(driver201->fio, "Petrova Anna");
    EXPECT_EQ(driver201->carBrand, "Skoda");

    auto orders200 = integrator.ordersForDriver("TK-200");
    ASSERT_EQ(orders200.size(), 2u);
    EXPECT_EQ(orders200[0].address, "Nevsky 10");
    EXPECT_EQ(orders200[1].address, "Sadovaya 33");

    auto orders201 = integrator.ordersForDriver("TK-201");
    ASSERT_EQ(orders201.size(), 1u);
    EXPECT_EQ(orders201[0].address, "Mira 20");
}

void IntegratorLoadsConfigNoOrders::TestBody() {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_no_orders.cfg").string()));

    EXPECT_EQ(integrator.driverCount(), 2u);
    EXPECT_EQ(integrator.orderCount(), 0u);

    auto driver300 = integrator.findDriver("TK-300");
    ASSERT_TRUE(driver300.has_value());
    EXPECT_EQ(driver300->carBrand, "Hyundai");

    auto driver301 = integrator.findDriver("TK-301");
    ASSERT_TRUE(driver301.has_value());
    EXPECT_EQ(driver301->fio, "Semenova Olga");

    EXPECT_TRUE(integrator.ordersForDriver("TK-300").empty());
    EXPECT_TRUE(integrator.ordersForDriver("TK-301").empty());
}

void IntegratorSavesToFile::TestBody() {
    DataIntegrator integrator;
    DriverRecord first{"DL-001", "Alpha Tester", "Tesla", 1};
    DriverRecord second{"DL-002", "Beta Tester", "BMW", 2};
    ASSERT_TRUE(integrator.addDriver(first));
    ASSERT_TRUE(integrator.addDriver(second));

    OrderRecord orderA{first.licenseNumber, "Street 7", "100", "2024-12-31"};
    OrderRecord orderB{second.licenseNumber, "Street 8", "200", "2025-01-01"};
    ASSERT_TRUE(integrator.addOrder(orderA));
    ASSERT_TRUE(integrator.addOrder(orderB));

    auto tempPath = TempFilePathForCurrentTest();
    RemoveIfExists(tempPath);
    ASSERT_TRUE(integrator.saveToFile(tempPath.string()));

    std::ifstream input(tempPath);
    ASSERT_TRUE(input.is_open());
    std::ostringstream buffer;
    buffer << input.rdbuf();
    input.close();

    std::string expected =
        "drivers 2\n"
        "DL-001|Alpha Tester|Tesla|1\n"
        "DL-002|Beta Tester|BMW|2\n"
        "orders 2\n"
        "DL-001|Street 7|100|2024-12-31\n"
        "DL-002|Street 8|200|2025-01-01\n";
    EXPECT_EQ(buffer.str(), expected);

    RemoveIfExists(tempPath);
}

void IntegratorSavesAndReloadsModifications::TestBody() {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.cfg").string()));

    auto driver = integrator.findDriver("TK-100");
    ASSERT_TRUE(driver.has_value());
    driver->carBrand = "UpdatedBrand";
    driver->originalLine = 15;
    EXPECT_TRUE(integrator.updateDriver(driver->licenseNumber, *driver));

    auto existingOrders = integrator.ordersForDriver(driver->licenseNumber);
    ASSERT_EQ(existingOrders.size(), 1u);
    OrderRecord original = existingOrders[0];
    OrderRecord updatedOrder = original;
    updatedOrder.address = "Prospekt Mira 10";
    updatedOrder.cost = "2700";
    EXPECT_TRUE(integrator.updateOrder(original, updatedOrder));

    OrderRecord newOrder{driver->licenseNumber, "Tverskaya 5", "3100", "2025-01-15"};
    ASSERT_TRUE(integrator.addOrder(newOrder));
    EXPECT_EQ(integrator.orderCount(), 2u);

    auto tempPath = TempFilePathForCurrentTest();
    RemoveIfExists(tempPath);
    ASSERT_TRUE(integrator.saveToFile(tempPath.string()));

    DataIntegrator reloaded;
    ASSERT_TRUE(reloaded.loadFromFile(tempPath.string()));
    auto reloadedDriver = reloaded.findDriver(driver->licenseNumber);
    ASSERT_TRUE(reloadedDriver.has_value());
    EXPECT_EQ(reloadedDriver->carBrand, "UpdatedBrand");
    EXPECT_EQ(reloadedDriver->originalLine, 15);

    auto reloadedOrders = reloaded.ordersForDriver(driver->licenseNumber);
    ASSERT_EQ(reloadedOrders.size(), 2u);
    EXPECT_NE(reloadedOrders.end(), std::find(reloadedOrders.begin(), reloadedOrders.end(), updatedOrder));
    EXPECT_NE(reloadedOrders.end(), std::find(reloadedOrders.begin(), reloadedOrders.end(), newOrder));

    RemoveIfExists(tempPath);
}

void IntegratorRejectsInvalidConfigFile::TestBody() {
    DataIntegrator integrator;
    DriverRecord existing{"SAFE-1", "Safe Driver", "VW", 3};
    ASSERT_TRUE(integrator.addDriver(existing));
    EXPECT_EQ(integrator.driverCount(), 1u);

    auto invalidPath = ConfigPath("invalid_unknown_driver.cfg");
    EXPECT_FALSE(integrator.loadFromFile(invalidPath.string()));

    EXPECT_EQ(integrator.driverCount(), 1u);
    EXPECT_EQ(integrator.orderCount(), 0u);
    auto stored = integrator.findDriver(existing.licenseNumber);
    ASSERT_TRUE(stored.has_value());
    EXPECT_EQ(stored->fio, existing.fio);
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
    ::testing::RegisterTest("DataIntegratorTest", "IntegratorLoadsConfigBasic", nullptr, nullptr, __FILE__, __LINE__,
        []() -> ::testing::Test* { return new IntegratorLoadsConfigBasic; });
    ::testing::RegisterTest("DataIntegratorTest", "IntegratorLoadsConfigMultiple", nullptr, nullptr, __FILE__, __LINE__,
        []() -> ::testing::Test* { return new IntegratorLoadsConfigMultiple; });
    ::testing::RegisterTest("DataIntegratorTest", "IntegratorLoadsConfigNoOrders", nullptr, nullptr, __FILE__, __LINE__,
        []() -> ::testing::Test* { return new IntegratorLoadsConfigNoOrders; });
    ::testing::RegisterTest("DataIntegratorTest", "IntegratorSavesToFile", nullptr, nullptr, __FILE__, __LINE__,
        []() -> ::testing::Test* { return new IntegratorSavesToFile; });
    ::testing::RegisterTest("DataIntegratorTest", "IntegratorSavesAndReloadsModifications", nullptr, nullptr, __FILE__, __LINE__,
        []() -> ::testing::Test* { return new IntegratorSavesAndReloadsModifications; });
    ::testing::RegisterTest("DataIntegratorTest", "IntegratorRejectsInvalidConfigFile", nullptr, nullptr, __FILE__, __LINE__,
        []() -> ::testing::Test* { return new IntegratorRejectsInvalidConfigFile; });
}
