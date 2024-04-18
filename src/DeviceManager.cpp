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

#include "Brewpi.h"
#include "BrewpiStrings.h"
#include "DeviceManager.h"
#include "TempControl.h"
#include "Actuator.h"
#include "Sensor.h"
#include "TempSensorDisconnected.h"
#include "TempSensorExternal.h"
#include "PiLink.h"
#include "EepromFormat.h"
#include "Wire.h"

#if AUTO_CAP
#include "AutoCapControl.h"
#endif

#if EanbleParasiteTempControl
#include "ParasiteTempController.h"
#endif

#define CALIBRATION_OFFSET_PRECISION (4)

#ifdef ARDUINO
#include "OneWireTempSensor.h"
#include "OneWireActuator.h"
#include "DS2413.h"
#include <OneWire.h>
#include "DallasTemperature.h"
#include "ActuatorArduinoPin.h"
#include "SensorArduinoPin.h"
#endif

#if BREWPI_EXTERNAL_SENSOR
#include "TempSensorWireless.h"
WirelessTempSensor* WirelessTempSensor::theWirelessTempSensor=NULL;
#endif

#if EnableDHTSensorSupport
#include "HumidityControl.h"
#include "Bme280Sensor.h"
#include "TempSensorEnv.h"

#endif

/*
 * Defaults for sensors, actuators and temperature sensors when not defined in the eeprom.
 */

ValueSensor<bool> defaultSensor(false);			// off
ValueActuator defaultActuator;
DisconnectedTempSensor defaultTempSensor;

#if !BREWPI_SIMULATE
#ifdef oneWirePin
OneWire DeviceManager::primaryOneWireBus(oneWirePin);
#else
OneWire DeviceManager::beerSensorBus(beerSensorPin);
OneWire DeviceManager::fridgeSensorBus(fridgeSensorPin);
#endif
#endif


OneWire* DeviceManager::oneWireBus(uint8_t pin) {
#if !BREWPI_SIMULATE
#ifdef oneWirePin
	if (pin == oneWirePin)
		return &primaryOneWireBus;
#else
	if (pin==beerSensorPin)
		return &beerSensorBus;
	if (pin==fridgeSensorPin)
		return &fridgeSensorBus;
#endif
#endif
	return NULL;
}

bool DeviceManager::firstDeviceOutput;
bool DeviceManager::fridgeSensorFallBack = false;

bool DeviceManager::isDefaultTempSensor(BasicTempSensor* sensor) {
	return sensor==&defaultTempSensor;
}
#if EnableHumidityControlSupport
bool isEnvironmentSensorAvailable(){
	return humidityControl.roomSensor != &nullEnvironmentSensor
			|| humidityControl.chamberSensor != &nullEnvironmentSensor;
}
#endif

/**
 * Sets devices to their unconfigured states. Each device is initialized to a static no-op instance.
 * This method is idempotent, and is called each time the eeprom is reset.
 */
void DeviceManager::setupUnconfiguredDevices()
{
	// right now, uninstall doesn't care about chamber/beer distinction.
	// but this will need to match beer/function when multiferment is available
	DeviceConfig cfg;
	cfg.chamber = 1; cfg.beer = 1;
	for (uint8_t i=0; i<DEVICE_MAX; i++) {
		cfg.deviceFunction = DeviceFunction(i);
		uninstallDevice(cfg);
	}
}


/**
 * Creates a new device for the given config.
 */
void* DeviceManager::createDevice(DeviceConfig& config, DeviceType dt)
{
	
	switch (config.deviceHardware) {
		case DEVICE_HARDWARE_NONE:
			break;
		case DEVICE_HARDWARE_PIN:
			if (dt==DEVICETYPE_SWITCH_SENSOR)
			#if BREWPI_SIMULATE
				return new ValueSensor<bool>(false);
			#else
				return new DigitalPinSensor(config.hw.pinNr, config.hw.invert);
			#endif

#if EnableDHTSensorSupport 
			else if(dt == DEVICETYPE_ENVIRONMENT_SENSOR){
//				Serial.printf("create sensorType:%d, pinNr:%d\n",config.hw.humiditySensorType,config.hw.pinNr);
				// calibration for temperature is in fixed_4_4 fromat
				DHTSensor *dht= new DHTSensor(config.hw.pinNr,config.hw.humiditySensorType);
				dht->setHumidityCalibration(tempDiffToInt(temperature(config.hw.calibration)<<(TEMP_FIXED_POINT_BITS-CALIBRATION_OFFSET_PRECISION)));
				return dht;
			}
#endif
			else
#if BREWPI_SIMULATE
				return new ValueActuator();
#else
				// use hardware actuators even for simulator
				return new DigitalPinActuator(config.hw.pinNr, config.hw.invert);
#endif
		case DEVICE_HARDWARE_ONEWIRE_TEMP:
		#if BREWPI_SIMULATE
			return new ExternalTempSensor(false);// initially disconnected, so init doesn't populate the filters with the default value of 0.0
		#else
			return new OneWireTempSensor(oneWireBus(config.hw.pinNr), config.hw.address, config.hw.calibration);
		#endif
#if BREWPI_EXTERNAL_SENSOR //vito: create sensor object
		case DEVICE_HARDWARE_EXTERNAL_SENSOR:
			return new WirelessTempSensor(false,config.hw.calibration);// initially disconnected, so init doesn't populate the filters with the default value of 0.0
#endif

#if EnableDHTSensorSupport 
		case DEVICE_HARDWARE_ENVIRONMENT_TEMP:
			return new EnvTempSensor( config.hw.pinNr==0? HumidityControl::chamberSensor:HumidityControl::roomSensor,
				config.hw.calibration); //VTODO
#endif				
#if EnableBME280Support
		case DEVICE_HARDWARE_BME280:{
			Bme280Sensor* bme=new Bme280Sensor(config.hw.pinNr);
			bme->setHumidityCalibration(tempDiffToInt(temperature(config.hw.calibration)<<(TEMP_FIXED_POINT_BITS-CALIBRATION_OFFSET_PRECISION)));
			return bme;
		}
#endif



#if BREWPI_DS2413
		case DEVICE_HARDWARE_ONEWIRE_2413:
		#if BREWPI_SIMULATE
		if (dt==DEVICETYPE_SWITCH_SENSOR)
			return new ValueSensor<bool>(false);
		else
			return new ValueActuator();
		#else
			return new OneWireActuator(oneWireBus(config.hw.pinNr), config.hw.address, config.hw.pio, config.hw.invert);
		#endif
#endif
	}
	return NULL;
}

