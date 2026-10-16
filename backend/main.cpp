#include "../httplib.h"
#include "TrafficOptimizer.h"
#include <iostream>
#include <string>

int main()
{
    TrafficOptimizer optimizer;
    httplib::Server server;

    server.Get("/api/sensors", [&optimizer](const httplib::Request&, httplib::Response& res) {
        res.set_content(optimizer.getSensorsJson(), "application/json");
    });

    server.Get("/api/priority", [&optimizer](const httplib::Request&, httplib::Response& res) {
        res.set_content(optimizer.getPriorityJson(), "application/json");
    });

    server.Post("/api/update", [&optimizer](const httplib::Request&, httplib::Response& res) {
        optimizer.updateSensors();
        res.set_content("{\"success\":true}", "application/json");
    });

    server.Get("/api/route", [&optimizer](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_param("source") || !req.has_param("destination")) {
            res.status = 400;
            res.set_content("{\"error\":\"Source and destination are required\"}", "application/json");
            return;
        }

        int source = std::atoi(req.get_param_value("source").c_str());
        int destination = std::atoi(req.get_param_value("destination").c_str());
        res.set_content(optimizer.getRouteJson(source, destination), "application/json");
    });

    if (!server.set_mount_point("/", "./frontend")) {
        std::cerr << "ERROR: frontend/index.html was not found.\n";
        std::cerr << "Run this program from the TrafficOptimizationSystem folder.\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "   TRAFFIC OPTIMIZATION SYSTEM\n";
    std::cout << "========================================\n";
    std::cout << "Server started successfully!\n";
    std::cout << "Open: http://localhost:8080\n";
    std::cout << "========================================\n\n";

    if (!server.listen("0.0.0.0", 8080)) {
        std::cerr << "Could not start server on port 8080.\n";
        return 1;
    }

    return 0;
}
