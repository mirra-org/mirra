#ifndef __CONFIG_H__
#define __CONFIG_H__

// Pin definitions
#include "pins_sensornode_rev5.h"

// Filepaths
#define NODES_FP "/nodes.dat"
#define DATA_FP "/data.dat"
#define DATA_TEMP_FP "/data_temp.dat"

// WiFi settings
#define WIFI_SSID "GontrodeWiFi2"
#define WIFI_PASS "b5uJeswA"
#define NTP_URL "be.pool.ntp.org"

// MQTT settings
#define MQTT_SERVER "mirra.ugent.be"
#define MQTT_PORT 1883
#define MQTT_IDENTITY 0
#define MQTT_PSK 0
#define TOPIC_PREFIX "fornalab" // MQTT topic = `TOPIC_PREFIX` + '/' + `GATEWAY MAC` + '/' + `SENSOR MODULE MAC`
#define MQTT_ATTEMPTS 5         // amount of attempts made to connect to MQTT server
#define MQTT_TIMEOUT 1000       // ms, timeout to connect with MQTT server
#define MAX_MQTT_ERRORS 3       // max number of MQTT publish errors after which uploading should be aborted

// Communication and sensor settings

// s, time between each node's communication period
#define COMM_PERIOD_PADDING 3
// s, time between communication times for every nodes
#define DEFAULT_COMM_INTERVAL (60 * 60)

#define WAKE_BEFORE_COMM_PERIOD 5 // s, time before comm period when gateway should wake from deep sleep
#define WAKE_COMM_PERIOD(X) ((X)-WAKE_BEFORE_COMM_PERIOD)
#define LISTEN_COMM_PERIOD(X) ((X)-COMM_PERIOD_PADDING)

#define UPLOAD_EVERY 3 // amount of times the gateway will communicate with the nodes before uploading data to the server

#define DEFAULT_SAMPLE_INTERVAL (20 * 60) // s, time between sensor sampling for every node
#define DEFAULT_SAMPLE_ROUNDING (20 * 60) // s, round sampling time to nearest ...
#define DEFAULT_SAMPLE_OFFSET (0)

#define DISCOVERY_TIMEOUT 5 * 60 * 1000 // ms

#define TIME_CONFIG_TIMEOUT 6000 // ms
#define TIME_CONFIG_ATTEMPTS 1

#define SENSOR_DATA_TIMEOUT 6000 // ms
#define SENSOR_DATA_ATTEMPTS 1

#define MAX_SENSORDATA_FILESIZE 512 * 1024 // bytes

#define MAX_SENSOR_NODES 20

#endif
