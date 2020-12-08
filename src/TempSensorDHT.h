#ifndef DHTTempSensor_H
#define DHTTempSensor_H

#include "Brewpi.h"
#include "TempSensor.h"
#include "OneWireTempSensor.h"
#include "HumidityControl.h"

class DHTTempSensor: public BasicTempSensor
{
public:
	DHTTempSensor(fixed4_4 cal=0){
	
		const uint8_t shift = TEMP_FIXED_POINT_BITS-ONEWIRE_TEMP_SENSOR_PRECISION; // difference in precision between DS18B20 format and temperature adt
		//temperature i fixed7_9, calibration fixed4_4
		_calibrationOffset =constrainTemp16(temperature(cal)<<shift);
	}
	~DHTTempSensor(){
    }
     
	bool isConnected() { return HumidityControl::dhtSensor != NULL; }

	bool init() {
		return read()!=TEMP_SENSOR_DISCONNECTED;
	}

	temperature read() {
		if (!isConnected()) return TEMP_SENSOR_DISCONNECTED;
		float t=HumidityControl::dhtSensor->temperature(false);
		if(isnan(t) || t == NAN || t > 120.0 || t<-30.0) {
//			DBG_PRINTF("**\nDHTxx temp read fail!\n**\n");
			return TEMP_SENSOR_DISCONNECTED;
		}
//			DBG_PRINTF("DHTxx temp:");
//			Serial.print(t);
//			DBG_PRINTF("\n");
		//debugging
		temperature temp =  _calibrationOffset + doubleToTemp((double)t);
		return temp;
	}

	private:
	temperature _calibrationOffset;
};

#endif