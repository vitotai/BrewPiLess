#ifndef ExternalData_H
#define ExternalData_H
#include "BrewKeeper.h"
#include "BrewLogger.h"
#include "BrewPiProxy.h"

#define INVALID_VOLTAGE -1
#define INVALID_GRAVITY -1
#define INVALID_TEMP  -250

#define IsVoltageValid(v) ((v) > 0)
#define IsGravityValid(g) ((g) > 0)

extern BrewKeeper brewKeeper;
extern BrewLogger brewLogger;
extern BrewPiProxy brewPi;

class SimpleFilter
{
	float _y;
	float _b;
public:
	SimpleFilter(){ _b = 0.1;}
	void setInitial(float v){ _y=v;}
	void setBeta(float b) { _b = b; }

	float addData(float x){
		_y = _y + _b * (x - _y);
		return _y;
	}
};

class ExternalData
{
protected:
	float _gravity;
	float _auxTemp;
	time_t _lastUpdate;
	float  _deviceVoltage;
	float _og;
	SimpleFilter filter;
public:
	ExternalData(void):_gravity(INVALID_GRAVITY),_auxTemp(INVALID_TEMP),_deviceVoltage(INVALID_VOLTAGE),_lastUpdate(0){}

	void setOriginalGravity(float og){
		_og = og;
		brewLogger.addGravity(og,true);
	}
	
	void setPlato(float plato, time_t now){
		float sg=1 + (plato / (258.6 -((plato/258.2)*227.1)));
		setGravity(sg,now);
	}
	
	void setGravity(float sg, time_t now){
	
		if(!IsVoltageValid(_gravity)) filter.setInitial(sg);
		
		_gravity = sg; 
		_lastUpdate=now;
#if EnableGravitySchedule		
		brewKeeper.updateGravity(filter.addData(sg));
#endif
		brewLogger.addGravity(_gravity,false);
	}

	float gravity(void){ return _gravity;}
	void invalidateGravity(void){  _gravity = INVALID_GRAVITY;}

	
	void setAuxTemperatureCelsius(float temp){
		char unit;
		float max,min;
		brewPi.getTemperatureSetting(&unit,&min,&max);
		if(unit == 'C'){
			_auxTemp= temp;
		}else{
			_auxTemp= temp * 1.8 +32 ;
		}
		brewLogger.addAuxTemp(_auxTemp);
	}
	
	
	float auxTemp(void){return _auxTemp; }
	void invalidateAuxTemp(void){ _auxTemp=INVALID_TEMP;}

//	void setUpdateTime(time_t update){ _lastUpdate=update;}
	time_t lastUpdate(void){return _lastUpdate;}
	void setDeviceVoltage(float vol){ _deviceVoltage = vol; }
	float deviceVoltage(void){return _deviceVoltage;}
	void invalidateDeviceVoltage(void) { _deviceVoltage= INVALID_VOLTAGE; }
};

extern ExternalData externalData;

#endif






















































































