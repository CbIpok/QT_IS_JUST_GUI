#include <algorithm>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "data_integrator.hpp"

namespace {

std::filesystem::path ConfigPath(const std::string& name) {
    static const std::filesystem::path base = std::filesystem::path(__FILE__).parent_path() / "data" / "integrator";
    return base / name;
}

std::filesystem::path TempFilePathForCurrentTest() {
    const auto* info = ::testing::UnitTest::GetInstance()->current_test_info();
    std::filesystem::path tempDir = std::filesystem::temp_directory_path();
    std::string fileName = "integrator_use_case_";
    if (info && info->name()) {
        fileName += info->name();
    }
    else {
        fileName += "temp";
    }
    fileName += ".tmp";
    return tempDir / fileName;
}

void RemoveIfExists(const std::filesystem::path& path) {
    std::error_code ec;
    std::filesystem::remove(path, ec);
}

}  // namespace

TEST(DataIntegratorUseCasesTest, UseCase01_CreateIntegratorInitializesEmptyState) {
    DataIntegrator integrator;

    EXPECT_EQ(integrator.driverCount(), 0u);
    EXPECT_EQ(integrator.orderCount(), 0u);
    EXPECT_FALSE(integrator.hasDriver("NON-EXISTENT"));
    EXPECT_TRUE(integrator.ordersForDriver("NON-EXISTENT").empty());
}

TEST(DataIntegratorUseCasesTest, UseCase02_AddDriverHandlesUniqueLicenses) {
    DataIntegrator integrator;
    DriverRecord first{"UC-DR-01", "Case Driver", "Brand", 10};

    EXPECT_TRUE(integrator.addDriver(first));
    EXPECT_EQ(integrator.driverCount(), 1u);

    DriverRecord duplicate = first;
    duplicate.fio = "Another Name";
    EXPECT_FALSE(integrator.addDriver(duplicate));
    EXPECT_EQ(integrator.driverCount(), 1u);
}

TEST(DataIntegratorUseCasesTest, UseCase03_UpdateDriverRequiresExistingAndSameLicense) {
    DataIntegrator integrator;
    DriverRecord original{"UC-DR-02", "Update Person", "Ford", 7};

    EXPECT_FALSE(integrator.updateDriver(original.licenseNumber, original));

    ASSERT_TRUE(integrator.addDriver(original));

    DriverRecord updated = original;
    updated.fio = "Updated Person";
    updated.carBrand = "Tesla";
    updated.originalLine = 42;
    EXPECT_TRUE(integrator.updateDriver(original.licenseNumber, updated));

    auto stored = integrator.findDriver(original.licenseNumber);
    ASSERT_TRUE(stored.has_value());
    EXPECT_EQ(*stored, updated);

    DriverRecord wrongLicense = updated;
    wrongLicense.licenseNumber = "UC-DR-02-OTHER";
    EXPECT_FALSE(integrator.updateDriver(original.licenseNumber, wrongLicense));

    stored = integrator.findDriver(original.licenseNumber);
    ASSERT_TRUE(stored.has_value());
    EXPECT_EQ(*stored, updated);
}

TEST(DataIntegratorUseCasesTest, UseCase04_QueryDriverUtilitiesReflectCurrentState) {
    DataIntegrator integrator;
    DriverRecord driver{"UC-DR-03", "Lookup Person", "BMW", 11};

    EXPECT_FALSE(integrator.hasDriver(driver.licenseNumber));
    EXPECT_FALSE(integrator.findDriver(driver.licenseNumber).has_value());
    EXPECT_EQ(integrator.driverCount(), 0u);

    ASSERT_TRUE(integrator.addDriver(driver));
    EXPECT_TRUE(integrator.hasDriver(driver.licenseNumber));
    auto found = integrator.findDriver(driver.licenseNumber);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(*found, driver);
    EXPECT_EQ(integrator.driverCount(), 1u);
}

