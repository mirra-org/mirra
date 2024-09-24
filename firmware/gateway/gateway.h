#ifndef __GATEWAY_H__
#define __GATEWAY_H__

#include "MIRRAModule.h"
#include "PubSubClient.h"
#include "WiFiClientSecure.h"
#include "config.h"
#include <vector>

namespace mirra
{

/// @brief Representation of a Sensor Node's attributes relevant for communication, used for
/// tracking the status of nodes from the gateway.
struct Node
{
    comm::Address address;
    comm::MACAddress mac;
    uint32_t sampleInterval;
    uint32_t sampleRounding;
    uint32_t sampleOffset;
    uint32_t commInterval;
    uint32_t nextCommTime;
    uint32_t timeBudgetMs;
    uint8_t spreadingFactor;

    Node() = default;
    Node(const comm::MACAddress& mac, uint8_t spreadingFactor, comm::Message<comm::CONFIG>& m)
        : mac{mac}, spreadingFactor{spreadingFactor}
    {
        timeConfig(m);
    }
    /// @brief Configures the Node with a time config message, the same way the actual module would
    /// do.
    /// @param m Time Config message used to saturate the representation's attributes.
    void timeConfig(comm::Message<comm::CONFIG>& m);
    /// @brief Configures the Node as if the time config message was missed, the same way the actual
    /// module would do.
    void naiveTimeConfig(uint32_t cTime);

    /// @return A Time Config message that yields the same exact configuration as this node.
    comm::Message<comm::CONFIG> currentTimeConfig(uint32_t cTime);
};

class Gateway : public MIRRAModule
{
public:
    Gateway();
    void wake();

private:
    struct Commands : MIRRAModule::Commands
    {
        Gateway* parent;
        Commands(Gateway* parent) : parent{parent} {};

        /// @brief Prompts new credentials for the WiFi network to connect to.
        CommandCode changeWifi();
        /// @brief Attempts to configure the local RTC to the correct time using NTP.
        CommandCode rtcUpdateTime();
        /// @brief  Resets the local RTC time to 2000-01-01 00:00:00.
        CommandCode rtcReset();
        /// @brief Forcibly sets the time. Input must be expressed like '2000-03-23 14:32:01'.
        CommandCode rtcSet();
        /// @brief Change the associated server to which sensor data will be uploaded.
        CommandCode changeServer();
        /// @brief Change settings like communication and sampling interval.
        CommandCode changeIntervals();
        /// @brief Forces a discovery period.
        CommandCode discovery();
        /// @brief Convenience command that configures WiFi, RTC and server settings.
        CommandCode setup();
        /// @brief Prints scheduling information about the connected nodes, including MAC address,
        /// next comm time, sample interval and max number of messages per comm period.
        CommandCode printSchedule();
        /// @brief  Uploads a single dummy message with configurable timestamp and sensor value to
        /// the MQTT server.
        /// @param timestamp Timestamp for the dummy message.
        /// @param value Value for the sensor value contained in the dummy message.

        CommandCode testMQTT(uint32_t timestamp, uint32_t value);

        static constexpr auto getCommands()
        {
            return std::tuple_cat(
                MIRRAModule::Commands::getCommands(),
                std::make_tuple(CommandAliasesPair(&Commands::changeWifi, "wifi"),
                                CommandAliasesPair(&Commands::rtcUpdateTime, "rtc"),
                                CommandAliasesPair(&Commands::rtcReset, "rtcreset"),
                                CommandAliasesPair(&Commands::rtcSet, "rtcset"),
                                CommandAliasesPair(&Commands::changeServer, "server"),
                                CommandAliasesPair(&Commands::changeIntervals, "intervals"),
                                CommandAliasesPair(&Commands::discovery, "discovery"),
                                CommandAliasesPair(&Commands::setup, "setup"),
                                CommandAliasesPair(&Commands::printSchedule, "printschedule"),
                                CommandAliasesPair(&Commands::testMQTT, "testmqtt")));
        }
    };

    struct MQTTClient
    {
        WiFiClientSecure wifi;
        PubSubClient mqtt;
        char identity[comm::MACAddress::rawStringLength]{0};

    public:
        MQTTClient(const char* url, uint16_t port, const comm::MACAddress& mac, const char* psk)
        {
            mac.toStringRaw(identity);
            wifi.setPreSharedKey(identity, psk);
            mqtt.setServer(url, port);
            mqtt.setClient(wifi);
            mqtt.setBufferSize(512);
        };

        /// @brief Attempts to connect to the designated MQTT server.
        /// @return Whether the connection was successful or not.
        bool clientConnect();
    };

    std::vector<Node> nodes;
    /// @brief Returns the local node corresponding to the MAC address.
    /// @param mac The MAC address string in the "00:00:00:00:00:00" format.
    /// @return A reference to the matching node. Disenaged if no match is found.
    std::optional<std::reference_wrapper<Node>> macToNode(const MACAddress& mac);

    /// @brief Sends a single discovery message, storing the new node and configuring its timings if
    /// there is a response.
    void discovery();

    /// @brief Imports the nodes stored on the local NVS filesystem. Used to retain the Nodes
    /// objects through deep sleep.
    void loadNodes();
    /// @brief Updates the nodes stored on the local NVS filesystem. Used to retain the Nodes
    /// objects through deep sleep.
    void storeNodes();

    /// @brief Initiates a gateway-wide communication period.
    void commPeriod();
    /// @brief Retrieves, if possible, the next scheduled start time for a node communication
    /// period.
    /// @return The next scheduled comm period if it exists, else -1.
    uint32_t nextScheduledCommTime();
    /// @brief Initiates a comm period with a node, retrieving its sensor data and updating its
    /// timings.
    /// @param n The node to communicate with.
    /// @param data Vector to store the data in.
    /// @return Whether the communication period was successful or not.
    bool nodeCommPeriod(Node& n, std::vector<Message<SENSOR_DATA>>& data);

    static constexpr size_t topicSize =
        sizeof(TOPIC_PREFIX) + MACAddress::stringLength + MACAddress::stringLength;
    /// @brief Creates a topic string with the following layout: "(TOPIC_PREFIX)/(MAC address
    /// gateway)/(MAC address node)"
    /// @param topic The topic string.
    /// @param nodeMAC The associated node's MAC address.
    /// @return The topic string.
    char* createTopic(char* topic, const MACAddress& nodeMAC);
    /// @brief Uploads stored sensor data messages to the MQTT server, and marks uploaded messages
    /// as such in the filesystem by setting the 'upload' flag to 1.
    void uploadPeriod();
    /// (UNUSED)
    /// @brief Parses a node update string with the following layout: "(MAC address node)/(sample
    /// interval)/(sample rounding)/(sample offset)"
    /// @param update The node update string.
    void parseNodeUpdate(char* updateString);

    /// @brief Attempts to connect to the WiFi network with the given credentials. If successful,
    /// these credentials are stored for later connections.
    /// @param SSID The SSID of the WiFi network to connect to.
    /// @param password The password of the WiFi network to connect to.
    void wifiConnect(const char* SSID, const char* password);
    /// @brief Attempts to connect to the WiFi network with the stored credentials.
    void wifiConnect();
};
};

#endif
