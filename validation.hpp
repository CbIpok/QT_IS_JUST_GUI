#pragma once

#include <string>

struct Date;

namespace validation {

bool isValidLicense(const std::string& value);

bool isValidFio(const std::string& value);

bool isValidCarBrand(const std::string& value);

bool isValidAddress(const std::string& value);

bool isValidCost(const std::string& value);

bool parseDate(const std::string& value, Date& out);

bool isValidDate(const std::string& value);

}  // namespace validation
