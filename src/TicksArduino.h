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

#ifdef ARDUINO

#include "Brewpi.h"

/*
 * The Ticks class provides the time period since the device was powered up.
 */
class HardwareTicks {
public:
	ticks_millis_t millis() { return ::millis(); }
	ticks_micros_t micros() { return ::micros(); }
	ticks_seconds_t seconds();

	ticks_seconds_t timeSince(ticks_seconds_t timeStamp);
};


class HardwareDelay {
public:
	HardwareDelay() {}
	void seconds(uint16_t seconds);
	void millis(uint16_t millis);
	void microseconds(uint32_t micros) { ::delayMicroseconds(micros); }
};

#endif
