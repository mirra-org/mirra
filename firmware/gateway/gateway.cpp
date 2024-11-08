#include "gateway.h"
#include "HTTPClient.h"
#include <cstring>
#include <esp_sntp.h>

using namespace mirra;

void Node::timeConfig(Message<TIME_CONFIG>& m)
{
    this->sampleInterval = m.getSampleInterval();
    this->sampleRounding = m.getSampleRounding();
    this->sampleOffset = m.getSampleOffset();
    this->lastCommTime = m.getCTime();
    this->commInterval = m.getCommInterval();
    this->nextCommTime = m.getCommTime();
    this->maxMessages = m.getMaxMessages();
    if (this->errors > 0)
        this->errors--;
}

void Node::naiveTimeConfig(uint32_t cTime)
{
    while (this->nextCommTime <= cTime)
        this->nextCommTime += commInterval;
    this->errors++;
}

Message<TIME_CONFIG> Node::currentTimeConfig(const MACAddress& src, uint32_t cTime)
{
    return Message<TIME_CONFIG>(src, mac, cTime, sampleInterval, sampleRounding, sampleOffset,
                                commInterval, nextCommTime, maxMessages);
}

RTC_DATA_ATTR bool initialBoot{true};
RTC_DATA_ATTR int commPeriods{0};

RTC_DATA_ATTR char ssid[33]{WIFI_SSID};
RTC_DATA_ATTR char pass[33]{WIFI_PASS};

RTC_DATA_ATTR char mqttServer[65]{MQTT_SERVER};
RTC_DATA_ATTR uint16_t mqttPort{MQTT_PORT};
RTC_DATA_ATTR char mqttPsk[65]{MQTT_PSK};

RTC_DATA_ATTR uint32_t sampleInterval{DEFAULT_SAMPLE_INTERVAL};
RTC_DATA_ATTR uint32_t sampleRounding{DEFAULT_SAMPLE_ROUNDING};
RTC_DATA_ATTR uint32_t sampleOffset{DEFAULT_SAMPLE_OFFSET};
RTC_DATA_ATTR uint32_t commInterval{DEFAULT_COMM_INTERVAL};

// TODO: Instead of using this lambda to determine if a node is lost, use a bool stored in each node
// that signifies if a node is ' well-scheduled ', implying both that the node is not lost and that
// it follows the scheduled comm time of the previous node tightly (i.e., without scheduling gaps).
// Removal of a node would imply loss of well-scheduledness of all following nodes, and changing of
// the comm period would imply loss of well-scheduledness for all nodes. Currently only the latter
// portion of this functionality is available with just the isLost lambda, During the comm period, a
// node that is not 'well-scheduled' would be scheduled so that it would become so.

auto lambdaIsLost = [](const Node& e) { return e.getCommInterval() != commInterval; };

Gateway::Gateway(const MIRRAPins& pins) : MIRRAModule(pins)
{
    if (initialBoot)
    {
        Log::info("First boot.");
        Commands(this).rtcUpdateTime();
        initialBoot = false;
    }
    loadNodes();
}

void Gateway::wake()
{
    Log::debug("Running wake()...");
    if (!nodes.empty() && rtc.getSysTime() >= (WAKE_COMM_PERIOD(nodes[0].getNextCommTime()) - 3))
        commPeriod();
    // send data to server only every UPLOAD_EVERY comm periods
    if (commPeriods >= UPLOAD_EVERY)
    {
        uploadPeriod();
    }
    Serial.printf("Welcome! This is Gateway %s\n", lora.getMACAddress().toString());
    commandEntry.prompt(Commands(this));
    Log::debug("Entering deep sleep...");
    if (nodes.empty())
        deepSleep(commInterval);
    else
        deepSleepUntil(WAKE_COMM_PERIOD(nodes[0].getNextCommTime()));
}

