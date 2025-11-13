#include <algorithm>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <optional>
#include <sstream>
#include <string>
#include "data_integrator.hpp"

namespace {

constexpr std::size_t kConfigTableSize = 64;

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
    fileName += ".txt";
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

TEST(DataIntegratorUseCasesTest, UseCase01_IntegratorStartsEmpty) {
    DataIntegrator integrator;

    EXPECT_EQ(integrator.driverCount(), 0u);
    EXPECT_EQ(integrator.orderCount(), 0u);
    EXPECT_FALSE(integrator.hasDriverTable());
    EXPECT_FALSE(integrator.hasOrderTree());
    EXPECT_FALSE(integrator.hasDriver("ANYDRIVER1"));
    EXPECT_TRUE(integrator.ordersForDriver("ANYDRIVER1").empty());
}

TEST(DataIntegratorUseCasesTest, UseCase02_AddNovikovaDriver) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    DriverRecord driver{"TK251111112023", "Новикова Дарья Сергеевна", "Audi"};

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
    DriverRecord driver{"TK251111112023", "Новикова Дарья Сергеевна", "Audi"};
    ASSERT_TRUE(integrator.addDriver(driver));

    DriverRecord duplicate = driver;
    duplicate.fio = "Новикова Дарья Павловна";
    duplicate.carBrand = "Skoda";
    EXPECT_FALSE(integrator.addDriver(duplicate));
    EXPECT_EQ(integrator.driverCount(), 1u);
}

TEST(DataIntegratorUseCasesTest, UseCase04_LoadBasicAndUpdateDriver) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.txt").string(), kConfigTableSize));
    EXPECT_TRUE(integrator.hasDriverTable());
    EXPECT_TRUE(integrator.hasOrderTree());

    auto record = integrator.findDriver("VB100");
    ASSERT_TRUE(record.has_value());

    DriverRecord original = *record;
    DriverRecord updated = original;
    updated.carBrand = "Nissan";

    EXPECT_TRUE(integrator.updateDriver(original, updated));

    auto stored = integrator.findDriver("VB100");
    ASSERT_TRUE(stored.has_value());
    EXPECT_EQ(stored->carBrand, "Nissan");
}

TEST(DataIntegratorUseCasesTest, UseCase05_DeleteDriverRemovesOrders) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    ASSERT_TRUE(integrator.createOrderTree());
    DriverRecord driver{"TK254444442025", "Мельников Игорь Петрович", "Skoda"};
    OrderRecord orderA = MakeOrder(driver.licenseNumber, "Улица Мира 12", "400,00", "10 Feb 2025");
    OrderRecord orderB = MakeOrder(driver.licenseNumber, "Улица Ленина 18", "600,00", "12 Feb 2025");

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
    DriverRecord driver{"TK251111112023", "Новикова Дарья Сергеевна", "Audi"};
    OrderRecord order = MakeOrder(driver.licenseNumber, "Улица Лесная 5", "300,00", "02 Jan 2025");

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
    OrderRecord order = MakeOrder("TK253333332025", "Улица Мира 10", "500,00", "05 Feb 2025");

    EXPECT_FALSE(integrator.addOrder(order));
    EXPECT_EQ(integrator.orderCount(), 0u);
}

TEST(DataIntegratorUseCasesTest, UseCase08_EditOrderFields) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    ASSERT_TRUE(integrator.createOrderTree());
    DriverRecord driver{"TK256666662025", "Алексеева Ольга Викторовна", "Kia"};
    OrderRecord original = MakeOrder(driver.licenseNumber, "Улица Старая 7", "150,00", "01 Mar 2025");
    OrderRecord updated = MakeOrder(driver.licenseNumber, "Улица Новая 9", "155,00", "02 Mar 2025");

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
    DriverRecord driverA{"DLA001", "Водитель А Аркадьевич", "Lada"};
    DriverRecord driverB{"DLB002", "Водитель Б Борисович", "Renault"};
    OrderRecord original = MakeOrder(driverA.licenseNumber, "Площадь Центральная 5", "100,00", "01 Jun 2025");

    ASSERT_TRUE(integrator.addDriver(driverA));
    ASSERT_TRUE(integrator.addDriver(driverB));
    ASSERT_TRUE(integrator.addOrder(original));

    OrderRecord moved = MakeOrder(driverB.licenseNumber, "Площадь Центральная 5", "100,00", "01 Jun 2025");
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
    DriverRecord driver{"TK255555552025", "Соколов Пётр Андреевич", "Renault"};
    OrderRecord order1 = MakeOrder(driver.licenseNumber, "Улица Первая 1", "100,00", "01 Jan 2025");
    OrderRecord order2 = MakeOrder(driver.licenseNumber, "Улица Вторая 2", "200,00", "02 Jan 2025");
    OrderRecord order3 = MakeOrder(driver.licenseNumber, "Улица Третья 3", "300,00", "03 Jan 2025");

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
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_multiple.txt").string(), kConfigTableSize));
    EXPECT_TRUE(integrator.hasDriverTable());
    EXPECT_TRUE(integrator.hasOrderTree());

    auto orders = integrator.ordersForDriver("VM200");
    ASSERT_EQ(orders.size(), 3u);
    EXPECT_EQ(orders[0].address, "Площадь Центральная 1А");
    EXPECT_EQ(orders[1].address, "Площадь Центральная 1Б");
    EXPECT_EQ(orders[2].address, "Площадь Центральная 1В");
}

