#include <gtest/gtest.h>
#include <cstdio>

int main(int argc, char** argv) {
#ifdef _WIN32
#endif
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
