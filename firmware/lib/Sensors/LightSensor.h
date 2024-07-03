#ifndef __LIGHT_SENSOR_H__
#define __LIGHT_SENSOR_H__

#include "Sensor.h"
#include <AsyncAPDS9306.h>

#define LIGHT_KEY 22

class LightSensor final : public Sensor
{
private:
    AsyncAPDS9306 baseSensor{};

public:
    LightSensor() = default;
    void startMeasurement();
    SensorValue getMeasurement();
    uint32_t getTypeTag() const { return LIGHT_KEY; };
};
#endif