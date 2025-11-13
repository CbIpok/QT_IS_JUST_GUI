#pragma once

#include <cstddef>
#include <string>
#include <cstdint>

namespace string_utils {

std::string trim(const std::string& value);

bool parseInteger(const std::string& text, int& out);

std::string toLower(std::string text);

void trimCarriageReturn(std::string& value);

void stripUtf8Bom(std::string& value);

bool splitLine(const std::string& line, char delimiter, std::string* fields, std::size_t expectedCount);

bool decodeUtf8(const std::string& text, std::u32string& out);

std::string extractFileName(const std::string& path);

bool hasTxtExtension(const std::string& path);

bool isLatinFileName(const std::string& path);

bool isValidTxtFilePath(const std::string& path);

} // namespace string_utils

