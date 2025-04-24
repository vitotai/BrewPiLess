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

#include "Brewpi.h"

// Values returned by 'process'
// No complete step yet.
#define DIR_NONE 0x0
// Clockwise step.
#define DIR_CW 0x10
// Anti-clockwise step.
#define DIR_CCW 0x20



class RotaryEncoder
{
	public:
	static void init(void);
	static void setRange(int16_t start, int16_t min, int16_t max);
	static void process(void);

	static bool changed(void); // returns one if the value changed since the last call of changed.
	static int16_t read(void);

	static int16_t readsteps(void){
		return steps;
	}
#if BREWPI_BUTTONS || ButtonViaPCF8574 || ESP32
	static bool pushed(void);
#else
	static bool pushed(void){
		return pushFlag;
	}
#endif
	static void resetPushed(void){
		pushFlag = false;
	}

	static void setPushed(void);

	private:

	static int16_t maximum;
	static int16_t minimum;
	static volatile int16_t steps;
	static volatile bool pushFlag;
};

extern RotaryEncoder rotaryEncoder;
