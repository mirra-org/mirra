#include "SoilTempSensor.h"
#include <DallasTemperature.h>

void SoilTemperatureSensor::startMeasurement() { dallas.begin(); }

SensorValue SoilTemperatureSensor::getMeasurement()
{

    DeviceAddress addr;
    dallas.getAddress(addr, this->busIndex);
    dallas.requestTemperaturesByAddress(addr);
    return SensorValue(getID(), dallas.getTempC(addr));
}