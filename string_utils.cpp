#include "string_utils.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>

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

bool decodeUtf8(const std::string& text, std::u32string& out) {
    out.clear();

    std::size_t i = 0;
    while (i < text.size()) {
        unsigned char byte = static_cast<unsigned char>(text[i]);
        char32_t      codePoint = 0;
        std::size_t   remaining = 0;

        if ((byte & 0x80) == 0) {
            codePoint = byte;
            remaining = 0;
        }
        else if ((byte & 0xE0) == 0xC0) {
            codePoint = byte & 0x1F;
            remaining = 1;
        }
        else if ((byte & 0xF0) == 0xE0) {
            codePoint = byte & 0x0F;
            remaining = 2;
        }
        else if ((byte & 0xF8) == 0xF0) {
            codePoint = byte & 0x07;
            remaining = 3;
        }
        else {
            return false;
        }

        if (i + remaining >= text.size()) {
            return false;
        }

        for (std::size_t j = 0; j < remaining; ++j) {
            unsigned char continuation = static_cast<unsigned char>(text[i + j + 1]);
            if ((continuation & 0xC0) != 0x80) {
                return false;
            }
            codePoint = (codePoint << 6) | (continuation & 0x3F);
        }

        if ((remaining == 1 && codePoint < 0x80) || (remaining == 2 && codePoint < 0x800)
            || (remaining == 3 && codePoint < 0x10000)) {
            return false;
        }

        if (codePoint > 0x10FFFF || (codePoint >= 0xD800 && codePoint <= 0xDFFF)) {
            return false;
        }

        out.push_back(codePoint);
        i += remaining + 1;
    }

    return true;
}

std::string extractFileName(const std::string& path) {
    if (path.empty()) {
        return {};
    }

    std::size_t index = path.size();
    while (index > 0) {
        char ch = path[index - 1];
        if (ch == '/' || ch == '\\') {
            break;
        }
        --index;
    }

    return path.substr(index);
}

bool hasTxtExtension(const std::string& path) {
    std::string fileName = extractFileName(path);
    if (fileName.size() < 4) {
        return false;
    }

    std::string extension = toLower(fileName.substr(fileName.size() - 4));
    return extension == ".txt";
}

namespace {
bool isLatinOrDigit(char ch) {
    if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) {
        return true;
    }
    if (ch >= '0' && ch <= '9') {
        return true;
    }
    return ch == '_' || ch == '-';
}
} // namespace

bool isLatinFileName(const std::string& path) {
    std::string fileName = extractFileName(path);
    if (!hasTxtExtension(fileName)) {
        return false;
    }

    if (fileName.size() <= 4) {
        return false;
    }

    std::size_t limit = fileName.size() - 4;
    for (std::size_t i = 0; i < limit; ++i) {
        if (!isLatinOrDigit(fileName[i])) {
            return false;
        }
    }
    return true;
}

bool isValidTxtFilePath(const std::string& path) {
    if (!hasTxtExtension(path)) {
        return false;
    }
    return isLatinFileName(path);
}

} // namespace string_utils

