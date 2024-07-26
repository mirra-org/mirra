#ifndef __TEMP_RH_SENSOR_H__
#define __TEMP_RH_SENSOR_H__

#include "Sensor.h"
#include "SharedSensor.h"
#include <Arduino.h>
#include <SHTSensor.h>

#define TEMP_SHT_KEY 12
#define HUMI_SHT_KEY 13

class SharedSHTSensor final : public SharedSensor<SHTSensor, SharedSHTSensor>
{
public:
    SharedSHTSensor() : SharedSensor(SHTSensor::SHT3X) {}
    void setup();
    void startMeasurement();
};

class TempSHTSensor final : public SharingSensor<SharedSHTSensor>
{
public:
    using SharingSensor<SharedSensor>::SharingSensor;
    SensorValue getMeasurement();
    uint32_t getTypeTag() const { return TEMP_SHT_KEY; }
};

class HumiSHTSensor final : public SharingSensor<SharedSHTSensor>
{
public:
    using SharingSensor<SharedSensor>::SharingSensor;
    SensorValue getMeasurement();
    uint32_t getTypeTag() const { return HUMI_SHT_KEY; }
};
#endif