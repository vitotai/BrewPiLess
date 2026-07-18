#pragma once

#ifdef ARDUINO
#include "DallasTemperature.h"	// for DeviceAddress
#endif


// These structs were moved from TempControl.h
struct ControlSettings {
	char mode;
	temperature beerSetting;
	temperature fridgeSetting;
	temperature heatEstimator; // updated automatically by self learning algorithm
	temperature coolEstimator; // updated automatically by self learning algorithm
};

struct ControlConstants {
	char tempFormat;
	temperature tempSettingMin;
	temperature tempSettingMax;
	temperature Kp;
	temperature Ki;
	temperature Kd;
	temperature iMaxError;
	// idleRangeHigh/idleRangeLow removed in EEPROM_FORMAT_VERSION 5. The on/off
	// hysteresis decision is now driven independently per actuator by
	// heatingTargetLower (heat trigger) and coolingTargetUpper (cool trigger)
	// below - see ControlConstants_v1/migrateControlConstantsFromV1 for the
	// migration of pre-existing EEPROM/LittleFS data.
	temperature heatingTargetUpper;
	temperature heatingTargetLower;
	temperature coolingTargetUpper;
	temperature coolingTargetLower;
	uint16_t maxHeatTimeForEstimate; // max time for heat estimate in seconds
	uint16_t maxCoolTimeForEstimate; // max time for heat estimate in seconds
									 // for the filter coefficients the b value is stored. a is calculated from b.
	uint8_t fridgeFastFilter;	// for display, logging and on-off control
	uint8_t fridgeSlowFilter;	// for peak detection
	uint8_t fridgeSlopeFilter;	// not used in current control algorithm
	uint8_t beerFastFilter;	// for display and logging
	uint8_t beerSlowFilter;	// for on/off control algorithm
	uint8_t beerSlopeFilter;	// for PID calculation
	uint8_t lightAsHeater;		// use the light to heat rather than the configured heater device
	uint8_t rotaryHalfSteps; // define whether to use full or half steps for the rotary encoder
	temperature pidMax;
#if 1 // SettableMinimumCoolTime
    uint16_t minCoolTime;
    uint16_t minCoolIdleTime;
    uint16_t minHeatTime;
    uint16_t minHeatIdleTime;
    uint16_t mutexDeadTime;
#endif
};

/*
 * Byte-identical mirror of the pre-EEPROM_FORMAT_VERSION-5 ControlConstants layout
 * (back when idleRangeHigh/idleRangeLow still existed). Kept ONLY so that existing
 * EEPROM (ESP8266) / LittleFS-SPIFFS (ESP32) data can be migrated in place - do not
 * use this struct anywhere else and do not change its layout.
 */
struct ControlConstants_v1 {
	char tempFormat;
	temperature tempSettingMin;
	temperature tempSettingMax;
	temperature Kp;
	temperature Ki;
	temperature Kd;
	temperature iMaxError;
	temperature idleRangeHigh;
	temperature idleRangeLow;
	temperature heatingTargetUpper;
	temperature heatingTargetLower;
	temperature coolingTargetUpper;
	temperature coolingTargetLower;
	uint16_t maxHeatTimeForEstimate;
	uint16_t maxCoolTimeForEstimate;
	uint8_t fridgeFastFilter;
	uint8_t fridgeSlowFilter;
	uint8_t fridgeSlopeFilter;
	uint8_t beerFastFilter;
	uint8_t beerSlowFilter;
	uint8_t beerSlopeFilter;
	uint8_t lightAsHeater;
	uint8_t rotaryHalfSteps;
	temperature pidMax;
#if 1 // SettableMinimumCoolTime
	uint16_t minCoolTime;
	uint16_t minCoolIdleTime;
	uint16_t minHeatTime;
	uint16_t minHeatIdleTime;
	uint16_t mutexDeadTime;
#endif
};

/*
 * Migrates a pre-v5 ControlConstants blob (with idleRangeHigh/idleRangeLow) into the
 * current layout. heatingTargetLower/coolingTargetUpper now drive the on/off trigger
 * that idleRangeLow/idleRangeHigh used to drive, so they are seeded from those old
 * values to preserve the previously configured deadband. heatingTargetUpper and
 * coolingTargetLower already existed under the same name/role (peak estimator tuning
 * in TempControl::detectPeaks) and are simply carried over unchanged.
 */
