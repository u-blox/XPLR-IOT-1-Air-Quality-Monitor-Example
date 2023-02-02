# Air Quality Monitor Gateway

## Description

The application in this folder, implements the Gateway of the Air Quality Monitor example. It shows how XPLR-IOT-1 could be used to read measurements from an external Bluetooth sensor who broadcasts its measurements.

- At startup, this application powers up and configures NINA-W156 Wi-Fi module of XPLR-IOT-1. 
- It connects to a Wi-Fi network and then to Thingstream MQTT Broker.
- It then scans for nearby Bluetooth devices and when it finds the name of the Sensor Bluetooth Broadcaster implemented in [sensor_boradcaster folder](../sensor_broadcaster/) it starts reading the measurements broadcasted.

Every time a new measurement is received, a JSON message containing the measurements of the sensor (Temperature, Humidity, CO2) is published at the corresponding topic in Thingstream MQTT Broker. 

<div align="center"><img src="/readme_images/Gateway.jpg" width="200" height="400" /></div>

#### ubxlib
To handle the NINA-W156 Wi-Fi module (connection to network, connection to MQTT, publish messages to topics), [ubxlib](https://github.com/u-blox/ubxlib) library is used.
ubxlib is a u-blox library that makes it easy to use u-blox modules. ubxlib is used as a submodule in this repository, so make sure when this repo is cloned that ubxlib is also downloaded properly, e.g. by using commands such as:
`clone <repository_link> -recurse-submodules`
or after cloning something like:
`git submodule update --init`


## Configuration

#### Configure Wi-Fi

A Wi-Fi network should be provided to the application. This is done using the following definitions in [main.c](./src/main.c) file.
```
#define WIFI_SSID       "your_ssid"
#define WIFI_PASSWORD   "your_password"
```

#### Configure Thingstream

When the user wants to connect to Thingstream via Wi-Fi, an [MQTT Now](https://developer.thingstream.io/guides/iot-communication-as-a-service/mqtt-now) plan should be used. The following steps describe what should be done to configure the MQTT client in XPLR-IOT-1 to connect to Thingstream via Wi-FI.

1. In Thingstream platform: Create an IP thing in Thingstream
2. In Thingstream platform: Select your Thing in the “Things” menu
3. In Thingstream platform: From the credentials Tab, copy the Client ID, Username, and password (use the copy button)
4. Paste those credentials in the respective definitions in [main.c](./src/main.c) file.

```
#define MQTT_DEVICE_ID      "device:xxxx-xxxxx-xxx-xxx"
#define MQTT_USERNAME       "Paste and copy IP thing username here"
#define MQTT_PASSWORD       "Paste and copy IP thing password here"
```

## Building

The example is built using **nRF Connect SDK version 1.9.1** and it is advised to use this version to build the Gateway (**version 1.7.0** will also work for Gateway). 

You can build the application as any other Zephyr project (you can get some help [here](https://github.com/u-blox/XPLR-IOT-1-software#building--the-firmware)). 

This application is set up to compile and create an image that needs a J-Link to be programmed in XPLR-IOT-1 (this will also erase the bootloader - see [here](https://github.com/u-blox/XPLR-IOT-1-software/tree/main/compile_options/bootloader_inclusion) how you can program the bootloader again).

The option to compile it as an updateable image for use with a bootloader is not implemented.

**Possible Issue:**
When you compile the project for the first time, an error: *"Permission Denied"* may appear and the compilation may halt. In this case you can just resume building the project (not pristine build, just repeat the build action). The second time the project should compile without issues.

<div align="center"><img src="/readme_images/PossibleGatewayError.jpg" /></div>


## Flashing

You can flash the project as any other Zephyr project (using VS Code is recommended). A J-Link is required to Flash the application.
You can get some help on Flashing [here](https://github.com/u-blox/XPLR-IOT-1-software#programming-the-firmware-using-a-j-link-debugger).

After building: 
- You can use VS Code to Flash the device (recommended)
- You can use `build/zephyr/merged_domains.hex` file to program the app with nRF Connect -> Programmer application
- You can use nrftools to program the device with command line (need to identify the proper files for this)

This application uses both the APPLICATION and NETWORK cores of nRF5340 in NORA-B1.


## Implementation Details

Some implementation details are given in this section, to clarify some aspects of the code.

At startup NINA-W156 is powered up. The `nina_config` files contain functions to handle the pins and power up of NINA-W156.
The Wi-Fi and Thingstream connections are handled using ubxlib library functions.
The Bluetooth funtionality (scanning/parsing advertisement data) is handled by nRF Connect SDK functions.

The broadcaster, broadcasts its measurements several times for a certain period of time (default: for 1 second)
That means it may broadcast the same measurement multiple times. That is why in the measurement data, a message (or measurement) ID is added.
This ID is an ascending number.

The Gateway starts by scanning all Bluetooth devices in the area and checking if their names match the the expected name of the broadcaster. When the name of the broadcaster is found, its address is saved, and only advertisements from this address are parsed after that.

The type of the advertisement message its checked. In the case of the Sensor Bluetooth Broadcaster imeplemted the types can be:
- 0x09: This type contains the name of the advertising device
- 0x255: Contains the measurement data

After checking the name of the broadcater and getting its address, only advertisements of this address are checked, and 0x09 types are discarded (they no longer contain information useful to the applicatio, since we got the device address)

0x255 types are checked, the measurements contained in those are read along with the message id - the measurement is published to MQTT broker, and all subsequent messages with the same message id, are ignored. The broadcaster, broadcasts the same measurement many times, but we only need to read each measurement once.
When the next measurement with different message id is read, it is again read and published and subsequent messages with the same id are ignored and so on.

The measurements are published in a JSON format to the MQTT broker.


## Disclaimer
Copyright &copy; u-blox 

u-blox reserves all rights in this deliverable (documentation, software, etc.,
hereafter “Deliverable”). 

u-blox grants you the right to use, copy, modify and distribute the
Deliverable provided hereunder for any purpose without fee.

THIS DELIVERABLE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
WARRANTY. IN PARTICULAR, NEITHER THE AUTHOR NOR U-BLOX MAKES ANY
REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY OF THIS
DELIVERABLE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.

In case you provide us a feedback or make a contribution in the form of a
further development of the Deliverable (“Contribution”), u-blox will have the
same rights as granted to you, namely to use, copy, modify and distribute the
Contribution provided to us for any purpose without fee.