std::optional<std::reference_wrapper<Node>> Gateway::macToNode(const MACAddress& mac)
{
    for (Node& n : nodes)
    {
        if (mac == n.getMACAddress())
            return std::make_optional(std::ref(n));
    }
    return std::nullopt;
}

void Gateway::discovery()
{
    Log::info("Starting discovery...");
    while (true)
    {
        Log::info("Awaiting discovery message...");
        auto hello{lora.listenMessage<HELLO>(DISCOVERY_TIMEOUT, pins.bootPin)};
        if (!hello)
        {
            if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT1)
            {
                Log::info("Discovery aborted with BOOT button.");
                return;
            }
            continue;
        }
        MACAddress candidate = hello->getSource();
        Log::info("Node found at ", candidate.toString());

        auto duplicate{macToNode(candidate)};

        uint32_t cTime{rtc.getSysTime()};
        Log::debug("Sending time config message to ", candidate.toString());
        if (duplicate)
        {
            lora.sendMessage(duplicate->get().currentTimeConfig(lora.getMACAddress(), cTime));
        }
        else
        {
            if (nodes.size() >= MAX_SENSOR_NODES)
            {
                Log::info("Could not add node because maximum amount of nodes has been reached.");
                return;
            }
            uint32_t commTime{std::all_of(nodes.cbegin(), nodes.cend(), lambdaIsLost)
                                  ? cTime + commInterval
                                  : nextScheduledCommTime()};
            Message<TIME_CONFIG> timeConfig{
                lora.getMACAddress(), candidate,      cTime,
                sampleInterval,       sampleRounding, sampleOffset,
                commInterval,         commTime,       MAX_MESSAGES(commInterval, sampleInterval)};
            Log::debug("Time config constructed. cTime = ", cTime,
                       " sampleInterval = ", sampleInterval, " sampleRounding = ", sampleRounding,
                       " sampleOffset = ", sampleOffset, " commInterval = ", commInterval,
                       " comTime = ", commTime);
            nodes.emplace_back(timeConfig);
            lora.sendMessage(timeConfig);
        }
        auto timeAck{
            lora.receiveMessage<ACK_TIME>(TIME_CONFIG_TIMEOUT, TIME_CONFIG_ATTEMPTS, candidate)};
        if (!timeAck)
        {
            Log::error("Error while receiving ack to time config message from ",
                       candidate.toString(), ". Aborting discovery.");
            if (!duplicate)
                nodes.pop_back();
            continue;
        }

        Log::info("Node ", timeAck->getSource().toString(), " has been registered.");
        storeNodes();
    }
}

void Gateway::loadNodes()
{
    Log::debug("Recovering nodes from file...");
    fs::NVS nvsNodes{"nodes"};
    for (const char* nodeMac : nvsNodes)
    {
        nodes.push_back(nvsNodes.getValue<Node>(nodeMac));
    }
    Log::debug(nodes.size(), " nodes found in NVS.");
}

void Gateway::storeNodes()
{
    fs::NVS nvsNodes{"nodes"};
    for (const Node& node : nodes)
    {
        Node defaultNode;
        char key[7]{0};
        strncpy(key, reinterpret_cast<const char*>(node.getMACAddress().getAddress()),
                MACAddress::length);
        nvsNodes.getValue<Node>(key, defaultNode) = node;
    }
}

void Gateway::removeNode(Node& node)
{
    fs::NVS nvsNodes{"nodes"};
    char key[7]{0};
    strncpy(key, reinterpret_cast<const char*>(node.getMACAddress().getAddress()),
            MACAddress::length);
    nvsNodes.eraseKey(key);
    nvsNodes.commit();
    nodes.clear();
    loadNodes();
}

