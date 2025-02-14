# MIRRA Firmware

This folder contains all the firmwares necessary to set up a MIRRA installation. It is set up as a single PlatformIO project where each different firmware is configured as a seperate PlatformIO 'environment'. 
To initiate a firmware upload, hold the boot button and press the reset button (multiple tries may be necessary), then initiate the upload from the PlatformIO console.

## Command Line Interface

Both the gateway and the sensor nodes can be interacted with via a serial monitor using a command line interface, either using PlatformIO's built in monitor command or a terminal emulator with similar functionality like PuTTY.

To enter a given module's command mode, hold the BOOT button. This will wake up the module and, if held properly, make it enter the command phase. On success, the module should print a message signifying that command phase has been entered.

Note: While in command line mode, the module cannot execute any other function, meaning it can miss sampling or communication periods. While the command phase should automatically timeout after one minute, it is still recommended to `exit` command mode to let the module return to sleep as soon as possible.

### Common Commands

The following commands are available to both gateway and nodes:

- `exit` or `close`: Exits command phase. The module will most likely enter sleep right after this.

- `echo ARG`: echoes `ARG` to the serial output. Used for testing purposes.

- `setlog ARG` or `setloglevel ARG` : sets the current logging level to `ARG`: all log messages on and below this level will be printed and saved in the logging file. Pick from `DEBUG`, `INFO` and `ERROR`.

- `printlog` or `printlogs` or `printlogfile`: prints the entire logfile to the serial output. Depending on its size, this may take some time.

- `printdata` or `printdatafile`: Prints all stored data to the serial output in a human-readable format. Depending on the amount of data stored, this may take some time.

- `printdataraw` or `printdatahex`: Prints all stored data to the serial output in a hexadecimal format.

- `format`: Formats the filesystem. This effectively removes all the data stored in flash, and subsequently resets the module.

- `spam COUNT`: Spams the logfile with a preset message repeated `COUNT` times. Used for testing purposes.


### Gateway Commands

The following commands are exclusive to the gateway:

- `wifi`: Enters an interactive mode in which the WiFi SSID and password can be changed. On a successful connect, the newly provided credentials will be stored and used for future connections,

- `rtc`: Updates the module's time via the Network Time Protocol (NTP) and applies it to the RTC module. WiFi required. If this command takes forever, it might be because no internet connection is available.

- `rtcreset`: Forcibly resets the gateway's time to `2000-01-01 00:00:00` and applies it to the RTC module.

- `rtcset`: Enters an interactive mode to set the gateway's time to `TIME`, where `TIME` is of format `'2000-03-23 14:32:01'`.

- `server`: Enters an interactive mode in which the connection (URL, port and PSK) to the web server may be configured.

- `intervals`: Enters an interactive mode in which the gateway's default intervals (comm interval, sample interval, sample rounding, sample offset) may be changed. Newly added nodes will assume these intervals by default.

- `discovery`: Enters a listening loop for discovery messages from nearby nodes. The gateway will answer with relevant configuration for the node, and will then add the node to its interal list of nodes on success. This loop may only be exited by pressing the BOOT button.

- `addnode`: Enters an interactive mode to forcibly add a node to the gateway. Use only when the added node is already bound to the gateway with total certainty.

- `removenode MAC`: Forcibly removes a node with the address `MAC`from the gateway. Use only when the to-be removed node has errored or has verifiably stopped transmitting.

- `setup`: Convenience command that executes `wifi`, `rtc` and `server` sequentially after each other.

- `printschedule` : Prints scheduling information about the connected nodes, including MAC address, next comm time, sample interval and max number of messages per comm period.

### Sensor Node Commands

The following commands are exclusive to the sensor nodes:

- `discovery`: Manually triggers the sending of a discovery message. Since nodes automatically send a discovery message on RESET, this should not be necessary.

- `sample` : Forcefully initiates a sample period. This samples the sensors and stores the resulting data in the local data file, which will be sent to the gateway. This may impact sensor scheduling.

- `printsample` : Samples the sensors and prints each sample. Unlike `sample`, this does not forward any data to the local data file and will not impact sensor scheduling or communications.

- `printschedule` : Prints scheduling information about the sensors, including tag identifier and next sample time.

