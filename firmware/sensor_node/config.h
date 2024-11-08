#ifndef __CONFIG_H__
#define __CONFIG_H__

// Pin definitions
#include "pins_sensornode_rev5.h"

// Filepaths
#define DATA_FP "/data.dat"
#define DATA_TEMP_FP "/data_temp.dat"

// Communication and sensor settings
#define WAKE_BEFORE_COMM_PERIOD                                                                    \
    3 // s, time before comm period when node should wake from deep sleep
#define WAKE_COMM_PERIOD(X) ((X) - WAKE_BEFORE_COMM_PERIOD)

#define DEFAULT_SAMPLING_INTERVAL                                                                  \
    (60 * 60) // s, default sensor sampling interval to resort to when no communication with gateway
              // is established
#define DEFAULT_SAMPLING_ROUNDING                                                                  \
    (60) // s, round sampling time to nearest... (only used in case of DEFAULT_SAMPLING_INTERVAL)
#define DEFAULT_SAMPLING_OFFSET (0)

#define DISCOVERY_TIMEOUT (5 * 60 * 1000) // ms, time to wait for a discovery message from gateway

#define TIME_CONFIG_TIMEOUT 6000 // ms
#define TIME_CONFIG_ATTEMPTS 1

#define SENSOR_DATA_TIMEOUT 6000 // ms
#define SENSOR_DATA_ATTEMPTS 1

#define MAX_SENSORS 20

// Sensor pins

#define SOILTEMP_PIN 17
#define SOILTEMP_BUS_INDEX 0

#define CAM_PIN GPIO_NUM_2

#endif
