/*
 * Copyright 2013 BrewPi/Elco Jacobs.
 * Copyright 2013 Matthew McGowan.
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


#if defined(ESP8266)

#error Incorrect processor type!

#else

#include <avr/eeprom.h>
#include "EepromStructs.h"

//TODO - Update to match ESPEepromAccess.h

class ArduinoEepromAccess
{
public:
	static uint8_t readByte(eptr_t offset) {
		return eeprom_read_byte((uint8_t*)offset);
	}
	static void writeByte(eptr_t offset, uint8_t value) {
		eeprom_update_byte((uint8_t*)offset, value);
	}

	static void readBlock(void* target, eptr_t offset, uint16_t size) {
		eeprom_read_block(target, (uint8_t*)offset, size);
	}
	static void writeBlock(eptr_t target, const void* source, uint16_t size) {
		eeprom_update_block(source, (void*)target, size);
	}
};
#endif