/**
 * Returns the pointer to where the device pointer resides. This can be used to delete the current device and install a new one.
 * For Temperature sensors, the returned pointer points to a TempSensor*. The basic device can be fetched by calling
 * TempSensor::getSensor().
 */
inline void** deviceTarget(DeviceConfig& config)
{
	// for multichamber, will write directly to the multi-chamber managed storage.
	// later...
	if (config.chamber>1 || config.beer>1)
		return NULL;

	void** ppv;
	switch (config.deviceFunction) {
	case DEVICE_CHAMBER_ROOM_TEMP:
		ppv = (void**)&tempControl.ambientSensor;
		break;
	case DEVICE_CHAMBER_DOOR:
		ppv = (void**)&tempControl.door;
		break;
	case DEVICE_CHAMBER_LIGHT:
		ppv = (void**)&tempControl.light;
		break;
	case DEVICE_CHAMBER_HEAT:
		ppv = (void**)&tempControl.heater;
		break;
	case DEVICE_CHAMBER_COOL:
		ppv = (void**)&tempControl.cooler;
		break;
	case DEVICE_CHAMBER_TEMP:
		ppv = (void**)&tempControl.fridgeSensor;
		break;
	case DEVICE_CHAMBER_FAN:
		ppv = (void**)&tempControl.fan;
		break;

#if AUTO_CAP
	case DEVICE_BEER_CAPPER:
		ppv = (void**)&autoCapControl.capper;
	break;
#endif

#if EanbleParasiteTempControl
	case DEVICE_PTC_COOL:
		ppv = (void**)& parasiteTempController.cooler;
		break;
#endif

#if EnableDHTSensorSupport	
	case DEVICE_CHAMBER_HUMIDITY_SENSOR:
		ppv =(void**) & humidityControl.chamberSensor;
		break;

	case DEVICE_CHAMBER_ROOM_HUMIDITY_SENSOR:
		ppv =(void**) & humidityControl.roomSensor;
		break;


	case DEVICE_CHAMBER_HUMIDIFIER:
		ppv = (void**) & humidityControl.humidifier;
		break;
	case DEVICE_CHAMBER_DEHUMIDIFIER:
		ppv = (void**) & humidityControl.dehumidifier;
		break;
#endif
	case DEVICE_BEER_TEMP:
		ppv = (void**)&tempControl.beerSensor;
		break;
	default:
		ppv = NULL;
	}
	return ppv;
}

// A pointer to a "temp sensor" may be a TempSensor* or a BasicTempSensor* .
// These functions allow uniform treatment.
inline bool isBasicSensor(DeviceFunction function) {
	// currently only ambient sensor is basic. The others are wrapped in a TempSensor.
	return function==DEVICE_CHAMBER_ROOM_TEMP;
}

inline BasicTempSensor& unwrapSensor(DeviceFunction f, void* pv) {
	return isBasicSensor(f) ? *(BasicTempSensor*)pv : ((TempSensor*)pv)->sensor();
}

inline void setSensor(DeviceFunction f, void** ppv, BasicTempSensor* sensor) {
	if (isBasicSensor(f))
		*ppv = sensor;
	else
		((TempSensor*)*ppv)->setSensor(sensor);
}

/**
 * Removes an installed device.
 * /param config The device to remove. The fields that are used are
 *		chamber, beer, hardware and function.
 */

void DeviceManager::uninstallDevice(DeviceConfig& config)
{
	DeviceType dt = deviceType(config.deviceFunction);
	void** ppv = deviceTarget(config);
	if (ppv==NULL)
		return;

	BasicTempSensor* s;
	switch(dt) {
		case DEVICETYPE_NONE:
			break;
		case DEVICETYPE_TEMP_SENSOR:
			// sensor may be wrapped in a TempSensor class, or may stand alone.
			s = &unwrapSensor(config.deviceFunction, *ppv);
			if (s!=&defaultTempSensor) {
				setSensor(config.deviceFunction, ppv, &defaultTempSensor);
				//#if FridgeSensorFallBack
				if(fridgeSensorFallBack){ 
					if(config.deviceFunction == DEVICE_BEER_TEMP)
						tempControl.fridgeSensor->setBackupSensor(&defaultTempSensor);
				} 
				//#endif

//				DEBUG_ONLY(logInfoInt(INFO_UNINSTALL_TEMP_SENSOR, config.deviceFunction));
				delete s;
			}
			break;
		case DEVICETYPE_SWITCH_ACTUATOR:
			if (*ppv!=&defaultActuator) {
//				DEBUG_ONLY(logInfoInt(INFO_UNINSTALL_ACTUATOR, config.deviceFunction));
				delete (Actuator*)*ppv;
				*ppv = &defaultActuator;
			}
			break;
		case DEVICETYPE_SWITCH_SENSOR:
			if (*ppv!=&defaultSensor) {
//				DEBUG_ONLY(logInfoInt(INFO_UNINSTALL_SWITCH_SENSOR, config.deviceFunction));
				delete (SwitchSensor*)*ppv;
				*ppv = &defaultSensor;
			}
			break;
		#if EnableDHTSensorSupport	 
		case DEVICETYPE_ENVIRONMENT_SENSOR:
			if (*ppv!=&nullEnvironmentSensor) {
				if( ((EnvironmentSensor*)*ppv)->sensorType() == SensorType_BME280)  delete (Bme280Sensor*)*ppv;
				else delete (DHTSensor*)*ppv;
				*ppv = &nullEnvironmentSensor;
			}
			break;
		#endif
	}
}

