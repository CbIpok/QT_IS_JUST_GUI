#include <algorithm>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

#include "data_integrator.hpp"
#include "date_utils.hpp"

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

OrderRecord MakeOrder(const std::string& license,
                      const std::string& address,
                      const std::string& cost,
                      const std::string& dateText) {
    OrderRecord record{};
    record.licenseNumber = license;
    record.address = address;
    record.cost = cost;
    if (!parseDate(dateText, record.date)) {
        throw std::runtime_error("Invalid date in test data");
    }
    return record;
}

}  // namespace

TEST(DataIntegratorUseCasesTest, UseCase01_IntegratorStartsEmpty) {
    DataIntegrator integrator;

    EXPECT_EQ(integrator.driverCount(), 0u);
    EXPECT_EQ(integrator.orderCount(), 0u);
    EXPECT_FALSE(integrator.hasDriverTable());
    EXPECT_FALSE(integrator.hasOrderTree());
    EXPECT_FALSE(integrator.hasDriver("ANY-DRIVER"));
    EXPECT_TRUE(integrator.ordersForDriver("ANY-DRIVER").empty());
}

TEST(DataIntegratorUseCasesTest, UseCase02_AddNovikovaDriver) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    DriverRecord driver{"TK-25-111111-2023", "Novikova Daria", "BMW", 10};

    EXPECT_TRUE(integrator.addDriver(driver));
    EXPECT_EQ(integrator.driverCount(), 1u);
    EXPECT_EQ(integrator.orderCount(), 0u);

    auto stored = integrator.findDriver(driver.licenseNumber);
    ASSERT_TRUE(stored.has_value());
    EXPECT_EQ(*stored, driver);

    auto storedByRecord = integrator.findDriver(driver);
    ASSERT_TRUE(storedByRecord.has_value());
    EXPECT_EQ(storedByRecord->fio, driver.fio);
}

TEST(DataIntegratorUseCasesTest, UseCase03_DuplicateDriverRejected) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    DriverRecord driver{"TK-25-111111-2023", "Novikova Daria", "BMW", 10};
    ASSERT_TRUE(integrator.addDriver(driver));

    DriverRecord duplicate = driver;
    duplicate.fio = "Changed Name";
    duplicate.carBrand = "Audi";
    EXPECT_FALSE(integrator.addDriver(duplicate));
    EXPECT_EQ(integrator.driverCount(), 1u);
}

TEST(DataIntegratorUseCasesTest, UseCase04_LoadBasicAndUpdateDriver) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.cfg").string()));
    EXPECT_TRUE(integrator.hasDriverTable());
    EXPECT_TRUE(integrator.hasOrderTree());

    auto record = integrator.findDriver("VB-100");
    ASSERT_TRUE(record.has_value());

    DriverRecord original = *record;
    DriverRecord updated = original;
    updated.carBrand = "UpdatedBrand";
    updated.originalLine = 15;

    EXPECT_TRUE(integrator.updateDriver(original, updated));

    auto stored = integrator.findDriver("VB-100");
    ASSERT_TRUE(stored.has_value());
    EXPECT_EQ(stored->carBrand, "UpdatedBrand");
    EXPECT_EQ(stored->originalLine, 15);
}

TEST(DataIntegratorUseCasesTest, UseCase05_DeleteDriverRemovesOrders) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    ASSERT_TRUE(integrator.createOrderTree());
    DriverRecord driver{"TK-25-444444-2025", "Melnikov Igor", "Audi", 5};
    OrderRecord orderA = MakeOrder(driver.licenseNumber, "Ul. Mira", "400 r.", "10 feb 2025");
    OrderRecord orderB = MakeOrder(driver.licenseNumber, "Ul. Lenina", "600 r.", "12 feb 2025");

    ASSERT_TRUE(integrator.addDriver(driver));
    ASSERT_TRUE(integrator.addOrder(orderA));
    ASSERT_TRUE(integrator.addOrder(orderB));
    EXPECT_EQ(integrator.orderCount(), 2u);

    EXPECT_TRUE(integrator.removeDriver(driver));
    EXPECT_EQ(integrator.driverCount(), 0u);
    EXPECT_EQ(integrator.orderCount(), 0u);
    EXPECT_FALSE(integrator.hasDriver(driver.licenseNumber));
    EXPECT_FALSE(integrator.hasOrder(orderA));
    EXPECT_FALSE(integrator.hasOrder(orderB));
    EXPECT_TRUE(integrator.ordersForDriver(driver.licenseNumber).empty());
}

