#include "LightSensor.h"
#include <Arduino.h>

void LightSensor::startMeasurement()
{
    baseSensor.begin(APDS9306_ALS_GAIN_1, APDS9306_ALS_MEAS_RES_16BIT_25MS);
    baseSensor.startLuminosityMeasurement();
}

SensorValue LightSensor::getMeasurement()
{
    while (!baseSensor.isMeasurementReady())
        ;
    return SensorValue(getTypeTag(), getInstanceTag(),
                       static_cast<float>(baseSensor.getLuminosityMeasurement().raw));
}