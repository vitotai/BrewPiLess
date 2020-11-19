#include <ArduinoJson.h>
#include "ExternalData.h"
#if SupportTiltHydrometer
#include "TiltListener.h"
#endif

ExternalData externalData;

#define TiltFilterParameter 0.15

float Brix2SG(float brix){
 return (brix / (258.6-((brix / 258.2)*227.1))) + 1;
}

float SG2Brix(float SG){
 return (((182.4601 * SG -775.6821) * SG +1262.7794) * SG -669.5622);
}

float ExternalData::plato(bool filtered){ 
	float sg= filtered? _filteredGravity:_gravity;
	return _cfg->usePlato? sg:SG2Brix(sg);
}
float ExternalData::gravity(bool filtered){ 
	float sg= filtered? _filteredGravity:_gravity;
	return _cfg->usePlato? Brix2SG(sg):sg;
}

void ExternalData::waitFormula(){
    _cfg->numberCalPoints =0;
}

bool ExternalData::iSpindelEnabled(void){
    return _cfg->gravityDeviceType == GravityDeviceIspindel;
}

bool ExternalData::gravityDeviceEnabled(void){
    return _cfg->gravityDeviceType != GravityDeviceNone;
}


float ExternalData::hydrometerCalibration(void){ 
    return _cfg->ispindelCalibrationBaseTemp;
}

void ExternalData::sseNotify(char *buf){

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

/*		char coeff[4][20];
		for(int i=0;i<4;i++){
			len=sprintFloat(coeff[i],_cfg->ispindelCoefficients[i],9);
			coeff[i][len]='\0';	
		}
		*/
		char strRssi[32];
		if(_rssiValid){
			len=sprintf(strRssi,",\"rssi\":%d",_rssi);
			strRssi[len]='\0';
		}else{
			strRssi[1]=' ';
			strRssi[0]='\0';
		} 
		
		#if SupportTiltHydrometer
		char strRawTilt[8];
		len=sprintFloat(strRawTilt,_tiltRawGravity,3);
		strRawTilt[len]='\0';
		#else
		char *strRawTilt ="0";
		#endif

		const char *spname=(_ispindelName)? _ispindelName:"Unknown";
		sprintf(buf,"G:{\"dev\":%d,\"name\":\"%s\",\"battery\":%s,\"sg\":%s,\"angle\":%s %s,\"lu\":%ld,\"lpf\":%s,\"stpt\":%d,\"fpt\":%d,\"ctemp\":%d,\"plato\":%d,\"tiltraw\":%s}",
					_cfg->gravityDeviceType,
					spname, 
					strbattery,
					strgravity,
					strtilt,
					strRssi,
					_lastUpdate,
					slowpassfilter,
					_cfg->stableThreshold,
					_cfg->numberCalPoints,
                    _cfg->ispindelCalibrationBaseTemp,
					_cfg->usePlato,
					strRawTilt);
}

#if SupportTiltHydrometer
void ExternalData::setTiltInfo(uint16_t gravity, uint16_t temperature, int rssi){
	_tiltRawGravity = gravity;
	float sg =(float) gravity /1000.0;
	float csg = _tcfg->coefficients[0] 
			 +  _tcfg->coefficients[1] * sg 
			 +  _tcfg->coefficients[2] * sg * sg
			 +  _tcfg->coefficients[3] * sg * sg * sg; 


	setAuxTemperatureCelsius( ((float)temperature  -32.0)* 5.0/9.0);
	setDeviceRssi(rssi);
	setGravity(csg, TimeKeeper.getTimeSeconds());
}
#endif

void ExternalData::reconfig(void){

	#if SupportTiltHydrometer
	if(_cfg->gravityDeviceType != GravityDeviceTilt) tiltListener.stopListen();
	#endif

	if(_cfg->gravityDeviceType == GravityDeviceIspindel){	    
		filter.setBeta(_cfg->lpfBeta);		
	}
	#if SupportTiltHydrometer
	else if(_cfg->gravityDeviceType == GravityDeviceTilt){
	    filter.setBeta(TiltFilterParameter);
		tiltListener.listen((TiltColor) _tcfg->tiltColor,[&](TiltHydrometerInfo& info){
			setTiltInfo(info.gravity,info.temperature,info.rssi);
		});
	}
	#endif
}

void ExternalData::loadConfig(void){
    _cfg = theSettings.GravityConfig();
#if SupportTiltHydrometer	
	_tcfg = theSettings.tiltConfiguration();
#endif
	reconfig();
}


bool ExternalData::processconfig(char* configdata){
   bool ret= theSettings.dejsonGravityConfig(configdata);
   if(ret){
	   #if !SupportTiltHydrometer
	   if(_cfg->gravityDeviceType == GravityDeviceTilt){
		   return false;
	   }
	   #endif

	   theSettings.save();
	   reconfig();
   }
   return ret;
}

void ExternalData::formula(float coeff[4],uint32_t npt){
    if(_cfg->numberCalPoints == npt){ 
		DBG_PRINTF("formula nochanged\n");
		return;
	}
	_cfg->numberCalPoints =npt;

	for(int i=0;i<4;i++){
		_cfg->ispindelCoefficients[i] = coeff[i];
	}
	 theSettings.save();
}