TEST(DataIntegratorUseCasesTest, UseCase06_AddOrderForExistingDriver) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    ASSERT_TRUE(integrator.createOrderTree());
    DriverRecord driver{"TK-25-111111-2023", "Novikova Daria", "BMW", 10};
    OrderRecord order = MakeOrder(driver.licenseNumber, "Ul. Lesnaya", "300 r.", "02 jan 2025");

    ASSERT_TRUE(integrator.addDriver(driver));
    EXPECT_TRUE(integrator.addOrder(order));
    EXPECT_EQ(integrator.orderCount(), 1u);

    auto orders = integrator.ordersForDriver(driver.licenseNumber);
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders.front(), order);
}

TEST(DataIntegratorUseCasesTest, UseCase07_AddOrderFailsWithoutDriver) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    ASSERT_TRUE(integrator.createOrderTree());
    OrderRecord order = MakeOrder("TK-25-333333-2025", "Ul. Mira", "500 r.", "05 feb 2025");

    EXPECT_FALSE(integrator.addOrder(order));
    EXPECT_EQ(integrator.orderCount(), 0u);
}

TEST(DataIntegratorUseCasesTest, UseCase08_EditOrderFields) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    ASSERT_TRUE(integrator.createOrderTree());
    DriverRecord driver{"TK-25-666666-2025", "Alexeeva Olga", "Kia", 0};
    OrderRecord original = MakeOrder(driver.licenseNumber, "Old Street", "150 r.", "01 mar 2025");
    OrderRecord updated = MakeOrder(driver.licenseNumber, "New Street", "155 r.", "02 mar 2025");

    ASSERT_TRUE(integrator.addDriver(driver));
    ASSERT_TRUE(integrator.addOrder(original));

    EXPECT_TRUE(integrator.updateOrder(original, updated));
    EXPECT_EQ(integrator.orderCount(), 1u);
    EXPECT_FALSE(integrator.hasOrder(original));
    EXPECT_TRUE(integrator.hasOrder(updated));
}

TEST(DataIntegratorUseCasesTest, UseCase09_ReassignOrderBetweenDrivers) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(32));
    ASSERT_TRUE(integrator.createOrderTree());
    DriverRecord driverA{"DL-A", "Driver A", "Brand A", 1};
    DriverRecord driverB{"DL-B", "Driver B", "Brand B", 2};
    OrderRecord original = MakeOrder(driverA.licenseNumber, "Main Square", "100", "2025-06-01");

    ASSERT_TRUE(integrator.addDriver(driverA));
    ASSERT_TRUE(integrator.addDriver(driverB));
    ASSERT_TRUE(integrator.addOrder(original));

    OrderRecord moved = MakeOrder(driverB.licenseNumber, "Main Square", "100", "2025-06-01");
    EXPECT_TRUE(integrator.updateOrder(original, moved));
    EXPECT_EQ(integrator.orderCount(), 1u);

    EXPECT_TRUE(integrator.ordersForDriver(driverA.licenseNumber).empty());
    auto ordersB = integrator.ordersForDriver(driverB.licenseNumber);
    ASSERT_EQ(ordersB.size(), 1u);
    EXPECT_EQ(ordersB.front(), moved);
}

