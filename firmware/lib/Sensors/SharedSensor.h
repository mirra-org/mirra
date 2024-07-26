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
    void setup() final { sharedSensor.get()->sharedSetup(); }
    void startMeasurement() final { sharedSensor.get()->sharedStartMeasurement(); }
};

template <class T, class D> class SharedSensor
{
    bool setupDone{false};
    bool measurementStarted{false};

    void sharedSetup()
    {
        if (!setupDone)
            static_cast<D*>(this)->setup();
        setupDone = true;
    }
    void sharedStartMeasurement()
    {
        if (!measurementStarted)
            static_cast<D*>(this)->startMeasurement();
        measurementStarted = true;
    }

protected:
    using BaseSensor = T;
    SharedSensor(BaseSensor&& baseSensor) : baseSensor{std::move(baseSensor)} {}
    BaseSensor baseSensor;

public:
    template <class... Args> static std::shared_ptr<D> make(Args&&... args)
    {
        return std::make_shared<D>(std::forward<Args>(args)...);
    }

    friend class SharingSensor<D>;
};

#endif