/**
 * Creates and installs a device in the current chamber.
 */
void DeviceManager::installDevice(DeviceConfig& config)
{
	DeviceType dt = deviceType(config.deviceFunction);
	void** ppv = deviceTarget(config);
	if (ppv==NULL || config.hw.deactivate)
		return;
	BasicTempSensor* s;
	TempSensor* ts;
	switch(dt) {
		case DEVICETYPE_NONE:
			break;
		case DEVICETYPE_TEMP_SENSOR:
			DEBUG_ONLY(logInfoInt(INFO_INSTALL_TEMP_SENSOR, config.deviceFunction));
			// sensor may be wrapped in a TempSensor class, or may stand alone.
			s = (BasicTempSensor*)createDevice(config, dt);
			if (*ppv==NULL){
				logErrorInt(ERROR_OUT_OF_MEMORY_FOR_DEVICE, config.deviceFunction);
			}
			if (isBasicSensor(config.deviceFunction)) {
				s->init();
				*ppv = s;
			}
			else {
				ts = ((TempSensor*)*ppv);
				ts->setSensor(s);
				//#if FridgeSensorFallBack
				if(fridgeSensorFallBack){
					if(config.deviceFunction == DEVICE_BEER_TEMP)
						tempControl.fridgeSensor->setBackupSensor(s);
				}
				//#endif
				ts->init();
			}
#if BREWPI_SIMULATE
			((ExternalTempSensor*)s)->setConnected(true);	// now connect the sensor after init is called
#endif
			break;
		#if EnableDHTSensorSupport
		case DEVICETYPE_ENVIRONMENT_SENSOR:	
		//fall through
		#endif
		case DEVICETYPE_SWITCH_ACTUATOR:
		case DEVICETYPE_SWITCH_SENSOR:
			DEBUG_ONLY(logInfoInt(INFO_INSTALL_DEVICE, config.deviceFunction));
			*ppv = createDevice(config, dt);
#if (BREWPI_DEBUG > 0)
			if (*ppv==NULL)
				logErrorInt(ERROR_OUT_OF_MEMORY_FOR_DEVICE, config.deviceFunction);
#endif
			break;
	}
}

struct DeviceDefinition {
	int8_t id;
	int8_t chamber;
	int8_t beer;
	int8_t deviceFunction;
	int8_t deviceHardware;
	int8_t pinNr;
	int8_t invert;
	int8_t pio;
	int8_t deactivate;
	int8_t calibrationAdjust;
	int8_t hsensorType;
	DeviceAddress address;

	/**
	 * Lists the first letter of the key name for each attribute.
	 */
	static const char ORDER[13];
};

// the special cases are placed at the end. All others should map directly to an int8_t via atoi().
const char DeviceDefinition::ORDER[13] = "icbfhpxndjsa";

const char DEVICE_ATTRIB_INDEX = 'i';
const char DEVICE_ATTRIB_CHAMBER = 'c';
const char DEVICE_ATTRIB_BEER = 'b';
const char DEVICE_ATTRIB_FUNCTION = 'f';
const char DEVICE_ATTRIB_HARDWARE = 'h';
const char DEVICE_ATTRIB_PIN = 'p';
const char DEVICE_ATTRIB_INVERT = 'x';
const char DEVICE_ATTRIB_DEACTIVATED = 'd';
const char DEVICE_ATTRIB_ADDRESS = 'a';
#if BREWPI_DS2413
const char DEVICE_ATTRIB_PIO = 'n';
#endif
#if EnableDHTSensorSupport
const char DEVICE_ATTRIB_HUMIDITY_SENSOR_TYPE = 's';
#endif
const char DEVICE_ATTRIB_CALIBRATEADJUST = 'j';	// value to add to temp sensors to bring to correct temperature

const char DEVICE_ATTRIB_VALUE = 'v';		// print current values
const char DEVICE_ATTRIB_WRITE = 'w';		// write value to device

const char DEVICE_ATTRIB_TYPE = 't';


void handleDeviceDefinition(const char* key, const char* val, void* pv)
{
	DeviceDefinition* def = (DeviceDefinition*) pv;
//	logDebug("deviceDef %s:%s", key, val);
	// the characters are listed in the same order as the DeviceDefinition struct.
	int8_t idx = indexOf(DeviceDefinition::ORDER, key[0]);
	if (key[0]==DEVICE_ATTRIB_ADDRESS)
		parseBytes(def->address, val, 8);
	else if (key[0]==DEVICE_ATTRIB_CALIBRATEADJUST) {
		def->calibrationAdjust = fixed4_4(stringToTempDiff(val)>>(TEMP_FIXED_POINT_BITS-CALIBRATION_OFFSET_PRECISION));
	}
	else if (idx>=0)
		((uint8_t*)def)[idx] = (uint8_t)atol(val);
}

bool inRangeUInt8(uint8_t val, uint8_t min, int8_t max) {
	return min<=val && val<=max;
}

bool inRangeInt8(int8_t val, int8_t min, int8_t max) {
	return min<=val && val<=max;
}

void assignIfSet(int8_t value, uint8_t* target) {
	if (value>=0)
		*target = (uint8_t)value;
}
#define assignIfSetMacro(value,target,type)  if ((value)>=0) target =(type) value;

/**
 * Updates the device definition. Only changes that result in a valid device, with no conflicts with other devices
 * are allowed.
 */