TEST(DataIntegratorUseCasesTest, UseCase10_DeleteMiddleOrder) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    ASSERT_TRUE(integrator.createOrderTree());
    DriverRecord driver{"TK-25-555555-2025", "Sokolov Petr", "VW", 0};
    OrderRecord order1 = MakeOrder(driver.licenseNumber, "Street 1", "100 r.", "01 jan 2025");
    OrderRecord order2 = MakeOrder(driver.licenseNumber, "Street 2", "200 r.", "02 jan 2025");
    OrderRecord order3 = MakeOrder(driver.licenseNumber, "Street 3", "300 r.", "03 jan 2025");

    ASSERT_TRUE(integrator.addDriver(driver));
    ASSERT_TRUE(integrator.addOrder(order1));
    ASSERT_TRUE(integrator.addOrder(order2));
    ASSERT_TRUE(integrator.addOrder(order3));

    EXPECT_TRUE(integrator.removeOrder(order2));
    EXPECT_EQ(integrator.orderCount(), 2u);
    EXPECT_FALSE(integrator.hasOrder(order2));
    EXPECT_TRUE(integrator.hasOrder(order1));
    EXPECT_TRUE(integrator.hasOrder(order3));
}

TEST(DataIntegratorUseCasesTest, UseCase11_LoadMultipleAndInspectVM200) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_multiple.cfg").string()));
    EXPECT_TRUE(integrator.hasDriverTable());
    EXPECT_TRUE(integrator.hasOrderTree());

    auto orders = integrator.ordersForDriver("VM-200");
    ASSERT_EQ(orders.size(), 3u);
    EXPECT_EQ(orders[0].address, "Multiple Hub 1A");
    EXPECT_EQ(orders[1].address, "Multiple Hub 1B");
    EXPECT_EQ(orders[2].address, "Multiple Hub 1C");
}

TEST(DataIntegratorUseCasesTest, UseCase12_CreateAndClearStructures) {
    DataIntegrator integrator;

    ASSERT_TRUE(integrator.createDriverTable(8));
    ASSERT_TRUE(integrator.createOrderTree());
    EXPECT_TRUE(integrator.hasDriverTable());
    EXPECT_TRUE(integrator.hasOrderTree());

    DriverRecord driver{"REP-100", "Reporter User", "Skoda", 0};
    OrderRecord order = MakeOrder(driver.licenseNumber, "Lenina 10", "700", "2025-04-01");
    ASSERT_TRUE(integrator.addDriver(driver));
    ASSERT_TRUE(integrator.addOrder(order));

    integrator.clearDriverTable();
    EXPECT_FALSE(integrator.hasDriverTable());
    EXPECT_EQ(integrator.driverCount(), 0u);
    EXPECT_EQ(integrator.orderCount(), 1u);

    integrator.clearOrderTree();
    EXPECT_FALSE(integrator.hasOrderTree());
    EXPECT_EQ(integrator.orderCount(), 0u);
}

TEST(DataIntegratorUseCasesTest, UseCase13_GenerateReportByCriteria) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    ASSERT_TRUE(integrator.createOrderTree());

    DriverRecord driver{"REP-200", "Ivanov Petr", "Toyota", 5};
    ASSERT_TRUE(integrator.addDriver(driver));

    OrderRecord matchOrder = MakeOrder(driver.licenseNumber, "Central Square", "800", "2025-02-15");
    OrderRecord laterOrder = MakeOrder(driver.licenseNumber, "Central Square", "850", "2025-03-20");
    OrderRecord otherAddress = MakeOrder(driver.licenseNumber, "Side Street", "400", "2025-02-10");
    ASSERT_TRUE(integrator.addOrder(matchOrder));
    ASSERT_TRUE(integrator.addOrder(laterOrder));
    ASSERT_TRUE(integrator.addOrder(otherAddress));

    auto results = integrator.generateReport(driver.licenseNumber,
                                             driver.carBrand,
                                             matchOrder.address,
                                             "2025-02-01",
                                             "2025-02-28");
    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results.front().address, matchOrder.address);
    EXPECT_EQ(results.front().date, formatDateDisplay(matchOrder.date));

    std::string formatted = integrator.formatReport(results);
    EXPECT_NE(formatted.find(driver.fio), std::string::npos);
    EXPECT_NE(formatted.find(formatDateDisplay(matchOrder.date)), std::string::npos);

    auto none = integrator.generateReport(driver.licenseNumber,
                                          "IncorrectBrand",
                                          matchOrder.address,
                                          "2025-02-01",
                                          "2025-02-28");
    EXPECT_TRUE(none.empty());

    auto outsideRange = integrator.generateReport(driver.licenseNumber,
                                                  driver.carBrand,
                                                  matchOrder.address,
                                                  "2025-03-01",
                                                  "2025-03-30");
    ASSERT_EQ(outsideRange.size(), 1u);
    EXPECT_EQ(outsideRange.front().date, formatDateDisplay(laterOrder.date));
}

