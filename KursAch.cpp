#if defined(_WIN32)
#include <windows.h>
#endif
#include <gtest/gtest.h>
#include <cstdio>

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    std::cout << reinterpret_cast<const char*>(u8"Привет!");
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
