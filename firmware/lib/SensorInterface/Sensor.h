#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <memory>
#include <stdint.h>

struct SensorValue
{
    uint8_t typeTag;
    uint8_t instanceTag;
    float value;

    SensorValue() = default;
    /// @brief Constructs a SensorValue from the appropriate tags and value.
    /// @param typeTag Type ID of the sensor.
    /// @param instanceTag Instance ID of the sensor.
    /// @param value Concrete value.
    SensorValue(uint8_t typeTag, u_int8_t instanceTag, float value)
        : typeTag{typeTag}, instanceTag{instanceTag}, value{value} {};
} __attribute__((packed));

class Sensor
{
public:
    /// @brief Initialises the sensor.
    virtual void setup() {}
    /// @brief Starts the measurement of the sensor.
    virtual void startMeasurement() {}
    // @return The measured value.
    virtual SensorValue getMeasurement() = 0;
    /// @return The sensor's type ID.
    virtual uint8_t getTypeTag() const = 0;
    uint8_t getInstanceTag() { return instanceTag; }
    /// @return The sensor's name.
    virtual const char* getName() const { return "Unnamed Sensor"; };
    /// @brief Updates the sensor's next sample time according to the sensor-specific algorithm.
    /// (usually simply addition)
    /// @param sampleInterval Sample interval with which to update.
    virtual void updateNextSampleTime(uint32_t sampleInterval)
    {
        this->nextSampleTime += sampleInterval;
    };
    /// @brief Forcibly sets the nextSampleTime of the sensor.
    void setNextSampleTime(uint32_t nextSampleTime) { this->nextSampleTime = nextSampleTime; };
    /// @return The next sample time in seconds from UNIX epoch.
    uint32_t getNextSampleTime() const { return nextSampleTime; };
    virtual ~Sensor() = default;

protected:
    uint8_t instanceTag{0};
    uint32_t nextSampleTime = -1;
};

#endif
