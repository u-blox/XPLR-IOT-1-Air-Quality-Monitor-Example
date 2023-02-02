# Sensor Bluetooth LE Broadcaster

## Description

This application implements a basic wireless sensor. The application reads the sensor and uses NORA-B1 Bluetooth to broadcast the sensor measurements. It does that by apending the measurement data to Bluetooth LE Manufacturer-specific Advertising Data. 

So the device broadcasts the measurements, and any Bluetooth device that performs a scan operation within range, can get these data without the need for a specific connection.

<div align="center"><img src="/readme_images/BLE_broadcaster.jpg" width="200" height="400" /></div>

This is the starting point, to read and broadcast the measurements, so that a Gateway can receive those measurements and send them to the MQTT Broker.

## Hardware Setup option 1

One way to implement this sensor Bluetooth LE broadcaster is to use an XPLR-IOT-1 device and a [MikroE HVAC Click board MIKROE-4290](https://www.mikroe.com/hvac-click) which contains a [Sensirion SCD41 CO2 sensor](https://sensirion.com/products/catalog/SCD41/).

You can connect the sensor to the Qwiic connector of the XPLR-IOT-1 device.

![BLE_Broadcaster_XPLR_IOT_option should be here.](/readme_images/BLE_Broadcaster_XPLR_IOT_option.jpg "BLE_Broadcaster_XPLR_IOT_option")

## Hardware Setup option 2

Another option is to use a [MINI-NORA-B1 Evaluation kit](https://www.u-blox.com/en/product/mini-nora-b1) and connect the [MikroE HVAC Click board MIKROE-4290](https://www.mikroe.com/hvac-click) to mikroBUS slot 1.

If you decide to use Hardware setup option 2, change the uart rx/tx pins as indicated by the comments in [nrf5340dk_nrf5340_cpuapp.overlay](./nrf5340dk_nrf5340_cpuapp.overlay) file.

<div align="center"><img src="/readme_images/BLE_Broadcaster_MINI_option.jpg"/></div>


## Building

The example is build using **nRF Connect SDK version 1.9.1** and is advised to use this version to build the Sensor Bluetooth LE Broadcaster.

You can build the application as any other Zephyr project (you can get some help [here](https://github.com/u-blox/XPLR-IOT-1-software#building--the-firmware)). 

This application is set up to compile and create an image that needs a J-Link to be programmed in MINI-NORA-B1 or XPLR-IOT-1.
The option to compile it as an updateable image for use with a bootloader is not implemented.

**Important Note:** 
- If you intend to use Hardware setup option 1, leave the project as is and just build it.
- If you intend to use Hardware setup option 2, change the uart rx/tx pins as indicated by the comments in [nrf5340dk_nrf5340_cpuapp.overlay](./nrf5340dk_nrf5340_cpuapp.overlay) file.

[binaries folder](./binaries/) contain pre-compiled images, if you do not want to build the project.

**Known Issue**
During building you may see some warning messages. These are harmless for this application. They are produced by the [driver of SCD41](./scd4x_oot_driver) which is used as an out-of-tree module to the Zephyr project (driver source can be found [here](https://github.com/zephyrproject-rtos/zephyr/pull/46276)). 

## Flashing

You can flash the project as any other Zephyr project (using VS Code is recommended). A J-Link is required to Flash the application.
You can get some help on Flashing [here](https://github.com/u-blox/XPLR-IOT-1-software#programming-the-firmware-using-a-j-link-debugger).

This application uses both the APPLICATION and NETWORK cores of nRF5340 in NORA-B1.

If do not want to build the application, you can use the pre-compiled images and programming scripts in [binaries folder](./binaries/)

If you want to flash the project, after building: 
- You can use VS Code to Flash the device (recommended)
- You can use `build/zephyr/merged_domains.hex` file to program the app with nRF Connect -> Programmer application.
- You can use nrftools to program the device with command line. You can use `build/zephyr/zephyr.hex` and `build\hci_rpmsg\zephyr\zephyr.hex` files for this.


## Configuration

The sensor driver gives some options but leaving as is should work properly. You may want to check the temperature offset, and altitude (affects CO2 measurement) in [nrf5340dk_nrf5340_cpuapp.overlay](./nrf5340dk_nrf5340_cpuapp.overlay) file.

The application itself does not contain a lot of configuration options, just the measurement period.
You can adjust the measurement period with the `MEASUREMENT_PERIOD` definition in [main.c file](./src/main.c).

You can also change the name under which the device advertises, but it is not advised (should change the name in the Gateway code too, to be able to recognize it). 
To change the name you can change the `DEVICE_NAME` definition in [main.c file](./src/main.c).

## Expected Console Output

When running the application, at the uart console output you should see something like this

<div align="center"><img src="/readme_images/BroadcasterConsoleOutput.jpg" /></div>

During initialization of broadcaster application, an error may appear in the initialization stage of the sensor (this is a driver issue), but this does not affect the functionality of the example or the sensor measurements.

The console outputs the measurements and each measurement's ID, and the hex data that are transmitted to Bluetooth LE Manufacturer-specific Advertising Data.


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