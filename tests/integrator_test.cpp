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

OrderRecord MakeOrder(const std::string& license,
                      const std::string& address,
                      const std::string& cost,
                      const std::string& dateText) {
    Date parsed{};
    if (!Date::parse(dateText, parsed)) {
        ADD_FAILURE() << "Не удалось разобрать дату: " << dateText;
        parsed = Date();
    }
    return OrderRecord{license, address, cost, parsed};
}

template <typename T>
bool Contains(const DoublyLinkedList<T>& list, const T& value) {
    bool found = false;
    list.for_each([&](const T& element, std::size_t) {
        if (element == value) {
            found = true;
        }
    });
    return found;
}

}  // namespace

TEST(DataIntegratorTest, IntegratorAddsDriverAndOrder) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    ASSERT_TRUE(integrator.createOrderTree());
    DriverRecord driver{"TK-25-111111-2023", "Novikova Daria", "BMW"};
    OrderRecord order = MakeOrder(driver.licenseNumber, "Ul. Lesnaya", "300 r.", "02 jan 2025");

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

TEST(DataIntegratorTest, IntegratorRejectsOrderWithoutDriver) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    ASSERT_TRUE(integrator.createOrderTree());
    OrderRecord order = MakeOrder("TK-25-333333-2025", "Ul. Mira", "500 r.", "05 feb 2025");
    EXPECT_FALSE(integrator.addOrder(order));
    EXPECT_EQ(integrator.orderCount(), 0u);
}

TEST(DataIntegratorTest, IntegratorCascadesDriverRemoval) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    ASSERT_TRUE(integrator.createOrderTree());
    DriverRecord driver{"TK-25-444444-2025", "Melnikov Igor", "Audi"};
    OrderRecord o1 = MakeOrder(driver.licenseNumber, "Ul. Mira", "400 r.", "10 feb 2025");
    OrderRecord o2 = MakeOrder(driver.licenseNumber, "Ul. Lenina", "600 r.", "12 feb 2025");

    ASSERT_TRUE(integrator.addDriver(driver));
    ASSERT_TRUE(integrator.addOrder(o1));
    ASSERT_TRUE(integrator.addOrder(o2));
    EXPECT_EQ(integrator.orderCount(), 2u);

    EXPECT_TRUE(integrator.removeDriver(driver));
    EXPECT_FALSE(integrator.hasDriver(driver.licenseNumber));
    EXPECT_EQ(integrator.orderCount(), 0u);
    EXPECT_FALSE(integrator.hasOrder(o1));
    EXPECT_FALSE(integrator.hasOrder(o2));
}

TEST(DataIntegratorTest, IntegratorMaintainsIndicesAfterOrderRemoval) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    ASSERT_TRUE(integrator.createOrderTree());
    DriverRecord driver{"TK-25-555555-2025", "Sokolov Petr", "VW"};
    OrderRecord o1 = MakeOrder(driver.licenseNumber, "Street 1", "100 r.", "01 jan 2025");
    OrderRecord o2 = MakeOrder(driver.licenseNumber, "Street 2", "200 r.", "02 jan 2025");
    OrderRecord o3 = MakeOrder(driver.licenseNumber, "Street 3", "300 r.", "03 jan 2025");

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

TEST(DataIntegratorTest, ClearDriverTableRemovesOrdersAndTrees) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    ASSERT_TRUE(integrator.createOrderTree());
    DriverRecord driver{"TK-25-CLR-2025", "Cleanup User", "VW"};
    OrderRecord o1 = MakeOrder(driver.licenseNumber, "Street 1", "100 r.", "01 jan 2025");
    OrderRecord o2 = MakeOrder(driver.licenseNumber, "Street 2", "200 r.", "02 jan 2025");

    ASSERT_TRUE(integrator.addDriver(driver));
    ASSERT_TRUE(integrator.addOrder(o1));
    ASSERT_TRUE(integrator.addOrder(o2));
    EXPECT_EQ(integrator.driverCount(), 1u);
    EXPECT_EQ(integrator.orderCount(), 2u);

    integrator.clearDriverTable();

    EXPECT_FALSE(integrator.hasDriverTable());
    EXPECT_FALSE(integrator.hasOrderTree());
    EXPECT_EQ(integrator.driverCount(), 0u);
    EXPECT_EQ(integrator.orderCount(), 0u);
    EXPECT_FALSE(integrator.hasOrder(o1));
    EXPECT_FALSE(integrator.hasOrder(o2));
    EXPECT_TRUE(integrator.ordersForDriver(driver.licenseNumber).empty());
}