void Gateway::commPeriod()
{
    Log::info("Starting comm period...");
    std::vector<Message<SENSOR_DATA>> data;
    size_t expectedMessages{0};
    for (const Node& n : nodes)
        expectedMessages += n.getMaxMessages();
    data.reserve(expectedMessages);
    auto lambdaByNextCommTime = [](const Node& a, const Node& b) {
        return a.getNextCommTime() < b.getNextCommTime();
    };
    std::sort(nodes.begin(), nodes.end(), lambdaByNextCommTime);
    uint32_t farCommTime = -1;
    for (Node& n : nodes)
    {
        if (n.getNextCommTime() > farCommTime)
            break;
        farCommTime = n.getNextCommTime() +
                      2 * (COMM_PERIOD_LENGTH(MAX_MESSAGES(commInterval, n.getSampleInterval())) +
                           COMM_PERIOD_PADDING);
        if (!nodeCommPeriod(n, data))
            n.naiveTimeConfig(rtc.getSysTime());
    }
    std::sort(nodes.begin(), nodes.end(), lambdaByNextCommTime);
    if (nodes.empty())
    {
        Log::info("No comm periods performed because no nodes have been registered.");
    }
    else
    {
        storeNodes();
    }
    SensorFile file{};
    for (const Message<SENSOR_DATA>& m : data)
    {
        file.push(m);
    }
    commPeriods++;
}

uint32_t Gateway::nextScheduledCommTime()
{
    for (size_t i{1}; i <= nodes.size(); i++)
    {
        const Node& n{nodes[nodes.size() - i]};
        if (!lambdaIsLost(n))
            return n.getNextCommTime() + COMM_PERIOD_LENGTH(n.getMaxMessages()) +
                   COMM_PERIOD_PADDING;
    }
    Log::error("Next scheduled comm time was asked but all nodes are lost!");
    return -1;
}

bool Gateway::nodeCommPeriod(Node& n, std::vector<Message<SENSOR_DATA>>& data)
{
    uint32_t cTime{rtc.getSysTime()};
    if (cTime > n.getNextCommTime())
    {
        Log::error("Node ", n.getMACAddress().toString(),
                   "'s comm time was faultily scheduled before this gateway's comm period. "
                   "Skipping communication with this node.");
        return false;
    }
    lightSleepUntil(
        LISTEN_COMM_PERIOD(n.getNextCommTime()));  // light sleep until scheduled comm period
    uint32_t listenMs{COMM_PERIOD_PADDING * 1000}; // pre-listen in anticipation of message
    size_t messagesReceived{0};
    while (true)
    {
        Log::debug("Awaiting data from ", n.getMACAddress().toString(), " ...");
        auto sensorData{lora.receiveMessage<SENSOR_DATA>(SENSOR_DATA_TIMEOUT, SENSOR_DATA_ATTEMPTS,
                                                         n.getMACAddress(), listenMs)};
        listenMs = 0;
        if (!sensorData)
        {
            Log::error("Error while awaiting/receiving data from ", n.getMACAddress().toString(),
                       ". Skipping communication with this node.");
            return false;
        }
        Log::info("Sensor data received from ", n.getMACAddress().toString(), " with length ",
                  sensorData->getLength());
        data.push_back(*sensorData);
        messagesReceived++;
        if (sensorData->isLast() || messagesReceived >= n.getMaxMessages())
        {
            Log::debug("Last message received.");
            break;
        }
        Log::debug("Sending data ACK to ", n.getMACAddress().toString(), " ...");
        lora.sendMessage(Message<ACK_DATA>(lora.getMACAddress(), n.getMACAddress()));
    }
    uint32_t commTime{n.getNextCommTime() + commInterval};
    if (lambdaIsLost(n) && !(std::all_of(nodes.cbegin(), nodes.cend(), lambdaIsLost)))
        commTime = nextScheduledCommTime();
    Log::info("Sending time config message to ", n.getMACAddress().toString(), " ...");
    cTime = rtc.getSysTime();
    Message<TIME_CONFIG> timeConfig{lora.getMACAddress(),
                                    n.getMACAddress(),
                                    cTime,
                                    n.getSampleInterval(),
                                    n.getSampleRounding(),
                                    n.getSampleOffset(),
                                    commInterval,
                                    commTime,
                                    MAX_MESSAGES(commInterval, n.getSampleInterval())};
    lora.sendMessage(timeConfig);
    auto timeAck =
        lora.receiveMessage<ACK_TIME>(TIME_CONFIG_TIMEOUT, TIME_CONFIG_ATTEMPTS, n.getMACAddress());
    if (!timeAck)
    {
        Log::error("Error while receiving ack to time config message from ",
                   n.getMACAddress().toString(), ". Skipping communication with this node.");
        return false;
    }
    Log::info("Communication with node ", n.getMACAddress().toString(),
              " successful: ", messagesReceived, " messages received");
    n.timeConfig(timeConfig);
    return true;
}

