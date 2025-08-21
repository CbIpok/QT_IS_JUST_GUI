#pragma once

#include <vector>
#include <utility>

std::pair<int, int> binarySearch(const std::vector<int>& arr, int key);

std::pair<std::vector<int>, int> linearSearch(const std::vector<int>& arr, int key);