TEST(DataIntegratorUseCasesTest, UseCase12_CreateAndClearStructures) {
    DataIntegrator integrator;

    ASSERT_TRUE(integrator.createDriverTable(8));
    ASSERT_TRUE(integrator.createOrderTree());
    EXPECT_TRUE(integrator.hasDriverTable());
    EXPECT_TRUE(integrator.hasOrderTree());

    DriverRecord driver{"REP100", "Репортёр Антон Львович", "Skoda"};
    OrderRecord order = MakeOrder(driver.licenseNumber, "Улица Ленина 10", "700,00", "01 Apr 2025");
    ASSERT_TRUE(integrator.addDriver(driver));
    ASSERT_TRUE(integrator.addOrder(order));

    integrator.clearDriverTable();
    EXPECT_FALSE(integrator.hasDriverTable());
    EXPECT_FALSE(integrator.hasOrderTree());
    EXPECT_EQ(integrator.driverCount(), 0u);
    EXPECT_EQ(integrator.orderCount(), 0u);
    EXPECT_FALSE(integrator.hasOrder(order));
    EXPECT_TRUE(integrator.ordersForDriver(driver.licenseNumber).empty());

    integrator.clearOrderTree();
    EXPECT_FALSE(integrator.hasOrderTree());
    EXPECT_EQ(integrator.orderCount(), 0u);
}

TEST(DataIntegratorUseCasesTest, UseCase13_GenerateReportByCriteria) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    ASSERT_TRUE(integrator.createOrderTree());

    DriverRecord driver{"REP200", "Иванов Пётр Сергеевич", "Toyota"};
    ASSERT_TRUE(integrator.addDriver(driver));

    OrderRecord matchOrder = MakeOrder(driver.licenseNumber, "Площадь Центральная 10", "800,00", "15 Feb 2025");
    OrderRecord laterOrder = MakeOrder(driver.licenseNumber, "Площадь Центральная 10", "850,00", "20 Mar 2025");
    OrderRecord otherAddress = MakeOrder(driver.licenseNumber, "Боковая Улица 3", "400,00", "10 Feb 2025");
    ASSERT_TRUE(integrator.addOrder(matchOrder));
    ASSERT_TRUE(integrator.addOrder(laterOrder));
    ASSERT_TRUE(integrator.addOrder(otherAddress));

    auto results = integrator.generateReport(driver.licenseNumber,
                                             driver.carBrand,
                                             matchOrder.address,
                                             "01 Feb 2025",
                                             "28 Feb 2025");
    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results.front().address, matchOrder.address);
    EXPECT_EQ(results.front().date, matchOrder.date.displayString());

    std::string formatted = integrator.formatReport(results);
    EXPECT_NE(formatted.find(driver.fio), std::string::npos);
    EXPECT_NE(formatted.find(matchOrder.date.displayString()), std::string::npos);

    auto none = integrator.generateReport(driver.licenseNumber,
                                          "IncorrectBrand",
                                          matchOrder.address,
                                          "01 Feb 2025",
                                          "28 Feb 2025");
    EXPECT_TRUE(none.empty());

    auto outsideRange = integrator.generateReport(driver.licenseNumber,
                                                  driver.carBrand,
                                                  matchOrder.address,
                                                  "01 Mar 2025",
                                                  "30 Mar 2025");
    ASSERT_EQ(outsideRange.size(), 1u);
    EXPECT_EQ(outsideRange.front().date, laterOrder.date.displayString());
}

TEST(DataIntegratorUseCasesTest, UseCase12_LoadMultipleDriversWithoutOrders) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_multiple.txt").string(), kConfigTableSize));

    EXPECT_TRUE(integrator.ordersForDriver("VM247").empty());
    EXPECT_TRUE(integrator.ordersForDriver("VM248").empty());
    EXPECT_TRUE(integrator.ordersForDriver("VM249").empty());
}

