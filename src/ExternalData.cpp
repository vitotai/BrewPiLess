#include <ArduinoJson.h>
#include <math.h>
#include "ExternalData.h"
#if SupportTiltHydrometer
#include "BleTiltListener.h"
#endif

#if SupportPillHydrometer
#include "BlePillListener.h"
#endif


#if TWOFACED_LCD

#include "SharedLcd.h"


#endif

ExternalData externalData;

#define TiltFilterParameter 0.15

#define F2C(d) (((d)-32)/1.8)
//#define C2F(d) (((d)*1.8)+32)


float Brix2SG(float brix){
 return (brix / (258.6-((brix / 258.2)*227.1))) + 1;
}

float SG2Brix(float SG){
 return (((182.4601 * SG -775.6821) * SG +1262.7794) * SG -669.5622);
}

float  temperatureCorrection(float sg, float t, float c){

	float nsg= sg*((1.00130346-0.000134722124*t+0.00000204052596*t*t -0.00000000232820948*t*t*t)/
	    (1.00130346-0.000134722124*c+0.00000204052596*c*c-0.00000000232820948*c*c*c));
	return nsg;
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
/*
bool ExternalData::iSpindelEnabled(void){
    return _cfg->gravityDeviceType == GravityDeviceIspindel;
}
*/

bool ExternalData::gravityDeviceEnabled(void){
    return _cfg->gravityDeviceType != GravityDeviceNone;
}


float ExternalData::hydrometerCalibrationTemp(void){ 
    return _cfg->ispindelCalibrationBaseTemp;
}

// Provide information when greeting.
void ExternalData::sseNotify(char *buf){

	DynamicJsonDocument doc(1024);

	doc["dev"] = _cfg->gravityDeviceType;
	// shared values: sg, last update, rssi, temp
	doc["sg"] =_gravity;
	doc["lu"] = _lastUpdate;

	doc["lpf"] = filter.beta();
	doc["stpt"] = _cfg->stableThreshold;
	doc["fpt"] = _cfg->numberCalPoints;
	doc["ctemp"] = _cfg->ispindelCalibrationBaseTemp;
	doc["plato"] = _cfg->usePlato;

	doc["angle"] = _tiltAngle;
	doc["name"] = (_ispindelName)? _ispindelName:"Unknown";
	doc["battery"] =_deviceVoltage;

	#if SupportTiltHydrometer
	doc["tiltraw"] = _tiltRawGravity;
	#endif

	buf[0]='G';
	buf[1]=':';
	serializeJson(doc, buf+2,256);
}

#if SupportTiltHydrometer
void ExternalData::_gotTiltInfo(TiltHydrometerInfo* info){
	TiltConfiguration* tcfg=theSettings.tiltConfiguration();
	_tiltRawGravity = info->gravity;
	float sg =(float) info->gravity /1000.0;

	float csg=sg;
	
	if(tcfg->numCalPoints > 0){
	 	csg = tcfg->coefficients[0] 
			 +  tcfg->coefficients[1] * sg 
			 +  tcfg->coefficients[2] * sg * sg
			 +  tcfg->coefficients[3] * sg * sg * sg; 
	}
	// Tilt temperature is in Farenheit
	_setAuxTemperatureCelsius( F2C((float) info->temperature));
	_setDeviceRssi(info->rssi);
	float fgravity =(_cfg->usePlato)? SG2Brix(csg):csg;
	_setGravity(fgravity, TimeKeeper.getTimeSeconds());
	// display

}
#endif

#if SupportPillHydrometer

float calcTilt(int16_t x, int16_t y, int16_t z){
	if (x == 0 && y == 0 && z == 0) return 0.f;
	float ax=(float)x;
	float ay=(float)y;
	float az=(float)z;
	
	return acos(abs(az) / (sqrt(ax * ax + ay * ay + az * az))) * 180.0 / M_PI;
}

void ExternalData::_gotPillInfo(PillHydrometerInfo* info){
	PillConfiguration* pcfg=theSettings.pillConfiguration();
	float csg = info->gravity;

	_setAuxTemperatureCelsius( info->temperature);
	_setDeviceRssi(info->rssi);
	_deviceVoltage=info->battery;

	// Pill seems to calculate tilt by using X in place of Z.
	//_tiltAngle = calcTilt(info->accX,info->accY,info->accZ);
	_tiltAngle = calcTilt(info->accZ,info->accY,info->accX);
	DBG_PRINTF(" Pill Tilt:");
	DBG_PRINT(_tiltAngle);
	DBG_PRINT("\n");

	brewLogger.addTiltAngle(_tiltAngle);
	float fgravity;
	if(_calibrating){
		// calculate SG by polynomial
		fgravity=_calculateGravitybyAngle(_tiltAngle, info->temperature);
		_setGravity(fgravity, TimeKeeper.getTimeSeconds(),false);
	}else{
		fgravity =(_cfg->usePlato)? SG2Brix(csg):csg;
		_setGravity(fgravity, TimeKeeper.getTimeSeconds());
	}

}
#endif

void ExternalData::_reconfig(void){
	DBG_PRINTF("refoncig: devicetype:%d, %d\n",_cfg->gravityDeviceType,_bleHydrometerType);
	#if SupportTiltHydrometer || SupportPillHydrometer
	if(_cfg->gravityDeviceType != _bleHydrometerType){
		if((_bleHydrometerType == GravityDeviceTilt) || (_bleHydrometerType == GravityDevicePill) ){
			if(_bleHydrometer){
				_bleHydrometer->stopListen();
				delete _bleHydrometer;
				_bleHydrometer=NULL;
			}
		}
	}
	#endif

	if(_cfg->gravityDeviceType == GravityDeviceIspindel){	    
		filter.setBeta(_cfg->lpfBeta);		
	}
	#if SupportTiltHydrometer 
	else if(_cfg->gravityDeviceType == GravityDeviceTilt){
		
		TiltConfiguration* tcfg = theSettings.tiltConfiguration();
		if(_bleHydrometerType == GravityDeviceTilt){
			// already tilt, change color, maybe
			DBG_PRINTF("Change Tilt color to %d\n",tcfg->tiltColor);
			((TiltListener *)_bleHydrometer)->setColor((TiltColor) tcfg->tiltColor);
		}else{
			DBG_PRINTF("Create Tilt color of %d\n",tcfg->tiltColor);

			TiltListener *tilt=new TiltListener;
			_bleHydrometer = tilt;
	    	filter.setBeta(TiltFilterParameter);		
			tilt->listen((TiltColor) tcfg->tiltColor,[&](TiltHydrometerInfo* info){
				_gotTiltInfo(info);
			});
		}
	}
	#endif
	#if SupportPillHydrometer
	else if(_cfg->gravityDeviceType == GravityDevicePill){
		PillConfiguration* pcfg = theSettings.pillConfiguration();

		if(_bleHydrometerType == GravityDevicePill){
			// already tilt, change color, maybe
			DBG_PRINTF("Change Pill address\n");

			((PillListener *)_bleHydrometer)->setMac(pcfg->macAddress);
		}else{
			DBG_PRINTF("Create Pill:\n");

			PillListener *pill=new PillListener(pcfg->macAddress);
			_bleHydrometer = pill;
	    	filter.setBeta(TiltFilterParameter);		
			pill->listen([&](PillHydrometerInfo* info){
				_gotPillInfo(info);
			});
		}
	}
	#endif

	_bleHydrometerType = _cfg->gravityDeviceType;
}

void ExternalData::loadConfig(void){
    _cfg = theSettings.GravityConfig();
	_reconfig();
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
	   _reconfig();
   }
   return ret;
}

