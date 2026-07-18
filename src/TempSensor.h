/*
 * Copyright 2012-2013 BrewPi/Elco Jacobs.
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
#include "FilterCascaded.h"
#include "TempSensorBasic.h"
#include <stdlib.h>

#define TEMP_SENSOR_DISCONNECTED INVALID_TEMP

#ifndef TEMP_SENSOR_CASCADED_FILTER
#define TEMP_SENSOR_CASCADED_FILTER 1
#endif

#if TEMP_SENSOR_CASCADED_FILTER
typedef CascadedFilter TempSensorFilter;
#else
typedef FixedFilter TempSensorFilter;
#endif


enum TempSensorType {
	TEMP_SENSOR_TYPE_FRIDGE=1,
	TEMP_SENSOR_TYPE_BEER
};


class TempSensor {
	public:
	TempSensor(TempSensorType sensorType, BasicTempSensor* sensor =NULL)  {
		updateCounter = 255; // first update for slope filter after (255-4s)
		setSensor(sensor);
		#if 1 // FridgeSensorFallBack
		_useBackupSensor=false;
		_backupSensor=NULL;
		#endif
	 }

	 void setSensor(BasicTempSensor* sensor) {
		 _sensor = sensor;
		 failedReadCount = -1;
	 }
	#if 1 //FridgeSensorFallBack
	void setBackupSensor(BasicTempSensor* sensor) {
		 _backupSensor = sensor;
	 }
	#endif
	bool hasSlowFilter() { return true; }
	bool hasFastFilter() { return true; }
	bool hasSlopeFilter() { return true; }

	void init();
	
	#if 1 // FridgeSensorFallBack
	bool isConnected(bool physical=false) {
			if(physical) return _sensor->isConnected(); 
			
			if(_useBackupSensor && _backupSensor)
				return _backupSensor->isConnected();
			else
				return _sensor->isConnected(); 
		}
	#else
	bool isConnected() { return _sensor->isConnected(); }
	#endif
	void update();

	temperature readFastFiltered(void);

	temperature readSlowFiltered(void){
		return slowFilter.readOutput(); //return most recent unfiltered value
	}

	temperature readSlope(void);

	temperature detectPosPeak(void);

	temperature detectNegPeak(void);

	void setFastFilterCoefficients(uint8_t b);

	void setSlowFilterCoefficients(uint8_t b);

	void setSlopeFilterCoefficients(uint8_t b);

	BasicTempSensor& sensor();

	private:
	BasicTempSensor* _sensor;
	TempSensorFilter fastFilter;
	TempSensorFilter slowFilter;
	TempSensorFilter slopeFilter;
	unsigned char updateCounter;
	temperature_precise prevOutputForSlope;
	#if 1 //FridgeSensorFallBack
	BasicTempSensor* _backupSensor;
	bool _useBackupSensor;
	#endif
	// An indication of how stale the data is in the filters. Each time a read fails, this value is incremented.
	// It's used to reset the filters after a large enough disconnect delay, and on the first init.
	int8_t failedReadCount;		// -1 for uninitialized, >=0 afterwards.

	friend class ChamberManager;
	friend class Chamber;
	friend class DeviceManager;
};
