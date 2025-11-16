#include <algorithm>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <system_error>
#include <string>

#include "data_integrator.hpp"
#include "string_utils.hpp"

namespace {

constexpr std::size_t kConfigTableSize = 64;

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
    double parsedCost = 0.0;
    if (!string_utils::parseCost(cost, parsedCost)) {
        ADD_FAILURE() << "Не удалось преобразовать стоимость: " << cost;
        parsedCost = 0.0;
    }
    return OrderRecord{license, address, parsedCost, parsed};
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
    DriverRecord driver{"TK251111112023", "Новикова Дарья Сергеевна", "Audi"};
    OrderRecord order = MakeOrder(driver.licenseNumber, "Улица Лесная 5", "300,00", "02 Jan 2025");

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
    OrderRecord order = MakeOrder("TK253333332025", "Улица Мира 10", "500,00", "05 Feb 2025");
    EXPECT_FALSE(integrator.addOrder(order));
    EXPECT_EQ(integrator.orderCount(), 0u);
}

TEST(DataIntegratorTest, IntegratorCascadesDriverRemoval) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    ASSERT_TRUE(integrator.createOrderTree());
    DriverRecord driver{"TK254444442025", "Мельников Игорь Петрович", "Skoda"};
    OrderRecord o1 = MakeOrder(driver.licenseNumber, "Улица Мира 12", "400,00", "10 Feb 2025");
    OrderRecord o2 = MakeOrder(driver.licenseNumber, "Улица Ленина 18", "600,00", "12 Feb 2025");

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
    DriverRecord driver{"TK255555552025", "Соколов Пётр Андреевич", "Renault"};
    OrderRecord o1 = MakeOrder(driver.licenseNumber, "Улица Первая 1", "100,00", "01 Jan 2025");
    OrderRecord o2 = MakeOrder(driver.licenseNumber, "Улица Вторая 2", "200,00", "02 Jan 2025");
    OrderRecord o3 = MakeOrder(driver.licenseNumber, "Улица Третья 3", "300,00", "03 Jan 2025");

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
    DriverRecord driver{"TK25CLR2025", "Чистяков Артём Валерьевич", "Renault"};
    OrderRecord o1 = MakeOrder(driver.licenseNumber, "Улица Свободы 1", "100,00", "01 Jan 2025");
    OrderRecord o2 = MakeOrder(driver.licenseNumber, "Улица Свободы 2", "200,00", "02 Jan 2025");

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
    DriverRecord driver{"TK256666662025", "Алексеева Ольга Викторовна", "Kia"};
    OrderRecord original = MakeOrder(driver.licenseNumber, "Улица Старая 7", "150,00", "01 Mar 2025");
    OrderRecord updated = MakeOrder(driver.licenseNumber, "Улица Новая 9", "155,00", "02 Mar 2025");

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
    DriverRecord driver{"TK257777772025", "Смирнов Илья Николаевич", "Ford"};
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
    auto path = ConfigPath("valid_basic.txt");
    ASSERT_TRUE(integrator.loadFromFile(path.string(), kConfigTableSize));
    EXPECT_TRUE(integrator.hasDriverTable());
    EXPECT_TRUE(integrator.hasOrderTree());

    EXPECT_EQ(integrator.driverCount(), 50u);
    EXPECT_EQ(integrator.orderCount(), 50u);

    auto firstDriver = integrator.findDriver("VB100");
    ASSERT_TRUE(firstDriver.has_value());
    EXPECT_EQ(firstDriver->fio, "Смирнова Алина Павловна");
    EXPECT_EQ(firstDriver->carBrand, "Лада");

    auto firstOrders = integrator.ordersForDriver("VB100");
    ASSERT_EQ(firstOrders.size(), 1u);
    EXPECT_EQ(firstOrders[0].address, "Улица Сиреневая 1");
    EXPECT_DOUBLE_EQ(firstOrders[0].cost, 1000.0);
    EXPECT_EQ(firstOrders[0].date.displayString(), "01 Dec 2024");

    auto lastDriver = integrator.findDriver("VB149");
    ASSERT_TRUE(lastDriver.has_value());
    EXPECT_EQ(lastDriver->fio, "Полякова Инга Сергеевна");
    EXPECT_EQ(lastDriver->carBrand, "Toyota");

    auto lastOrders = integrator.ordersForDriver("VB149");
    ASSERT_EQ(lastOrders.size(), 1u);
    EXPECT_EQ(lastOrders[0].address, "Улица Сиреневая 50");
    EXPECT_DOUBLE_EQ(lastOrders[0].cost, 1490.0);
    EXPECT_EQ(lastOrders[0].date.displayString(), "19 Jan 2025");
}