void DeviceManager::parseDeviceDefinition()
{
	static DeviceDefinition dev;
	fill((int8_t*)&dev, sizeof(dev));

	piLink.parseJson(&handleDeviceDefinition, &dev);

	if (!inRangeInt8(dev.id, 0, MAX_DEVICE_SLOT))			// no device id given, or it's out of range, can't do anything else.
		return;

	// save the original device so we can revert
	DeviceConfig target;
	DeviceConfig original;

	// todo - should ideally check if the eeprom is correctly initialized.
	eepromManager.fetchDevice(original, dev.id);
	memcpy(&target, &original, sizeof(target));
#ifndef ESP8266_ONE
	piLink.print("Dev Chamber: %d, Dev Beer: %d, Dev Function: %d, Dev Hardware: %d, Dev PinNr: %d\r\n", dev.chamber, dev.beer, dev.deviceFunction, dev.deviceHardware, dev.pinNr);
	piLink.print("target Chamber: %d, target Beer: %d, target Function: %d, target Hardware: %d, target PinNr: %d\r\n", target.chamber,
		target.beer, target.deviceFunction, target.deviceHardware, target.hw.pinNr);
#endif

	assignIfSet(dev.chamber, &target.chamber);
	assignIfSet(dev.beer, &target.beer);
//	assignIfSet(dev.deviceFunction, (uint8_t*)&target.deviceFunction);
//	assignIfSet(dev.deviceHardware, (uint8_t*)&target.deviceHardware);
//  above code woudl break when the size of enum is 4. 
	assignIfSetMacro(dev.deviceFunction, target.deviceFunction, DeviceFunction);
	assignIfSetMacro(dev.deviceHardware, target.deviceHardware, DeviceHardware);

	assignIfSet(dev.pinNr, &target.hw.pinNr);
//	Serial.printf("sizeof(target.deviceFunction) =%d, sizeof(uint8_t)=%d\n",sizeof(target.deviceFunction), sizeof(uint8_t));

#if EnableDHTSensorSupport
	assignIfSet(dev.hsensorType, &target.hw.humiditySensorType);
#endif

#if BREWPI_DS2413
	assignIfSet(dev.pio, &target.hw.pio);
#endif

	if (dev.calibrationAdjust!=-1)		// since this is a union, it also handles pio for 2413 sensors
		target.hw.calibration = dev.calibrationAdjust;

	//assignIfSet(dev.invert, (uint8_t*)&target.hw.invert);
	assignIfSetMacro(dev.invert, target.hw.invert,bool);
	
	if (dev.address[0] != 0xFF) {// first byte is family identifier. I don't have a complete list, but so far 0xFF is not used.
#ifndef ESP8266_ONE
		piLink.print("Dev Address: %s, Target Address: %s\r\n", dev.address, target.hw.address);
#endif
		memcpy(target.hw.address, dev.address, 8);
	}
	//assignIfSet(dev.deactivate, (uint8_t*)&target.hw.deactivate);
	assignIfSetMacro(dev.deactivate, target.hw.deactivate,bool);

	// setting function to none clears all other fields.
	if (target.deviceFunction==DEVICE_NONE) {
#ifndef ESP8266_ONE
		piLink.print("Function set to NONE\r\n");
#endif
		clear((uint8_t*)&target, sizeof(target));
	}

	bool valid = isDeviceValid(target, original, dev.id);
	DeviceConfig* print = &original;
	if (valid) {
		print = &target;
		// remove the device associated with the previous function
		uninstallDevice(original);
		// also remove any existing device for the new function, since install overwrites any existing definition.
		uninstallDevice(target);
		installDevice(target);
		eepromManager.storeDevice(target, dev.id);
	}
	else {
		logError(ERROR_DEVICE_DEFINITION_UPDATE_SPEC_INVALID);

	}
	piLink.printResponse('U');
	deviceManager.beginDeviceOutput();
	deviceManager.printDevice(dev.id, *print, NULL);
	piLink.printNewLine();
}

/**
 * Determines if a given device definition is valid.
 * chamber/beer must be within bounds
 * device function must match the chamber/beer spec, and must not already be defined for the same chamber/beer combination
 * device hardware type must be applicable with the device function
 * pinNr must be unique for digital pin devices?
 * pinNr must be a valid OneWire bus for one wire devices.
 * for onewire temp devices, address must be unique.
 * for onewire ds2413 devices, address+pio must be unique.
 */
bool DeviceManager::isDeviceValid(DeviceConfig& config, DeviceConfig& original, uint8_t deviceIndex)
{
#if 1
	/* Implemented checks to ensure the system will not crash when supplied with invalid data.
	   More refined checks that may cause confusing results are not yet implemented. See todo below. */

	/* chamber and beer within range.*/
	if (!inRangeUInt8(config.chamber, 0, EepromFormat::MAX_CHAMBERS))
	{
		logErrorInt(ERROR_INVALID_CHAMBER, config.chamber);
		return false;
	}

	/* 0 is allowed - represents a chamber device not assigned to a specific beer */
	if (!inRangeUInt8(config.beer, 0, ChamberBlock::MAX_BEERS))
	{
		logErrorInt(ERROR_INVALID_BEER, config.beer);
		return false;
	}

	if (!inRangeUInt8(config.deviceFunction, 0, DEVICE_MAX-1))
	{
		logErrorInt(ERROR_INVALID_DEVICE_FUNCTION, config.deviceFunction);
		return false;
	}

	DeviceOwner owner = deviceOwner(config.deviceFunction);
	if (!((owner==DEVICE_OWNER_BEER && config.beer) || (owner==DEVICE_OWNER_CHAMBER && config.chamber)
		|| (owner==DEVICE_OWNER_NONE && !config.beer && !config.chamber)))
	{
		logErrorIntIntInt(ERROR_INVALID_DEVICE_CONFIG_OWNER, owner, config.beer, config.chamber);
		return false;
	}

	// todo - find device at another index with the same chamber/beer/function spec.
	// with duplicate function defined for the same beer, that they will both try to create/delete the device in the target location.
	// The highest id will win.
	DeviceType dt = deviceType(config.deviceFunction);
	if (!isAssignable(dt, config.deviceHardware)) {
		logErrorIntInt(ERROR_CANNOT_ASSIGN_TO_HARDWARE, dt, config.deviceHardware);
		return false;
	}

	// todo - check pinNr uniqueness for direct digital I/O devices?

	/* pinNr for a onewire device must be a valid bus. While this won't cause a crash, it's a good idea to validate this. */
	if (isOneWire(config.deviceHardware)) {
		if (!oneWireBus(config.hw.pinNr)) {
			logErrorInt(ERROR_NOT_ONEWIRE_BUS, config.hw.pinNr);
			return false;
		}
	}
	else {		// regular pin device
		// todo - could verify that the pin nr corresponds to enumActuatorPins/enumSensorPins
	}

#endif
	// todo - for onewire temp, ensure address is unique
	// todo - for onewire 2413 check address+pio nr is unique
	return true;
}