TEST(DataIntegratorUseCasesTest, UseCase05_RemoveDriverCascadesOrders) {
    DataIntegrator integrator;
    DriverRecord driver{"UC-DR-04", "Cascade Person", "Audi", 3};
    OrderRecord orderA{driver.licenseNumber, "Cascade Street", "400", "2025-02-10"};
    OrderRecord orderB{driver.licenseNumber, "Cascade Avenue", "600", "2025-02-12"};

    ASSERT_TRUE(integrator.addDriver(driver));
    ASSERT_TRUE(integrator.addOrder(orderA));
    ASSERT_TRUE(integrator.addOrder(orderB));
    EXPECT_EQ(integrator.orderCount(), 2u);

    EXPECT_TRUE(integrator.removeDriver(driver.licenseNumber));
    EXPECT_FALSE(integrator.hasDriver(driver.licenseNumber));
    EXPECT_EQ(integrator.orderCount(), 0u);
    EXPECT_FALSE(integrator.hasOrder(orderA));
    EXPECT_FALSE(integrator.hasOrder(orderB));
}

TEST(DataIntegratorUseCasesTest, UseCase06_AddOrderRequiresExistingDriver) {
    DataIntegrator integrator;
    DriverRecord driver{"UC-DR-05", "Order Person", "Toyota", 5};
    OrderRecord valid{driver.licenseNumber, "Order Street", "200", "2025-03-01"};
    OrderRecord invalid{"UC-NO-DRIVER", "Ghost Street", "300", "2025-03-02"};

    ASSERT_TRUE(integrator.addDriver(driver));
    EXPECT_TRUE(integrator.addOrder(valid));
    EXPECT_EQ(integrator.orderCount(), 1u);

    EXPECT_FALSE(integrator.addOrder(invalid));
    EXPECT_EQ(integrator.orderCount(), 1u);
}

TEST(DataIntegratorUseCasesTest, UseCase07_UpdateOrderValidatesKeyAndDriver) {
    DataIntegrator integrator;
    DriverRecord driverA{"UC-DR-06A", "Order Owner", "Nissan", 6};
    DriverRecord driverB{"UC-DR-06B", "Another Owner", "Skoda", 8};
    OrderRecord base{driverA.licenseNumber, "Original Street", "150", "2025-04-01"};

    ASSERT_TRUE(integrator.addDriver(driverA));
    ASSERT_TRUE(integrator.addDriver(driverB));
    ASSERT_TRUE(integrator.addOrder(base));

    OrderRecord nonexistent{driverA.licenseNumber, "Missing", "999", "2025-04-05"};
    OrderRecord updateAttempt = base;
    updateAttempt.cost = "151";
    EXPECT_FALSE(integrator.updateOrder(nonexistent, updateAttempt));

    OrderRecord updated = base;
    updated.address = "Updated Street";
    updated.cost = "175";
    EXPECT_TRUE(integrator.updateOrder(base, updated));
    EXPECT_TRUE(integrator.hasOrder(updated));

    OrderRecord moved = updated;
    moved.licenseNumber = driverB.licenseNumber;
    moved.address = "Driver B Street";
    EXPECT_TRUE(integrator.updateOrder(updated, moved));
    EXPECT_FALSE(integrator.hasOrder(updated));
    EXPECT_TRUE(integrator.hasOrder(moved));
    EXPECT_TRUE(integrator.ordersForDriver(driverA.licenseNumber).empty());
    auto ordersB = integrator.ordersForDriver(driverB.licenseNumber);
    ASSERT_EQ(ordersB.size(), 1u);
    EXPECT_EQ(ordersB[0], moved);

    OrderRecord invalidDriver = moved;
    invalidDriver.licenseNumber = "UC-DR-UNKNOWN";
    EXPECT_FALSE(integrator.updateOrder(moved, invalidDriver));
    EXPECT_TRUE(integrator.hasOrder(moved));
}

TEST(DataIntegratorUseCasesTest, UseCase08_RemoveOrderPurgesStructures) {
    DataIntegrator integrator;
    DriverRecord driver{"UC-DR-07", "Cleanup Owner", "Opel", 9};
    OrderRecord orderA{driver.licenseNumber, "First Street", "90", "2025-05-01"};
    OrderRecord orderB{driver.licenseNumber, "Second Street", "190", "2025-05-02"};

    ASSERT_TRUE(integrator.addDriver(driver));
    ASSERT_TRUE(integrator.addOrder(orderA));
    ASSERT_TRUE(integrator.addOrder(orderB));
    EXPECT_EQ(integrator.orderCount(), 2u);

    EXPECT_TRUE(integrator.removeOrder(orderA));
    EXPECT_FALSE(integrator.hasOrder(orderA));
    EXPECT_EQ(integrator.orderCount(), 1u);
    EXPECT_FALSE(integrator.removeOrder(orderA));
}