TEST(DataIntegratorTest, IntegratorLoadsConfigMultiple) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_multiple.txt").string(), kConfigTableSize));
    EXPECT_TRUE(integrator.hasDriverTable());
    EXPECT_TRUE(integrator.hasOrderTree());

    EXPECT_EQ(integrator.driverCount(), 50u);
    EXPECT_EQ(integrator.orderCount(), 50u);

    auto driver200 = integrator.findDriver("VM200");
    ASSERT_TRUE(driver200.has_value());
    EXPECT_EQ(driver200->fio, "Егоров Тимур Кириллович");
    EXPECT_EQ(driver200->carBrand, "Hyundai");

    auto orders200 = integrator.ordersForDriver("VM200");
    ASSERT_EQ(orders200.size(), 3u);
    EXPECT_EQ(orders200[0].address, "Площадь Центральная 1А");
    EXPECT_EQ(orders200[1].address, "Площадь Центральная 1Б");
    EXPECT_EQ(orders200[2].address, "Площадь Центральная 1В");

    auto driver201 = integrator.findDriver("VM201");
    ASSERT_TRUE(driver201.has_value());
    EXPECT_EQ(driver201->carBrand, "Skoda");

    auto orders201 = integrator.ordersForDriver("VM201");
    ASSERT_EQ(orders201.size(), 2u);
    EXPECT_EQ(orders201[0].address, "Площадь Центральная 2А");
    EXPECT_EQ(orders201[1].address, "Площадь Центральная 2Б");

    EXPECT_TRUE(integrator.ordersForDriver("VM247").empty());
    EXPECT_TRUE(integrator.ordersForDriver("VM248").empty());
    EXPECT_TRUE(integrator.ordersForDriver("VM249").empty());
}

TEST(DataIntegratorTest, IntegratorLoadsConfigNoOrders) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_no_orders.txt").string(), kConfigTableSize));
    EXPECT_TRUE(integrator.hasDriverTable());
    EXPECT_TRUE(integrator.hasOrderTree());

    EXPECT_EQ(integrator.driverCount(), 50u);
    EXPECT_EQ(integrator.orderCount(), 0u);

    auto driver300 = integrator.findDriver("VN300");
    ASSERT_TRUE(driver300.has_value());
    EXPECT_EQ(driver300->carBrand, "Лада");

    auto driver349 = integrator.findDriver("VN349");
    ASSERT_TRUE(driver349.has_value());
    EXPECT_EQ(driver349->fio, "Орлова Мария Аркадьевна");

    EXPECT_TRUE(integrator.ordersForDriver("VN300").empty());
    EXPECT_TRUE(integrator.ordersForDriver("VN349").empty());
}

TEST(DataIntegratorTest, IntegratorSavesToFile) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    ASSERT_TRUE(integrator.createOrderTree());
    DriverRecord first{"DL001", "Испытатель Альфа Сергеевич", "Tesla"};
    DriverRecord second{"DL002", "Испытатель Бета Андреевич", "Audi"};
    ASSERT_TRUE(integrator.addDriver(first));
    ASSERT_TRUE(integrator.addDriver(second));

    OrderRecord orderA = MakeOrder(first.licenseNumber, "Улица Опытная 7", "100,00", "31 Dec 2024");
    OrderRecord orderB = MakeOrder(second.licenseNumber, "Улица Опытная 8", "200,00", "01 Jan 2025");
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

