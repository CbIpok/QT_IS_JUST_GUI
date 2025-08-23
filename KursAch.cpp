#include <gtest/gtest.h>

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv); // Инициализация gtest
    return RUN_ALL_TESTS();                 // Запуск всех тестов
}
