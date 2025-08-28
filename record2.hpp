#pragma once
#include <string>

// Record describing a taxi order from the "Заказы" directory.
// This structure will be stored in the AVL tree.
struct OrderRecord {
    std::string licenseNumber; // Номер лицензии водителя
    std::string address;       // Адрес подачи
    int         cost;          // Стоимость заказа в рублях
    std::string date;          // Дата выполнения
    int         originalLine;  // строка в исходном файле (или -1, если добавлен вручную)
};
