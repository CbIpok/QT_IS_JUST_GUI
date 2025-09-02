#include <gtest/gtest.h>
#include "report_system.hpp"
#include <fstream>

static void writeFile(const std::string& name, const std::string& content) {
    std::ofstream out(name);
    out << content;
}

TEST(ReportSystemTest, LoadSearchFilterExport) {
    writeFile("drivers.txt",
        "L1;Driver One;BMW\n"
        "L2;Driver Two;Mercedes\n");
    writeFile("orders.txt",
        "L1;Lenina;100;01 jan 2025\n"
        "L1;Lesnaya;150;05 jan 2025\n"
        "L2;Lenina;200;03 feb 2025\n");

    ReportSystem sys;
    ASSERT_TRUE(sys.loadDrivers("drivers.txt"));
    ASSERT_TRUE(sys.loadOrders("orders.txt"));

    auto ordersL2 = sys.ordersByLicense("L2");
    ASSERT_EQ(ordersL2.size(), 1u);
    EXPECT_EQ(ordersL2[0].address, "Lenina");

    Date start{1,1,2025};
    Date end{31,1,2025};
    auto filtered = sys.filter("BMW", "Lenina", start, end);
    ASSERT_EQ(filtered.size(), 1u);
    EXPECT_EQ(filtered[0].licenseNumber, "L1");

    ASSERT_TRUE(sys.exportToFile("report.txt", filtered));
    std::ifstream in("report.txt");
    std::string line;
    ASSERT_TRUE(std::getline(in, line));
    EXPECT_EQ(line, "L1;Lenina;100;01 jan 2025");
}
