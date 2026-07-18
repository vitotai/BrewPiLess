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

#ifdef ARDUINO

#include "Brewpi.h"
#include "Actuator.h"
#include "DS2413.h"
#include "PiLink.h"

/**
 * An actuator or sensor that operates by communicating with a DS2413 device.
 *
 */
class OneWireActuator : public Actuator
#if DS2413_SUPPORT_SENSE
	, SwitchSensor
#endif
{
public:

	OneWireActuator(OneWire* bus, DeviceAddress address, pio_t pio, bool invert=true) {
		init(bus, address, pio, invert);
	}

	void init(OneWire* bus, DeviceAddress address, pio_t pio, bool invert=true) {
		this->invert = invert;
		this->pio = pio;
		device.init(bus, address);
	}

	void setActive(bool active) {
		device.channelWrite(pio, active^invert);
	}

	bool isActive() {
		return device.channelRead(pio, false);
	}

#if DS2413_SUPPORT_SENSE
	bool sense() {
		device.channelWrite(pio, 0);
		return device.channelSense(pio, invert);	// on device failure, default is high for invert, low for regular.
	}
#endif

private:
	DS2413 device;
	pio_t pio;
	bool invert;
};

#endif