void printAttrib(Print& p, char c, int8_t val, bool first=false)
{
	if (!first)
        	p.print(',');

	char tempString[32]; // resulting string limited to 128 chars
	sprintf_P(tempString, PSTR("\"%c\":%d"), c, val);
	p.print(tempString);
}

// I really want to buffer stuff rather than printing directly to the serial stream
void appendAttrib(String& str, char c, int8_t val, bool first = false)
{
	if (!first)
		str += ",";

	char tempString[32]; // resulting string limited to 128 chars
	sprintf_P(tempString, PSTR("\"%c\":%d"), c, val);
	str += tempString;
}

inline bool hasInvert(DeviceHardware hw)
{
	return hw==DEVICE_HARDWARE_PIN
#if BREWPI_DS2413
	|| hw==DEVICE_HARDWARE_ONEWIRE_2413
#endif
	;
}

inline bool hasOnewire(DeviceHardware hw)
{
	return
#if BREWPI_DS2413
	hw==DEVICE_HARDWARE_ONEWIRE_2413 ||
#endif
	hw==DEVICE_HARDWARE_ONEWIRE_TEMP;
}

void DeviceManager::printDevice(device_slot_t slot, DeviceConfig& config, const char* value)
{
	String deviceString;
	char buf[17];

	DeviceType dt = deviceType(config.deviceFunction);
	if (!firstDeviceOutput) {
		// p.print('\n');
//		p.print(',');
		deviceString = ",";
	} else {
		deviceString = "";
	}
	firstDeviceOutput = false;
	deviceString += "{";


	appendAttrib(deviceString, DEVICE_ATTRIB_INDEX, slot, true);
	appendAttrib(deviceString, DEVICE_ATTRIB_TYPE, dt);

	appendAttrib(deviceString, DEVICE_ATTRIB_CHAMBER, config.chamber);
	appendAttrib(deviceString, DEVICE_ATTRIB_BEER, config.beer);
	appendAttrib(deviceString, DEVICE_ATTRIB_FUNCTION, config.deviceFunction);
	appendAttrib(deviceString, DEVICE_ATTRIB_HARDWARE, config.deviceHardware);
	appendAttrib(deviceString, DEVICE_ATTRIB_DEACTIVATED, config.hw.deactivate);
	appendAttrib(deviceString, DEVICE_ATTRIB_PIN, config.hw.pinNr);
#if EnableDHTSensorSupport
	if(config.deviceFunction == DEVICE_CHAMBER_HUMIDITY_SENSOR
			|| config.deviceFunction == DEVICE_CHAMBER_ROOM_HUMIDITY_SENSOR) //VTODO: PIN device: DHT serious, DEVICE_HARDWARE_BME280
		appendAttrib(deviceString,DEVICE_ATTRIB_HUMIDITY_SENSOR_TYPE,config.hw.humiditySensorType);
#endif
	if (value && *value) {
		deviceString += ",\"v\":";
		deviceString += value;
	}
	if (hasInvert(config.deviceHardware))
		appendAttrib(deviceString, DEVICE_ATTRIB_INVERT, config.hw.invert);

	if (hasOnewire(config.deviceHardware)) {
		deviceString += ",\"a\":\"";
		printBytes(config.hw.address, 8, buf);
		deviceString += buf;
		deviceString += '"';
	}
#if BREWPI_DS2413
	if (config.deviceHardware==DEVICE_HARDWARE_ONEWIRE_2413) {
		appendAttrib(deviceString, DEVICE_ATTRIB_PIO, config.hw.pio);
	}
#endif
	if (config.deviceHardware==DEVICE_HARDWARE_ONEWIRE_TEMP
#if BREWPI_EXTERNAL_SENSOR
		||  config.deviceHardware==DEVICE_HARDWARE_EXTERNAL_SENSOR
#endif	
#if EnableDHTSensorSupport

		||  config.deviceFunction==DEVICE_CHAMBER_HUMIDITY_SENSOR //VTODO
		||  config.deviceFunction == DEVICE_CHAMBER_ROOM_HUMIDITY_SENSOR
		||  config.deviceHardware == DEVICE_HARDWARE_ENVIRONMENT_TEMP
#endif
	) {
		tempDiffToString(buf, temperature(config.hw.calibration)<<(TEMP_FIXED_POINT_BITS-CALIBRATION_OFFSET_PRECISION), 3, 8);
		deviceString += ",\"j\":";
		deviceString += buf;
	}
	deviceString += '}';

	piLink.print_P(deviceString.c_str());
}

bool DeviceManager::allDevices(DeviceConfig& config, uint8_t deviceIndex)
{
	return eepromManager.fetchDevice(config, deviceIndex);
}

