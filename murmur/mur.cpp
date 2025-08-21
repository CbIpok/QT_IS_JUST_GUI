#include "mur.h"
#include <cstring>

int boyer_moore_search(const char* text, const char* pattern) {
    int count = 0;
    int m = (int)strlen(pattern);
    int n = (int)strlen(text);
    if (m == 0) return 0;

    int bad[256];
    for (int i = 0; i < 256; i++) {
        bad[i] = -1;
    }
    for (int i = 0; i < m; i++) {
        bad[(unsigned char)pattern[i]] = i;
    }

    int s = 0; 
    while (s <= n - m) {
        int j = m - 1;
        while (j >= 0 && pattern[j] == text[s + j])
            j--;
        if (j < 0) {
            count++;
            s += (s + m < n) ? m - bad[(unsigned char)text[s + m]] : 1;
        }
        else {
            int shift = j - bad[(unsigned char)text[s + j]];
            if (shift < 1)
                shift = 1;
            s += shift;
        }
    }
    return count;
}
