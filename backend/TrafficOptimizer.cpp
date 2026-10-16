#include "TrafficOptimizer.h"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <limits>
#include <queue>
#include <sstream>

TrafficOptimizer::TrafficOptimizer() : graph(6)
{
    std::srand(static_cast<unsigned>(std::time(NULL)));
    initialize();
}

void TrafficOptimizer::initialize()
{
    graph.addRoad(0, 1, 5.0);
    graph.addRoad(0, 2, 4.0);
    graph.addRoad(1, 2, 3.0);
    graph.addRoad(1, 3, 6.0);
    graph.addRoad(2, 3, 2.5);
    graph.addRoad(2, 4, 5.0);
    graph.addRoad(3, 4, 3.0);
    graph.addRoad(3, 5, 4.0);
    graph.addRoad(4, 5, 2.5);

    sensors.push_back(TrafficSensor("Clock Tower - ISBT", 25, 40, 20, false));
    sensors.push_back(TrafficSensor("Clock Tower - Rajpur Road", 35, 32, 30, false));
    sensors.push_back(TrafficSensor("ISBT - Rajpur Road", 45, 25, 50, false));
    sensors.push_back(TrafficSensor("ISBT - Ballupur", 60, 18, 70, false));
    sensors.push_back(TrafficSensor("Rajpur Road - Ballupur", 30, 35, 25, false));
    sensors.push_back(TrafficSensor("Rajpur Road - Prem Nagar", 20, 45, 15, true));
}

std::vector<TrafficSensor> TrafficOptimizer::getSensors() const { return sensors; }

double TrafficOptimizer::getTrafficMultiplier(const TrafficSensor& sensor) const
{
    if (sensor.getVehicles() >= 60) return 2.5;
    if (sensor.getVehicles() >= 45) return 2.0;
    if (sensor.getVehicles() >= 25) return 1.5;
    return 1.0;
}

void TrafficOptimizer::updateSensors()
{
    for (size_t i = 0; i < sensors.size(); ++i)
    {
        int vehicles = 10 + std::rand() % 61;
        double speed = 15 + std::rand() % 36;
        int waiting = 10 + std::rand() % 81;
        bool emergency = (std::rand() % 10 == 0);
        sensors[i].update(vehicles, speed, waiting, emergency);
    }
}

std::string TrafficOptimizer::getTrafficLevel(int vehicles) const
{
    if (vehicles < 25) return "LOW";
    if (vehicles < 45) return "MEDIUM";
    if (vehicles < 60) return "HIGH";
    return "VERY HIGH";
}

std::vector<int> TrafficOptimizer::dijkstra(int source, int destination) const
{
    const double INF = std::numeric_limits<double>::infinity();
    int n = graph.getVertexCount();
    std::vector<double> distance(n, INF);
    std::vector<int> parent(n, -1);

    typedef std::pair<double, int> Node;
    std::priority_queue<Node, std::vector<Node>, std::greater<Node> > pq;

    distance[source] = 0.0;
    pq.push(Node(0.0, source));

    while (!pq.empty())
    {
        Node current = pq.top();
        pq.pop();
        double currentDistance = current.first;
        int u = current.second;

        if (currentDistance > distance[u]) continue;

        std::vector<Edge> edges = graph.getEdges(u);
        for (size_t i = 0; i < edges.size(); ++i)
        {
            int v = edges[i].destination;
            double cost = edges[i].distance;
            int sensorIndex = (u + v) % static_cast<int>(sensors.size());
            cost *= getTrafficMultiplier(sensors[sensorIndex]);
            double newDistance = distance[u] + cost;

            if (newDistance < distance[v])
            {
                distance[v] = newDistance;
                parent[v] = u;
                pq.push(Node(newDistance, v));
            }
        }
    }

    std::vector<int> path;
    if (distance[destination] == INF) return path;

    int current = destination;
    while (current != -1)
    {
        path.push_back(current);
        current = parent[current];
    }
    std::reverse(path.begin(), path.end());
    return path;
}

double TrafficOptimizer::calculateRouteCost(const std::vector<int>& path) const
{
    double total = 0.0;
    if (path.size() < 2) return total;

    for (size_t i = 0; i + 1 < path.size(); ++i)
    {
        int u = path[i];
        int v = path[i + 1];
        std::vector<Edge> edges = graph.getEdges(u);
        for (size_t j = 0; j < edges.size(); ++j)
        {
            if (edges[j].destination == v)
            {
                int sensorIndex = (u + v) % static_cast<int>(sensors.size());
                total += edges[j].distance * getTrafficMultiplier(sensors[sensorIndex]);
                break;
            }
        }
    }
    return total;
}

int TrafficOptimizer::getHighestPrioritySensor() const
{
    if (sensors.empty()) return -1;
    int best = 0;
    for (size_t i = 1; i < sensors.size(); ++i)
    {
        if (sensors[i].calculatePriority() > sensors[best].calculatePriority())
            best = static_cast<int>(i);
    }
    return best;
}

std::string TrafficOptimizer::getSensorsJson() const
{
    std::stringstream json;
    json << "[";
    for (size_t i = 0; i < sensors.size(); ++i)
    {
        const TrafficSensor& s = sensors[i];
        json << "{";
        json << "\"road\":\"" << s.getRoad() << "\",";
        json << "\"vehicles\":" << s.getVehicles() << ",";
        json << "\"speed\":" << s.getSpeed() << ",";
        json << "\"waiting\":" << s.getWaitingTime() << ",";
        json << "\"emergency\":" << (s.hasEmergency() ? "true" : "false") << ",";
        json << "\"priority\":" << s.calculatePriority() << ",";
        json << "\"level\":\"" << getTrafficLevel(s.getVehicles()) << "\"";
        json << "}";
        if (i + 1 < sensors.size()) json << ",";
    }
    json << "]";
    return json.str();
}

std::string TrafficOptimizer::getPriorityJson() const
{
    std::vector<int> indices(sensors.size());
    for (size_t i = 0; i < sensors.size(); ++i) indices[i] = static_cast<int>(i);

    std::sort(indices.begin(), indices.end(), [this](int a, int b) {
        return sensors[a].calculatePriority() > sensors[b].calculatePriority();
    });

    std::stringstream json;
    json << "[";
    for (size_t i = 0; i < indices.size(); ++i)
    {
        const TrafficSensor& s = sensors[indices[i]];
        json << "{";
        json << "\"road\":\"" << s.getRoad() << "\",";
        json << "\"priority\":" << s.calculatePriority() << ",";
        json << "\"emergency\":" << (s.hasEmergency() ? "true" : "false");
        json << "}";
        if (i + 1 < indices.size()) json << ",";
    }
    json << "]";
    return json.str();
}

std::string TrafficOptimizer::getRouteJson(int source, int destination) const
{
    if (source < 0 || destination < 0 || source >= graph.getVertexCount() || destination >= graph.getVertexCount())
        return "{\"success\":false,\"error\":\"Invalid location\"}";

    std::vector<int> path = dijkstra(source, destination);
    std::stringstream json;
    json << "{";
    if (path.empty())
    {
        json << "\"success\":false";
        json << "}";
        return json.str();
    }

    json << "\"success\":true,\"route\":[";
    for (size_t i = 0; i < path.size(); ++i)
    {
        json << "\"" << graph.getName(path[i]) << "\"";
        if (i + 1 < path.size()) json << ",";
    }
    json << "],\"cost\":" << calculateRouteCost(path) << "}";
    return json.str();
}
