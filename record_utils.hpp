#pragma once

#include <algorithm>
#include <cctype>
#include <codecvt>
#include <cstdlib>
#include <iomanip>
#include <locale>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "record_common.hpp"

namespace record {

inline std::string trim(std::string_view s) {
    const char* begin = s.data();
    const char* end = begin + s.size();
    while (begin < end && std::isspace(static_cast<unsigned char>(*begin))) {
        ++begin;
    }
    while (end > begin && std::isspace(static_cast<unsigned char>(*(end - 1)))) {
        --end;
    }
    return std::string(begin, end);
}

inline std::vector<std::string> split(const std::string& text, char delimiter) {
    std::vector<std::string> parts;
    std::string current;
    for (char ch : text) {
        if (ch == delimiter) {
            parts.push_back(current);
            current.clear();
        }
        else {
            current.push_back(ch);
        }
    }
    parts.push_back(current);
    return parts;
}

inline std::u32string toUtf32(const std::string& value) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.from_bytes(value);
}

inline bool isUpperLatin(char32_t ch) {
    return ch >= U'A' && ch <= U'Z';
}

inline bool isLowerLatin(char32_t ch) {
    return ch >= U'a' && ch <= U'z';
}

inline bool isUpperCyrillic(char32_t ch) {
    return (ch >= U'\u0410' && ch <= U'\u042F') || ch == U'\u0401';
}

inline bool isLowerCyrillic(char32_t ch) {
    return (ch >= U'\u0430' && ch <= U'\u044F') || ch == U'\u0451';
}

inline bool isDigit(char32_t ch) {
    return ch >= U'0' && ch <= U'9';
}

inline bool validateLicenseNumber(const std::string& value, std::string& error) {
    static const std::regex pattern("^[A-Z0-9]+$");
    if (value.empty()) {
        error = "Номер лицензии не может быть пустым";
        return false;
    }
    if (!std::regex_match(value, pattern)) {
        error = "Номер лицензии должен состоять из заглавных латинских букв и цифр без пробелов";
        return false;
    }
    return true;
}

inline bool validateFio(const std::string& value, std::string& error) {
    if (value.empty()) {
        error = "ФИО не может быть пустым";
        return false;
    }
    std::u32string utf = toUtf32(value);
    std::vector<std::u32string> parts;
    std::u32string current;
    for (char32_t ch : utf) {
        if (ch == U' ') {
            if (current.empty()) {
                error = "ФИО должно состоять из трёх слов, разделённых одним пробелом";
                return false;
            }
            parts.push_back(current);
            current.clear();
        }
        else {
            current.push_back(ch);
        }
    }
    if (!current.empty()) {
        parts.push_back(current);
    }
    if (parts.size() != 3) {
        error = "ФИО должно состоять из трёх слов, разделённых одним пробелом";
        return false;
    }
    for (const auto& part : parts) {
        if (part.empty()) {
            error = "Каждое слово ФИО должно содержать буквы";
            return false;
        }
        if (!isUpperCyrillic(part.front())) {
            error = "Первая буква каждой части ФИО должна быть заглавной русской";
            return false;
        }
        for (size_t i = 1; i < part.size(); ++i) {
            if (!isLowerCyrillic(part[i])) {
                error = "После первой буквы каждая часть ФИО должна содержать только строчные русские буквы";
                return false;
            }
        }
    }
    return true;
}

inline bool validateCarBrand(const std::string& value, std::string& error) {
    if (value.empty()) {
        error = "Марка автомобиля не может быть пустой";
        return false;
    }
    std::u32string utf = toUtf32(value);
    if (!isUpperCyrillic(utf.front()) && !isUpperLatin(utf.front())) {
        error = "Марка автомобиля должна начинаться с заглавной буквы";
        return false;
    }
    for (size_t i = 1; i < utf.size(); ++i) {
        if (!isLowerCyrillic(utf[i]) && !isLowerLatin(utf[i])) {
            error = "Марка автомобиля может содержать только буквы русского или латинского алфавита";
            return false;
        }
    }
    return true;
}

