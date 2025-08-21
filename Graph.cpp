#include "Graph.hpp"
#include <fstream>

Graph::Graph() {
    n = 0;
    m = 0;
    for (int i = 0; i < MAX_N; i++)
        for (int j = 0; j < MAX_N; j++)
            adj[i][j] = INF;
}

bool Graph::loadFromFile(const char* filename) {
    std::ifstream in(filename);
    if (!in.is_open()) return false;
    in >> n;
    int u, v, w;
    m = 0;
    while (in >> u >> v >> w) {
        if (m >= MAX_M) break;
        edges[m].u = u;
        edges[m].v = v;
        edges[m].w = w;
        m++;
    }
    in.close();
    return true;
}

int Graph::getN() {
    return n;
}

int Graph::getM() {
    return m;
}

Edge* Graph::getEdges() {
    return edges;
}

void Graph::buildAdjMatrix() {
    int i, j;
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
            adj[i][j] = INF;

    for (i = 0; i < m; i++) {
        int u = edges[i].u;
        int v = edges[i].v;
        int w = edges[i].w;
        if (w < adj[u][v]) {
            adj[u][v] = w;
            adj[v][u] = w;
        }
    }
    for (i = 0; i < n; i++)
        adj[i][i] = 0;
}

int Graph::weight(int i, int j) {
    return adj[i][j];
}
