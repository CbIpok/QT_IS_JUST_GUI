#pragma once
#include <iostream>
#include "Edge.hpp"
#include "Graph.hpp"
#include "DFS.hpp"
#include "Prim.hpp"

int main4() {
    Graph g;
    if (!g.loadFromFile("input.txt")) {
        std::cout << "Cannot open file input.txt\n";
        return 1;
    }

    g.buildAdjMatrix();

    int n = g.getN();
    int order[MAX_N];
    dfsNonRecursive(&g, 0, order);

    std::cout << "DFS order:\n";
    for (int i = 0; i < n; i++)
        std::cout << order[i] << " ";
    std::cout << "\n";

    Edge mst[MAX_N];
    int edgeCount;
    primMST(&g, mst, &edgeCount);

    std::cout << "MST edges (" << edgeCount << "):\n";
    for (int i = 0; i < edgeCount; i++)
        std::cout << mst[i].u << "-" << mst[i].v
        << " w=" << mst[i].w << "\n";

    return 0;
}
