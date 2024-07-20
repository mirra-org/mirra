#include "sensornode.h"

#include <BatterySensor.h>
#include <ESPCamUART.h>
#include <LightSensor.h>
#include <RandomSensor.h>
#include <SoilTempSensor.h>
#include <TempHumiSensor.h>

using namespace mirra;

RTC_DATA_ATTR bool initialBoot = true;

RTC_DATA_ATTR std::array<uint32_t, MAX_SENSORS> sensorsNextSampleTimes{0};
RTC_DATA_ATTR uint32_t sampleInterval{DEFAULT_SAMPLING_INTERVAL};
RTC_DATA_ATTR uint32_t sampleRounding{DEFAULT_SAMPLING_ROUNDING};
RTC_DATA_ATTR uint32_t sampleOffset{DEFAULT_SAMPLING_OFFSET};
RTC_DATA_ATTR uint32_t nextSampleTime = -1;
RTC_DATA_ATTR uint32_t commInterval;
RTC_DATA_ATTR uint32_t nextCommTime = -1;
RTC_DATA_ATTR uint32_t maxMessages;
RTC_DATA_ATTR MACAddress gatewayMAC;

SensorNode::SensorNode(const MIRRAPins& pins) : MIRRAModule(pins)
{
    if (initialBoot)
    {
        initSensors();
        clearSensors();
        discovery();
        initialBoot = false;
    }
}

void SensorNode::wake()
{
    Log::debug("Running wake()...");
    uint32_t cTime{rtc.getSysTime()};
    if (cTime >= WAKE_COMM_PERIOD(nextCommTime))
        commPeriod();
    cTime = rtc.getSysTime();
    if (cTime >= nextSampleTime)
    {
        samplePeriod();
    }
    cTime = rtc.getSysTime();
    Log::info("Next sample in ", nextSampleTime - cTime, "s, next comm period in ",
              nextCommTime - cTime, "s");
    Serial.printf("Welcome! This is Sensor Node %s\n", lora.getMACAddress().toString());
    commandEntry.prompt(Commands(this));
    cTime = rtc.getSysTime();
    if (cTime >= nextCommTime || cTime >= nextSampleTime)
        wake();
    Log::debug("Entering deep sleep...");
    deepSleepUntil(std::min(WAKE_COMM_PERIOD(nextCommTime), nextSampleTime));
}

void SensorNode::discovery()
{
    Log::info("Sending hello message...");
    lora.sendMessage(Message<HELLO>(lora.getMACAddress(), MACAddress::broadcast));
    Log::debug("Awaiting time config message...");
    auto timeConfig{lora.receiveMessage<TIME_CONFIG>(TIME_CONFIG_TIMEOUT, TIME_CONFIG_ATTEMPTS,
                                                     MACAddress::broadcast)};
    const MACAddress& gatewayMAC{timeConfig->getSource()};
    if (!timeConfig)
    {
        Log::error("Error while awaiting time config message from gateway. Aborting discovery.");
        return;
    }
    this->timeConfig(*timeConfig);
    Log::debug("Time config message received. Sending TIME_ACK");
    lora.sendMessage(Message<ACK_TIME>(lora.getMACAddress(), gatewayMAC));
    lora.receiveMessage<REPEAT>(TIME_CONFIG_TIMEOUT, 0, gatewayMAC);
}

void SensorNode::timeConfig(Message<TIME_CONFIG>& m)
{
    rtc.writeTime(m.getCTime());
    rtc.setSysTime();
    bool scheduleValid{sampleInterval == m.getSampleInterval() &&
                       sampleRounding == m.getSampleRounding() &&
                       sampleOffset == m.getSampleOffset()};
    sampleInterval = m.getSampleInterval() == 0 ? DEFAULT_SAMPLING_INTERVAL : m.getSampleInterval();
    sampleRounding = m.getSampleRounding() == 0 ? DEFAULT_SAMPLING_ROUNDING : m.getSampleRounding();
    sampleOffset = m.getSampleOffset();
    commInterval = m.getCommInterval();
    nextCommTime = m.getCommTime();
    maxMessages = m.getMaxMessages();
    gatewayMAC = m.getSource();
    if (!scheduleValid)
    {
        sensorsNextSampleTimes.fill(0);
        initSensors();
        clearSensors();
    }
    Log::info("Sample interval: ", sampleInterval, ", Comm interval: ", commInterval,
              ", Max messages: ", maxMessages, ", Gateway MAC: ", gatewayMAC.toString());
}