TEST(DataIntegratorUseCasesTest, UseCase13_FilterByLicenseNumber) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.txt").string(), kConfigTableSize));

    auto driver = integrator.findDriver("VB100");
    ASSERT_TRUE(driver.has_value());
    EXPECT_EQ(driver->fio, "Смирнова Алина Павловна");
    EXPECT_EQ(driver->carBrand, "Лада");

    auto orders = integrator.ordersForDriver("VB100");
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders.front().address, "Улица Сиреневая 1");
    EXPECT_EQ(orders.front().cost, "1000,00");
    EXPECT_EQ(orders.front().date.displayString(), "01 Dec 2024");
}

TEST(DataIntegratorUseCasesTest, UseCase14_FilterByFio) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.txt").string(), kConfigTableSize));

    auto driver = integrator.findDriver("VB149");
    ASSERT_TRUE(driver.has_value());
    EXPECT_EQ(driver->fio, "Полякова Инга Сергеевна");
}

TEST(DataIntegratorUseCasesTest, UseCase15_FilterByCarBrand) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.txt").string(), kConfigTableSize));

    auto driverA = integrator.findDriver("VB104");
    auto driverB = integrator.findDriver("VB149");
    ASSERT_TRUE(driverA.has_value());
    ASSERT_TRUE(driverB.has_value());
    EXPECT_EQ(driverA->carBrand, "Toyota");
    EXPECT_EQ(driverB->carBrand, "Toyota");
}

TEST(DataIntegratorUseCasesTest, UseCase16_VerifyDriverPresenceAfterLoad) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.txt").string(), kConfigTableSize));

    EXPECT_TRUE(integrator.hasDriver("VB100"));
}

TEST(DataIntegratorUseCasesTest, UseCase17_FilterOrdersByDriverLicense) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.txt").string(), kConfigTableSize));

    auto orders = integrator.ordersForDriver("VB149");
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders.front().address, "Улица Сиреневая 50");
    EXPECT_EQ(orders.front().cost, "1490,00");
    EXPECT_EQ(orders.front().date.displayString(), "19 Jan 2025");
}

TEST(DataIntegratorUseCasesTest, UseCase18_FilterOrdersByAddress) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_multiple.txt").string(), kConfigTableSize));

    OrderRecord order = MakeOrder("VM201", "Площадь Центральная 2А", "2328,00", "04 Jan 2025");
    EXPECT_TRUE(integrator.hasOrder(order));

    OrderRecord paired = MakeOrder("VM201", "Площадь Центральная 2Б", "2335,00", "05 Jan 2025");
    EXPECT_TRUE(integrator.hasOrder(paired));
}

TEST(DataIntegratorUseCasesTest, UseCase19_FilterOrdersByCost) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.txt").string(), kConfigTableSize));

    OrderRecord order = MakeOrder("VB149", "Улица Сиреневая 50", "1490,00", "19 Jan 2025");
    EXPECT_TRUE(integrator.hasOrder(order));
}

TEST(DataIntegratorUseCasesTest, UseCase20_FilterOrdersByDate) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.txt").string(), kConfigTableSize));

    OrderRecord order = MakeOrder("VB100", "Улица Сиреневая 1", "1000,00", "01 Dec 2024");
    EXPECT_TRUE(integrator.hasOrder(order));
}

TEST(DataIntegratorUseCasesTest, UseCase21_UpdateOrderAndAddNewForDriver) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.txt").string(), kConfigTableSize));

    auto driver = integrator.findDriver("VB100");
    ASSERT_TRUE(driver.has_value());

    DriverRecord originalDriver = *driver;
    DriverRecord updatedDriver = originalDriver;
    updatedDriver.carBrand = "Nissan";
    EXPECT_TRUE(integrator.updateDriver(originalDriver, updatedDriver));

    OrderRecord originalOrder = MakeOrder("VB100", "Улица Сиреневая 1", "1000,00", "01 Dec 2024");
    OrderRecord modified = MakeOrder("VB100", "Проспект Мира 10", "2700,00", "01 Dec 2024");
    EXPECT_TRUE(integrator.updateOrder(originalOrder, modified));

    OrderRecord additional = MakeOrder("VB100", "Тверская 5", "3100,00", "15 Jan 2025");
    EXPECT_TRUE(integrator.addOrder(additional));

    auto stored = integrator.findDriver("VB100");
    ASSERT_TRUE(stored.has_value());
    EXPECT_EQ(stored->carBrand, "Nissan");

    auto orders = integrator.ordersForDriver("VB100");
    ASSERT_EQ(orders.size(), 2u);
    EXPECT_EQ(integrator.orderCount(), 51u);
    EXPECT_TRUE(Contains(orders, modified));
    EXPECT_TRUE(Contains(orders, additional));
}

