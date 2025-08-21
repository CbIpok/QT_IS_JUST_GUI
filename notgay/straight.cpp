#include "straight.h"
#include <cstring>

int naive_search(const char* text, const char* pattern) {
    int count = 0;
    int n = (int)strlen(text);
    int m = (int)strlen(pattern);
    if (m == 0) return 0;
    for (int i = 0; i <= n - m; i++) {
        int j = 0;
        while (j < m && text[i + j] == pattern[j])
            j++;
        if (j == m)
            count++;
    }
    return count;
}