void Gateway::wifiConnect(const char* SSID, const char* password)
{
    Log::info("Connecting to WiFi with SSID: ", SSID);
    WiFi.begin(SSID, password);
    for (size_t i = 10; i > 0 && WiFi.status() != WL_CONNECTED; i--)
    {
        delay(1000);
        Serial.print('.');
    }
    Serial.print('\n');
    if (WiFi.status() != WL_CONNECTED)
    {
        Log::error("Could not connect to WiFi.");
        return;
    }
    Log::info("Connected to WiFi.");
}

void Gateway::wifiConnect()
{
    wifiConnect(ssid, pass);
}

bool Gateway::MQTTClient::clientConnect()
{
    for (size_t i = 0; i < MQTT_ATTEMPTS; i++)
    {
        if (mqtt.connected())
            return true;
        if (mqtt.connect(identity))
            return true;
        delay(MQTT_TIMEOUT);
    }
    return false;
}

char* Gateway::createTopic(char* topic, const MACAddress& nodeMAC)
{
    char macStringBuffer[MACAddress::stringLength];
    snprintf(topic, topicSize, "%s/%s/%s", TOPIC_PREFIX, lora.getMACAddress().toString(),
             nodeMAC.toString(macStringBuffer));
    return topic;
}

void Gateway::uploadPeriod()
{
    Log::info("Commencing upload to MQTT server...");
    wifiConnect();
    SensorFile file{};
    if (WiFi.status() == WL_CONNECTED)
    {
        MQTTClient mqtt{mqttServer, mqttPort, lora.getMACAddress(), mqttPsk};
        size_t nErrors{0}; // amount of errors while uploading
        size_t messagesPublished{0};
        while (true)
        {
            auto entry = file.getUnuploaded(0);
            if (!entry)
                break; // no more unuploaded entries remaining
            char topic[topicSize];
            createTopic(topic, entry->source);

            if (mqtt.clientConnect())
            {
                if (mqtt.mqtt.publish(topic, reinterpret_cast<uint8_t*>(&entry), entry->getSize()))
                {
                    Log::debug("MQTT message successfully published.");
                    file.setUploaded();
                    messagesPublished++;
                }
                else
                {
                    Log::error("Error while publishing to MQTT server. State: ", mqtt.mqtt.state());
                    nErrors++;
                }
            }
            else
            {
                Log::error("Error while connecting to MQTT server. Aborting upload. State: ",
                           mqtt.mqtt.state());
                break;
            }
            if (nErrors >= MAX_MQTT_ERRORS)
            {
                Log::error("Too many errors while publishing to MQTT server. Aborting upload.");
                break;
            }
        }
        mqtt.mqtt.disconnect();
        WiFi.disconnect();
        Log::info("MQTT upload finished with ", messagesPublished, " messages sent.");
    }
    else
    {
        Log::error("WiFi not connected. Aborting upload to MQTT server...");
    }
    commPeriods = 0;
}