TEST(DataIntegratorTest, IntegratorUpdatesOrderKey) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    ASSERT_TRUE(integrator.createOrderTree());
    DriverRecord driver{"TK-25-666666-2025", "Alexeeva Olga", "Kia"};
    OrderRecord original = MakeOrder(driver.licenseNumber, "Old Street", "150 r.", "01 mar 2025");
    OrderRecord updated = MakeOrder(driver.licenseNumber, "New Street", "155 r.", "02 mar 2025");

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

TEST(DataIntegratorTest, IntegratorUpdateDriverKeepsData) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    DriverRecord driver{"TK-25-777777-2025", "Smirnov Ilya", "Ford"};
    ASSERT_TRUE(integrator.addDriver(driver));

    DriverRecord updated = driver;
    updated.carBrand = "Tesla";
    EXPECT_TRUE(integrator.updateDriver(driver, updated));
    auto stored = integrator.findDriver(driver.licenseNumber);
    ASSERT_TRUE(stored.has_value());
    EXPECT_EQ(stored->carBrand, "Tesla");
}

TEST(DataIntegratorTest, IntegratorLoadsConfigBasic) {
    DataIntegrator integrator;
    auto path = ConfigPath("valid_basic.cfg");
    ASSERT_TRUE(integrator.loadFromFile(path.string()));
    EXPECT_TRUE(integrator.hasDriverTable());
    EXPECT_TRUE(integrator.hasOrderTree());

    EXPECT_EQ(integrator.driverCount(), 50u);
    EXPECT_EQ(integrator.orderCount(), 50u);

    auto firstDriver = integrator.findDriver("VB-100");
    ASSERT_TRUE(firstDriver.has_value());
    EXPECT_EQ(firstDriver->fio, "Basic Driver 1");
    EXPECT_EQ(firstDriver->carBrand, "Brand 1");

    auto firstOrders = integrator.ordersForDriver("VB-100");
    ASSERT_EQ(firstOrders.size(), 1u);
    EXPECT_EQ(firstOrders[0].address, "Basic Street 1");
    EXPECT_EQ(firstOrders[0].cost, "1000");
    EXPECT_EQ(firstOrders[0].date.storageString(), "2024-12-01");

    auto lastDriver = integrator.findDriver("VB-149");
    ASSERT_TRUE(lastDriver.has_value());
    EXPECT_EQ(lastDriver->fio, "Basic Driver 50");
    EXPECT_EQ(lastDriver->carBrand, "Brand 5");

    auto lastOrders = integrator.ordersForDriver("VB-149");
    ASSERT_EQ(lastOrders.size(), 1u);
    EXPECT_EQ(lastOrders[0].address, "Basic Street 50");
    EXPECT_EQ(lastOrders[0].cost, "1490");
}

TEST(DataIntegratorTest, IntegratorLoadsConfigMultiple) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_multiple.cfg").string()));
    EXPECT_TRUE(integrator.hasDriverTable());
    EXPECT_TRUE(integrator.hasOrderTree());

    EXPECT_EQ(integrator.driverCount(), 50u);
    EXPECT_EQ(integrator.orderCount(), 50u);

    auto driver200 = integrator.findDriver("VM-200");
    ASSERT_TRUE(driver200.has_value());
    EXPECT_EQ(driver200->fio, "Multiple Driver 1");
    EXPECT_EQ(driver200->carBrand, "MultiBrand 1");

    auto orders200 = integrator.ordersForDriver("VM-200");
    ASSERT_EQ(orders200.size(), 3u);
    EXPECT_EQ(orders200[0].address, "Multiple Hub 1A");
    EXPECT_EQ(orders200[1].address, "Multiple Hub 1B");
    EXPECT_EQ(orders200[2].address, "Multiple Hub 1C");

    auto driver201 = integrator.findDriver("VM-201");
    ASSERT_TRUE(driver201.has_value());
    EXPECT_EQ(driver201->carBrand, "MultiBrand 2");

    auto orders201 = integrator.ordersForDriver("VM-201");
    ASSERT_EQ(orders201.size(), 2u);
    EXPECT_EQ(orders201[0].address, "Multiple Hub 2A");
    EXPECT_EQ(orders201[1].address, "Multiple Hub 2B");

    EXPECT_TRUE(integrator.ordersForDriver("VM-247").empty());
    EXPECT_TRUE(integrator.ordersForDriver("VM-248").empty());
    EXPECT_TRUE(integrator.ordersForDriver("VM-249").empty());
}