void SensorNode::addSensor(std::unique_ptr<Sensor>&& sensor)
{
    if (nSensors > MAX_SENSORS)
        return;
    uint32_t cTime{rtc.getSysTime()};
    if (sensorsNextSampleTimes[nSensors] == 0)
    {
        sensor->setNextSampleTime(((cTime / sampleRounding) * sampleRounding + sampleOffset));
        while (sensor->getNextSampleTime() <= cTime)
            sensor->updateNextSampleTime(sampleInterval);
    }
    else
    {
        sensor->setNextSampleTime(sensorsNextSampleTimes[nSensors]);
    }
    sensor->setup();
    sensors[nSensors] = std::move(sensor);
    nSensors++;
}

void SensorNode::initSensors()
{
    addSensor(std::make_unique<RandomSensor>(rtc.getSysTime()));
    addSensor(std::make_unique<SoilTemperatureSensor>(SOILTEMP_PIN, SOILTEMP_BUS_INDEX));
    addSensor(std::make_unique<LightSensor>());
    auto shtSensor = SharedSHTSensor::make();
    addSensor(std::make_unique<TempSHTSensor>(shtSensor));
    addSensor(std::make_unique<HumiSHTSensor>(shtSensor));
    addSensor(std::make_unique<BatterySensor>(BATT_PIN, BATT_EN_PIN));
}

void SensorNode::clearSensors()
{
    nextSampleTime = -1;
    for (size_t i{0}; i < nSensors; i++)
    {
        sensorsNextSampleTimes[i] = sensors[i]->getNextSampleTime();
        if (sensors[i]->getNextSampleTime() < nextSampleTime)
            nextSampleTime = sensors[i]->getNextSampleTime();
        sensors[i].reset();
    }
    nSensors = 0;
}

SensorNode::SensorFile::DataEntry SensorNode::sampleAll()
{
    Log::info("Sampling all sensors...");
    for (size_t i{0}; i < nSensors; i++)
    {
        Serial.printf("Starting measurement for %u\n", sensors[i]->getTypeTag());
        sensors[i]->startMeasurement();
    }
    SensorFile::DataEntry::SensorValueArray values;
    for (size_t i{0}; i < nSensors; i++)
    {
        Serial.printf("Getting measurement for %u\n", sensors[i]->getTypeTag());
        values[i] = sensors[i]->getMeasurement();
    }
    return SensorFile::DataEntry{
        lora.getMACAddress(), 0,
        SensorFile::DataEntry::Flags{static_cast<uint8_t>(nSensors), false}, values};
}

SensorNode::SensorFile::DataEntry SensorNode::sampleScheduled(uint32_t cTime)
{
    Log::info("Sampling scheduled sensors...");
    for (size_t i{0}; i < nSensors; i++)
    {
        if (sensors[i]->getNextSampleTime() == cTime)
            sensors[i]->startMeasurement();
    }
    SensorFile::DataEntry::SensorValueArray values;
    uint8_t nValues{0};
    for (size_t i{0}; i < nSensors; i++)
    {
        if (sensors[i]->getNextSampleTime() == cTime)
        {
            values[nValues] = sensors[i]->getMeasurement();
            nValues++;
        }
    }
    return SensorFile::DataEntry{lora.getMACAddress(), cTime,
                                 SensorFile::DataEntry::Flags{nValues, false}, values};
}

void SensorNode::updateSensorsSampleTimes(uint32_t cTime)
{
    for (size_t i = 0; i < nSensors; i++)
    {
        while (sensors[i]->getNextSampleTime() <= cTime)
            sensors[i]->updateNextSampleTime(sampleInterval);
    }
}
void SensorNode::samplePeriod()
{
    initSensors();
    auto lambdaByNextSampleTime = [](const std::unique_ptr<Sensor>& a,
                                     const std::unique_ptr<Sensor>& b) {
        return a->getNextSampleTime() < b->getNextSampleTime();
    };
    uint32_t cTime{(*std::min_element(sensors.begin(), std::next(sensors.begin(), nSensors),
                                      lambdaByNextSampleTime))
                       ->getNextSampleTime()};
    SensorFile::DataEntry entry{sampleScheduled(cTime)};
    SensorFile file;
    file.push(entry);
    updateSensorsSampleTimes(cTime);
    clearSensors();
}

