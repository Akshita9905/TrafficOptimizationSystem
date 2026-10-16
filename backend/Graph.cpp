#include "Graph.h"
#include <algorithm>
#include <cctype>

Graph::Graph(int vertices)
{
    adjacencyList.resize(vertices);
    names.push_back("Clock Tower");
    names.push_back("ISBT");
    names.push_back("Rajpur Road");
    names.push_back("Ballupur");
    names.push_back("Prem Nagar");
    names.push_back("Pacific Mall");
}

void Graph::addRoad(int source, int destination, double distance)
{
    adjacencyList[source].push_back(Edge{destination, distance});
    adjacencyList[destination].push_back(Edge{source, distance});
}

std::vector<Edge> Graph::getEdges(int vertex) const { return adjacencyList[vertex]; }
std::string Graph::getName(int vertex) const { return names[vertex]; }
int Graph::getVertexCount() const { return static_cast<int>(adjacencyList.size()); }

int Graph::findVertex(std::string name) const
{
    for (size_t i = 0; i < names.size(); ++i)
    {
        if (names[i] == name) return static_cast<int>(i);
    }
    return -1;
}
