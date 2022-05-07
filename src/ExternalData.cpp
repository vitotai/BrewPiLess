#include <ArduinoJson.h>
#include "ExternalData.h"
#if SupportTiltHydrometer
#include "TiltListener.h"
#endif

#if TWOFACED_LCD

#include "SharedLcd.h"


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

	DynamicJsonDocument doc(1024);

	doc["dev"] = _cfg->gravityDeviceType;
	doc["name"] = (_ispindelName)? _ispindelName:"Unknown";
	doc["battery"] =_deviceVoltage;
	doc["sg"] =_gravity;
	doc["angle"] = _ispindelTilt;
	doc["lu"] = _lastUpdate;
	doc["lpf"] = filter.beta();
	doc["stpt"] = _cfg->stableThreshold;
	doc["fpt"] = _cfg->numberCalPoints;
	doc["ctemp"] = _cfg->ispindelCalibrationBaseTemp;
	doc["plato"] = _cfg->usePlato;

	#if SupportTiltHydrometer
	doc["tiltraw"] = _tiltRawGravity;
	#endif

	buf[0]='G';
	buf[1]=':';
	serializeJson(doc, buf+2,256);
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
	float fgravity =(_cfg->usePlato)? SG2Brix(csg):csg;
	setGravity(fgravity, TimeKeeper.getTimeSeconds());
	// display

	#if SMART_DISPLAY
	// duplicated code, I know...
	char unit;
	float max,min;

	brewPi.getTemperatureSetting(&unit,&min,&max);
	smartDisplay.gravityDeviceData(fgravity,(unit =='C')? ((float)temperature  -32.0)* 5.0/9.0:temperature,_lastUpdate,unit,_cfg->usePlato,0,0,rssi);
	#endif

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

float ExternalData::calculateGravitybyAngle(float tilt,float temp){

	if(_calibrating && _cfg->numberCalPoints ==0){
		DBG_PRINTF("No valid formula!\n");
		return 0; // don't calculate if formula is not available.
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
	return sg;
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
		// force to set to iSpindel.
		_cfg->gravityDeviceType = GravityDeviceIspindel;

		if(!_ispindelName){
			_ispindelName=(char*) malloc(name.length()+1);
			if(_ispindelName) strcpy(_ispindelName,name.c_str());
		}

		if(! root.containsKey("temperature")){
		    DBG_PRINTF("iSpindel report no temperature!\n");
		    return false;
		}
		_lastUpdate=TimeKeeper.getTimeSeconds();

        float itemp=root["temperature"];
		float tempC=itemp;
		char iTU='C';
		if(root.containsKey("temp_units")){
			const char *TU=root["temp_units"];
			if(*TU == 'F') tempC = (itemp-32)/1.8;
			else if(*TU == 'K') tempC = itemp- 273.15;
			iTU = TU[0];
		}

		setAuxTemperatureCelsius(tempC);
		float battery=0;

        if(root.containsKey("battery")){
			battery=root["battery"];
    	    setDeviceVoltage(battery);
		}
		int8_t rssi=-120;
        if(root.containsKey("RSSI")){
			rssi=root["RSSI"];
    	    setDeviceRssi(rssi);
		}
		//Serial.print("temperature:");
		//Serial.println(itemp);

		if(root.containsKey("angle")){
			_ispindelTilt=root["angle"];
			// add tilt anyway
			brewLogger.addTiltAngle(_ispindelTilt);
		}

		if(root.containsKey("angle") && (_cfg->calculateGravity ||  _calibrating ) ){
	        float calculatedSg=calculateGravitybyAngle(_ispindelTilt,itemp);

			// update, gravity data calculated
			// in "brew N cal" mode, only tilt is logged.
			setGravity(calculatedSg,_lastUpdate,!_calibrating); // save only when not calibrating.
			#if SMART_DISPLAY
			char unit;
			float max,min;

		    brewPi.getTemperatureSetting(&unit,&min,&max);
			smartDisplay.gravityDeviceData(calculatedSg,itemp,_lastUpdate,iTU,_cfg->usePlato,_deviceVoltage,_ispindelTilt,rssi);
			#endif

		}else if(root.containsKey("gravity")){
			// gravity information directly from iSpindel
			float sgreading=root["gravity"];
			
			if((_cfg->usePlato && sgreading >0 && sgreading <90)
			  ||(!_cfg->usePlato && sgreading >0.8 && sgreading <1.3) ){ 

				setGravity(sgreading, TimeKeeper.getTimeSeconds());
			}
			#if SMART_DISPLAY
			char unit;
			float max,min;
    		brewPi.getTemperatureSetting(&unit,&min,&max);			
			smartDisplay.gravityDeviceData(sgreading,itemp,_lastUpdate,iTU,_cfg->usePlato,_deviceVoltage,_ispindelTilt,rssi);
			#endif

        }
	}else{
		    error = ErrorUnknownSource;
		    return false;
	}
	return true;
}

