#ifndef EnvTempSensor_H
#define EnvTempSensor_H

#include "Brewpi.h"
#include "TempSensor.h"
#include "OneWireTempSensor.h"
#include "EnvironmentSensor.h"
#include "HumidityControl.h"

class EnvTempSensor: public BasicTempSensor
{
public:
	EnvTempSensor(EnvironmentSensor *sensor,fixed4_4 cal=0){
	
		const uint8_t shift = TEMP_FIXED_POINT_BITS-ONEWIRE_TEMP_SENSOR_PRECISION; // difference in precision between DS18B20 format and temperature adt
		//temperature i fixed7_9, calibration fixed4_4
		_calibrationOffset =constrainTemp16(temperature(cal)<<shift);
		_sensor = sensor;
	}
	~EnvTempSensor(){
    }
     
	bool isConnected() { return _sensor ->isConnected(); }

	bool init() {
		return read()!=TEMP_SENSOR_DISCONNECTED;
	}

	temperature read() {
		if (!isConnected()){
//			DBG_PRINTF("**\nDHTxx temp read disconnect!\n**\n");
			 return TEMP_SENSOR_DISCONNECTED;
		}
		float t=_sensor->readTemperature();
		if(isnan(t) || t == NAN || t > 120.0 || t<-30.0) {
//			DBG_PRINTF("**\nDHTxx temp read fail!\n**\n");
			return TEMP_SENSOR_DISCONNECTED;
		}
//		DBG_PRINTF("**\nDHTxx temp:");
//		DBG_PRINT(t);
//			DBG_PRINTF("DHTxx temp:");
//			Serial.print(t);
//			DBG_PRINTF("\n");
		//debugging
		temperature temp =  _calibrationOffset + doubleToTemp((double)t);
		return temp;
	}

	private:
	temperature _calibrationOffset;
	EnvironmentSensor *_sensor;
};

#endif