void Gateway::parseNodeUpdate(char* update)
{
    if (strlen(update) < (MACAddress::stringLength + 2 + 2 + 1))
    {
        Log::error("Update string '", update, "' has invalid length.");
        return;
    }
    auto node{macToNode(MACAddress::fromString(update))};
    if (!node)
    {
        Log::error("Could not deduce node from update string.");
        return;
    }
    char* timeString{&update[MACAddress::stringLength - 1]};
    uint32_t sampleInterval, sampleRounding, sampleOffset;
    if (sscanf(timeString, "/%u/%u/%u", &sampleInterval, &sampleRounding, &sampleOffset) != 3)
    {
        Log::error("Could not deduce updated timings from update string '", update, "'.");
        return;
    }
    node->get().setSampleInterval(sampleInterval);
    node->get().setSampleRounding(sampleRounding);
    node->get().setSampleOffset(sampleOffset);
}

CommandCode Gateway::Commands::changeWifi()
{
    char ssidBuffer[sizeof(ssid)];
    strncpy(ssidBuffer, ssid, sizeof(ssid));
    Serial.printf("Enter WiFi SSID (current: '%s') :\n", ssidBuffer);
    if (!CommandParser::editValue(ssidBuffer))
        return COMMAND_TIMEOUT;

    char passBuffer[sizeof(pass)];
    strncpy(passBuffer, ssid, sizeof(pass));
    Serial.println("Enter WiFi password:");
    if (!CommandParser::editValue(passBuffer))
        return COMMAND_TIMEOUT;

    parent->wifiConnect(ssidBuffer, passBuffer);
    if (WiFi.status() == WL_CONNECTED)
    {
        WiFi.disconnect();
        strncpy(ssid, ssidBuffer, sizeof(ssid));
        strncpy(pass, passBuffer, sizeof(pass));
        Serial.println("WiFi connected! This WiFi network has been set as the default.");
        return COMMAND_SUCCESS;
    }
    else
    {
        Serial.println("Could not connect to the supplied WiFi network.");
        return COMMAND_ERROR;
    }
}

CommandCode Gateway::Commands::rtcUpdateTime()
{
    parent->wifiConnect();
    if (WiFi.status() == WL_CONNECTED)
    {
        Log::info("Fetching NTP time.");
        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setservername(0, NTP_URL);
        sntp_set_sync_interval(15000);
        sntp_init();
        while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED)
        {
            delay(500);
            Serial.print(".");
        }
        Serial.println("\nWriting time to RTC...");
        parent->rtc.writeTime(parent->rtc.getSysTime());
        Log::info("RTC and system time updated.");
        sntp_stop();
    }
    WiFi.disconnect();
    return COMMAND_SUCCESS;
}

CommandCode Gateway::Commands::rtcReset()
{
    Serial.println("Setting time to 2000-01-01 00:00:00...");
    parent->rtc.writeTime(946684800); // 2000-01-01
    parent->rtc.setSysTime();
    return COMMAND_SUCCESS;
}

CommandCode Gateway::Commands::rtcSet()
{
    time_t ctime{time(nullptr)};
    tm* curTm{gmtime(&ctime)};
    char timeStr[20]{0};
    strftime(timeStr, sizeof(timeStr), "%F %T", curTm);
    Serial.printf("Enter new time (current: '%s') :\n", timeStr);
    auto timeBuffer{CommandParser::readLine()};
    if (!timeBuffer)
        return COMMAND_TIMEOUT;
    tm tm{};
    if (strptime(timeBuffer->data(), "%F %T", &tm) == nullptr)
    {
        Serial.printf("Could not parse '%s' as a date/time value. Ensure the format '2000-03-23 "
                      "14:32:01' is respected.\n",
                      timeBuffer->data());
        return COMMAND_ERROR;
    }
    else
    {
        strftime(timeStr, sizeof(timeStr), "%F %T", &tm);
        Serial.printf("New time successfully set to '%s'\n", timeStr);
    }
    ctime = mktime(&tm);
    parent->rtc.writeTime(static_cast<uint32_t>(ctime));
    parent->rtc.setSysTime();
    return COMMAND_SUCCESS;
}