TEST(DataIntegratorUseCasesTest, UseCase12_LoadMultipleDriversWithoutOrders) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_multiple.cfg").string()));

    EXPECT_TRUE(integrator.ordersForDriver("VM-247").empty());
    EXPECT_TRUE(integrator.ordersForDriver("VM-248").empty());
    EXPECT_TRUE(integrator.ordersForDriver("VM-249").empty());
}

TEST(DataIntegratorUseCasesTest, UseCase13_FilterByLicenseNumber) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.cfg").string()));

    auto driver = integrator.findDriver("VB-100");
    ASSERT_TRUE(driver.has_value());
    EXPECT_EQ(driver->fio, "Basic Driver 1");
    EXPECT_EQ(driver->carBrand, "Brand 1");
    EXPECT_EQ(driver->originalLine, 1);

    auto orders = integrator.ordersForDriver("VB-100");
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders.front().address, "Basic Street 1");
    EXPECT_EQ(orders.front().cost, "1000");
    EXPECT_EQ(formatDateStorage(orders.front().date), "2024-12-01");
}

TEST(DataIntegratorUseCasesTest, UseCase14_FilterByFio) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.cfg").string()));

    auto driver = integrator.findDriver("VB-149");
    ASSERT_TRUE(driver.has_value());
    EXPECT_EQ(driver->fio, "Basic Driver 50");
}

TEST(DataIntegratorUseCasesTest, UseCase15_FilterByCarBrand) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.cfg").string()));

    auto driverA = integrator.findDriver("VB-104");
    auto driverB = integrator.findDriver("VB-149");
    ASSERT_TRUE(driverA.has_value());
    ASSERT_TRUE(driverB.has_value());
    EXPECT_EQ(driverA->carBrand, "Brand 5");
    EXPECT_EQ(driverB->carBrand, "Brand 5");
}

TEST(DataIntegratorUseCasesTest, UseCase16_FilterByOriginalLine) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.cfg").string()));

    auto driver = integrator.findDriver("VB-100");
    ASSERT_TRUE(driver.has_value());
    EXPECT_EQ(driver->originalLine, 1);
}

TEST(DataIntegratorUseCasesTest, UseCase17_FilterOrdersByDriverLicense) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.cfg").string()));

    auto orders = integrator.ordersForDriver("VB-149");
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders.front().address, "Basic Street 50");
    EXPECT_EQ(orders.front().cost, "1490");
    EXPECT_EQ(formatDateStorage(orders.front().date), "2024-12-50");
}

TEST(DataIntegratorUseCasesTest, UseCase18_FilterOrdersByAddress) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_multiple.cfg").string()));

    OrderRecord order = MakeOrder("VM-201", "Multiple Hub 2A", "2328", "2025-01-04");
    EXPECT_TRUE(integrator.hasOrder(order));

    OrderRecord paired = MakeOrder("VM-201", "Multiple Hub 2B", "2335", "2025-01-05");
    EXPECT_TRUE(integrator.hasOrder(paired));
}

