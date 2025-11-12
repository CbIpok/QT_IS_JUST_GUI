#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <cstdint>

namespace string_utils {

std::string trim(const std::string& value);

bool parseInteger(const std::string& text, int& out);

std::string toLower(std::string text);

void trimCarriageReturn(std::string& value);

void stripUtf8Bom(std::string& value);

bool splitLine(const std::string& line, char delimiter, std::string* fields, std::size_t expectedCount);

bool decodeUtf8(const std::string& text, std::u32string& out);

bool splitBySpaces(const std::string& value, std::vector<std::string>& words);

} // namespace string_utils