CommandCode Gateway::Commands::changeServer()
{
    char serverBuffer[sizeof(mqttServer)];
    strncpy(serverBuffer, mqttServer, sizeof(mqttServer));
    Serial.printf("Enter server URL or IP address (current: '%s') :\n", serverBuffer);
    if (!CommandParser::editValue(serverBuffer))
        return COMMAND_ERROR;

    uint16_t portBuffer{mqttPort};
    Serial.printf("Enter the server's MQTT port (current: '%u') :\n", portBuffer);
    if (!CommandParser::editValue(portBuffer))
        return COMMAND_ERROR;

    Serial.printf("Start the gateway setup on the web portal and enter the access code:\n");
    char pskBuffer[sizeof(mqttPsk)]{0};
    auto code{CommandParser::readLine()};
    if (!code)
        return COMMAND_TIMEOUT;
    {
        parent->wifiConnect();
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("Could not connect to WiFi. Aborting linking procedure with server...");
            return COMMAND_ERROR;
        }
        WiFiClientSecure client;
        client.setInsecure();
        HTTPClient https;
        https.addHeader("mirra-gateway", parent->lora.getMACAddress().toString());
        https.addHeader("mirra-access-code", code->data());
        char url[128];
        snprintf(url, sizeof(url), "%s%s%s", "https://", serverBuffer, "/gateway/code");
        if (!https.begin(client, url))
            Serial.printf("Error while connecting to '%s'. Ensure the address is correct.\n", url);
        int status = https.GET();
        if (status == HTTP_CODE_OK)
        {
            strncpy(pskBuffer, https.getString().c_str(), sizeof(mqttPsk));
            https.end();
        }
        else
        {
            https.end();
            if (status == HTTP_CODE_UNAUTHORIZED)
            {
                Serial.printf("Access code '%s' was incorrect.\n", code->data());
            }
            else
            {
                Serial.printf("Error while submitting access code. Error code: '%i'.\n", status);
            }
            Serial.println("Fallback: enter PSK manually (64 hexadecimal characters):");
            if (!CommandParser::editValue(pskBuffer))
            {
                WiFi.disconnect();
                return COMMAND_ERROR;
            }
        }
    }
    {
        MQTTClient client{serverBuffer, portBuffer, parent->lora.getMACAddress(), pskBuffer};
        if (client.clientConnect())
        {
            strncpy(mqttServer, serverBuffer, sizeof(mqttServer));
            mqttPort = portBuffer;
            strncpy(mqttPsk, pskBuffer, sizeof(mqttPsk));
            Serial.println(
                "Connection to provided server successful. Configuration has been changed.");
            Serial.printf("mqttServer: %s\n", mqttServer);
            Serial.printf("mqttPort: %u\n", mqttPort);
            Serial.printf("mqttPsk: %s\n", mqttPsk);
            WiFi.disconnect();
            return COMMAND_SUCCESS;
        }
    }
    WiFi.disconnect();
    Serial.println(
        "Could not connect to the provided MQTT server. Configuration has not been changed.");
    return COMMAND_ERROR;
}

