#include "string_utils.hpp"

#include <algorithm>
#include <cctype>

namespace string_utils {

std::string trim(const std::string& value) {
    std::size_t start = 0;
    std::size_t end = value.size();
    while (start < end && std::isspace(static_cast<unsigned char>(value[start]))) {
        ++start;
    }
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }
    return value.substr(start, end - start);
}

bool parseInteger(const std::string& text, int& out) {
    if (text.empty()) return false;
    int value = 0;
    for (char ch : text) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
        value = value * 10 + (ch - '0');
    }
    out = value;
    return true;
}

std::string toLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

void trimCarriageReturn(std::string& value) {
    if (!value.empty() && value.back() == '\r') {
        value.pop_back();
    }
}

void stripUtf8Bom(std::string& value) {
    if (value.size() >= 3) {
        const unsigned char first = static_cast<unsigned char>(value[0]);
        const unsigned char second = static_cast<unsigned char>(value[1]);
        const unsigned char third = static_cast<unsigned char>(value[2]);
        if (first == 0xEF && second == 0xBB && third == 0xBF) {
            value.erase(0, 3);
        }
    }
}

bool splitLine(const std::string& line, char delimiter, std::string* fields, std::size_t expectedCount) {
    std::size_t fieldIndex = 0;
    std::size_t start = 0;
    std::size_t length = line.size();
    for (std::size_t i = 0; i <= length; ++i) {
        bool isDelimiter = (i < length && line[i] == delimiter);
        bool isEnd = (i == length);
        if (!isDelimiter && !isEnd) {
            continue;
        }
        if (fieldIndex >= expectedCount) {
            return false;
        }
        fields[fieldIndex] = line.substr(start, i - start);
        ++fieldIndex;
        start = i + 1;
    }
    return fieldIndex == expectedCount;
}

} // namespace string_utils