TEST(DataIntegratorTest, IntegratorLoadsConfigNoOrders) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_no_orders.cfg").string()));
    EXPECT_TRUE(integrator.hasDriverTable());
    EXPECT_TRUE(integrator.hasOrderTree());

    EXPECT_EQ(integrator.driverCount(), 50u);
    EXPECT_EQ(integrator.orderCount(), 0u);

    auto driver300 = integrator.findDriver("VN-300");
    ASSERT_TRUE(driver300.has_value());
    EXPECT_EQ(driver300->carBrand, "CalmBrand 1");

    auto driver349 = integrator.findDriver("VN-349");
    ASSERT_TRUE(driver349.has_value());
    EXPECT_EQ(driver349->fio, "NoOrder Driver 50");

    EXPECT_TRUE(integrator.ordersForDriver("VN-300").empty());
    EXPECT_TRUE(integrator.ordersForDriver("VN-349").empty());
}

TEST(DataIntegratorTest, IntegratorSavesToFile) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    ASSERT_TRUE(integrator.createOrderTree());
    DriverRecord first{"DL-001", "Alpha Tester", "Tesla"};
    DriverRecord second{"DL-002", "Beta Tester", "BMW"};
    ASSERT_TRUE(integrator.addDriver(first));
    ASSERT_TRUE(integrator.addDriver(second));

    OrderRecord orderA = MakeOrder(first.licenseNumber, "Street 7", "100", "2024-12-31");
    OrderRecord orderB = MakeOrder(second.licenseNumber, "Street 8", "200", "2025-01-01");
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
        "DL-001|Alpha Tester|Tesla\n"
        "DL-002|Beta Tester|BMW\n"
        "orders 2\n"
        "DL-001|Street 7|100|2024-12-31\n"
        "DL-002|Street 8|200|2025-01-01\n";
    EXPECT_EQ(buffer.str(), expected);

    RemoveIfExists(tempPath);
}

TEST(DataIntegratorTest, IntegratorSavesAndReloadsModifications) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.cfg").string()));
    EXPECT_TRUE(integrator.hasDriverTable());
    EXPECT_TRUE(integrator.hasOrderTree());

    auto driverOpt = integrator.findDriver("VB-100");
    ASSERT_TRUE(driverOpt.has_value());
    DriverRecord originalDriver = *driverOpt;
    DriverRecord updatedDriver = originalDriver;
    updatedDriver.carBrand = "UpdatedBrand";
    EXPECT_TRUE(integrator.updateDriver(originalDriver, updatedDriver));

    auto existingOrders = integrator.ordersForDriver(updatedDriver.licenseNumber);
    ASSERT_EQ(existingOrders.size(), 1u);
    OrderRecord original = existingOrders[0];
    OrderRecord updatedOrder = original;
    updatedOrder.address = "Prospekt Mira 10";
    updatedOrder.cost = "2700";
    EXPECT_TRUE(integrator.updateOrder(original, updatedOrder));

    std::size_t initialOrderCount = integrator.orderCount();
    OrderRecord newOrder = MakeOrder(updatedDriver.licenseNumber, "Tverskaya 5", "3100", "2025-01-15");
    ASSERT_TRUE(integrator.addOrder(newOrder));
    EXPECT_EQ(integrator.orderCount(), initialOrderCount + 1);

    auto tempPath = TempFilePathForCurrentTest();
    RemoveIfExists(tempPath);
    ASSERT_TRUE(integrator.saveToFile(tempPath.string()));

    DataIntegrator reloaded;
    ASSERT_TRUE(reloaded.loadFromFile(tempPath.string()));
    auto reloadedDriver = reloaded.findDriver(updatedDriver.licenseNumber);
    ASSERT_TRUE(reloadedDriver.has_value());
    EXPECT_EQ(reloadedDriver->carBrand, "UpdatedBrand");
    auto reloadedOrders = reloaded.ordersForDriver(updatedDriver.licenseNumber);
    ASSERT_EQ(reloadedOrders.size(), 2u);
    EXPECT_TRUE(Contains(reloadedOrders, updatedOrder));
    EXPECT_TRUE(Contains(reloadedOrders, newOrder));

    RemoveIfExists(tempPath);
}