TEST(DataIntegratorTest, IntegratorSavesDriversSeparately) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(8));
    DriverRecord first{"DL100", "Водитель Один Никитич", "Audi"};
    DriverRecord second{"DL200", "Водитель Два Викторович", "Volvo"};
    ASSERT_TRUE(integrator.addDriver(first));
    ASSERT_TRUE(integrator.addDriver(second));

    auto tempPath = TempFilePathForCurrentTest();
    RemoveIfExists(tempPath);
    ASSERT_TRUE(integrator.saveDriversToFile(tempPath.string()));

    std::ifstream input(tempPath);
    ASSERT_TRUE(input.is_open());
    std::ostringstream buffer;
    buffer << input.rdbuf();
    input.close();

    std::string expected =
        "drivers 2\n"
        "DL100|Водитель Один Никитич|Audi\n"
        "DL200|Водитель Два Викторович|Volvo\n";
    EXPECT_EQ(buffer.str(), expected);

    RemoveIfExists(tempPath);
}

TEST(DataIntegratorTest, IntegratorSavesOrdersSeparately) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(8));
    ASSERT_TRUE(integrator.createOrderTree());

    DriverRecord first{"DL300", "Водитель Три Семёнович", "Skoda"};
    DriverRecord second{"DL400", "Водитель Четыре Владимирович", "Toyota"};
    ASSERT_TRUE(integrator.addDriver(first));
    ASSERT_TRUE(integrator.addDriver(second));

    OrderRecord orderA = MakeOrder(first.licenseNumber, "Проспект Центральный 10", "150,00", "01 May 2024");
    OrderRecord orderB = MakeOrder(second.licenseNumber, "Проспект Северный 12", "210,00", "02 May 2024");
    ASSERT_TRUE(integrator.addOrder(orderA));
    ASSERT_TRUE(integrator.addOrder(orderB));

    auto tempPath = TempFilePathForCurrentTest();
    RemoveIfExists(tempPath);
    ASSERT_TRUE(integrator.saveOrdersToFile(tempPath.string()));

    std::ifstream input(tempPath);
    ASSERT_TRUE(input.is_open());
    std::ostringstream buffer;
    buffer << input.rdbuf();
    input.close();

    std::string expected =
        "orders 2\n"
        "DL300|Проспект Центральный 10|150,00|01 May 2024\n"
        "DL400|Проспект Северный 12|210,00|02 May 2024\n";
    EXPECT_EQ(buffer.str(), expected);

    RemoveIfExists(tempPath);
}

