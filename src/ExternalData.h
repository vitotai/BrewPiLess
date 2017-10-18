#ifndef ExternalData_H
#define ExternalData_H
#include "BrewPiProxy.h"
#include "BrewKeeper.h"
#include "BrewLogger.h"
#include "mystrlib.h"

#if BREWPI_EXTERNAL_SENSOR
#include "TempSensorWireless.h"
#endif

#define INVALID_VOLTAGE -1
#define INVALID_GRAVITY -1
#ifdef INVALID_TEMP
#undef INVALID_TEMP
#endif
#define INVALID_TEMP  -250

#define IsVoltageValid(v) ((v) > 0)
//#define IsGravityValid(g) ((g) > 0)

#define IsGravityInValidRange(g) ((g) > 0.8 && (g) < 1.25)

extern BrewKeeper brewKeeper;
extern BrewLogger brewLogger;
extern BrewPiProxy brewPi;

#define ErrorNone 0
#define ErrorAuthenticateNeeded 1
#define ErrorJSONFormat 2
#define ErrorMissingField 3
#define ErrorUnknownSource 4

#define C2F(t) ((t)*1.8+32)

class SimpleFilter
{
	float _y;
	float _b;
public:
	SimpleFilter(){ _b = 0.1;}
	void setInitial(float v){ _y=v;}
	void setBeta(float b) { _b = b; }
	float beta(void){ return _b; }

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

    bool _ispindelEnable;
    float _ispindelCalibration;
    bool _ispindelTempCal;
    float _ispindelCalibrationBaseTemp;
    float _ispindelCoefficients[4];
    char *_ispindelName;
	float _ispindelTilt;

    bool _calculateGravity;
	uint8_t _stableThreshold;
	
	#if BREW_AND_CALIBRATION	
	float _tiltInWater;
	bool  _calibrating;
	#endif

public:
	ExternalData(void):_gravity(INVALID_GRAVITY),_auxTemp(INVALID_TEMP),_deviceVoltage(INVALID_VOLTAGE),_lastUpdate(0)
	,_ispindelEnable(false),_ispindelName(NULL),_calculateGravity(false),_stableThreshold(1)
	#if BREW_AND_CALIBRATION	
	 ,_calibrating(false)
	#endif	
	{}

    bool iSpindelEnabled(void){return _ispindelEnable;}

	#if BREW_AND_CALIBRATION	
	bool isCalibrating(void){return _calibrating;}
	float titltInWater(void){ return _tiltInWater;}
	#endif

    void sseNotify(char *buf){


		char strbattery[8];
		int len=sprintFloat(strbattery,_deviceVoltage,2);
		strbattery[len]='\0';

		char strgravity[8];
		len=sprintFloat(strgravity,_gravity,3);
		strgravity[len]='\0';

		char slowpassfilter[8];
		len=sprintFloat(slowpassfilter,filter.beta(),2);
		slowpassfilter[len]='\0';

		char strtilt[8];
		len=sprintFloat(strtilt,_ispindelTilt,2);
		strtilt[len]='\0';


		const char *spname=(_ispindelName)? _ispindelName:"Unknown";
		sprintf(buf,"G:{\"name\":\"%s\",\"battery\":%s,\"sg\":%s,\"angle\":%s,\"lu\":%ld,\"lpf\":%s,\"stpt\":%d}",
					spname, 
					strbattery,
					strgravity,
					strtilt,
					_lastUpdate,slowpassfilter,_stableThreshold);
    }
    void config(char* configdata)
    {
    	const int BUFFER_SIZE = JSON_OBJECT_SIZE(12);
		StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(configdata);

		if (!root.success()
		    || !root.containsKey("ispindel")
		    || !root.containsKey("gc")
		    || !root.containsKey("tc")
		    || !root.containsKey("a3")
		    || !root.containsKey("a2")
		    || !root.containsKey("a1")
		    || !root.containsKey("a0")){
  			DBG_PRINTF("Invalid JSON config\n");
  			return;
		}

		#if BREW_AND_CALIBRATION	
		if(root.containsKey("cal") 
			&& root.containsKey("tiltw")){
			_calibrating = root["cal"];
			_tiltInWater = root["tiltw"];
		}
		#endif

		_ispindelEnable=root["ispindel"];
		_ispindelCalibration =root["gc"];
		_ispindelCalibration = _ispindelCalibration / 1000.0;
		_ispindelTempCal = root["tc"];
		if(_ispindelTempCal){
		    _ispindelCalibrationBaseTemp=(root.containsKey("ctemp"))? root["ctemp"]:20;
		}
		_calculateGravity =(root.containsKey("cbpl"))? root["cbpl"]:false;

		_ispindelCoefficients[0]=root["a0"];
		_ispindelCoefficients[1]=root["a1"];
		_ispindelCoefficients[2]=root["a2"];
		_ispindelCoefficients[3]=root["a3"];

        if(root.containsKey("lpc")) filter.setBeta(root["lpc"]);
        if(root.containsKey("stpt")){
            _stableThreshold=root["stpt"];
             brewKeeper.setStableThreshold(_stableThreshold);
        }
		// debug
		#if 0
		Serial.print("Coefficient:");
		for(int i=0;i<4;i++){
		    Serial.print(_ispindelCoefficients[i],10);
		    Serial.print(", ");
		}
		Serial.println("");
		#endif
    }

