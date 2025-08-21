#include "Prim.hpp"

void primMST(Graph* g, Edge mst[], int* count) {
    int n = g->getN();
    bool inMST[MAX_N];
    int minEdge[MAX_N];
    int selEdge[MAX_N];
    int i, j;

    for (i = 0; i < n; i++) {
        inMST[i] = false;
        minEdge[i] = INF;
        selEdge[i] = -1;
    }

    minEdge[0] = 0;
    *count = 0;

    for (i = 0; i < n; i++) {
        int v = -1;
        for (j = 0; j < n; j++) {
            if (!inMST[j] && (v == -1 || minEdge[j] < minEdge[v]))
                v = j;
        }
        if (minEdge[v] == INF) break;
        inMST[v] = true;
        if (selEdge[v] != -1) {
            mst[*count].u = selEdge[v];
            mst[*count].v = v;
            mst[*count].w = minEdge[v];
            (*count)++;
        }
        for (j = 0; j < n; j++) {
            int w = g->weight(v, j);
            if (!inMST[j] && w < minEdge[j]) {
                minEdge[j] = w;
                selEdge[j] = v;
            }
        }
    }
}