static inline void migrateControlConstantsFromV1(const ControlConstants_v1& legacy, ControlConstants& target) {
	target.tempFormat = legacy.tempFormat;
	target.tempSettingMin = legacy.tempSettingMin;
	target.tempSettingMax = legacy.tempSettingMax;
	target.Kp = legacy.Kp;
	target.Ki = legacy.Ki;
	target.Kd = legacy.Kd;
	target.iMaxError = legacy.iMaxError;
	target.heatingTargetUpper = legacy.heatingTargetUpper;
	target.heatingTargetLower = legacy.idleRangeLow;	// was the heat-trigger bound
	target.coolingTargetUpper = legacy.idleRangeHigh;	// was the cool-trigger bound
	target.coolingTargetLower = legacy.coolingTargetLower;
	target.maxHeatTimeForEstimate = legacy.maxHeatTimeForEstimate;
	target.maxCoolTimeForEstimate = legacy.maxCoolTimeForEstimate;
	target.fridgeFastFilter = legacy.fridgeFastFilter;
	target.fridgeSlowFilter = legacy.fridgeSlowFilter;
	target.fridgeSlopeFilter = legacy.fridgeSlopeFilter;
	target.beerFastFilter = legacy.beerFastFilter;
	target.beerSlowFilter = legacy.beerSlowFilter;
	target.beerSlopeFilter = legacy.beerSlopeFilter;
	target.lightAsHeater = legacy.lightAsHeater;
	target.rotaryHalfSteps = legacy.rotaryHalfSteps;
	target.pidMax = legacy.pidMax;
#if 1 // SettableMinimumCoolTime
	target.minCoolTime = legacy.minCoolTime;
	target.minCoolIdleTime = legacy.minCoolIdleTime;
	target.minHeatTime = legacy.minHeatTime;
	target.minHeatIdleTime = legacy.minHeatIdleTime;
	target.mutexDeadTime = legacy.mutexDeadTime;
#endif
}





/*
* Describes the logical function of each device.
*/
enum DeviceFunction {
	DEVICE_NONE = 0,			// used as a sentry to mark end of list
	// chamber devices
	DEVICE_CHAMBER_DOOR = 1,	// switch sensor
	DEVICE_CHAMBER_HEAT = 2,
	DEVICE_CHAMBER_COOL = 3,
	DEVICE_CHAMBER_LIGHT = 4,		// actuator
	DEVICE_CHAMBER_TEMP = 5,
	DEVICE_CHAMBER_ROOM_TEMP = 6,	// temp sensors
	DEVICE_CHAMBER_FAN = 7,			// a fan in the chamber
	DEVICE_CHAMBER_HUMIDITY_SENSOR = 8,	//  (reserved for future use)
	// carboy devices
	DEVICE_BEER_FIRST = 9,
	DEVICE_BEER_TEMP = DEVICE_BEER_FIRST,									// primary beer temp sensor
	DEVICE_BEER_TEMP2 = 10,								// secondary beer temp sensor
	DEVICE_BEER_HEAT = 11, DEVICE_BEER_COOL = 12,				// individual actuators
	DEVICE_BEER_SG = 13,									// SG sensor
	DEVICE_BEER_CAPPER = 14, 
	DEVICE_PTC_COOL = 15,	// reserved
	
	DEVICE_CHAMBER_EXT=16,
	DEVICE_CHAMBER_HUMIDIFIER = DEVICE_CHAMBER_EXT,
	DEVICE_CHAMBER_DEHUMIDIFIER = 17,
	DEVICE_CHAMBER_ROOM_HUMIDITY_SENSOR = 18,
	
	DEVICE_MAX = 19
};



/*
* The concrete type of the device.
*/
enum DeviceHardware {
	DEVICE_HARDWARE_NONE = 0,
	DEVICE_HARDWARE_PIN = 1,			// a digital pin, either input or output
	DEVICE_HARDWARE_ONEWIRE_TEMP = 2,	// a onewire temperature sensor
//#if BREWPI_DS2413
	DEVICE_HARDWARE_ONEWIRE_2413 = 3,	// a onewire 2-channel PIO input or output.
//#endif
//#if BREWPI_EXTERNAL_SENSOR
	DEVICE_HARDWARE_EXTERNAL_SENSOR = 5,
//#endif
//#if EnableDHTSensorSupport
	DEVICE_HARDWARE_ENVIRONMENT_TEMP = 6, 
//#endif
//#if EnableBME280Support
	DEVICE_HARDWARE_BME280 = 7,
//#endif
	DEVICE_HARDWARE_BTHOME_HUMIDITY =8,
	DEVICE_HARDWARE_BTHOME_THERMOMETER = 9,
	DEVICE_HARDWARE_RAPT_THERMOMETER = 10
};

/*
* A union of all device types.
*/
struct DeviceConfig {

	uint8_t chamber;			// 0 means no chamber. 1 is the first chamber.
	uint8_t beer;				// 0 means no beer, 1 is the first beer

	DeviceFunction deviceFunction;				// The function of the device to configure
	DeviceHardware deviceHardware;				// flag to indicate the runtime type of device
	struct Hardware {
		uint8_t pinNr;							// the arduino pin nr this device is connected to
		bool invert;							// for actuators/sensors, controls if the signal value is inverted.
		bool deactivate;							// disable this device - the device will not be installed.
		DeviceAddress address;					// for onewire devices, if address[0]==0 then use the first matching device type, otherwise use the device with the specific address

												/* The pio and sensor calibration are never needed at the same time so they are a union.
												* To ensure the eeprom format is stable when including/excluding DS2413 support, ensure all fields are the same size.
												*/
		union {
#if BREWPI_DS2413
			uint8_t pio;						// for ds2413 (deviceHardware==3) : the pio number (0,1)
#endif
			int8_t /* fixed4_4 */ calibration;	// for temp sensors (deviceHardware==2), calibration adjustment to add to sensor readings
												// this is intentionally chosen to match the raw value precision returned by the ds18b20 sensors
		};
		#if  EnableHumidityControlSupport
		uint8_t humiditySensorType;
		#else
		bool reserved;								// extra space so that additional fields can be added without breaking layout
		#endif
	} hw;
	bool reserved2;
};