CommandCode Gateway::Commands::changeIntervals()
{
    uint32_t commIntervalBuffer{commInterval};
    Serial.printf("Enter communication interval in seconds (current: '%u') : ", commIntervalBuffer);
    if (!CommandParser::editValue(commIntervalBuffer))
        return COMMAND_ERROR;
    uint32_t sampleIntervalBuffer{sampleInterval};
    Serial.printf("Enter sample interval in seconds (current: '%u') : ", sampleIntervalBuffer);
    if (!CommandParser::editValue(sampleIntervalBuffer))
        return COMMAND_ERROR;
    uint32_t sampleRoundingBuffer{sampleRounding};
    Serial.printf("Enter sample rounding in seconds (current: '%u') : ", sampleRoundingBuffer);
    if (!CommandParser::editValue(sampleRoundingBuffer))
        return COMMAND_ERROR;
    uint32_t sampleOffsetBuffer{sampleOffset};
    Serial.printf("Enter sample offset in seconds (current: '%u') : ", sampleOffsetBuffer);
    if (!CommandParser::editValue(sampleOffsetBuffer))
        return COMMAND_ERROR;

    commInterval = commIntervalBuffer;
    sampleInterval = sampleIntervalBuffer;
    sampleRounding = sampleRoundingBuffer;
    sampleOffset = sampleOffsetBuffer;

    return COMMAND_SUCCESS;
}

CommandCode Gateway::Commands::discovery()
{
    parent->discovery();
    return COMMAND_SUCCESS;
}

CommandCode Gateway::Commands::removeNode(const char* macString)
{
    auto mac{parent->macToNode(MACAddress::fromString(macString))};
    if (mac)
    {
        parent->removeNode(*mac);
    }
    else
    {
        Serial.printf("No valid node found for MAC address '%s'.\n", macString);
    }
    return COMMAND_SUCCESS;
}

CommandCode Gateway::Commands::setup()
{
    changeWifi();
    rtcUpdateTime();
    changeServer();
    changeIntervals();
    discovery();
    return COMMAND_SUCCESS;
}

CommandCode Gateway::Commands::printSchedule()
{
    constexpr size_t timeLength{sizeof("0000-00-00 00:00:00")};
    char buffer[timeLength]{0};
    Serial.println("MAC\tNEXT COMM TIME\tSAMPLE INTERVAL\tMAX MESSAGES");
    for (const Node& n : parent->nodes)
    {
        tm time;
        time_t nextNodeCommTime{static_cast<time_t>(n.getNextCommTime())};
        gmtime_r(&nextNodeCommTime, &time);
        strftime(buffer, timeLength, "%F %T", &time);
        Serial.printf("%s\t%s\t%u\t%u\n", n.getMACAddress().toString(), buffer,
                      n.getSampleInterval(), n.getMaxMessages());
    }
    return COMMAND_SUCCESS;
}

CommandCode Gateway::Commands::testMQTT(uint32_t timestamp, uint32_t value)
{
    using DataEntry = SensorFile::DataEntry;
    SensorValue sensorValue{0, 0, static_cast<float>(value)};
    DataEntry::SensorValueArray sensorValues = {sensorValue};
    DataEntry entry{MACAddress{}, timestamp, DataEntry::Flags{1, 0}, sensorValues};

    Serial.println("Commencing upload to MQTT server...");
    parent->wifiConnect();
    if (WiFi.status() == WL_CONNECTED)
    {
        MQTTClient mqtt{mqttServer, mqttPort, parent->lora.getMACAddress(), mqttPsk};
        char topic[topicSize];
        parent->createTopic(topic, entry.source);

        if (mqtt.clientConnect())
        {
            if (mqtt.mqtt.publish(topic, reinterpret_cast<uint8_t*>(&entry), entry.getSize()))
            {
                Serial.println("MQTT message successfully published.");
                mqtt.mqtt.disconnect();
                WiFi.disconnect();
                return COMMAND_SUCCESS;
            }
            else
            {
                Serial.printf("Error while publishing to MQTT server. State: %i\n",
                              mqtt.mqtt.state());
                mqtt.mqtt.disconnect();
                WiFi.disconnect();
                return COMMAND_ERROR;
            }
        }
        else
        {
            Serial.printf("Error while connecting to MQTT server. Aborting upload. State: %i\n",
                          mqtt.mqtt.state());
            WiFi.disconnect();
            return COMMAND_ERROR;
        }
    }
    else
    {
        Serial.println("WiFi not connected. Aborting upload to MQTT server...");
        return COMMAND_ERROR;
    }
}