TEST(DataIntegratorUseCasesTest, UseCase19_FilterOrdersByCost) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.cfg").string()));

    OrderRecord order = MakeOrder("VB-149", "Basic Street 50", "1490", "2024-12-50");
    EXPECT_TRUE(integrator.hasOrder(order));
}

TEST(DataIntegratorUseCasesTest, UseCase20_FilterOrdersByDate) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.cfg").string()));

    OrderRecord order = MakeOrder("VB-100", "Basic Street 1", "1000", "2024-12-01");
    EXPECT_TRUE(integrator.hasOrder(order));
}

TEST(DataIntegratorUseCasesTest, UseCase21_UpdateOrderAndAddNewForDriver) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.cfg").string()));

    auto driver = integrator.findDriver("VB-100");
    ASSERT_TRUE(driver.has_value());

    DriverRecord originalDriver = *driver;
    DriverRecord updatedDriver = originalDriver;
    updatedDriver.carBrand = "UpdatedBrand";
    updatedDriver.originalLine = 15;
    EXPECT_TRUE(integrator.updateDriver(originalDriver, updatedDriver));

    OrderRecord originalOrder = MakeOrder("VB-100", "Basic Street 1", "1000", "2024-12-01");
    OrderRecord modified = MakeOrder("VB-100", "Prospekt Mira 10", "2700", "2024-12-01");
    EXPECT_TRUE(integrator.updateOrder(originalOrder, modified));

    OrderRecord additional = MakeOrder("VB-100", "Tverskaya 5", "3100", "2025-01-15");
    EXPECT_TRUE(integrator.addOrder(additional));

    auto stored = integrator.findDriver("VB-100");
    ASSERT_TRUE(stored.has_value());
    EXPECT_EQ(stored->carBrand, "UpdatedBrand");
    EXPECT_EQ(stored->originalLine, 15);

    auto orders = integrator.ordersForDriver("VB-100");
    ASSERT_EQ(orders.size(), 2u);
    EXPECT_EQ(integrator.orderCount(), 51u);
    EXPECT_TRUE(std::find(orders.begin(), orders.end(), modified) != orders.end());
    EXPECT_TRUE(std::find(orders.begin(), orders.end(), additional) != orders.end());
}

TEST(DataIntegratorUseCasesTest, UseCase22_SaveTwoDriversAndOrders) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    ASSERT_TRUE(integrator.createOrderTree());
    DriverRecord driverA{"DL-001", "Alpha Tester", "Tesla", 1};
    DriverRecord driverB{"DL-002", "Beta Tester", "BMW", 2};
    OrderRecord orderA = MakeOrder(driverA.licenseNumber, "Street 7", "100", "2024-12-31");
    OrderRecord orderB = MakeOrder(driverB.licenseNumber, "Street 8", "200", "2025-01-01");

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
        "drivers 2\n"
        "DL-001|Alpha Tester|Tesla|1\n"
        "DL-002|Beta Tester|BMW|2\n"
        "orders 2\n"
        "DL-001|Street 7|100|2024-12-31\n"
        "DL-002|Street 8|200|2025-01-01\n";

    EXPECT_EQ(buffer.str(), expected);

    RemoveIfExists(tempPath);
}

