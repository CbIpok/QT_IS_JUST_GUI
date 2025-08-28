#ifndef RECORD_HPP
#define RECORD_HPP

#include <string>

// Record describing a driver in the "Водители" directory.
// Used as the element stored inside the hash table.  Each driver is
// uniquely identified by a license number issued by the authorities.
struct DriverRecord {
    std::string licenseNumber; // Номер лицензии – уникальный идентификатор
    std::string fio;           // ФИО водителя
    std::string carBrand;      // Марка автомобиля
    int         originalLine;  // строка в исходном файле (или -1, если добавлен вручную)
};

#endif // RECORD_HPP
