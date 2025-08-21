#include "modnaminecraft.hpp"

std::pair<int, int> binarySearch(const std::vector<int>& arr, int key) {
    int n = arr.size();
    std::vector<int> S;
    int k = 1;
    while ((k << 1) <= n) k <<= 1;
    for (; k > 0; k >>= 1) S.push_back(k);

    int m = S[0] - 1;
    int steps = 0;
    for (int i = 0; i < S.size(); ++i) {
        ++steps;
        if (arr[m] == key) {
            int first = m;
            while (first > 0 && arr[first - 1] == key) {
                ++steps;
                --first;
            }

            return { first, steps };
        }
        if (i + 1 < S.size()) {
            if (arr[m] < key) m += S[i + 1];
            else m -= S[i + 1];
        }
    }
    return { -1, steps };
}

std::pair<std::vector<int>, int> linearSearch(const std::vector<int>& arr, int key) {
    std::vector<int> found;
    int steps = 0;
    for (int i = 0, n = arr.size(); i < n; ++i) {
        ++steps;
        if (arr[i] == key) {
            found.push_back(i);
        }
    }
    return { found, steps };
}