TEST(DataIntegratorUseCasesTest, UseCase23_SaveEditsAndReload) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.cfg").string()));

    DriverRecord driver{"VB-100", "Basic Driver 1", "Brand 1", 1};
    DriverRecord updated{"VB-100", "Basic Driver 1", "UpdatedBrand", 15};
    EXPECT_TRUE(integrator.updateDriver(driver, updated));

    OrderRecord originalOrder = MakeOrder(driver.licenseNumber, "Basic Street 1", "1000", "2024-12-01");
    OrderRecord modifiedOrder = MakeOrder(driver.licenseNumber, "Prospekt Mira 10", "2700", "2024-12-01");
    EXPECT_TRUE(integrator.updateOrder(originalOrder, modifiedOrder));

    OrderRecord additional = MakeOrder(driver.licenseNumber, "Tverskaya 5", "3100", "2025-01-15");
    EXPECT_TRUE(integrator.addOrder(additional));

    auto tempPath = TempFilePathForCurrentTest();
    RemoveIfExists(tempPath);
    ASSERT_TRUE(integrator.saveToFile(tempPath.string()));

    DataIntegrator reloaded;
    ASSERT_TRUE(reloaded.loadFromFile(tempPath.string()));

    auto storedDriver = reloaded.findDriver(driver.licenseNumber);
    ASSERT_TRUE(storedDriver.has_value());
    EXPECT_EQ(storedDriver->carBrand, "UpdatedBrand");
    EXPECT_EQ(storedDriver->originalLine, 15);

    auto orders = reloaded.ordersForDriver(driver.licenseNumber);
    ASSERT_EQ(orders.size(), 2u);
    EXPECT_TRUE(std::find(orders.begin(), orders.end(), modifiedOrder) != orders.end());
    EXPECT_TRUE(std::find(orders.begin(), orders.end(), additional) != orders.end());

    RemoveIfExists(tempPath);
}

TEST(DataIntegratorUseCasesTest, UseCase24_ClearAfterLoading) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.cfg").string()));

    integrator.clear();

    EXPECT_EQ(integrator.driverCount(), 0u);
    EXPECT_EQ(integrator.orderCount(), 0u);
    EXPECT_FALSE(integrator.hasDriver("VB-100"));
    EXPECT_FALSE(integrator.hasOrder(MakeOrder("VB-100", "Basic Street 1", "1000", "2024-12-01")));
}

TEST(DataIntegratorUseCasesTest, UseCase25_LoadBasicShowsTotals) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.cfg").string()));

    EXPECT_EQ(integrator.driverCount(), 50u);
    EXPECT_EQ(integrator.orderCount(), 50u);

    auto driver = integrator.findDriver("VB-149");
    ASSERT_TRUE(driver.has_value());
    EXPECT_EQ(driver->fio, "Basic Driver 50");
}

TEST(DataIntegratorUseCasesTest, UseCase26_LoadMultipleShowsTotalsAndOrders) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_multiple.cfg").string()));

    EXPECT_EQ(integrator.driverCount(), 50u);
    EXPECT_EQ(integrator.orderCount(), 50u);

    auto orders200 = integrator.ordersForDriver("VM-200");
    auto orders201 = integrator.ordersForDriver("VM-201");
    ASSERT_EQ(orders200.size(), 3u);
    ASSERT_EQ(orders201.size(), 2u);
}

TEST(DataIntegratorUseCasesTest, UseCase27_LoadNoOrdersShowsEmptyOrders) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_no_orders.cfg").string()));

    EXPECT_EQ(integrator.driverCount(), 50u);
    EXPECT_EQ(integrator.orderCount(), 0u);
    EXPECT_TRUE(integrator.hasDriverTable());
    EXPECT_TRUE(integrator.hasOrderTree());
    EXPECT_TRUE(integrator.ordersForDriver("VN-300").empty());
    EXPECT_TRUE(integrator.ordersForDriver("VN-349").empty());
}

namespace {

void PrepareDiagnosticsData(DataIntegrator& integrator) {
    ASSERT_TRUE(integrator.createDriverTable(32));
    ASSERT_TRUE(integrator.createOrderTree());
    DriverRecord driver1{"DL-HASH-1", "Alpha Tester", "Tesla", 1};
    DriverRecord driver2{"DL-HASH-2", "Beta Tester", "Audi", 2};
    DriverRecord driver3{"DL-HASH-3", "Gamma Tester", "BMW", 3};
    ASSERT_TRUE(integrator.addDriver(driver1));
    ASSERT_TRUE(integrator.addDriver(driver2));
    ASSERT_TRUE(integrator.addDriver(driver3));

    ASSERT_TRUE(integrator.addOrder(MakeOrder(driver1.licenseNumber, "Alpha", "100", "2025-04-01")));
    ASSERT_TRUE(integrator.addOrder(MakeOrder(driver2.licenseNumber, "Beta", "200", "2025-04-02")));
    ASSERT_TRUE(integrator.addOrder(MakeOrder(driver3.licenseNumber, "Gamma", "300", "2025-04-03")));
    ASSERT_TRUE(integrator.addOrder(MakeOrder(driver1.licenseNumber, "Alpha Avenue", "400", "2025-04-04")));
}

}  // namespace

