/*
 * Copyright 2022 u-blox Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/** @file
 * @brief This application works together with scd4x_broadcaster application.
 * scd4x_broadcaster application broadcasts the measurements of an SCD41 sensor
 * via Bluetooth advertisements.
 *
 * Required: One XPLR-IOT-1 device. Account to thingstream with an MQTT now thing enabled.
 *
 * This application:
 *  - Connects to WiFi via NINA-W156
 *  - Connects to thingstream via MQTT
 *  - Scans for Bluetooth LE devices
 *  - Finds among scanned devices the one named as the scd4x_broadcaster
 *  - Gets the address of that device and then parses the data from this device only
 *  - Each measurement may be bradcasted several times from scd4x_broadcaster. This
 *    application recognizes the meaurement id of each measurement and if already
 *    read it ignores it, until the next measurement comes (different measurement id)
 * -  Each time a new measurement is received, a JSON message is prepared and sent
 *    to Thingstream (you can then see those measurements in the Dashboard that comes
 *    with this example)
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#include "ubxlib.h"

#include "nina_config.h"


/* ----------------------------------------------------------------
 * APPLICATION DEFINITIONS
 * -------------------------------------------------------------- */

// Cradentials of the Wi-FI network the user wants to connect to
#define WIFI_SSID       "your_ssid"
#define WIFI_PASSWORD   "your_password"

// Topic name where the received measurements are going to be published
// Should be defined to Thingstream as well
#define MQTT_TOPIC "airquality"

// MQTT broked credentials
#define MQTT_BROKER_NAME    "mqtt.thingstream.io"
#define MQTT_PORT           1883
#define MQTT_DEVICE_ID      "device:xxxx-xxxxx-xxx-xxx"
#define MQTT_USERNAME       "Paste and copy IP thing username here"
#define MQTT_PASSWORD       "Paste and copy IP thing password here"

// The name of the broadcaster (under which name the broadcaster advertises)
#define BROADCASTER_NAME    "ZephyrAQM"


/* ----------------------------------------------------------------
 * GLOBALS
 * -------------------------------------------------------------- */

/** Structure holding the measurements of the SCD41 sensor along with
 * an ascending number which is used as a message id, to separate the
 * measurement messages
 * Note: This struct declaration should be the same as the one used by the
 * broadcaster
 */
struct {
    float temperature;     /**< Temperature measurement */
    float humitity;        /**< Humidity measurement */
    float co2;             /**< CO2 measurement */
    uint32_t message_id;   /**< Ascending number to identify measurement */
} gMeasurement = {0};


//uint8_t adv_data[ sizeof(float)*3 ]= { 0 };

/** Message to be pubished via MQTT*/
char gMessageToPublish[200] = "";

//uint8_t adv_dataheader[16] = {};

/** Has the address of the Sensor broadcaster been obtained? */
static bool gAddressObtained = false;

/** The address of the broadcaster */
bt_addr_t gAddress;

/** The last measurement(message) ID obtained*/
uint32_t gLastMeasId = 0;

/** When a new measurement has been received */
static bool gMeasReceived = false;


/* ----------------------------------------------------------------
 * MACROS
 * -------------------------------------------------------------- */

/** Macro to verify that a condition is met. If the condition is met,
 *  nothing happens. If the condition in not met the failed function is
 *  called.
 *  (In this implementetion of the failed function the fail_msg is typed and
 *  the application halts via an infinite loop) */
#define VERIFY(cond, fail_msg) \
    if (!(cond)) {\
        failed(fail_msg); \
    }

/* ----------------------------------------------------------------
 * STATIC FUNCTION DECLARATION
 * -------------------------------------------------------------- */

/** The scan callback to be executed when a new device is found be the BLE scanner
 *
 * @param addr       See bt_le_scan_cb_t description.
 * @param rssi       See bt_le_scan_cb_t description.
 * @param adv_type   See bt_le_scan_cb_t description.
 * @param buf        See bt_le_scan_cb_t description.
 */
static void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
                    struct net_buf_simple *buf);


/** Callback to be executed when the device disconnects from MQTT broker.
 *
 * @param adv_type  See uMqttClientSetDisconnectCallback description.
 * @param buf       See uMqttClientSetDisconnectCallback description.
*/
static void mqttDisconnectCb(int32_t errorCode, void *pParam);


/** Function to be called when something fails. Halts execution.
 *
 * @param msg       Message to be typed before halt.
 */
static void failed(const char *msg);


/** To be used as a parameter of bt_data_parse() within the scan callback.
 *  Checks if the scanned device name is the one requested by
 *  BROADCASTER_NAME definition
 *
 *  @param data       see bt_data_parse() description.
 *  @param user_data  see bt_data_parse() description. Returns true if
 *                    requested device name is found.
 *  @return           see bt_data_parse() description.
 */
