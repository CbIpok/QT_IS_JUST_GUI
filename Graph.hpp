#pragma once

#include "Edge.hpp"

#define MAX_N 100
#define MAX_M (MAX_N*(MAX_N-1)/2)
#define INF 1000000000

class Graph {
public:
    Graph();
    bool loadFromFile(const char* filename);
    int getN();
    int getM();
    Edge* getEdges();
    void buildAdjMatrix();
    int weight(int i, int j);

private:
    int n;
    int m;
    Edge edges[MAX_M];
    int adj[MAX_N][MAX_N];
};
