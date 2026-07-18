/*
 * Copyright 2012-2013 BrewPi/Elco Jacobs.
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

/*	Overview of debug messages and ID's
	A copy of this file is included with the python script, so it can parse it to extract the log strings.
	The python script receives the messages as a few IDs and values in a JSON string.
	It uses this file to expand that to the full message.
	Not storing the full strings, but only the ID on the Arduino saves a lot of PROGMEM space.
	At startup the python script extracts the version number from this file and compares it to its own local copy.
	It will give a warning when the two strings do not match.
*/

/* bump this version number when changing this file and copy the new version to the brewpi-script repository. */
#define BREWPI_LOG_MESSAGES_VERSION 1

#define MSG(errorID, errorString, ...) errorID

// Errors
enum errorMessages{
// OneWireTempSensor.cpp
	MSG(ERROR_SRAM_SENSOR, "Not enough SRAM for temp sensor %s", addressString),
	MSG(ERROR_SENSOR_NO_ADDRESS_ON_PIN, "Cannot find address for sensor on pin %d", pinNr),
	MSG(ERROR_OUT_OF_MEMORY_FOR_DEVICE, "*** OUT OF MEMORY for device f=%d", config.deviceFunction),

// DeviceManager.cpp
	MSG(ERROR_DEVICE_DEFINITION_UPDATE_SPEC_INVALID, "Device definition update specification is invalid"),
	MSG(ERROR_INVALID_CHAMBER, "Invalid chamber id %d", config.chamber),
	MSG(ERROR_INVALID_BEER, "Invalid beer id %d", config.beer),
	MSG(ERROR_INVALID_DEVICE_FUNCTION, "Invalid device function id %d", config.deviceFunction),
	MSG(ERROR_INVALID_DEVICE_CONFIG_OWNER, "Invalid config for device owner type %d beer=%d chamber=%d", owner, config.beer, config.chamber),
	MSG(ERROR_CANNOT_ASSIGN_TO_HARDWARE, "Cannot assign device type %d to hardware %d", dt, config.deviceHardware),
	MSG(ERROR_NOT_ONEWIRE_BUS, "Device is onewire but pin %d is not configured as a onewire bus", pinNr),

// PiLink.cpp
	MSG(ERROR_EXPECTED_BRACKET, "Expected { got %c", character),

}; // END enum errorMessages

enum warningMessages{
// PiLink.cpp
	MSG(WARNING_COULD_NOT_PROCESS_SETTING, "Could not process setting"),
	MSG(WARNING_INVALID_COMMAND, "Invalid command received by Arduino: %c", character),

// OneWireTempSensor.cpp
	MSG(WARNING_TEMP_SENSOR_DISCONNECTED, "Temperature sensor disconnected pin %d, address %s", pinNr, addressString),

// SettingsManager.cpp
	MSG(WARNING_START_IN_SAFE_MODE, "EEPROM Settings not available. Starting in safe mode.")
}; // END enum warningMessages

// Info messages
enum infoMessages{
// OneWireTempSensor.cpp
	MSG(INFO_TEMP_SENSOR_CONNECTED, "Temp sensor connected on pin %d, address %s", pinNr, addressString),
	MSG(INFO_TEMP_SENSOR_DISCONNECTED, "Temp sensor disconnected on pin %d, address %s", pinNr, addressString),
	MSG(INFO_TEMP_SENSOR_INITIALIZED, "Sensor initialized: pin %d %s %s", pinNr, addressString, temperature),

// DeviceManager.cpp
	MSG(INFO_UNINSTALL_TEMP_SENSOR, "uninstalling temperature sensor with function %d", config.deviceFunction),
	MSG(INFO_UNINSTALL_ACTUATOR, "uninstalling actuator with function %d", config.deviceFunction),
	MSG(INFO_UNINSTALL_SWITCH_SENSOR, "uninstalling switch sensor with function %d", config.deviceFunction),
	MSG(INFO_INSTALL_TEMP_SENSOR, "installing temperature sensor with function %d", config.deviceFunction),
	MSG(INFO_INSTALL_ACTUATOR, "installing actuator with function %d", config.deviceFunction),
	MSG(INFO_INSTALL_SWITCH_SENSOR, "installing switch sensor with function %d", config.deviceFunction),
	MSG(INFO_INSTALL_DEVICE, "Installing device f=%d", config.deviceFunction),
	MSG(INFO_MATCHING_DEVICE, "Matching device at slot %d", out.slot),
	MSG(INFO_SETTING_ACTIVATOR_STATE, "Setting activator state %d", state),

// PiLink.cpp
	MSG(INFO_RECEIVED_SETTING, "Received new setting: %s = %s", key, val),
	MSG(INFO_DEFAULT_CONSTANTS_LOADED, "Default constants loaded."),
	MSG(INFO_DEFAULT_SETTINGS_LOADED, "Default settings loaded."),
	MSG(INFO_EEPROM_INITIALIZED, "EEPROM initialized"),
	MSG(INFO_EEPROM_ZAPPED, "EEPROM zapped!"),

// Tempcontrol.cpp
	MSG(INFO_POSITIVE_PEAK, "Positive peak detected: %s, estimated: %s. Previous heat estimator: %s, New heat estimator: %s.", temperature, temperature, estimator, estimator),
	MSG(INFO_NEGATIVE_PEAK, "Negative peak detected: %s, estimated: %s. Previous cool estimator: %s, New cool estimator: %s.", temperature, temperature, estimator, estimator),
	MSG(INFO_POSITIVE_DRIFT, "No peak detected. Drifting up after heating, current temp: %s, estimated peak: %s. Previous heat estimator: %s, New heat estimator: %s..", temperature, temperature, estimator, estimator),
	MSG(INFO_NEGATIVE_DRIFT, "No peak detected. Drifting down after cooling, current temp: %s, estimated peak: %s. Previous cool estimator: %s, New cool estimator: %s..", temperature, temperature, estimator, estimator)
}; // END enum infoMessages
