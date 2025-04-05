/*
 * Copyright 2012-2013 BrewPi/Elco Jacobs.
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
#include "TempSensor.h"
#include "FastDigitalPin.h"
#include "DallasTemperature.h"
#include "Ticks.h"

class DallasTemperature;
class OneWire;

#define ONEWIRE_TEMP_SENSOR_PRECISION (4)

class OneWireTempSensor : public BasicTempSensor {
public:
	/**
	 * Constructs a new onewire temp sensor.
	 * /param bus	The onewire bus this sensor is on.
	 * /param address	The onewire address for this sensor. If all bytes are 0 in the address, the first temp sensor
	 *    on the bus is used.
	 * /param calibration	A temperature value that is added to all readings. This can be used to calibrate the sensor.
	 */
	OneWireTempSensor(OneWire* bus, DeviceAddress address, fixed4_4 calibrationOffset)
	: oneWire(bus), sensor(NULL) {
		connected = true;  // assume connected. Transition from connected to disconnected prints a message.
		memcpy(sensorAddress, address, sizeof(DeviceAddress));
		this->calibrationOffset = calibrationOffset;
	};

	~OneWireTempSensor();

	bool isConnected(void){
		return connected;
	}

	bool init();
	temperature read();

	private:

	void setConnected(bool connected);
	bool requestConversion();
	void waitForConversion()
	{
		wait.millis(750);
	}


	/**
	 * Reads the temperature. If successful, constrains the temp to the range of the temperature type and
	 * updates lastRequestTime. On successful, leaves lastRequestTime alone and returns DEVICE_DISCONNECTED.
	 */
	temperature readAndConstrainTemp();

	OneWire * oneWire;
	DallasTemperature * sensor;
	DeviceAddress sensorAddress;

	fixed4_4 calibrationOffset;
	bool connected;

};
