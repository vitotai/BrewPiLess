/*
 * Copyright 2013 Matthew McGowan
 * Copyright 2013 BrewPi/Elco Jacobs.
 *
 * This file is part of BrewPi.
 *
 * BrewPi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BrewPi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BrewPi.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "Brewpi.h"

#include "Actuator.h"
#include "Sensor.h"
#include "TempSensor.h"
#include "OneWireDevices.h"
#include "Pins.h"
#include "EepromStructs.h"


#ifdef ARDUINO
#include "DallasTemperature.h"	// for DeviceAddress
#endif

/**
 * A user has freedom to connect various devices to the arduino, either via extending the oneWire bus, or by assigning to specific pins, e.g. actuators, switch sensors.
 * Rather than make this compile-time, the configuration is stored at runtime.
 * Also, the availability of various sensors will change. E.g. it's possible to have a fridge constant mode without a beer sensor.
 *
 * Since the data has to be persisted to EEPROM, references to the actual uses of the devices have to be encoded.  This is the function of the deviceID.
 */

class DeviceConfig;

typedef int8_t device_slot_t;
inline bool isDefinedSlot(device_slot_t s) { return s>=0; }
const device_slot_t MAX_DEVICE_SLOT = 16;		// exclusive
const device_slot_t INVALID_SLOT = -1;

/*
 * Describes the logical function of each device.
 */
// DeviceFunction moved to EepromStructs.h


/**
 * Describes where the device is most closely associated.
 */
enum DeviceOwner {
	DEVICE_OWNER_NONE=0,
	DEVICE_OWNER_CHAMBER=1,
	DEVICE_OWNER_BEER=2
};

enum DeviceType {
	DEVICETYPE_NONE = 0,
	DEVICETYPE_TEMP_SENSOR = 1,		/* BasicTempSensor - OneWire */
	DEVICETYPE_SWITCH_SENSOR = 2,		/* SwitchSensor - direct pin and onewire are supported */
	DEVICETYPE_SWITCH_ACTUATOR = 3	/* Actuator - both direct pin and onewire are supported */
//#if EnableDHTSensorSupport	
	,
	DEVICETYPE_ENVIRONMENT_SENSOR = 4
//#endif
};


// enum DeviceHardware was moved to EepromStructs.h


inline bool isAssignable(DeviceType type, DeviceHardware hardware)
{
	//VTODO
	return (hardware==DEVICE_HARDWARE_PIN && (type==DEVICETYPE_SWITCH_ACTUATOR || type==DEVICETYPE_SWITCH_SENSOR))
#if BREWPI_DS2413
	|| (hardware==DEVICE_HARDWARE_ONEWIRE_2413 && (type==DEVICETYPE_SWITCH_ACTUATOR || (DS2413_SUPPORT_SENSE && type==DEVICETYPE_SWITCH_SENSOR)))
#endif
#if BREWPI_EXTERNAL_SENSOR
	|| (hardware==DEVICE_HARDWARE_EXTERNAL_SENSOR && type==DEVICETYPE_TEMP_SENSOR)
#endif
#if EnableHumidityControlSupport	
	|| (hardware==DEVICE_HARDWARE_PIN && type==DEVICETYPE_ENVIRONMENT_SENSOR)
	|| (hardware==DEVICE_HARDWARE_ENVIRONMENT_TEMP && type==DEVICETYPE_TEMP_SENSOR)
	|| (hardware==DEVICE_HARDWARE_BME280 && type == DEVICETYPE_ENVIRONMENT_SENSOR)
#endif
	|| (hardware==DEVICE_HARDWARE_ONEWIRE_TEMP && type==DEVICETYPE_TEMP_SENSOR)
	|| (hardware==DEVICE_HARDWARE_NONE && type==DEVICETYPE_NONE);
}

inline bool isOneWire(DeviceHardware hardware) {
	return
#if BREWPI_DS2413
	hardware==DEVICE_HARDWARE_ONEWIRE_2413 ||
#endif
	hardware==DEVICE_HARDWARE_ONEWIRE_TEMP;
}