TEST(DataIntegratorUseCasesTest, UseCase09_QueryOrderUtilitiesProvideSnapshots) {
    DataIntegrator integrator;
    DriverRecord driver{"UC-DR-08", "Snapshot Owner", "Hyundai", 12};
    OrderRecord orderA{driver.licenseNumber, "Snapshot Street", "250", "2025-06-01"};
    OrderRecord orderB{driver.licenseNumber, "Archive Lane", "350", "2025-06-02"};

    ASSERT_TRUE(integrator.addDriver(driver));
    ASSERT_TRUE(integrator.addOrder(orderA));
    ASSERT_TRUE(integrator.addOrder(orderB));

    EXPECT_TRUE(integrator.hasOrder(orderA));
    EXPECT_TRUE(integrator.hasOrder(orderB));
    EXPECT_EQ(integrator.orderCount(), 2u);

    auto orders = integrator.ordersForDriver(driver.licenseNumber);
    ASSERT_EQ(orders.size(), 2u);
    EXPECT_EQ(orders[0], orderA);
    EXPECT_EQ(orders[1], orderB);

    OrderRecord nonexistent{driver.licenseNumber, "Missing", "0", "2025-06-03"};
    EXPECT_FALSE(integrator.hasOrder(nonexistent));
}

TEST(DataIntegratorUseCasesTest, UseCase10_ClearResetsAllStructures) {
    DataIntegrator integrator;
    DriverRecord driver{"UC-DR-09", "Reset Owner", "Peugeot", 13};
    OrderRecord order{driver.licenseNumber, "Reset Street", "180", "2025-07-01"};

    ASSERT_TRUE(integrator.addDriver(driver));
    ASSERT_TRUE(integrator.addOrder(order));

    integrator.clear();

    EXPECT_EQ(integrator.driverCount(), 0u);
    EXPECT_EQ(integrator.orderCount(), 0u);
    EXPECT_FALSE(integrator.hasDriver(driver.licenseNumber));
    EXPECT_FALSE(integrator.hasOrder(order));
}

TEST(DataIntegratorUseCasesTest, UseCase11_LoadFromFileValidatesAndKeepsStateOnFailure) {
    DataIntegrator integrator;

    auto basicPath = ConfigPath("valid_basic.cfg");
    ASSERT_TRUE(integrator.loadFromFile(basicPath.string()));
    EXPECT_EQ(integrator.driverCount(), 50u);
    EXPECT_EQ(integrator.orderCount(), 50u);

    auto driver = integrator.findDriver("VB-100");
    ASSERT_TRUE(driver.has_value());
    EXPECT_EQ(driver->fio, "Basic Driver 1");

    auto invalidPath = ConfigPath("invalid_unknown_driver.cfg");
    EXPECT_FALSE(integrator.loadFromFile(invalidPath.string()));

    EXPECT_EQ(integrator.driverCount(), 50u);
    EXPECT_EQ(integrator.orderCount(), 50u);
}

TEST(DataIntegratorUseCasesTest, UseCase12_SaveToFileExportsAllData) {
    DataIntegrator integrator;
    DriverRecord driverA{"UC-DR-10A", "Exporter One", "Lada", 14};
    DriverRecord driverB{"UC-DR-10B", "Exporter Two", "Volvo", 15};
    OrderRecord orderA{driverA.licenseNumber, "Export Street", "410", "2025-08-01"};
    OrderRecord orderB{driverB.licenseNumber, "Export Avenue", "510", "2025-08-02"};

    ASSERT_TRUE(integrator.addDriver(driverA));
    ASSERT_TRUE(integrator.addDriver(driverB));
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
        "drivers 2\n" +
        (driverA.licenseNumber + "|" + driverA.fio + "|" + driverA.carBrand + "|" + std::to_string(driverA.originalLine) + "\n") +
        (driverB.licenseNumber + "|" + driverB.fio + "|" + driverB.carBrand + "|" + std::to_string(driverB.originalLine) + "\n") +
        "orders 2\n" +
        (orderA.licenseNumber + "|" + orderA.address + "|" + orderA.cost + "|" + orderA.date + "\n") +
        (orderB.licenseNumber + "|" + orderB.address + "|" + orderB.cost + "|" + orderB.date + "\n");

    EXPECT_EQ(buffer.str(), expected);

    RemoveIfExists(tempPath);
}

