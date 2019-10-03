/*
 * Copyright 2013 BrewPi/Elco Jacobs.
 * Copyright 2013 Matthew McGowan
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

#include "EepromAccess.h"
#include "EepromStructs.h"


void fill(int8_t* p, uint8_t size);
void clear(uint8_t* p, uint8_t size);

class DeviceConfig;


// todo - the Eeprom manager should avoid too frequent saves to the eeprom since it supports 100,000 writes.
class EepromManager {
public:

	EepromManager();

	/**
	 * Write -1 to the entire eeprom, emulating the reset performed by avrdude.
	 */
	static void zapEeprom();

	/**
	 * Prepare the eeprom to accept device definitions. For RevA boards, the eeprom is populated with devices for
	 * beer/fridge temp sensor, and heating,cooling actuators and door switch.
	 */
	static void initializeEeprom();

	/**
	 * Determines if this eeprom has settings.
	 */
	static bool hasSettings();

	/**
	 * Applies the settings from the eeprom
	 */
	static bool applySettings();

	static void dumpEeprom(Print& stream, uint16_t offset);

	/**
	 * Save the chamber constants and beer settings to eeprom for the currently active chamber.
	 */
	static void storeTempConstantsAndSettings();

	/**
	 * Save just the beer temp settings.
	 */
	static void storeTempSettings();

	static bool fetchDevice(DeviceConfig& config, uint8_t deviceIndex);
	static bool storeDevice(const DeviceConfig& config, uint8_t deviceIndex);

	static uint8_t saveDefaultDevices();
};
#if 0
class EepromStream
{
	eptr_t pv;

	void writeByte(uint8_t value) {
		eepromAccess.writeByte(pv++, value);
	}
	// TODO - Clean this up
/*	void writeBlock(void* source, uint16_t size)
	{
		eepromAccess.writeBlock(pv, source, size);
		pv += size;
	}*/
	// Breaking this out into three functions so that we can have better control
	void writeControlSettings(ControlSettings& source, uint16_t size) {
		eepromAccess.writeControlSettings(pv, source, size);
	}

	void writeControlConstants(ControlConstants& source, uint16_t size) {
		eepromAccess.writeControlConstants(pv, source, size);
	}

	void writeDeviceDefinition(const DeviceConfig& source, uint16_t size) {
		eepromAccess.writeDeviceDefinition(pv, source, size);
	}
};
#endif
extern EepromManager eepromManager;
