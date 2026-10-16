#ifndef TRAFFIC_SENSOR_H
#define TRAFFIC_SENSOR_H

#include <string>

class TrafficSensor
{
private:
    std::string road;
    int vehicles;
    double speed;
    int waitingTime;
    bool emergency;

public:
    TrafficSensor();
    TrafficSensor(std::string road, int vehicles, double speed, int waitingTime, bool emergency);

    int calculatePriority() const;
    std::string getRoad() const;
    int getVehicles() const;
    double getSpeed() const;
    int getWaitingTime() const;
    bool hasEmergency() const;

    void update(int vehicles, double speed, int waitingTime, bool emergency);
};

#endif