TEST(DataIntegratorUseCasesTest, UseCase13_HashTableAsTextProvidesDiagnosticDump) {
    DataIntegrator integrator;
    auto emptyDump = integrator.hashTableAsText();
    EXPECT_NE(emptyDump.find("HashTable dump"), std::string::npos);
    EXPECT_NE(emptyDump.find("(no entries)"), std::string::npos);

    DriverRecord driver{"UC-DR-11", "Diagnostic", "Mazda", 16};
    ASSERT_TRUE(integrator.addDriver(driver));

    auto filledDump = integrator.hashTableAsText();
    EXPECT_NE(filledDump.find(driver.licenseNumber), std::string::npos);
    EXPECT_EQ(filledDump.find("(no entries)"), std::string::npos);
}

TEST(DataIntegratorUseCasesTest, UseCase14_OrderTreeAsTextRendersStructure) {
    DataIntegrator integrator;
    auto emptyDump = integrator.orderTreeAsText();
    EXPECT_NE(emptyDump.find("(empty tree)"), std::string::npos);

    DriverRecord driver{"UC-DR-12", "Tree Owner", "Citroen", 18};
    ASSERT_TRUE(integrator.addDriver(driver));

    OrderRecord orderA{driver.licenseNumber, "Tree Street", "600", "2025-09-01"};
    OrderRecord orderB{driver.licenseNumber, "Tree Avenue", "700", "2025-09-02"};
    OrderRecord orderC{driver.licenseNumber, "Tree Square", "800", "2025-09-03"};

    ASSERT_TRUE(integrator.addOrder(orderA));
    ASSERT_TRUE(integrator.addOrder(orderB));
    ASSERT_TRUE(integrator.addOrder(orderC));

    auto treeDump = integrator.orderTreeAsText();
    EXPECT_NE(treeDump.find(driver.licenseNumber), std::string::npos);
    EXPECT_NE(treeDump.find("|--"), std::string::npos);
    EXPECT_NE(treeDump.find(orderA.address), std::string::npos);
}

TEST(DataIntegratorUseCasesTest, UseCase15_SaveStructuresExportsDumpsOrReportsFailure) {
    DataIntegrator integrator;
    DriverRecord driver{"UC-DR-13", "Structure Owner", "Renault", 19};
    OrderRecord order{driver.licenseNumber, "Structure Street", "900", "2025-10-01"};

    ASSERT_TRUE(integrator.addDriver(driver));
    ASSERT_TRUE(integrator.addOrder(order));

    auto expectedHash = integrator.hashTableAsText();
    auto expectedTree = integrator.orderTreeAsText();

    auto basePath = TempFilePathForCurrentTest();
    auto hashPath = basePath;
    hashPath += ".hash";
    auto treePath = basePath;
    treePath += ".tree";

    RemoveIfExists(hashPath);
    RemoveIfExists(treePath);

    ASSERT_TRUE(integrator.saveStructures(hashPath.string(), treePath.string()));

    std::ifstream hashInput(hashPath);
    ASSERT_TRUE(hashInput.is_open());
    std::ostringstream hashBuffer;
    hashBuffer << hashInput.rdbuf();
    EXPECT_EQ(hashBuffer.str(), expectedHash);

    std::ifstream treeInput(treePath);
    ASSERT_TRUE(treeInput.is_open());
    std::ostringstream treeBuffer;
    treeBuffer << treeInput.rdbuf();
    EXPECT_EQ(treeBuffer.str(), expectedTree);

    RemoveIfExists(hashPath);
    RemoveIfExists(treePath);

    auto missingDir = hashPath.parent_path() / "missing_directory" / "dump.txt";
    EXPECT_FALSE(integrator.saveStructures(missingDir.string(), ""));
}
