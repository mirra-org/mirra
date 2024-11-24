#ifndef __CONFIG_H__
#define __CONFIG_H__
#include <array>
#include <cstdint>
// Pin definitions
#include "pins_sensornode_rev5.h"

// WiFi settings
constexpr std::array<char, 33> defaultWifiSsid{"GontrodeWiFi2"};
constexpr std::array<char, 33> defaultWifiPass{"b5uJeswA"};

#define NTP_URL "be.pool.ntp.org"

// MQTT settings
constexpr std::array<char, 65> defaultMqttServer{"mirra.ugent.be"};
constexpr uint16_t defaultMqttPort = 8883;
constexpr std::array<char, 65> defaultMqttPsk{""};

#define TOPIC_PREFIX                                                                               \
    "mirra" // MQTT topic = `TOPIC_PREFIX` + '/' + `GATEWAY MAC` + '/' + `SENSOR MODULE MAC`
#define MQTT_ATTEMPTS 5   // amount of attempts made to connect to MQTT server
#define MQTT_TIMEOUT 1000 // ms, timeout to connect with MQTT server
#define MAX_MQTT_ERRORS                                                                            \
    3 // max number of MQTT publish errors after which uploading should be aborted

// Communication and sensor settings

// s, time between each node's communication period
#define COMM_PERIOD_PADDING 3
// s, time between communication times for every nodes
constexpr uint32_t defaultCommInterval = 60 * 60;
// s, time between sensor sampling for every node
constexpr uint32_t defaultSampleInterval = 20 * 60;
// s, round sampling time to nearest ...
constexpr uint32_t defaultSampleRounding = 20 * 60;
constexpr uint32_t defaultSampleOffset = 0;

#define WAKE_BEFORE_COMM_PERIOD                                                                    \
    5 // s, time before comm period when gateway should wake from deep sleep
#define WAKE_COMM_PERIOD(X) ((X) - WAKE_BEFORE_COMM_PERIOD)
#define LISTEN_COMM_PERIOD(X) ((X) - COMM_PERIOD_PADDING)

#define DISCOVERY_TIMEOUT 5 * 60 * 1000 // ms

#define TIME_CONFIG_TIMEOUT 6000 // ms
#define TIME_CONFIG_ATTEMPTS 1

#define SENSOR_DATA_TIMEOUT 6000 // ms
#define SENSOR_DATA_ATTEMPTS 1

#define MAX_SENSOR_NODES 35

#endif