void ExternalData::setFormula(float coeff[4],uint32_t npt){
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

void ExternalData::_setOriginalGravity(float og){
//		_og = og;
		brewLogger.addGravity(og,true);
#if EnableGravitySchedule
		brewKeeper.updateOriginalGravity(og);
#endif
}

float ExternalData::_calculateGravitybyAngle(float tilt,float temp){

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
/*
 There are 3 ways to get gravity:
 1. manually set by user
 2. from gravity devices
 3. Derived from angles
*/
void ExternalData::_setGravity(float sg, time_t now,bool log){
        // copy these two for reporting to web interface
    float old_sg=_gravity;
	
    DBG_PRINTF("_setGravity:%d, saved:%d\n",(int)(sg*10000.0),log);
	if((_cfg->usePlato && (sg > 99.0 || sg < -1.0) ) || (!_cfg->usePlato && (sg > 2.0 || sg < 0.5))) return;
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

	#if SMART_DISPLAY
			char unit;
			float max,min;
    		brewPi.getTemperatureSetting(&unit,&min,&max);			
			smartDisplay.gravityDeviceData(sg,_auxTemp,_lastUpdate,unit,_cfg->usePlato,_deviceVoltage,_tiltAngle,_rssi);
	#endif

}


void ExternalData::_setAuxTemperatureCelsius(float temp){
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
				_setOriginalGravity(gravity);
		}else{
				// gravity data from user
			_setGravity(gravity,TimeKeeper.getTimeSeconds());
		}
	}else //if(name.startsWith("iSpindel")){
		if(root.containsKey("name") && root.containsKey("temperature") && root.containsKey("angle") && root.containsKey("battery")){
		//{"name": "iSpindel01", "id": "XXXXX-XXXXXX", "temperature": 20.5, "angle": 89.5, "gravityP": 13.6, "battery": 3.87}
		DBG_PRINTF("%s\n",name.c_str());
		// force to set to iSpindel.
		#if SupportTiltHydrometer || SupportPillHydrometer
		if(_cfg->gravityDeviceType != GravityDeviceIspindel) return false;
		#else
		_cfg->gravityDeviceType = GravityDeviceIspindel;
		#endif

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

		_setAuxTemperatureCelsius(tempC);
		float battery=0;

        if(root.containsKey("battery")){
			battery=root["battery"];
    	    _deviceVoltage = battery;
		}
		int8_t rssi=-120;
        if(root.containsKey("RSSI")){
			rssi=root["RSSI"];
    	    _setDeviceRssi(rssi);
		}
		//Serial.print("temperature:");
		//Serial.println(itemp);

		if(root.containsKey("angle")){
			_tiltAngle=root["angle"];
			// add tilt anyway
			brewLogger.addTiltAngle(_tiltAngle);
		}

		if(root.containsKey("angle") && (_cfg->calculateGravity ||  _calibrating ) ){
	        float calculatedSg=_calculateGravitybyAngle(_tiltAngle,itemp);

			// update, gravity data calculated
			// in "brew N cal" mode, only tilt is logged.
			_setGravity(calculatedSg,_lastUpdate,!_calibrating); // save only when not calibrating.

		}else if(root.containsKey("gravity")){
			// gravity information directly from iSpindel
			float sgreading=root["gravity"];
			
			if((_cfg->usePlato && sgreading >0 && sgreading <90)
			  ||(!_cfg->usePlato && sgreading >0.8 && sgreading <1.3) ){ 

				_setGravity(sgreading, TimeKeeper.getTimeSeconds());
			}

        }
	}else{
		    error = ErrorUnknownSource;
		    return false;
	}
	return true;
}