void ExternalData::setOriginalGravity(float og){
//		_og = og;
		brewLogger.addGravity(og,true);
#if EnableGravitySchedule
		brewKeeper.updateOriginalGravity(og);
#endif
}

void ExternalData::setIspindelAngle(float tilt,float temp,time_t now){
	_lastUpdate=now;
	_ispindelTilt=tilt;

	// add tilt anyway
	brewLogger.addTiltAngle(tilt);

	if(_calibrating && _cfg->numberCalPoints ==0){
		DBG_PRINTF("No valid formula!\n");
		return; // don't calculate if formula is not available.
	}
		// calculate plato
	float sg = _cfg->ispindelCoefficients[0]
            +  _cfg->ispindelCoefficients[1] * tilt
            +  _cfg->ispindelCoefficients[2] * tilt * tilt
            +  _cfg->ispindelCoefficients[3] * tilt * tilt * tilt;

	// temp. correction
	if(_cfg->ispindelTempCal){
		if(_cfg->usePlato){
			sg =SG2Brix(temperatureCorrection(Brix2SG(sg),C2F(temp),C2F((float)_cfg->ispindelCalibrationBaseTemp)));
		}else
	    	sg = temperatureCorrection(sg,C2F(temp),C2F((float)_cfg->ispindelCalibrationBaseTemp));
	}
	// update, gravity data calculated
	setGravity(sg,now,!_calibrating); // save only when not calibrating.
}

void ExternalData::setGravity(float sg, time_t now,bool log){
        // copy these two for reporting to web interface
    float old_sg=_gravity;
	
    DBG_PRINTF("setGravity:%d, saved:%d\n",(int)(sg*10000.0),log);
    // verfiy sg, even invalid value will be reported to web interface
	//if(!IsGravityInValidRange(sg)) return;
	_gravity = sg;
	_lastUpdate=now;

	if(!IsGravityValid(old_sg)) filter.setInitial(sg);
#if EnableGravitySchedule
    float _filteredGravity=filter.addData(sg);
		// use filter data as input to tracker and beer profile.
	brewKeeper.updateGravity(_filteredGravity);
	if(_cfg->usePlato)
		gravityTracker.add(Plato2TrackingGravity(_filteredGravity),now);
	else
		gravityTracker.add(SG2TrackingGravity(_filteredGravity),now);
#endif
	// don't save it if it is cal&brew
	if(log) brewLogger.addGravity(sg,false);
}


void ExternalData::setAuxTemperatureCelsius(float temp){
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
		WirelessTempSensor::theWirelessTempSensor->setTemp(temp);
	}
	#endif
}


float  ExternalData::temperatureCorrection(float sg, float t, float c){

	float nsg= sg*((1.00130346-0.000134722124*t+0.00000204052596*t*t -0.00000000232820948*t*t*t)/
	    (1.00130346-0.000134722124*c+0.00000204052596*c*c-0.00000000232820948*c*c*c));
	return nsg;
}

bool ExternalData::processGravityReport(char data[],size_t length, bool authenticated, uint8_t& error)
{
	//const int BUFFER_SIZE = JSON_OBJECT_SIZE(20);
	//StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
	#if ARDUINOJSON_VERSION_MAJOR == 6
	DynamicJsonDocument root(512);
	auto jsonerror=deserializeJson(root,data,length);
	if(jsonerror 
	#else
	DynamicJsonBuffer jsonBuffer(512);
	JsonObject& root = jsonBuffer.parseObject((char*)data,length);
	if (!root.success() 
	#endif
		|| !root.containsKey("name")){
  		DBG_PRINTF("Invalid JSON\n");
  		error = ErrorJSONFormat;
  		return false;
	}

	String name= root["name"];
    // web interface
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

		//if(!IsGravityInValidRange(gravity)) return true;
		if(root.containsKey("plato")){
			if(root["plato"] && !_cfg->usePlato){
				gravity = Brix2SG(gravity);
			}else if(!root["plato"] && _cfg->usePlato){
				gravity = SG2Brix(gravity);
			}
		} 

		if(root.containsKey("og")){
				setOriginalGravity(gravity);
		}else{
				// gravity data from user
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
		float tempC=itemp;
		if(root.containsKey("temp_units")){
			const char *TU=root["temp_units"];
			if(*TU == 'F') tempC = (itemp-32)/1.8;
			else if(*TU == 'K') tempC = itemp- 273.15;
		}

		setAuxTemperatureCelsius(tempC);

		//Serial.print("temperature:");
		//Serial.println(itemp);

		if(!root.containsKey("angle")){
        	DBG_PRINTF("iSpindel report no angle!\n");
			return false;
		}
    	
        setIspindelAngle(root["angle"],itemp,TimeKeeper.getTimeSeconds());

        if(root.containsKey("battery"))
    	    setDeviceVoltage(root["battery"]);

        if(root.containsKey("RSSI"))
    	    setDeviceRssi(root["RSSI"]);

		//setPlato(root["gravityP"],TimeKeeper.getTimeSeconds());
		if(root.containsKey("gravity") &&
                ! _cfg->calculateGravity 
				&& ! _calibrating ){
			// gravity information directly from iSpindel
			float sgreading=root["gravity"];
            setGravity(sgreading, TimeKeeper.getTimeSeconds());
        }
	}else{
		    error = ErrorUnknownSource;
		    return false;
	}
	return true;
}

