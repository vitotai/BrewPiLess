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

#pragma once

#include "EepromTypes.h"

#ifdef ARDUINO
// ARDUINO is defined for the ESP8266 implementation as well. Specifically break out
// the ESP8266 implementation, then default to Arduino if we don't use it.

#if defined(ESP8266) || defined(ESP32)
#include "ESPEepromAccess.h"
typedef ESPEepromAccess EepromAccess;
#else
#include "ArduinoEepromAccess.h"
typedef ArduinoEepromAccess EepromAccess;
#endif

#else

#include "ArrayEepromAccess.h"

typedef ArrayEepromAccess EepromAccess;

#endif

extern EepromAccess eepromAccess;