void parseBytes(uint8_t* data, const char* s, uint8_t len) {
	char c;
	while ((c=*s++)) {
		uint8_t d = (c>='A' ? c-'A'+10 : c-'0')<<4;
		c = *s++;
		d |= (c>='A' ? c-'A'+10 : c-'0');
		*data++ = d;
	}
}

void printBytes(uint8_t* data, uint8_t len, char* buf) // prints 8-bit data in hex
{
	for (int i=0; i<len; i++) {
		uint8_t b = (data[i] >> 4) & 0x0f;
		*buf++ = (b>9 ? b-10+'A' : b+'0');
		b = data[i] & 0x0f;
		*buf++ = (b>9 ? b-10+'A' : b+'0');
	}
	*buf = 0;
}

void DeviceManager::OutputEnumeratedDevices(DeviceConfig* config, void* pv)
{
	DeviceOutput* out = (DeviceOutput*)pv;
	printDevice(out->slot, *config, out->value);
}

bool DeviceManager::enumDevice(DeviceDisplay& dd, DeviceConfig& dc, uint8_t idx)
{
	if (dd.id==-1)
		return (dd.empty || dc.deviceFunction);	// if enumerating all devices, honor the unused request param
	else
		return (dd.id==idx);						// enumerate only the specific device requested
}

struct EnumerateHardware
{
	int8_t hardware;		// restrict the types of devices requested
	int8_t pin;				// pin to search
	int8_t values;			// fetch values for the devices.
	int8_t unused;			// 0 don't care about unused state, 1 unused only.
	int8_t function;		// restrict to devices that can be used with this function
};

void handleHardwareSpec(const char* key, const char* val, void* pv)
{
	EnumerateHardware* h = (EnumerateHardware*)pv;
//	logDebug("hardwareSpec %s:%s", key, val);

	int8_t idx = indexOf("hpvuf", key[0]);
	if (idx>=0) {
		*((int8_t*)h+idx) = atol(val);
	}
}

inline bool matchAddress(uint8_t* detected, uint8_t* configured, uint8_t count) {
	if (!configured[0])
		return true;
	while (count-->0) {
		if (detected[count]!=configured[count])
			return false;
	}
	return true;
}

/**
 * Find a device based on it's location.
 * A device's location is:
 *   pinNr  for simple digital pin devices
 *   pinNr+address for one-wire devices
 *   pinNr+address+pio for 2413
 */
device_slot_t findHardwareDevice(DeviceConfig& find)
{
	DeviceConfig config;
	for (device_slot_t slot= 0; deviceManager.allDevices(config, slot); slot++) {
		if (find.deviceHardware==config.deviceHardware) {
			bool match = true;
			switch (find.deviceHardware) {
#if BREWPI_DS2413
				case DEVICE_HARDWARE_ONEWIRE_2413:
					match &= find.hw.pio==config.hw.pio;
					// fall through
#endif
				case DEVICE_HARDWARE_ONEWIRE_TEMP:
					match &= matchAddress(find.hw.address, config.hw.address, 8);
					// fall through
				case DEVICE_HARDWARE_PIN:
					match &= find.hw.pinNr==config.hw.pinNr;
					break;
			#if EnableDHTSensorSupport
				case DEVICE_HARDWARE_ENVIRONMENT_TEMP:
					match &= isEnvironmentSensorAvailable();
					break;
			#endif

			#if BREWPI_EXTERNAL_SENSOR
				case DEVICE_HARDWARE_EXTERNAL_SENSOR:
					match &= true;
					break;
			#endif
	
					default:	// this should not happen - if it does the device will be returned as matching.
					break;
			}
			if (match)
				return slot;
		}
	}
	return INVALID_SLOT;
}

inline void DeviceManager::readTempSensorValue(DeviceConfig::Hardware hw, char* out)
{
#if !BREWPI_SIMULATE
	OneWire* bus = oneWireBus(hw.pinNr);
	OneWireTempSensor sensor(bus, hw.address, 0);		// NB: this value is uncalibrated, since we don't have the calibration offset until the device is configured
	temperature temp = INVALID_TEMP;
	if (sensor.init())
		temp = sensor.read();
	tempToString(out, temp, 3, 9);
#else
	strcpy_P(out, PSTR("0.00"));
#endif
}

void DeviceManager::handleEnumeratedDevice(DeviceConfig& config, EnumerateHardware& h, EnumDevicesCallback callback, DeviceOutput& out)
{
	if (h.function && !isAssignable(deviceType(DeviceFunction(h.function)), config.deviceHardware))
		return; // device not applicable for required function

//	logDebug("Handling device");
	out.slot = findHardwareDevice(config);
	DEBUG_ONLY(logInfoInt(INFO_MATCHING_DEVICE, out.slot));

	if (isDefinedSlot(out.slot)) {
		if (h.unused)	// only list unused devices, and this one is already used
			return;
		// display the actual matched value
		deviceManager.allDevices(config, out.slot);
	}

	out.value[0] = 0;
	if (h.values) {
//		logDebug("Fetching device value");
		switch (config.deviceHardware) {
			case DEVICE_HARDWARE_ONEWIRE_TEMP:
				readTempSensorValue(config.hw, out.value);
				break;
			// unassigned pins could be input or output so we can't determine any other details from here.
			// values can be read once the pin has been assigned a function
			default:
				break;
		}
	}
//	logDebug("Passing device to callback");
	callback(&config, &out);
}