static bool adv_check_name(struct bt_data *data, void *name_found);


/** To be used as a parameter of bt_data_parse() within the scan callback.
 *  Checks the advertising packet type and the data within it.
 *  If a NEW measurement has been received, it popoulates the gMeasurement struct
 *  and sets the gMeasReceived boolean to true.
 *
 *  @param data       see bt_data_parse() description.
 *  @param user_data  see bt_data_parse() description.
 *  @return           see bt_data_parse() description.
 */
static bool adv_data_found(struct bt_data *data, void *user_data);


/** Function description goes here.
 *
 * @param param1  param1 desc.
 * @param param2  param2 desc.
 * @return        zero on success else negative error code.
 */


/* ----------------------------------------------------------------
 * STATIC FUNCTION IMPLEMENTATION
 * -------------------------------------------------------------- */

static void failed(const char *msg)
{
    printk( "%s", msg );
    while ( 1 );
}


static bool adv_check_name(struct bt_data *data, void *name_found)
{
    // set name_found parameter to false
    memset(name_found, 0, 1);

    // check advertisement's AD type byte. 0x09 is for: Complete Local Name
    // we are only interested in that
    if ( data->type != 9 ) {
        return false;
    }

    // check if the name has the expected length
    if ( data->data_len != strlen( BROADCASTER_NAME ) ) {
        return false;
    }

    // check the name itself
    if ( memcmp( data->data, BROADCASTER_NAME, strlen( BROADCASTER_NAME ) ) == 0 ) {
        // name found, set name_found to true
        memset(name_found, 1, 1);
        return true;
    }

    return false;
}


static bool adv_data_found(struct bt_data *data, void *user_data)
{
    // check advertisement's AD type byte. 0x255 contains measurement data
    // we are only interested in that
    if ( data->type != 255 ) {
        return false;
    }

    // copy the advertisement data to the measurement structure. For this to
    // work, gMeasurement should be defined exactly the same in the sensor
    // broadcaster and the receiver/Gateway. The same MCU is also used in both sides,
    // so its safe to use for this example.
    memcpy( &gMeasurement, data->data, sizeof(gMeasurement) );

    // We only need to check if the id is different than the previous measurement
    // received. If it's higher or lower is not really of interest, because we only
    // want to exclude measurements multiply broadcasted. So, if the last measurement
    // we got had the same id, then this is a repetition of the previous message and
    // we abort it.
    if ( gLastMeasId != gMeasurement.message_id ) {

        printf("New measurement received\r\n");

        // print measurement
        printf( "Temp: %f  : Hum:%f   Co2: %f Id: %d \r\n",
                gMeasurement.temperature,
                gMeasurement.humitity,
                gMeasurement.co2,
                gMeasurement.message_id);

        // Notify for a new measurement and keep its ID
        gLastMeasId = gMeasurement.message_id;
        gMeasReceived = true;

        return true;
    } else {
        // measurement already received. This is a repetition,
        // abort measurement
        return false;
    }
}


static void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
                    struct net_buf_simple *buf)
{
    // search for Broadcaster device name and obtain its address
    if ( !gAddressObtained ) {

        //parse advertisement packet and search for device name
        bool name_found = false;
        bt_data_parse(buf, adv_check_name, &name_found);

        if ( name_found ) {
            printf( "Found Broadcaster Name. " );
            //save address
            gAddress = addr->a;
            gAddressObtained = true;

            printk("Address: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                   gAddress.val[5],
                   gAddress.val[4],
                   gAddress.val[3],
                   gAddress.val[2],
                   gAddress.val[1],
                   gAddress.val[0]);
        }
    }

    // if address has been obtained
    if ( gAddressObtained ) {
        // if the scanned device has the expected address
        if ( memcmp(addr->a.val, gAddress.val, sizeof(gAddress.val) ) == 0 ) {
            // parse data to get measurement
            bt_data_parse(buf, adv_data_found, NULL);
        }
    }
}


static void mqttDisconnectCb(int32_t errorCode, void *pParam)
{
    printk("MQTT Disconnected! \r\n");
}


/* ----------------------------------------------------------------
 * MAIN APPLICATION IMPLEMENTATION
 * -------------------------------------------------------------- */


