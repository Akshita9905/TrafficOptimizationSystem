#include "TrafficSensor.h"

TrafficSensor::TrafficSensor()
    : road(""), vehicles(0), speed(0), waitingTime(0), emergency(false) {}

TrafficSensor::TrafficSensor(std::string road, int vehicles, double speed, int waitingTime, bool emergency)
    : road(road), vehicles(vehicles), speed(speed), waitingTime(waitingTime), emergency(emergency) {}

int TrafficSensor::calculatePriority() const
{
    int priority = vehicles * 2 + waitingTime;
    if (emergency) priority += 100;
    return priority;
}

std::string TrafficSensor::getRoad() const { return road; }
int TrafficSensor::getVehicles() const { return vehicles; }
double TrafficSensor::getSpeed() const { return speed; }
int TrafficSensor::getWaitingTime() const { return waitingTime; }
bool TrafficSensor::hasEmergency() const { return emergency; }

void TrafficSensor::update(int vehicles, double speed, int waitingTime, bool emergency)
{
    this->vehicles = vehicles;
    this->speed = speed;
    this->waitingTime = waitingTime;
    this->emergency = emergency;
}