void DeviceManager::enumeratePinDevices(EnumerateHardware& h, EnumDevicesCallback callback, DeviceOutput& output)
{
	DeviceConfig config;
	clear((uint8_t*)&config, sizeof(config));
	config.deviceHardware = DEVICE_HARDWARE_PIN;
	config.chamber = 1; // chamber 1 is default

	int8_t pin;
	for (uint8_t count=0; (pin=deviceManager.enumerateActuatorPins(count))>=0; count++) {
		if (h.pin!=-1 && h.pin!=pin)
			continue;
		config.hw.pinNr = pin;
		config.hw.invert = true; // make inverted default, because shiels have transistor on them
		handleEnumeratedDevice(config, h, callback, output);
	}

	for (uint8_t count=0; (pin=deviceManager.enumerateSensorPins(count))>=0; count++) {
		if (h.pin!=-1 && h.pin!=pin)
			continue;
		config.hw.pinNr = pin;
		handleEnumeratedDevice(config, h, callback, output);
	}
}

void DeviceManager::enumerateOneWireDevices(EnumerateHardware& h, EnumDevicesCallback callback, DeviceOutput& output)
{
#if !BREWPI_SIMULATE
	int8_t pin;
	for (uint8_t count=0; (pin=deviceManager.enumOneWirePins(count))>=0; count++) {
		DeviceConfig config;
		clear((uint8_t*)&config, sizeof(config));
		if (h.pin!=-1 && h.pin!=pin)
			continue;
		config.hw.pinNr = pin;
		config.chamber = 1; // chamber 1 is default
//		logDebug("Enumerating one-wire devices on pin %d", pin);
		OneWire* wire = oneWireBus(pin);
		if (wire!=NULL) {
			wire->reset_search();
			while (wire->search(config.hw.address)) {
				// hardware device type from OneWire family ID
				switch (config.hw.address[0]) {
		#if BREWPI_DS2413
					case DS2413_FAMILY_ID:
						config.deviceHardware = DEVICE_HARDWARE_ONEWIRE_2413;
						break;
		#endif
					case DS18B20MODEL:
						config.deviceHardware = DEVICE_HARDWARE_ONEWIRE_TEMP;
						break;
					default:
						config.deviceHardware = DEVICE_HARDWARE_NONE;
				}

				switch (config.deviceHardware) {
		#if BREWPI_DS2413
					// for 2408 this will require iterating 0..7
					case DEVICE_HARDWARE_ONEWIRE_2413:
						// enumerate each pin separately
						for (uint8_t i=0; i<2; i++) {
							config.hw.pio = i;
							handleEnumeratedDevice(config, h, callback, output);
						}
						break;
		#endif
					case DEVICE_HARDWARE_ONEWIRE_TEMP:
		#if !ONEWIRE_PARASITE_SUPPORT
						{	// check that device is not parasite powered
							DallasTemperature sensor(wire);
							if(sensor.initConnection(config.hw.address)){
								handleEnumeratedDevice(config, h, callback, output);
							}
						}
		#else
						handleEnumeratedDevice(config, h, callback, output);
		#endif
						break;
					default:
						handleEnumeratedDevice(config, h, callback, output);
				}
			}
		}
	}
#endif
}

#if BREWPI_EXTERNAL_SENSOR //vito: enumerate device
void DeviceManager::enumerateExternalDevices(EnumerateHardware& h, EnumDevicesCallback callback, DeviceOutput& output){
	DeviceConfig config;
	clear((uint8_t*)&config, sizeof(config));
	config.deviceHardware = DEVICE_HARDWARE_EXTERNAL_SENSOR;
	config.chamber = 1; // chamber 1 is default
	config.hw.pinNr = -1;
	handleEnumeratedDevice(config, h, callback, output);
}
#endif

#if EnableDHTSensorSupport
void DeviceManager::enumerateEnvTempDevices(EnumerateHardware& h, EnumDevicesCallback callback, DeviceOutput& output){
	DeviceConfig config;
	EnvironmentSensor *sensors[]={
		HumidityControl::chamberSensor,
		HumidityControl::roomSensor
	};

	for(int i=0; i< 2;i++){
		if(sensors[i] != & nullEnvironmentSensor){
			clear((uint8_t*)&config, sizeof(config));
			config.deviceHardware = DEVICE_HARDWARE_ENVIRONMENT_TEMP;
			config.chamber = 1; // chamber 1 is default
			config.hw.pinNr =(uint8_t) i;
			handleEnumeratedDevice(config, h, callback, output);
		}
	}
}
#endif


#if EnableBME280Support
#define BME280_ADDRESS (0x77)           // Primary I2C Address
#define BME280_ADDRESS_ALTERNATE (0x76) // Alternate Address

void DeviceManager::enumerateBME280(EnumerateHardware& h, EnumDevicesCallback callback, DeviceOutput& output){
	// scan I2C
	Wire.begin(PIN_SDA,PIN_SCL);
	const uint8_t addresses[]={BME280_ADDRESS,BME280_ADDRESS_ALTERNATE};

	DeviceConfig config;
	clear((uint8_t*)&config, sizeof(config));
	config.deviceHardware = DEVICE_HARDWARE_BME280;
	config.chamber = 1; // chamber 1 is default

	uint8_t error,address;
 	for(uint32_t ai = 0; ai < sizeof(addresses); ai++ ){
		address = addresses[ai];
		
		Wire.beginTransmission(address);
    	error = Wire.endTransmission();
		
//		Serial.printf("search %x result:%d\n",address,error);

		if(error ==0){
			// found
//			Serial.printf("search %x found\n",address);
			config.hw.pinNr= address;
			handleEnumeratedDevice(config, h, callback, output);
		}
	}
}
#endif

