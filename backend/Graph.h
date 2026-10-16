#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <vector>

struct Edge
{
    int destination;
    double distance;
};

class Graph
{
private:
    std::vector< std::vector<Edge> > adjacencyList;
    std::vector<std::string> names;

public:
    explicit Graph(int vertices);
    void addRoad(int source, int destination, double distance);
    std::vector<Edge> getEdges(int vertex) const;
    std::string getName(int vertex) const;
    int getVertexCount() const;
    int findVertex(std::string name) const;
};

#endif