TEST(DataIntegratorTest, IntegratorDumpsStructuresToText) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(32));
    ASSERT_TRUE(integrator.createOrderTree());
    DriverRecord alpha{"DL-HASH-1", "Alpha Tester", "Tesla"};
    DriverRecord beta{"DL-HASH-2", "Beta Tester", "Audi"};
    DriverRecord gamma{"DL-HASH-3", "Gamma Tester", "BMW"};

    ASSERT_TRUE(integrator.addDriver(alpha));
    ASSERT_TRUE(integrator.addDriver(beta));
    ASSERT_TRUE(integrator.addDriver(gamma));

    OrderRecord orderA = MakeOrder(alpha.licenseNumber, "Alpha Street", "100", "2025-04-01");
    OrderRecord orderB = MakeOrder(beta.licenseNumber, "Beta Street", "200", "2025-04-02");
    OrderRecord orderC = MakeOrder(gamma.licenseNumber, "Gamma Street", "300", "2025-04-03");
    OrderRecord orderD = MakeOrder(alpha.licenseNumber, "Alpha Avenue", "400", "2025-04-04");

    ASSERT_TRUE(integrator.addOrder(orderA));
    ASSERT_TRUE(integrator.addOrder(orderB));
    ASSERT_TRUE(integrator.addOrder(orderC));
    ASSERT_TRUE(integrator.addOrder(orderD));

    auto hashDump = integrator.hashTableAsText();
    EXPECT_NE(hashDump.find("Водителей:"), std::string::npos);
    EXPECT_NE(hashDump.find("ФИО: Alpha Tester"), std::string::npos);
    EXPECT_NE(hashDump.find("Заказов: 2"), std::string::npos);

    auto treeDump = integrator.orderTreeAsText();
    EXPECT_NE(treeDump.find("Всего заказов: 4"), std::string::npos);
    EXPECT_NE(treeDump.find("Alpha Street"), std::string::npos);
    EXPECT_NE(treeDump.find("Alpha Avenue"), std::string::npos);
    EXPECT_NE(treeDump.find("|--"), std::string::npos);

    auto basePath = TempFilePathForCurrentTest();
    auto hashPath = basePath;
    hashPath += ".hash";
    auto treePath = basePath;
    treePath += ".tree";

    RemoveIfExists(hashPath);
    RemoveIfExists(treePath);

    ASSERT_TRUE(integrator.saveStructures(hashPath.string(), ""));
    std::ifstream hashInput(hashPath);
    ASSERT_TRUE(hashInput.is_open());
    std::ostringstream hashBuffer;
    hashBuffer << hashInput.rdbuf();
    EXPECT_EQ(hashBuffer.str(), hashDump);
    hashInput.close();

    ASSERT_TRUE(integrator.saveStructures("", treePath.string()));
    std::ifstream treeInput(treePath);
    ASSERT_TRUE(treeInput.is_open());
    std::ostringstream treeBuffer;
    treeBuffer << treeInput.rdbuf();
    EXPECT_EQ(treeBuffer.str(), treeDump);
    treeInput.close();
}

TEST(DataIntegratorTest, IntegratorRejectsInvalidConfigFile) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    DriverRecord existing{"SAFE-1", "Safe Driver", "VW"};
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