void DeviceManager::enumerateHardware()
{
	EnumerateHardware spec;
	// set up defaults
	spec.unused = 0;			// list all devices
	spec.values = 0;			// don't list values
	spec.pin = -1;				// any pin
	spec.hardware = -1;			// any hardware
	spec.function = 0;			// no function restriction

	piLink.parseJson(handleHardwareSpec, &spec);
	DeviceOutput out;


//	logDebug("Enumerating Hardware");
	firstDeviceOutput = true;
	if (spec.hardware==-1 || isOneWire(DeviceHardware(spec.hardware))) {
		enumerateOneWireDevices(spec, OutputEnumeratedDevices, out);
	}
	if (spec.hardware==-1 || isDigitalPin(DeviceHardware(spec.hardware))) {
		enumeratePinDevices(spec, OutputEnumeratedDevices, out);
	}
	#if BREWPI_EXTERNAL_SENSOR //vito: enumerate device
	if (spec.hardware==-1 || isExternalSensor(DeviceHardware(spec.hardware))) {
		enumerateExternalDevices(spec, OutputEnumeratedDevices, out);
	}
	#endif

	#if EnableDHTSensorSupport //vito: enumerate device
	if (isEnvironmentSensorAvailable() &&( spec.hardware==-1 || isEnvTempSensor(DeviceHardware(spec.hardware))) ) {
		enumerateEnvTempDevices(spec, OutputEnumeratedDevices, out);
	}
	#endif

	#if EnableBME280Support
	if (spec.hardware==-1 || isBME280(DeviceHardware(spec.hardware))) {
		enumerateBME280(spec, OutputEnumeratedDevices, out);
	}

	#endif
//	logDebug("Enumerating Hardware Complete");
}


void HandleDeviceDisplay(const char* key, const char* val, void* pv)
{
	DeviceDisplay& dd = *(DeviceDisplay*)pv;
	//logDeveloper("DeviceDisplay %s:%s"), key, val);

	int8_t idx = indexOf("irwe", key[0]);
	if (idx>=0) {
		*((int8_t*)&dd+idx) = atol(val);
	}
}

void UpdateDeviceState(DeviceDisplay& dd, DeviceConfig& dc, char* val)
{
	DeviceType dt = deviceType(dc.deviceFunction);
	if (dt==DEVICETYPE_NONE)
		return;

	void** ppv = deviceTarget(dc);
	if (ppv==NULL)
		return;

	if (dd.write>=0 && dt==DEVICETYPE_SWITCH_ACTUATOR) {
		// write value to a specific device. For now, only actuators are relevant targets
		DEBUG_ONLY(logInfoInt(INFO_SETTING_ACTIVATOR_STATE, dd.write!=0));
		((Actuator*)*ppv)->setActive(dd.write!=0);
	}
	else if (dd.value==1) {		// read values
		if (dt==DEVICETYPE_SWITCH_SENSOR) {
			sprintf_P(val, STR_FMT_U, (unsigned int) ((SwitchSensor*)*ppv)->sense()!=0); // cheaper than itoa, because it overlaps with vsnprintf
		}
		else if (dt==DEVICETYPE_TEMP_SENSOR) {
			BasicTempSensor& s = unwrapSensor(dc.deviceFunction, *ppv);
			temperature temp = s.read();
			tempToString(val, temp, 3, 9);
		}
#if EnableDHTSensorSupport	
		else if (dt == DEVICETYPE_ENVIRONMENT_SENSOR){
			EnvironmentSensor* sensor = (EnvironmentSensor*) *ppv;
			sprintf_P(val, STR_FMT_D,(int)sensor->humidity());
		}
#endif	
		else if (dt==DEVICETYPE_SWITCH_ACTUATOR) {
			sprintf_P(val, STR_FMT_U, (unsigned int) ((Actuator*)*ppv)->isActive()!=0);
		}
	}
}

void DeviceManager::listDevices() {
	DeviceConfig dc;
	DeviceDisplay dd;
	fill((int8_t*)&dd, sizeof(dd));
	dd.empty = 0;
	piLink.parseJson(HandleDeviceDisplay, (void*)&dd);
	if (dd.id==-2) {
		if (dd.write>=0)
			tempControl.cameraLight.setActive(dd.write!=0);
		return;
	}
	deviceManager.beginDeviceOutput();
	for (device_slot_t idx=0; deviceManager.allDevices(dc, idx); idx++) {
		if (deviceManager.enumDevice(dd, dc, idx))
		{
			char val[10];
			val[0] = 0;
			UpdateDeviceState(dd, dc, val);
			deviceManager.printDevice(idx, dc, val);
		}
	}
}

/**
 * Determines the class of device for the given DeviceID.
 */

DeviceType deviceType(DeviceFunction id) {
	switch (id) {
#if EnableDHTSensorSupport	
	case DEVICE_CHAMBER_HUMIDITY_SENSOR:
	case DEVICE_CHAMBER_ROOM_HUMIDITY_SENSOR:
		return DEVICETYPE_ENVIRONMENT_SENSOR;
#endif
	case DEVICE_CHAMBER_DOOR:
		return DEVICETYPE_SWITCH_SENSOR;

	case DEVICE_CHAMBER_HEAT:
	case DEVICE_CHAMBER_COOL:
	case DEVICE_CHAMBER_LIGHT:
	case DEVICE_CHAMBER_FAN:
	case DEVICE_BEER_HEAT:
	case DEVICE_BEER_COOL:
#if AUTO_CAP
	case DEVICE_BEER_CAPPER:
#endif
#if EanbleParasiteTempControl
	case DEVICE_PTC_COOL:
#endif
#if EnableDHTSensorSupport
	case DEVICE_CHAMBER_HUMIDIFIER:
	case DEVICE_CHAMBER_DEHUMIDIFIER:
#endif
		return DEVICETYPE_SWITCH_ACTUATOR;

	case DEVICE_CHAMBER_TEMP:
	case DEVICE_CHAMBER_ROOM_TEMP:
	case DEVICE_BEER_TEMP:
	case DEVICE_BEER_TEMP2:
		return DEVICETYPE_TEMP_SENSOR;

	default:
		return DEVICETYPE_NONE;
	}
}

DeviceManager deviceManager;