TEST(DataIntegratorTest, IntegratorSavesAndReloadsModifications) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.loadFromFile(ConfigPath("valid_basic.txt").string(), kConfigTableSize));
    EXPECT_TRUE(integrator.hasDriverTable());
    EXPECT_TRUE(integrator.hasOrderTree());

    auto driverOpt = integrator.findDriver("VB100");
    ASSERT_TRUE(driverOpt.has_value());
    DriverRecord originalDriver = *driverOpt;
    DriverRecord updatedDriver = originalDriver;
    updatedDriver.carBrand = "Nissan";
    EXPECT_TRUE(integrator.updateDriver(originalDriver, updatedDriver));

    auto existingOrders = integrator.ordersForDriver(updatedDriver.licenseNumber);
    ASSERT_EQ(existingOrders.size(), 1u);
    OrderRecord original = existingOrders[0];
    OrderRecord updatedOrder = original;
    updatedOrder.address = "Проспект Мира 10";
    updatedOrder.cost = 2700.0;
    EXPECT_TRUE(integrator.updateOrder(original, updatedOrder));

    std::size_t initialOrderCount = integrator.orderCount();
    OrderRecord newOrder = MakeOrder(updatedDriver.licenseNumber, "Тверская 5", "3100,00", "15 Jan 2025");
    ASSERT_TRUE(integrator.addOrder(newOrder));
    EXPECT_EQ(integrator.orderCount(), initialOrderCount + 1);

    auto tempPath = TempFilePathForCurrentTest();
    RemoveIfExists(tempPath);
    ASSERT_TRUE(integrator.saveToFile(tempPath.string()));

    DataIntegrator reloaded;
    ASSERT_TRUE(reloaded.loadFromFile(tempPath.string(), kConfigTableSize));
    auto reloadedDriver = reloaded.findDriver(updatedDriver.licenseNumber);
    ASSERT_TRUE(reloadedDriver.has_value());
    EXPECT_EQ(reloadedDriver->carBrand, "Nissan");

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
    DriverRecord alpha{"DLHASH1", "Испытатель Альфа Сергеевич", "Tesla"};
    DriverRecord beta{"DLHASH2", "Испытатель Бета Андреевич", "Audi"};
    DriverRecord gamma{"DLHASH3", "Испытатель Гамма Львович", "Bmw"};

    ASSERT_TRUE(integrator.addDriver(alpha));
    ASSERT_TRUE(integrator.addDriver(beta));
    ASSERT_TRUE(integrator.addDriver(gamma));

    OrderRecord orderA = MakeOrder(alpha.licenseNumber, "Проспект Альфа 1", "100,00", "01 Apr 2025");
    OrderRecord orderB = MakeOrder(beta.licenseNumber, "Проспект Бета 2", "200,00", "02 Apr 2025");
    OrderRecord orderC = MakeOrder(gamma.licenseNumber, "Проспект Гамма 3", "300,00", "03 Apr 2025");
    OrderRecord orderD = MakeOrder(alpha.licenseNumber, "Проспект Альфа 4", "400,00", "04 Apr 2025");

    ASSERT_TRUE(integrator.addOrder(orderA));
    ASSERT_TRUE(integrator.addOrder(orderB));
    ASSERT_TRUE(integrator.addOrder(orderC));
    ASSERT_TRUE(integrator.addOrder(orderD));

    auto hashDump = integrator.hashTableAsText();
    EXPECT_NE(hashDump.find("Водителей:"), std::string::npos);
    EXPECT_NE(hashDump.find("ФИО: Испытатель Альфа Сергеевич"), std::string::npos);
    EXPECT_NE(hashDump.find("Заказов: 2"), std::string::npos);

    auto treeDump = integrator.orderTreeAsText();
    EXPECT_NE(treeDump.find("Всего заказов: 4"), std::string::npos);
    EXPECT_NE(treeDump.find("Проспект Альфа 1"), std::string::npos);
    EXPECT_NE(treeDump.find("Проспект Альфа 4"), std::string::npos);
    EXPECT_NE(treeDump.find("|--"), std::string::npos);

    auto basePath = TempFilePathForCurrentTest();
    auto baseDir = basePath.parent_path();
    std::string stem = basePath.stem().string();
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
}

TEST(DataIntegratorTest, IntegratorRejectsInvalidConfigFile) {
    DataIntegrator integrator;
    ASSERT_TRUE(integrator.createDriverTable(16));
    DriverRecord existing{"SAFE1", "Безопаснов Пётр Алексеевич", "Vw"};
    ASSERT_TRUE(integrator.addDriver(existing));
    EXPECT_EQ(integrator.driverCount(), 1u);

    auto invalidPath = ConfigPath("invalid_unknown_driver.txt");
    EXPECT_FALSE(integrator.loadFromFile(invalidPath.string(), kConfigTableSize));

    EXPECT_EQ(integrator.driverCount(), 1u);
    EXPECT_EQ(integrator.orderCount(), 0u);
    auto stored = integrator.findDriver(existing.licenseNumber);
    ASSERT_TRUE(stored.has_value());
    EXPECT_EQ(stored->fio, existing.fio);
}