inline bool validateAddress(const std::string& value, std::string& error) {
    if (value.empty()) {
        error = "Адрес подачи не может быть пустым";
        return false;
    }
    if (value.find("  ") != std::string::npos) {
        error = "Между словами адреса должен быть один пробел";
        return false;
    }
    std::u32string utf = toUtf32(value);
    std::vector<std::u32string> parts;
    std::u32string current;
    for (char32_t ch : utf) {
        if (ch == U' ') {
            if (current.empty()) {
                error = "Адрес содержит несколько пробелов подряд";
                return false;
            }
            parts.push_back(current);
            current.clear();
        }
        else {
            current.push_back(ch);
        }
    }
    if (!current.empty()) {
        parts.push_back(current);
    }
    if (parts.size() < 2) {
        error = "Адрес должен содержать минимум улицу и номер дома";
        return false;
    }
    for (auto& token : parts) {
        if (token.empty()) {
            error = "Слова в адресе не могут быть пустыми";
            return false;
        }
        bool hasTrailingPunct = false;
        if (token.back() == U',' || token.back() == U'.') {
            hasTrailingPunct = true;
            if (token.size() == 1) {
                error = "Знаки препинания в адресе должны следовать после слова";
                return false;
            }
        }
        std::u32string core = token;
        if (hasTrailingPunct) {
            core.resize(core.size() - 1);
        }
        char32_t first = core.front();
        if (isUpperCyrillic(first)) {
            for (size_t i = 1; i < core.size(); ++i) {
                char32_t ch = core[i];
                if (!isLowerCyrillic(ch) && ch != U'-') {
                    error = "Слова в адресе должны быть на русском языке, допускается дефис";
                    return false;
                }
            }
        }
        else if (isDigit(first)) {
            for (size_t i = 1; i < core.size(); ++i) {
                char32_t ch = core[i];
                if (!isDigit(ch) && !isUpperCyrillic(ch)) {
                    error = "Номер дома может содержать цифры и литеры корпуса";
                    return false;
                }
            }
        }
        else {
            error = "Каждое слово адреса должно начинаться с заглавной буквы или цифры";
            return false;
        }
    }
    return true;
}

inline bool validateCost(const std::string& value, int& outCost, std::string& error) {
    static const std::regex pattern("^[0-9]+,[0-9]{2}$");
    if (!std::regex_match(value, pattern)) {
        error = "Стоимость должна быть положительным числом с двумя знаками после запятой";
        return false;
    }
    size_t commaPos = value.find(',');
    long long rubles = 0;
    int kopecks = 0;
    try {
        rubles = std::stoll(value.substr(0, commaPos));
        kopecks = std::stoi(value.substr(commaPos + 1));
    }
    catch (...) {
        error = "Не удалось разобрать стоимость";
        return false;
    }
    long long total = rubles * 100 + kopecks;
    if (total <= 0) {
        error = "Стоимость должна быть положительной";
        return false;
    }
    outCost = static_cast<int>(total);
    return true;
}

inline bool validateDate(const std::string& value, std::string& error) {
    static const std::regex pattern("^(0[1-9]|[12][0-9]|3[01]) (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) [0-9]{4}$");
    if (!std::regex_match(value, pattern)) {
        error = "Дата должна быть в формате DD Mon YYYY";
        return false;
    }
    return true;
}

inline std::string formatCost(int costKopecks) {
    if (costKopecks < 0) {
        costKopecks = 0;
    }
    int rubles = costKopecks / 100;
    int kopecks = std::abs(costKopecks % 100);
    std::ostringstream oss;
    oss << rubles << ',' << std::setw(2) << std::setfill('0') << kopecks;
    return oss.str();
}

inline bool parseRecordFields(const std::vector<std::string>& fields,
    Record& record,
    std::string& error,
    bool expectOriginalLine,
    int lineNumber) {

    Record temp{};
    if (!validateLicenseNumber(fields[0], error)) return false;
    temp.licenseNumber = fields[0];

    if (!validateFio(fields[1], error)) return false;
    temp.fio = fields[1];

    if (!validateCarBrand(fields[2], error)) return false;
    temp.carBrand = fields[2];

    if (!validateAddress(fields[3], error)) return false;
    temp.pickupAddress = fields[3];

    int cost = 0;
    if (!validateCost(fields[4], cost, error)) return false;
    temp.costKopecks = cost;

    if (!validateDate(fields[5], error)) return false;
    temp.date = fields[5];

    if (expectOriginalLine) {
        try {
            temp.originalLine = std::stoi(fields[6]);
        }
        catch (...) {
            error = "Некорректный номер строки источника";
            return false;
        }
    }
    else {
        temp.originalLine = lineNumber;
    }

    record = std::move(temp);
    return true;
}

inline bool parseRecordFromInput(const std::string& line, int lineNumber, Record& record, std::string& error) {
    auto fields = split(line, ';');
    if (fields.size() != 6) {
        error = "Ожидается 6 полей, разделённых точкой с запятой";
        return false;
    }
    for (auto& field : fields) {
        field = trim(field);
    }
    return parseRecordFields(fields, record, error, false, lineNumber);
}

inline bool parseRecordFromCache(const std::string& line, Record& record, std::string& error) {
    auto fields = split(line, ';');
    if (fields.size() != 7) {
        error = "Ожидается 7 полей в сохранённой записи";
        return false;
    }
    for (auto& field : fields) {
        field = trim(field);
    }
    return parseRecordFields(fields, record, error, true, -1);
}

inline std::string serializeRecord(const Record& record) {
    std::ostringstream oss;
    oss << record.licenseNumber << ';'
        << record.fio << ';'
        << record.carBrand << ';'
        << record.pickupAddress << ';'
        << formatCost(record.costKopecks) << ';'
        << record.date << ';'
        << record.originalLine;
    return oss.str();
}

} // namespace record