void SensorNode::commPeriod()
{
    uint32_t cTime{rtc.getSysTime()};
    if (cTime >= nextCommTime + (SENSOR_DATA_TIMEOUT / 1000))
    {
        Log::error("Too late to start comm period. Skipping and assuming next comm period from "
                   "given interval.");
        while (nextCommTime <= cTime)
            nextCommTime += commInterval;
        return;
    }
    MACAddress _gatewayMAC{gatewayMAC}; // avoid access to slow RTC memory
    Log::info("Communicating with gateway ", _gatewayMAC.toString(), " ...");
    size_t _maxMessages{maxMessages}; // avoid access to slow RTC memory
    Log::debug("Max messages to send: ", _maxMessages);
    SensorFile file;
    bool firstMessage{true};
    for (size_t i{0}; i < _maxMessages; i++)
    {
        auto entry = file.getUnuploaded(0);
        if (!entry)
            break;

        Message<SENSOR_DATA> message{entry->source, _gatewayMAC, entry->time, entry->flags.nValues,
                                     entry->values};
        if ((i == _maxMessages - 1) || (file.isLast(0)))
        {
            Log::debug("Last sensor data message...");
            message.setLast();
        }
        if (sendSensorMessage(message, firstMessage))
            file.setUploaded();
        firstMessage = false;
    }
}

bool SensorNode::sendSensorMessage(Message<SENSOR_DATA>& message, bool firstMessage)
{
    Log::debug("Sending data message...");
    if (firstMessage)
    {
        lightSleepUntil(nextCommTime);
        lora.sendMessage(message, 0); // gateway should already be listening for first message
    }
    else
    {
        lora.sendMessage(message);
    }
    Log::debug("Awaiting acknowledgement...");
    if (!message.isLast())
    {
        auto dataAck{lora.receiveMessage<ACK_DATA>(SENSOR_DATA_TIMEOUT, SENSOR_DATA_ATTEMPTS,
                                                   message.getDest())};
        if (dataAck)
        {
            return 1;
        }
        else
        {
            Log::error("Error while uploading to gateway.");
            return 0;
        }
    }
    else
    {
        auto timeConfig{lora.receiveMessage<TIME_CONFIG>(TIME_CONFIG_TIMEOUT, TIME_CONFIG_ATTEMPTS,
                                                         message.getDest())};
        if (timeConfig)
        {
            this->timeConfig(*timeConfig);
            lora.sendMessage(Message<ACK_TIME>(lora.getMACAddress(), message.getDest()));
            lora.receiveMessage<REPEAT>(TIME_CONFIG_TIMEOUT, 0, gatewayMAC);
            return 1;
        }
        else
        {
            Log::error("Error while receiving new time config from gateway. Assuming next comm "
                       "period from given interval.");
            while (nextCommTime <= rtc.getSysTime())
                nextCommTime += commInterval;
            return 0;
        }
    }
}

CommandCode SensorNode::Commands::discovery()
{
    parent->discovery();
    return COMMAND_SUCCESS;
}
CommandCode SensorNode::Commands::sample()
{
    parent->samplePeriod();
    return COMMAND_SUCCESS;
}

CommandCode SensorNode::Commands::printSample()
{
    parent->initSensors();
    SensorFile::DataEntry entry{parent->sampleAll()};
    const auto& values = entry.values;
    Serial.println("TAG\tVALUE");
    for (size_t i{0}; i < entry.flags.nValues; i++)
    {
        Serial.printf("%u\t%f\n", values[i].typeTag, values[i].value);
    }
    parent->clearSensors();
    return COMMAND_SUCCESS;
}

CommandCode SensorNode::Commands::printSchedule()
{
    parent->initSensors();
    constexpr size_t timeLength{sizeof("0000-00-00 00:00:00")};
    char buffer[timeLength]{0};
    Serial.println("TAG\tNEXT SAMPLE");
    for (size_t i{0}; i < parent->nSensors; i++)
    {
        tm time;
        time_t nextSensorSampleTime{static_cast<time_t>(parent->sensors[i]->getNextSampleTime())};
        gmtime_r(&nextSensorSampleTime, &time);
        strftime(buffer, timeLength, "%F %T", &time);
        Serial.printf("%u\t%s\n", parent->sensors[i]->getTypeTag(), buffer);
    }
    parent->clearSensors();
    return COMMAND_SUCCESS;
}