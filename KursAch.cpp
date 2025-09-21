#include <gtest/gtest.h>
#include <cstdio>

void RegisterDoublyLinkedListTests();
void RegisterHashTableTests();
void RegisterAVLTreeTests();
void RegisterIntegratorTests();

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    RegisterDoublyLinkedListTests();
    RegisterHashTableTests();
    RegisterAVLTreeTests();
    RegisterIntegratorTests();

    auto* unit = ::testing::UnitTest::GetInstance();
    std::printf("total test suites: %d\n", unit->total_test_suite_count());
    for (int i = 0; i < unit->total_test_suite_count(); ++i) {
        const auto* suite = unit->GetTestSuite(i);
        std::printf(" suite[%d]: %s has %d tests\n", i, suite->name(), suite->total_test_count());
        for (int j = 0; j < suite->total_test_count(); ++j) {
            const auto* info = suite->GetTestInfo(j);
            std::printf("   test[%d]: %s\n", j, info->name());
        }
    }
    std::printf("total tests before run: %d\n", unit->total_test_count());
    return RUN_ALL_TESTS();
}
