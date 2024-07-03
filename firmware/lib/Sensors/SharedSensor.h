#ifndef __SHARED_SENSOR_H__
#define __SHARED_SENSOR_H__

template <class T> class SharingSensor : public Sensor
{
protected:
    using SharedSensor = T;
    using SharedSensorPtr = std::shared_ptr<SharedSensor>;
    SharedSensorPtr sharedSensor;
    typename SharedSensor::BaseSensor& getSharedSensor() { return sharedSensor.get()->baseSensor; }

public:
    SharingSensor(SharedSensorPtr sharedSensor) : sharedSensor{sharedSensor} {}
    void setup() final { sharedSensor.get()->setup(); }
    void startMeasurement() final { sharedSensor.get()->startMeasurement(); }
};

template <class T, class D> class SharedSensor
{
public:
    using BaseSensor = T;
    BaseSensor baseSensor;

private:
    bool setupDone{false};
    bool measurementStarted{false};

protected:
    SharedSensor(BaseSensor&& baseSensor) : baseSensor{std::move(baseSensor)} {}

private:
    void setup()
    {
        if (!setupDone)
            D::setup();
        setupDone = true;
    }
    void startMeasurement()
    {
        if (!measurementStarted)
            D::startMeasurement();
        measurementStarted = true;
    }

public:
    template <class... Args> static std::shared_ptr<D> make(Args&&... args) { return std::make_shared<D>(std::forward<Args>(args)...); }

    friend class SharingSensor<SharedSensor>;
};

#endif