inline bool isDigitalPin(DeviceHardware hardware) {
	return hardware==DEVICE_HARDWARE_PIN;
}

extern DeviceType deviceType(DeviceFunction id);

#if BREWPI_EXTERNAL_SENSOR
inline bool isExternalSensor(DeviceHardware hardware) {
	return hardware == DEVICE_HARDWARE_EXTERNAL_SENSOR;
}
#endif


#if EnableHumidityControlSupport
inline bool isEnvTempSensor(DeviceHardware hardware) {
	return hardware == DEVICE_HARDWARE_ENVIRONMENT_TEMP;
}
#endif

#if EnableBME280Support

inline bool isBME280(DeviceHardware hardware) {
	return hardware == DEVICE_HARDWARE_BME280;
}

#endif

/**
 * Determines where this devices belongs.
 */
inline DeviceOwner deviceOwner(DeviceFunction id) {
//	return id==0 ? DEVICE_OWNER_NONE : id>=DEVICE_BEER_FIRST ? DEVICE_OWNER_BEER : DEVICE_OWNER_CHAMBER;

	return id==0 ? DEVICE_OWNER_NONE : (id>=DEVICE_BEER_FIRST && id < DEVICE_CHAMBER_EXT)?  DEVICE_OWNER_BEER : DEVICE_OWNER_CHAMBER;

}


/*
 * A union of all device types.
 */

// struct DeviceConfig was moved to EepromStructs.h

/**
 * Provides a single alternative value for a given definition point in a device.
 */
struct DeviceAlternatives {
	enum AlternativeType {
		DA_PIN, DA_ADDRESS, DA_PIO, DA_INVERT, DA_BOOLVALUE
	};
	AlternativeType type;
	union {
		uint8_t pinNr;					// type == DA_PIN
		uint8_t pio;					// type == DA_PIO
		DeviceAddress address;			// type == DA_ADDRESS
		bool invert;					// type == DA_INVERT
		bool boolValue;					// type == DA_BOOLVALUE
	};

};


typedef void (*EnumDevicesCallback)(DeviceConfig*, void* pv);
class EnumerateHardware;

struct DeviceOutput
{
	device_slot_t	slot;
	char value[10];
};

struct DeviceDisplay {
	int8_t id;		// -1 for all devices, >=0 for specific device
	int8_t value;	// set value
	int8_t write;	// write value
	int8_t empty;	// show unused devices when id==-1, default is 0
};

void HandleDeviceDisplay(const char* key, const char* value, void* pv);

/**
 * Reads or writes a value to a device.
 */
void UpdateDeviceState(DeviceDisplay& dd, DeviceConfig& dc, char* val);

class OneWire;

class DeviceManager
{
public:

	bool isDefaultTempSensor(BasicTempSensor* sensor);

	int8_t enumerateActuatorPins(uint8_t offset)
	{
#ifdef ESP32
		switch (offset) {
			case 0: return actuatorPin1;
			case 1: return actuatorPin2;
			case 2: return actuatorPin3;
			case 3: return actuatorPin4;
			case 4: return actuatorPin5;
#if MORE_PINS_CONFIGURATION
			case 5: return actuatorPin6;
			case 6: return fanPin;
#endif
			default: return -1;
		}
#else
#if BREWPI_ACTUATOR_PINS && defined(ARDUINO)
#if BREWPI_STATIC_CONFIG<=BREWPI_SHIELD_REV_A
		switch (offset) {
			case 0: return heatingPin;
			case 1: return coolingPin;
			default:
				return -1;
		}
#elif BREWPI_STATIC_CONFIG>=BREWPI_SHIELD_REV_C
		switch (offset) {
			case 0: return actuatorPin1;
			case 1: return actuatorPin2;
			case 2: return actuatorPin3;
			case 3: return actuatorPin4;
			default: return -1;
		}
#endif
#endif
#endif
		return -1;
	}