void main(void)
{

    uMqttClientContext_t *mqttClientCtx;
    static uDeviceHandle_t gDevHandle = NULL;

    // Wi-Fi module config for use by ubxlib
    static const uDeviceCfg_t deviceCfg = {
        .deviceType = U_DEVICE_TYPE_SHORT_RANGE,
        .deviceCfg = {
            .cfgSho = {
                .moduleType = U_SHORT_RANGE_MODULE_TYPE_NINA_W15
            }
        },
        .transportType = U_DEVICE_TRANSPORT_TYPE_UART,
        .transportCfg = {
            .cfgUart = {
                .uart = 2,     //as defined in board overlay file
                .baudRate = 115200,
                .pinTxd = -1,  //defined in board overlay file
                .pinRxd = -1,  //defined in board overlay file
                .pinCts = -1,  //defined in board overlay file
                .pinRts = -1   //defined in board overlay file
            }
        }
    };

    // Connection to Wifi network configuration- if Network requires password
    static const uNetworkCfgWifi_t  wifiConfig = {
        .type = U_NETWORK_TYPE_WIFI,
        .pSsid = WIFI_SSID,
        .authentication = 2,
        .pPassPhrase = WIFI_PASSWORD
    };

    // MQTT Configuration parameters
    const uMqttClientConnection_t mqttConnection = {
        .pBrokerNameStr = MQTT_BROKER_NAME,
        .localPort = MQTT_PORT,
        .pClientIdStr = MQTT_DEVICE_ID,
        .pUserNameStr = MQTT_USERNAME,
        .pPasswordStr = MQTT_PASSWORD
    };

    // BLE scanning parameters
    struct bt_le_scan_param scan_param = {
        .type       = BT_HCI_LE_SCAN_ACTIVE,
        .options    = BT_LE_SCAN_OPT_FILTER_DUPLICATE,
        .interval   = 0x0010,
        .window     = 0x0010,
    };

    printk("Air Quality Monitor Gateway Version: 1.0 \r\n\r\n");

    // Initilize NINA-W156 Wi-Fi module hardware (set appropriate pins)
    nina15InitPower();
    printk("NINA-W15 Powered on \r\n");
    ninaNoraCommEnable();


    // Set up Connection to Wi-Fi using ubxlib library (NINA-W156)
    VERIFY(uPortInit() == 0, "uPortInit failed\n");
    VERIFY(uDeviceInit() == 0, "uDeviceInit failed\n");
    VERIFY(uDeviceOpen( &deviceCfg, &gDevHandle ) == 0, "uDeviceOpen failed\n");

    uAtClientDebugSet(gDevHandle, false);

    printk("Bring up Wi-Fi\n");
    VERIFY(uNetworkInterfaceUp( gDevHandle, U_NETWORK_TYPE_WIFI, &wifiConfig ) == 0,
           "Could not connect to network" );
    printk("Wi-Fi connected\n");

    // Set up Connection to MQTT (Thingstream) using ubxlib library (NINA-W156)
    printk("Setup up MQTT\n");
    mqttClientCtx = pUMqttClientOpen( gDevHandle, NULL);
    VERIFY( mqttClientCtx != NULL, "Could not open MQTT Client" );

    printk("uMqttClientConnect...");
    VERIFY( uMqttClientConnect( mqttClientCtx, &mqttConnection ) == 0, "uMqttClientConnect failed\n" );
    printk("ok\n");

    VERIFY( uMqttClientSetDisconnectCallback( mqttClientCtx, mqttDisconnectCb,
                                              (void *)mqttClientCtx) == 0, "Failed to set MQTT disconnection callback \r\n");

    // Setup/Initialize BLE in NORA-B1
    printk("Starting BLE\n");
    VERIFY( bt_enable(NULL) == 0, "Bluetooth init failed\n" );
    printk("Bluetooth initialized\n");

    // Start Scanning for BLE devices and setup callback for incoming advertising packets
    VERIFY( bt_le_scan_start(&scan_param, scan_cb) == 0, "Scanning failed to start\n");
    printk("\nWaiting for sensor advertisements\n");

    do {
        // if new measurement has been received, publish it to MQTT
        if ( gMeasReceived ) {
            // clear any previous message bytes
            memset(gMessageToPublish, 0, sizeof(gMessageToPublish));

            // Prepare a JSON message containing the measurements
            sprintf(gMessageToPublish, "{\"c02level\":%f, \"humidity\":%f, \"temperature\":%f}",
                    gMeasurement.co2, gMeasurement.humitity, gMeasurement.temperature );
            printf("Message to publish: %s\r\n", gMessageToPublish);

            // Publish the JSON message
            if ( uMqttClientPublish(mqttClientCtx, MQTT_TOPIC, gMessageToPublish, strlen(gMessageToPublish), 0,
                                    0) == 0 ) {
                printk("Published\r\n\r\n");
            } else {
                printk("Publish failed\r\n");
            }

            // Wait for next measurement
            gMeasReceived = false;
        }
    } while (uMqttClientIsConnected(mqttClientCtx));

    // When disconnected from broker the application stops
    printk("Application stoped\r\n");
    bt_le_scan_stop();

    return;
}
