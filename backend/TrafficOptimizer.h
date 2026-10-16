#ifndef TRAFFIC_OPTIMIZER_H
#define TRAFFIC_OPTIMIZER_H

#include "Graph.h"
#include "TrafficSensor.h"
#include <string>
#include <vector>

class TrafficOptimizer
{
private:
    Graph graph;
    std::vector<TrafficSensor> sensors;

    double getTrafficMultiplier(const TrafficSensor& sensor) const;

public:
    TrafficOptimizer();
    void initialize();
    std::vector<TrafficSensor> getSensors() const;
    void updateSensors();
    std::vector<int> dijkstra(int source, int destination) const;
    double calculateRouteCost(const std::vector<int>& path) const;
    int getHighestPrioritySensor() const;
    std::string getTrafficLevel(int vehicles) const;
    std::string getRouteJson(int source, int destination) const;
    std::string getSensorsJson() const;
    std::string getPriorityJson() const;
};

#endif