	int8_t enumerateSensorPins(uint8_t offset) {
#ifdef ESP32
		if (offset==0)
			return doorPin;
#else
#if BREWPI_SENSOR_PINS && defined(ARDUINO)
		if (offset==0)
			return doorPin;
#endif
#endif
		return -1;
	}

	/* Enumerates the 1-wire pins.
	 *
	 */
	int8_t enumOneWirePins(uint8_t offset)
	{
#ifdef ARDUINO
#ifdef oneWirePin
		if (offset == 0)
			return oneWirePin;
#elif defined(beerSensorPin) && defined(fridgeSensorPin)
		if (offset==0)
			return beerSensorPin;
		if (offset==1)
			return fridgeSensorPin;
#endif
#endif
		return -1;
	}

	static void setupUnconfiguredDevices();

	/*
	 * Determines if the given device config is complete.
	 */
	static bool firstUndefinedAlternative(DeviceConfig& config, DeviceAlternatives& alternatives);


	/**
	 * Creates and Installs a device from the given device config.
	 * /return true if a device was installed. false if the config is not complete.
	 */
	static void installDevice(DeviceConfig& config);

	static void uninstallDevice(DeviceConfig& config);

	static void parseDeviceDefinition();
	static void printDevice(device_slot_t slot, DeviceConfig& config, const char* value);

	/**
	 * Iterate over the defined devices.
	 * Caller first calls with deviceIndex 0. If the return value is true, config is filled out with the
	 * config for the device. The caller can then increment deviceIndex and try again.
	 */
	static bool allDevices(DeviceConfig& config, uint8_t deviceIndex);

	static bool isDeviceValid(DeviceConfig& config, DeviceConfig& original, uint8_t deviceIndex);

	/**
	 * read hardware spec from stream and output matching devices
	 */
	static void enumerateHardware();

	static bool enumDevice(DeviceDisplay& dd, DeviceConfig& dc, uint8_t idx);

	static void listDevices();
	static void setFridgeSensorFallBack(bool fb){ fridgeSensorFallBack = fb; }
private:
	#if EnableDHTSensorSupport
	static void enumerateEnvTempDevices(EnumerateHardware& h, EnumDevicesCallback callback, DeviceOutput& output);
	#endif
	#if EnableBME280Support
	static void enumerateBME280(EnumerateHardware& h, EnumDevicesCallback callback, DeviceOutput& output);
	#endif
	#if BREWPI_EXTERNAL_SENSOR //vito: enumerate device
	static void enumerateExternalDevices(EnumerateHardware& h, EnumDevicesCallback callback, DeviceOutput& output);
	#endif
	static void enumerateOneWireDevices(EnumerateHardware& h, EnumDevicesCallback callback, DeviceOutput& output);
	static void enumeratePinDevices(EnumerateHardware& h, EnumDevicesCallback callback, DeviceOutput& output);
	static void OutputEnumeratedDevices(DeviceConfig* config, void* pv);
	static void handleEnumeratedDevice(DeviceConfig& config, EnumerateHardware& h, EnumDevicesCallback callback, DeviceOutput& out);
	static void readTempSensorValue(DeviceConfig::Hardware hw, char* out);


	static void* createDevice(DeviceConfig& config, DeviceType dc);
	static void* createOneWireGPIO(DeviceConfig& config, DeviceType dt);

	static void beginDeviceOutput() { firstDeviceOutput = true; }

	static OneWire* oneWireBus(uint8_t pin);

#ifdef ARDUINO

// There is no reason to separate the OneWire busses - if we have a single bus, use it.
#ifdef oneWirePin
	static OneWire primaryOneWireBus;
#else
	static OneWire beerSensorBus;
	static OneWire fridgeSensorBus;
#endif

#endif
	static bool firstDeviceOutput;
	static bool fridgeSensorFallBack;
};


extern DeviceManager deviceManager;