TEST(DataIntegratorUseCasesTest, UseCase22_SaveTwoDriversAndOrders) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    ASSERT_TRUE(integrator.createOrderTree());
    DriverRecord driverA{"DL001", "Испытатель Альфа Сергеевич", "Tesla"};
    DriverRecord driverB{"DL002", "Испытатель Бета Андреевич", "Audi"};
    OrderRecord orderA = MakeOrder(driverA.licenseNumber, "Улица Опытная 7", "100,00", "31 Dec 2024");
    OrderRecord orderB = MakeOrder(driverB.licenseNumber, "Улица Опытная 8", "200,00", "01 Jan 2025");

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
        "DL001|Испытатель Альфа Сергеевич|Tesla\n"
        "DL002|Испытатель Бета Андреевич|Audi\n"
        "orders 2\n"
        "DL001|Улица Опытная 7|100,00|31 Dec 2024\n"
        "DL002|Улица Опытная 8|200,00|01 Jan 2025\n";

    EXPECT_EQ(buffer.str(), expected);

    RemoveIfExists(tempPath);
}

TEST(DataIntegratorUseCasesTest, UseCase23_SaveEditsAndReload) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.txt").string(), kConfigTableSize));

    DriverRecord driver{"VB100", "Смирнова Алина Павловна", "Лада"};
    DriverRecord updated{"VB100", "Смирнова Алина Павловна", "Nissan"};
    EXPECT_TRUE(integrator.updateDriver(driver, updated));

    OrderRecord originalOrder = MakeOrder(driver.licenseNumber, "Улица Сиреневая 1", "1000,00", "01 Dec 2024");
    OrderRecord modifiedOrder = MakeOrder(driver.licenseNumber, "Проспект Мира 10", "2700,00", "01 Dec 2024");
    EXPECT_TRUE(integrator.updateOrder(originalOrder, modifiedOrder));

    OrderRecord additional = MakeOrder(driver.licenseNumber, "Тверская 5", "3100,00", "15 Jan 2025");
    EXPECT_TRUE(integrator.addOrder(additional));

    auto tempPath = TempFilePathForCurrentTest();
    RemoveIfExists(tempPath);
    ASSERT_TRUE(integrator.saveToFile(tempPath.string()));

    DataIntegrator reloaded;
    ASSERT_TRUE(reloaded.loadFromFile(tempPath.string(), kConfigTableSize));

    auto storedDriver = reloaded.findDriver(driver.licenseNumber);
    ASSERT_TRUE(storedDriver.has_value());
    EXPECT_EQ(storedDriver->carBrand, "Nissan");

    auto orders = reloaded.ordersForDriver(driver.licenseNumber);
    ASSERT_EQ(orders.size(), 2u);
    EXPECT_TRUE(Contains(orders, modifiedOrder));
    EXPECT_TRUE(Contains(orders, additional));

    RemoveIfExists(tempPath);
}

TEST(DataIntegratorUseCasesTest, UseCase24_ClearAfterLoading) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.txt").string(), kConfigTableSize));

    integrator.clear();

    EXPECT_EQ(integrator.driverCount(), 0u);
    EXPECT_EQ(integrator.orderCount(), 0u);
    EXPECT_FALSE(integrator.hasDriver("VB100"));
    EXPECT_FALSE(integrator.hasOrder(MakeOrder("VB100", "Улица Сиреневая 1", "1000,00", "01 Dec 2024")));
}

TEST(DataIntegratorUseCasesTest, UseCase25_LoadBasicShowsTotals) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.txt").string(), kConfigTableSize));

    EXPECT_EQ(integrator.driverCount(), 50u);
    EXPECT_EQ(integrator.orderCount(), 50u);

    auto driver = integrator.findDriver("VB149");
    ASSERT_TRUE(driver.has_value());
    EXPECT_EQ(driver->fio, "Полякова Инга Сергеевна");
}

TEST(DataIntegratorUseCasesTest, UseCase26_LoadMultipleShowsTotalsAndOrders) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_multiple.txt").string(), kConfigTableSize));

    EXPECT_EQ(integrator.driverCount(), 50u);
    EXPECT_EQ(integrator.orderCount(), 50u);

    auto orders200 = integrator.ordersForDriver("VM200");
    auto orders201 = integrator.ordersForDriver("VM201");
    ASSERT_EQ(orders200.size(), 3u);
    ASSERT_EQ(orders201.size(), 2u);
}

