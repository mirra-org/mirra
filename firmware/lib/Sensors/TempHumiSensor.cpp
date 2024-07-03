#include "Wire.h"
#include <TempHumiSensor.h>

void SharedSHTSensor::setup(void)
{
    Serial.println("SHT setup.");
    if (baseSensor.init())
    {
        Serial.println("SHT init OK.");
    }
}
void SharedSHTSensor::startMeasurement() { baseSensor.readSample(); }

SensorValue TempSHTSensor::getMeasurement() { return SensorValue(getTypeTag(), getInstanceTag(), getSharedSensor().getTemperature()); }

SensorValue HumiSHTSensor::getMeasurement() { return SensorValue(getTypeTag(), getInstanceTag(), getSharedSensor().getHumidity()); }
