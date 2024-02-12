#include "SoilTempSensor.h"
#include <DallasTemperature.h>

void SoilTemperatureSensor::startMeasurement() { dallas.begin(); }

SensorValue SoilTemperatureSensor::getMeasurement()
{

    DeviceAddress longWireThermometer;
    dallas.getAddress(longWireThermometer, this->busIndex);
    dallas.requestTemperaturesByAddress(longWireThermometer);
    return SensorValue(getID(), dallas.getTempCByIndex(this->busIndex));
}