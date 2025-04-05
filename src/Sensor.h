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

#include "Brewpi.h"

template<class T> class Sensor
{
	public:
	virtual T sense()=0;

	virtual ~Sensor() {}
};

template <class T>
class ValueSensor : public Sensor<T>
{
public:
	ValueSensor(T initial) : value(initial) {}

	virtual T sense() {
		return (T)0;
	}

	void setValue(T _value) {
		value = _value;
	}

private:
	T value;
};

typedef Sensor<bool> SwitchSensor;
