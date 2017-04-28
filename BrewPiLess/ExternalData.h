#ifndef ExternalData_H
#define ExternalData_H
#include "BrewPiProxy.h"
#include "BrewKeeper.h"
#include "BrewLogger.h"


#define INVALID_VOLTAGE -1
#define INVALID_GRAVITY -1
#define INVALID_TEMP  -250

#define IsVoltageValid(v) ((v) > 0)
#define IsGravityValid(g) ((g) > 0)

extern BrewKeeper brewKeeper;
extern BrewLogger brewLogger;
extern BrewPiProxy brewPi;

#define ErrorNone 0
#define ErrorAuthenticateNeeded 1
#define ErrorJSONFormat 2
#define ErrorMissingField 3
#define ErrorUnknownSource 4

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
	
		if(!IsGravityValid(_gravity)) filter.setInitial(sg);
		
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

	float temperatureCorrection(float sg, float t, float c){
	    
	    uint32_t time1=millis();
	    float nsg= sg*((1.00130346-0.000134722124*t+0.00000204052596*t*t -0.00000000232820948*t*t*t)/
	        (1.00130346-0.000134722124*c+0.00000204052596*c*c-0.00000000232820948*c*c*c));
	    uint32_t time2=millis();
	    
	    DBG_PRINTF("cal time:%ld\n", time2 - time1);
	    
	    return nsg;
	}
	

	bool processJSON(char data[],size_t length, bool authenticated, uint8_t& error)
	{
		const int BUFFER_SIZE = JSON_OBJECT_SIZE(8);
		StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject((char*)data,length);
        
		
		if (!root.success() || !root.containsKey("name")){
  			DBG_PRINTF("Invalid JSON\n");
  			error = ErrorJSONFormat;
  			return false;
		}
		
		String name= root["name"];
		
		if(name.equals("webjs")){
			if(! authenticated){
			    error = ErrorAuthenticateNeeded;
    	        return false;
    	    }
	        
			if(!root.containsKey("gravity")){
  				DBG_PRINTF("No gravity\n");
  				error = ErrorMissingField;
  				return false;
  			}
			float  gravity = root["gravity"];
			if(root.containsKey("og")){
				setOriginalGravity(gravity);
			}
			else{
				setGravity(gravity,TimeKeeper.getTimeSeconds());
			}
		}else if(name.startsWith("iSpindel")){
			//{"name": "iSpindel01", "id": "XXXXX-XXXXXX", "temperature": 20.5, "angle": 89.5, "gravityP": 13.6, "battery": 3.87}
			DBG_PRINTF("iSpindel01\n");
			
			setPlato(root["gravityP"],TimeKeeper.getTimeSeconds());
			setDeviceVoltage(root["battery"]);
			setAuxTemperatureCelsius(root["temperature"]);
			
		}else{
		    error = ErrorUnknownSource;
		    return false;
		}
		return true;
	}
};

extern ExternalData externalData;

#endif









































































































































