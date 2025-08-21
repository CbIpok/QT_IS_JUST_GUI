#include "DFS.hpp"

void dfsNonRecursive(Graph* g, int start, int order[]) {
    int n = g->getN();
    bool visited[MAX_N];
    int i;
    for (i = 0; i < n; i++) visited[i] = false;

    int stack[MAX_N];
    int top = -1;

    // стартуем
    stack[++top] = start;
    int idx = 0;

    while (top >= 0) {
        int u = stack[top--];
        if (!visited[u]) {
            visited[u] = true;
            order[idx++] = u;
            // добавляем соседей в обратном порядке
            for (i = n - 1; i >= 0; i--) {
                if (g->weight(u, i) != INF && !visited[i]) {
                    stack[++top] = i;
                }
            }
        }
    }
}
