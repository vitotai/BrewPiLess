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
#include "FastDigitalPin.h"

#define ACTUATOR_VIRTUAL 1

#if ACTUATOR_VIRTUAL
	#define ACTUATOR_METHOD virtual
	#define ACTUATOR_METHOD_IMPL =0
	#define ACTUATOR_BASE_CLASS_DECL : public Actuator
#else
	#define ACTUATOR_METHOD inline
	#define ACTUATOR_METHOD_IMPL {}
	#define ACTUATOR_BASE_CLASS_DECL
#endif


/*
 * An actuator simply turns something on or off.
 */

class Actuator
{
	public:
	ACTUATOR_METHOD void setActive(bool active) ACTUATOR_METHOD_IMPL;
	ACTUATOR_METHOD bool isActive() ACTUATOR_METHOD_IMPL;
#if ACTUATOR_VIRTUAL
	virtual ~Actuator() {}
#endif

};

/*
 * An actuator that simply remembers the set value. This is primary used for testing.
 */
class ValueActuator ACTUATOR_BASE_CLASS_DECL
{
public:
	ValueActuator() : state(false) {}
	ValueActuator(bool initial) : state(initial) {}

	ACTUATOR_METHOD void setActive(bool active) { state = active; }
	ACTUATOR_METHOD bool isActive() { return state; }

private:
	bool state;
};
