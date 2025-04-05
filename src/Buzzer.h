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
#include "Actuator.h"

#if BREWPI_BUZZER
class Buzzer : public ValueActuator
{
	public:
	Buzzer(){};
	~Buzzer(){};

	void init(void);

	/**
	 * Performs a number of beeps synchronously.
	 * @param numBeeps The number of beeps to emit
	 * @param duration the duration of each beep
	 */
	void beep(uint8_t numBeeps, uint16_t duration);

	void setActive(bool active);

};

extern Buzzer buzzer;
#endif