TEST(DataIntegratorUseCasesTest, UseCase28_ShowDiagnosticsDumps) {
    DataIntegrator integrator;
    PrepareDiagnosticsData(integrator);

    auto hashDump = integrator.hashTableAsText();
    EXPECT_NE(hashDump.find("Водителей: 3"), std::string::npos);
    EXPECT_NE(hashDump.find("ФИО: Alpha Tester"), std::string::npos);
    EXPECT_NE(hashDump.find("ФИО: Beta Tester"), std::string::npos);
    EXPECT_NE(hashDump.find("ФИО: Gamma Tester"), std::string::npos);

    auto treeDump = integrator.orderTreeAsText();
    EXPECT_NE(treeDump.find("Всего заказов: 4"), std::string::npos);
    EXPECT_NE(treeDump.find("DL-HASH-1"), std::string::npos);
    EXPECT_NE(treeDump.find("Alpha Avenue"), std::string::npos);
    EXPECT_NE(treeDump.find("|--"), std::string::npos);
}

TEST(DataIntegratorUseCasesTest, UseCase29_SaveDiagnosticsSeparately) {
    DataIntegrator integrator;
    PrepareDiagnosticsData(integrator);

    auto hashDump = integrator.hashTableAsText();
    auto treeDump = integrator.orderTreeAsText();

    auto base = TempFilePathForCurrentTest();
    auto hashPath = base;
    hashPath += ".hash";
    auto treePath = base;
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

    RemoveIfExists(hashPath);
    RemoveIfExists(treePath);
}

TEST(DataIntegratorUseCasesTest, UseCase30_SaveDiagnosticsBothStructures) {
    DataIntegrator integrator;
    PrepareDiagnosticsData(integrator);

    auto hashDump = integrator.hashTableAsText();
    auto treeDump = integrator.orderTreeAsText();

    auto base = TempFilePathForCurrentTest();
    auto hashPath = base;
    hashPath += ".hash";
    auto treePath = base;
    treePath += ".tree";
    RemoveIfExists(hashPath);
    RemoveIfExists(treePath);

    ASSERT_TRUE(integrator.saveStructures(hashPath.string(), treePath.string()));

    std::ifstream hashInput(hashPath);
    std::ifstream treeInput(treePath);
    ASSERT_TRUE(hashInput.is_open());
    ASSERT_TRUE(treeInput.is_open());

    std::ostringstream hashBuffer;
    std::ostringstream treeBuffer;
    hashBuffer << hashInput.rdbuf();
    treeBuffer << treeInput.rdbuf();
    EXPECT_EQ(hashBuffer.str(), hashDump);
    EXPECT_EQ(treeBuffer.str(), treeDump);

    RemoveIfExists(hashPath);
    RemoveIfExists(treePath);
}

TEST(DataIntegratorUseCasesTest, UseCase31_LoadInvalidDoesNotOverwrite) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    DriverRecord driver{"SAFE-1", "Safe Driver", "VW", 3};
    ASSERT_TRUE(integrator.addDriver(driver));

    EXPECT_FALSE(integrator.loadFromFile(ConfigPath("invalid_unknown_driver.cfg").string()));

    EXPECT_EQ(integrator.driverCount(), 1u);
    EXPECT_TRUE(integrator.hasDriver("SAFE-1"));
    EXPECT_EQ(integrator.orderCount(), 0u);
}