	void setOriginalGravity(float og){
		_og = og;
		brewLogger.addGravity(og,true);
#if EnableGravitySchedule
		brewKeeper.updateOriginalGravity(og);
#endif

	}
/*
	void setPlato(float plato, time_t now){
		float sg=1 + (plato / (258.6 -((plato/258.2)*227.1)));
		setGravity(sg,now);
	}*/

	void setTilt(float tilt,float temp,time_t now){
		_lastUpdate=now;
		_ispindelTilt=tilt;

		#if BREW_AND_CALIBRATION	
		if(brewLogger.calibrating()){
			brewLogger.addTiltAngle(tilt);
			return;
		}
		#endif

		// calculate plato
	    float sg = _ispindelCoefficients[0]
                    +  _ispindelCoefficients[1] * tilt
                    +  _ispindelCoefficients[2] * tilt * tilt
                    +  _ispindelCoefficients[3] * tilt * tilt * tilt;
      //  Serial.print("Plato:");
	   // Serial.print(plato,2);
	    // convert to SG
	    //float sg=1 + (plato / (258.6 -((plato/258.2)*227.1)));

	    //Serial.print(" SG:");
	    //Serial.print(sg,3);

	    // temp. correction
	    if(_ispindelTempCal){
	        sg = temperatureCorrection(sg,C2F(temp),C2F(_ispindelCalibrationBaseTemp));
        	//Serial.print(" TC:");
    	    //Serial.print(sg,3);
	    }

	    // add correction
	    sg += _ispindelCalibration;
        	//Serial.print(" Corrected:");
	        //Serial.println(sg,3);

	    // update
	    setGravity(sg,now);
	}

	void setGravity(float sg, time_t now){
        // copy these two for reporting to web interface
        float old_sg=_gravity;
		_gravity = sg;
		_lastUpdate=now;
        // verfiy sg, even invalid value will be reported to web interface
	    if(!IsGravityInValidRange(sg)) return;

		if(!IsGravityValid(old_sg)) filter.setInitial(sg);
#if EnableGravitySchedule
        float filtered_data=filter.addData(sg);
		// use filter data as input to tracker and beer profile.
		brewKeeper.updateGravity(filtered_data);
		gravityTracker.add(filtered_data,now);
#endif
		brewLogger.addGravity(sg,false);
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

		#if BREWPI_EXTERNAL_SENSOR
		if(WirelessTempSensor::theWirelessTempSensor){
			WirelessTempSensor::theWirelessTempSensor->setTemp(_auxTemp);
		}
		#endif
	}


	float auxTemp(void){return _auxTemp; }
	void invalidateAuxTemp(void){ _auxTemp=INVALID_TEMP;}

//	void setUpdateTime(time_t update){ _lastUpdate=update;}
	time_t lastUpdate(void){return _lastUpdate;}
	void setDeviceVoltage(float vol){ _deviceVoltage = vol; }
	float deviceVoltage(void){return _deviceVoltage;}
	float tiltValue(void){return _ispindelTilt;}
	void invalidateDeviceVoltage(void) { _deviceVoltage= INVALID_VOLTAGE; }

	float temperatureCorrection(float sg, float t, float c){

	    float nsg= sg*((1.00130346-0.000134722124*t+0.00000204052596*t*t -0.00000000232820948*t*t*t)/
	        (1.00130346-0.000134722124*c+0.00000204052596*c*c-0.00000000232820948*c*c*c));
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

			if(!IsGravityInValidRange(gravity)) return true;

			if(root.containsKey("og")){
				setOriginalGravity(gravity);
			}
			else{
				setGravity(gravity,TimeKeeper.getTimeSeconds());
			}
		}else if(name.startsWith("iSpindel")){
			//{"name": "iSpindel01", "id": "XXXXX-XXXXXX", "temperature": 20.5, "angle": 89.5, "gravityP": 13.6, "battery": 3.87}
			DBG_PRINTF("%s\n",name.c_str());

			if(!_ispindelName){
			    _ispindelName=(char*) malloc(name.length()+1);
			    if(_ispindelName) strcpy(_ispindelName,name.c_str());
			}
			if(! root.containsKey("temperature")){
			    DBG_PRINTF("iSpindel report no temperature!\n");
			    return false;
			}
			float itemp=root["temperature"];
			setAuxTemperatureCelsius(itemp);

			//Serial.print("temperature:");
			//Serial.println(itemp);

            if(root.containsKey("battery"))
    		    setDeviceVoltage(root["battery"]);



				//setPlato(root["gravityP"],TimeKeeper.getTimeSeconds());
			if(
				#if BREW_AND_CALIBRATION	
				! _calibrating &&
				#endif
				!_calculateGravity && root.containsKey("gravity"))
            	setGravity(root["gravity"], TimeKeeper.getTimeSeconds());
			else{
		    	if(! root.containsKey("angle")){
        		    DBG_PRINTF("iSpindel report no angle!\n");
			        return false;
			    }
    			setTilt(root["angle"],itemp,TimeKeeper.getTimeSeconds());
            }
		}else{
		    error = ErrorUnknownSource;
		    return false;
		}
		return true;
	}
};

extern ExternalData externalData;

#endif
