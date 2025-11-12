#include "validation.hpp"

#include <algorithm>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#include "date.hpp"
#include "string_utils.hpp"

namespace {

constexpr char32_t kCyrillicUpperA = U'\u0410';
constexpr char32_t kCyrillicUpperYa = U'\u042F';
constexpr char32_t kCyrillicLowerA = U'\u0430';
constexpr char32_t kCyrillicLowerYa = U'\u044F';
constexpr char32_t kCyrillicUpperYo = U'\u0401';
constexpr char32_t kCyrillicLowerYo = U'\u0451';

bool IsDigit(char32_t codePoint) {
    return codePoint >= U'0' && codePoint <= U'9';
}

bool IsLatinUpper(char32_t codePoint) {
    return codePoint >= U'A' && codePoint <= U'Z';
}

bool IsLatinLower(char32_t codePoint) {
    return codePoint >= U'a' && codePoint <= U'z';
}

bool IsRussianUpper(char32_t codePoint) {
    return (codePoint >= kCyrillicUpperA && codePoint <= kCyrillicUpperYa) || codePoint == kCyrillicUpperYo;
}

bool IsRussianLower(char32_t codePoint) {
    return (codePoint >= kCyrillicLowerA && codePoint <= kCyrillicLowerYa) || codePoint == kCyrillicLowerYo;
}

bool IsRussianLetter(char32_t codePoint) {
    return IsRussianUpper(codePoint) || IsRussianLower(codePoint);
}

bool ParseMonth(const std::string& token, Month& month) {
    static const std::pair<const char*, Month> months[] = {
        {"Jan", Month::Jan}, {"Feb", Month::Feb}, {"Mar", Month::Mar}, {"Apr", Month::Apr},
        {"May", Month::May}, {"Jun", Month::Jun}, {"Jul", Month::Jul}, {"Aug", Month::Aug},
        {"Sep", Month::Sep}, {"Oct", Month::Oct}, {"Nov", Month::Nov}, {"Dec", Month::Dec}};
    for (const auto& [name, value] : months) {
        if (token == name) {
            month = value;
            return true;
        }
    }
    return false;
}

}  // namespace

namespace validation {

bool isValidLicense(const std::string& value) {
    if (value.empty()) {
        return false;
    }
    return std::all_of(value.begin(), value.end(), [](unsigned char ch) {
        return (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9');
    });
}

bool isValidFio(const std::string& value) {
    std::vector<std::string> parts;
    if (!string_utils::splitBySpaces(value, parts) || parts.size() != 3) {
        return false;
    }

    for (const std::string& part : parts) {
        std::u32string codePoints;
        if (!string_utils::decodeUtf8(part, codePoints) || codePoints.empty()) {
            return false;
        }
        if (!IsRussianUpper(codePoints.front())) {
            return false;
        }
        for (std::size_t i = 1; i < codePoints.size(); ++i) {
            if (!IsRussianLower(codePoints[i])) {
                return false;
            }
        }
    }

    return true;
}

bool isValidCarBrand(const std::string& value) {
    if (value.empty() || value.find(' ') != std::string::npos) {
        return false;
    }

    std::u32string codePoints;
    if (!string_utils::decodeUtf8(value, codePoints) || codePoints.empty()) {
        return false;
    }

    bool useLatin = false;
    if (IsLatinUpper(codePoints.front())) {
        useLatin = true;
    }
    else if (!IsRussianUpper(codePoints.front())) {
        return false;
    }

    for (std::size_t i = 1; i < codePoints.size(); ++i) {
        char32_t cp = codePoints[i];
        if (useLatin) {
            if (!IsLatinLower(cp)) {
                return false;
            }
        }
        else {
            if (!IsRussianLower(cp)) {
                return false;
            }
        }
    }

    return true;
}

bool isValidAddress(const std::string& value) {
    std::vector<std::string> tokens;
    if (!string_utils::splitBySpaces(value, tokens) || tokens.empty()) {
        return false;
    }

    bool first = true;
    for (const std::string& token : tokens) {
        std::string core = token;
        while (!core.empty() && (core.back() == '.' || core.back() == ',')) {
            core.pop_back();
        }
        if (core.empty()) {
            return false;
        }

        std::u32string codePoints;
        if (!string_utils::decodeUtf8(core, codePoints) || codePoints.empty()) {
            return false;
        }

        bool hasLetters = false;
        bool hasDigits = false;
        for (char32_t cp : codePoints) {
            if (IsDigit(cp)) {
                hasDigits = true;
            }
            else if (IsRussianLetter(cp)) {
                hasLetters = true;
            }
            else {
                return false;
            }
        }

        if (hasLetters && hasDigits) {
            bool seenLetter = false;
            for (char32_t cp : codePoints) {
                if (IsDigit(cp)) {
                    if (seenLetter) {
                        return false;
                    }
                }
                else {
                    seenLetter = true;
                    if (!IsRussianUpper(cp)) {
                        return false;
                    }
                }
            }
        }
        else if (hasLetters) {
            if (!IsRussianUpper(codePoints.front())) {
                return false;
            }
            for (std::size_t i = 1; i < codePoints.size(); ++i) {
                if (!IsRussianLower(codePoints[i])) {
                    return false;
                }
            }
        }
        else {
            if (first) {
                return false;
            }
        }

        first = false;
    }

    return true;
}

bool isValidCost(const std::string& value) {
    if (value.empty()) {
        return false;
    }

    std::size_t commaPos = value.find(',');
    if (commaPos == std::string::npos) {
        return false;
    }

    std::string integerPart = value.substr(0, commaPos);
    std::string fractionalPart = value.substr(commaPos + 1);

    if (integerPart.empty() || fractionalPart.size() != 2) {
        return false;
    }

    if (!std::all_of(integerPart.begin(), integerPart.end(), [](unsigned char ch) { return ch >= '0' && ch <= '9'; })
        || !std::all_of(fractionalPart.begin(), fractionalPart.end(), [](unsigned char ch) { return ch >= '0' && ch <= '9'; })) {
        return false;
    }

    bool integerAllZeros = std::all_of(integerPart.begin(), integerPart.end(), [](char ch) { return ch == '0'; });
    if (integerAllZeros && fractionalPart == "00") {
        return false;
    }

    return true;
}

bool parseDate(const std::string& value, Date& out) {
    std::string trimmed = string_utils::trim(value);
    if (trimmed.empty()) {
        return false;
    }

    static const std::regex pattern(
        R"(^(0[1-9]|[12][0-9]|3[01]) (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) ([0-9]{4})$)");
    std::smatch match;
    if (!std::regex_match(trimmed, match, pattern)) {
        return false;
    }

    int day = std::stoi(match[1].str());
    std::string monthToken = match[2].str();
    int year = std::stoi(match[3].str());

    Month monthValue;
    if (!ParseMonth(monthToken, monthValue)) {
        return false;
    }

    out = Date(day, monthValue, year);
    return out.isValid();
}

bool isValidDate(const std::string& value) {
    Date parsed{};
    return parseDate(value, parsed);
}

}  // namespace validation
