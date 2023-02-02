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
 * @brief This file implements an SCD41 measurements broadcaster. 
 * 
 * Hardware Setup: Hardware that can be used for this implementation
 * is either an XPLR-IOT-1 device, with an SCD41 sensor attached to its
 * Qwiic connector, or a MINI-NORA-B1 Evaluation kit with an SCD41 clickboard
 * attached to it (or an SCD41 sensor attached to its Qwiic connector).
 * 
 * The firmware sets up and then reads the sensor and broadcasts its
 * measurements via Bluetooth LE advertisement data.
 */


#include <zephyr.h>
#include <stdio.h>
#include <drivers/sensor.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#include "scd4x.h"


/* ----------------------------------------------------------------
 * APPLICATION CONFIGURATION
 * -------------------------------------------------------------- */

/** How much time an single measurement will be advertised (msec).
 * Should be smaller than sensor MEASUREMENT_PERIOD
*/
#define ADVERTISING_MEAS_PERIOD   1000

/** The period between sensor measurements (msec)*/
#define MEASUREMENT_PERIOD        5000

// Device Name Configuration (How it advertises)
// The actual name that will appear is "ZephyrAQM"
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME "AQM"

// sanity check
#if( ADVERTISING_MEAS_PERIOD >= MEASUREMENT_PERIOD)
	#error "Advertising period should be smaller than measurement period"
#endif


/* ----------------------------------------------------------------
 * ZEPHYR RELATED DEFINITIONS/DECLARATIONS
 * -------------------------------------------------------------- */

#if !DT_HAS_COMPAT_STATUS_OKAY( sensirion_scd4x )
	#error "No sensirion,scd4x compatible node found in the device tree"
#endif

#define DEVICE_NAME_LEN ( sizeof( DEVICE_NAME ) - 1 )

/* ----------------------------------------------------------------
 * GLOBALS
 * -------------------------------------------------------------- */

/** Structure holding the measurements of the SCD41 sensor along with
 * an ascending number which is used as a message id, to separate the
 * measurement messages
 */
struct{
	float temperature;      /**< Temperature measurement */                    
	float humidity;         /**< Humidity measurement */                        
	float co2;              /**< CO2 measurement */                              
	uint32_t message_id;    /**< Ascending number to identify measurement */
}gMeasurement = { 0 };

/** This byte array holds the bytes of the gMeasurement struct and
 * is used to pass the contents of the struct to the advertising data
 * of the device. (The gMeasurement struct is basicaly used to make clear
 * the arrangement of bytes in the gMfgData)
*/
static uint8_t gMfgData[ sizeof( gMeasurement ) ] = { 0 };

/** Set up the advertising data of the device*/
static struct bt_data ad[] = {
	BT_DATA( BT_DATA_MANUFACTURER_DATA, gMfgData, sizeof( gMfgData ) ),
};

/** Set Scan Response data */
static const struct bt_data sd[] = {
	BT_DATA( BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN ),
};


/* ----------------------------------------------------------------
 * FUNCTION
 * -------------------------------------------------------------- */

void main( void )
{
	// Holds return codes of functions
	int err;

	// Holds zephyr sensor measurements
	struct sensor_value temp, hum, co2;
	
	// Get sensor device
	const struct device *scd = DEVICE_DT_GET_ANY( sensirion_scd4x );


	printk("Air Quality Monitor - Sensor Broadcaster Version: 1.0 \r\n\r\n");

	// check if device has been initialized properly by zephyr
	if ( !device_is_ready( scd ) ) {
		printf( "Device %s is not ready", scd->name );
		return;
	}
	 	
	printf( "Starting Broadcaster \r\n" );
		
	/* Initialize the Bluetooth Subsystem */
	err = bt_enable( NULL );
	if (err) {
		printf( "Bluetooth init failed (err %d)\n", err );
		return;
	}
	printf( "Bluetooth initialized\r\n\r\n" );


	// Get sensor measurements at set intervals and broadcast the 
	// data via Bluetooth advertisement data
	while( true ) {

		// Get measurements
		if( sensor_sample_fetch( scd ) ) {
			printf( "Failed to fetch sample from SCD4X device" );
			return;
		}

		sensor_channel_get( scd, SENSOR_CHAN_AMBIENT_TEMP, &temp );
		sensor_channel_get( scd, SENSOR_CHAN_HUMIDITY, &hum );
		sensor_channel_get( scd, SENSOR_CHAN_CO2, &co2 );
		
		// convert measurements
		double temperature = sensor_value_to_double( &temp );
		double humidity = sensor_value_to_double( &hum );
		double carbondioxide = sensor_value_to_double( &co2 );

		#ifdef CONFIG_APP_USE_FAHRENHEIT
			temperature = ((9.0 / 5.0) * temperature) + 32;
		#endif

		// pass measurements to structure
		gMeasurement.co2 = ( float )carbondioxide;
		gMeasurement.temperature = ( float )temperature;
		gMeasurement.humidity = ( float )humidity;
		gMeasurement.message_id = gMeasurement.message_id + 1;

		// print measurements
		printf( "\r\nSCD4x Temperature: %0.6f C, Humidity: %0.6f%%, CO2: %0.6f ppm,  Message ID: %d\r\n",
		       ( float )temperature,
		       ( float )humidity,
			   ( float )carbondioxide,
			   gMeasurement.message_id);

		// Copy measurement struct to gMfgData. This action passes the measurement
		// data to the advertising data of the device (see ad declaration).
		// (The gMeasurement struct is used to make clear the arrangement of bytes
		// in the message and make it easy to decompose this message on the receiver side -
		// since both sides use the same MCU -> NORA-B1 -> nRF5340)
		memcpy( gMfgData, &gMeasurement, sizeof( gMeasurement ) );

		// print hex contents of gMfgData, data that will be advetised
		printf( "Advertising Hex 0x  " );
		for( uint8_t i=0; i < sizeof( gMfgData ); i++ ){
			printf( ":%02x", gMfgData[ i ] );
		}
		printf( "\r\n\r\n" );

		// Start advertising 
		err = bt_le_adv_start( BT_LE_ADV_NCONN_IDENTITY, ad, ARRAY_SIZE( ad ),
				      sd, ARRAY_SIZE( sd ) );
		if( err ) {
			printf("Advertising failed to start (err %d)\n", err);
			return;
		}

		// advertise for ADVERTISING_MEAS_PERIOD
		k_msleep( ADVERTISING_MEAS_PERIOD );

		// stop advertising
		err = bt_le_adv_stop();
		if( err ) {
			printf( "Advertising failed to stop (err %d)\n", err );
			return;
		}

		// wait till next measurement
		k_sleep( K_MSEC( MEASUREMENT_PERIOD - ADVERTISING_MEAS_PERIOD ) );
	}

}