TEST(DataIntegratorUseCasesTest, UseCase27_LoadNoOrdersShowsEmptyOrders) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_no_orders.txt").string(), kConfigTableSize));

    EXPECT_EQ(integrator.driverCount(), 50u);
    EXPECT_EQ(integrator.orderCount(), 0u);
    EXPECT_TRUE(integrator.hasDriverTable());
    EXPECT_TRUE(integrator.hasOrderTree());
    EXPECT_TRUE(integrator.ordersForDriver("VN300").empty());
    EXPECT_TRUE(integrator.ordersForDriver("VN349").empty());
}

namespace {

void PrepareDiagnosticsData(DataIntegrator& integrator) {
    ASSERT_TRUE(integrator.createDriverTable(32));
    ASSERT_TRUE(integrator.createOrderTree());
    DriverRecord driver1{"DLHASH1", "Испытатель Альфа Сергеевич", "Tesla"};
    DriverRecord driver2{"DLHASH2", "Испытатель Бета Андреевич", "Audi"};
    DriverRecord driver3{"DLHASH3", "Испытатель Гамма Львович", "Bmw"};
    ASSERT_TRUE(integrator.addDriver(driver1));
    ASSERT_TRUE(integrator.addDriver(driver2));
    ASSERT_TRUE(integrator.addDriver(driver3));

    ASSERT_TRUE(integrator.addOrder(MakeOrder(driver1.licenseNumber, "Проспект Альфа 1", "100,00", "01 Apr 2025")));
    ASSERT_TRUE(integrator.addOrder(MakeOrder(driver2.licenseNumber, "Проспект Бета 2", "200,00", "02 Apr 2025")));
    ASSERT_TRUE(integrator.addOrder(MakeOrder(driver3.licenseNumber, "Проспект Гамма 3", "300,00", "03 Apr 2025")));
    ASSERT_TRUE(integrator.addOrder(MakeOrder(driver1.licenseNumber, "Проспект Альфа 4", "400,00", "04 Apr 2025")));
}

}  // namespace

TEST(DataIntegratorUseCasesTest, UseCase28_ShowDiagnosticsDumps) {
    DataIntegrator integrator;
    PrepareDiagnosticsData(integrator);

    auto hashDump = integrator.hashTableAsText();
    EXPECT_NE(hashDump.find("Водителей: 3"), std::string::npos);
    EXPECT_NE(hashDump.find("ФИО: Испытатель Альфа Сергеевич"), std::string::npos);
    EXPECT_NE(hashDump.find("ФИО: Испытатель Бета Андреевич"), std::string::npos);
    EXPECT_NE(hashDump.find("ФИО: Испытатель Гамма Львович"), std::string::npos);

    auto treeDump = integrator.orderTreeAsText();
    EXPECT_NE(treeDump.find("Всего заказов: 4"), std::string::npos);
    EXPECT_NE(treeDump.find("DLHASH1"), std::string::npos);
    EXPECT_NE(treeDump.find("Проспект Альфа 4"), std::string::npos);
    EXPECT_NE(treeDump.find("|--"), std::string::npos);
}

TEST(DataIntegratorUseCasesTest, UseCase29_SaveDiagnosticsSeparately) {
    DataIntegrator integrator;
    PrepareDiagnosticsData(integrator);

    auto hashDump = integrator.hashTableAsText();
    auto treeDump = integrator.orderTreeAsText();

    auto base = TempFilePathForCurrentTest();
    auto baseDir = base.parent_path();
    std::string stem = base.stem().string();
    auto hashPath = baseDir / (stem + "_hash.txt");
    auto treePath = baseDir / (stem + "_tree.txt");
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
    auto baseDir = base.parent_path();
    std::string stem = base.stem().string();
    auto hashPath = baseDir / (stem + "_hash.txt");
    auto treePath = baseDir / (stem + "_tree.txt");
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
    DriverRecord driver{"SAFE-1", "Safe Driver", "VW"};
    ASSERT_TRUE(integrator.addDriver(driver));

    EXPECT_FALSE(integrator.loadFromFile(ConfigPath("invalid_unknown_driver.txt").string(), kConfigTableSize));

    EXPECT_EQ(integrator.driverCount(), 1u);
    EXPECT_TRUE(integrator.hasDriver("SAFE-1"));
    EXPECT_EQ(integrator.orderCount(), 0u);
}

