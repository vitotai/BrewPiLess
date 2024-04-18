#include <ArduinoJson.h>
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
#define K2C(d) ((d) - 273.15)
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

bool ExternalData::gravityDeviceEnabled(void){
    return _cfg->gravityDeviceType != GravityDeviceNone;
}


// Provide information when greeting.
void ExternalData::gravityDeviceSetting(char *buf){

	DynamicJsonDocument doc(1024);

	doc["dev"] = _cfg->gravityDeviceType;

	doc["lpf"] = filter.beta();
	doc["stpt"] = _cfg->stableThreshold;
	doc["plato"] = _cfg->usePlato;

	doc["fpt"] = _cfg->numCalPoints;
	doc["name"] = getDeviceName();

	buf[0]='G';
	buf[1]=':';
	serializeJson(doc, buf+2,256);
}

#if SupportTiltHydrometer
void ExternalData::_gotTiltInfo(TiltHydrometerInfo* info){
	TiltConfiguration* tcfg=theSettings.tiltConfiguration();
	float sg =(float) info->gravity /1000.0;
	// Tilt temperature is in Farenheit
	float temp = F2C((float) info->temperature);
	_setAuxTemperatureCelsius( temp);
	_setDeviceRssi(info->rssi);

	// tilt always report SG.
	_remoteHydrometerReport(_cfg->usePlato? SG2Brix(sg):sg,sg);
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
	float sg = info->gravity;

	_setAuxTemperatureCelsius( info->temperature);
	_setDeviceRssi(info->rssi);
	_deviceVoltage=info->battery;

	// Pill seems to calculate tilt by using X in place of Z.
	//_tiltAngle = calcTilt(info->accX,info->accY,info->accZ);
	float angle = calcTilt(info->accZ,info->accY,info->accX);
	// PIll always reports SG

	_remoteHydrometerReport(_cfg->usePlato? SG2Brix(sg):sg,angle);
}
#endif

void ExternalData::_reconfig(bool reformula){
	// common parameters.
	filter.setBeta(_cfg->lpfBeta);
	// process calibration points

	// maintaining the calibratin data even not calculated.
		_formulaKeeper.reset();
		for(uint i=0;i<_cfg->numCalPoints;i++){
			float gravity= _cfg->usePlato?  PlatoFromSetting(_cfg->calPoints[i].calsg):SGFromSetting(_cfg->calPoints[i].calsg);
			float raw= (_cfg->gravityDeviceType == GravityDeviceTilt)? SGFromSetting(_cfg->calPoints[i].raw):AngleFromSetting(_cfg->calPoints[i].raw);
			_formulaKeeper.addPoint(raw,gravity);
		}
		// calibration points might be modified by users
		if(reformula){
			_deriveFormula();
		}else{
			// init. assume valid formula 
			if(_cfg->numCalPoints > 1) _formulaValid=true;
		}
	#if SupportTiltHydrometer || SupportPillHydrometer
	DBG_PRINTF("refoncig: devicetype:%d, %d\n",_cfg->gravityDeviceType,_bleHydrometerType);
	// Remove old hydrometer when necessary.
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

	#if SupportTiltHydrometer 
	if(_cfg->gravityDeviceType == GravityDeviceTilt){
		
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
#if SupportTiltHydrometer || SupportPillHydrometer
	_bleHydrometerType = _cfg->gravityDeviceType;
	#endif
}

void ExternalData::loadConfig(void){
    _cfg = theSettings.GravityConfig();

	_reconfig(false);
}


bool ExternalData::processconfig(char* configdata){
   bool ret= theSettings.dejsonGravityConfig(configdata);
   if(ret){
	   #if !SupportBleHydrometer
	   if(_cfg->gravityDeviceType != GravityDeviceIspindel){
		   return false;
	   }
	   #endif

	   theSettings.save();
	   _reconfig(true);
   }
   return ret;
}

void ExternalData::_setOriginalGravity(float og){
//		_og = og;
		brewLogger.addGravity(og,true);
#if EnableGravitySchedule
		brewKeeper.updateOriginalGravity(og);
#endif
}

float ExternalData::_calculateGravity(float raw){
		// calculate Gravity
	float sg = _cfg->coefficients[0]
            +  _cfg->coefficients[1] * raw
            +  _cfg->coefficients[2] * raw * raw
            +  _cfg->coefficients[3] * raw * raw * raw;

	// temp. correction
	float temp= (brewPi.getUnit() == 'C')? C2F(_auxTemp):_auxTemp;
	if(_cfg->usePlato){
		sg =SG2Brix(temperatureCorrection(Brix2SG(sg),temp,68));
	}else{
	    sg = temperatureCorrection(sg,temp,68);
	}

	return sg;
}
/*
 There are 3 ways to get gravity:
 1. manually set by user
 2. from gravity devices
 3. Derived from angles
*/
void ExternalData::_setGravity(float sg){
        // copy these two for reporting to web interface
    float old_sg=_gravity;
	
    DBG_PRINTF("_setGravity:%d\n",(int)(sg*10000.0));
	if((_cfg->usePlato && (sg > 99.0 || sg < -1.0) ) || (!_cfg->usePlato && (sg > 2.0 || sg < 0.5))) return;
    // verfiy sg, even invalid value will be reported to web interface
	//if(!IsGravityInValidRange(sg)) return;
	_gravity = sg;

	if(!IsGravityValid(old_sg)) filter.setInitial(sg);
#if EnableGravitySchedule
    float _filteredGravity=filter.addData(sg);
		// use filter data as input to tracker and beer profile.
	brewKeeper.updateGravity(_filteredGravity);
	uint32_t now=TimeKeeper.getTimeSeconds();
	if(_cfg->usePlato)
		gravityTracker.add(Plato2TrackingGravity(_filteredGravity),now);
	else
		gravityTracker.add(SG2TrackingGravity(_filteredGravity),now);
#endif

	#if SMART_DISPLAY
		char unit = brewPi.getUnit();
		smartDisplay.gravityDeviceData(_cfg->gravityDeviceType, sg,_auxTemp,_lastUpdate,unit,_cfg->usePlato,_deviceVoltage,_tiltAngle,_rssi);
	#endif

}


void ExternalData::_setAuxTemperatureCelsius(float temp){
	char unit = brewPi.getUnit();
	
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
		float tilt = -100;

		if(root.containsKey("raw")){
			tilt = root["raw"];
		}

		if(root.containsKey("og")){
			_setOriginalGravity(gravity);
	
		}else{
			// gravity data from user
			userSetGravity(gravity,tilt);
		}
	}else if(root.containsKey("name") && root.containsKey("temperature") && root.containsKey("angle") && root.containsKey("battery")&& root.containsKey("RSSI")){
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

        float itemp=root["temperature"];
		float tempC=itemp;
		if(root.containsKey("temp_units")){
			const char *TU=root["temp_units"];
			if(*TU == 'F') tempC = F2C(itemp);
			else if(*TU == 'K') tempC = K2C(itemp);
		}
		_setAuxTemperatureCelsius(tempC);
		_deviceVoltage=root["battery"];
		int8_t rssi=root["RSSI"];
    	_setDeviceRssi(rssi);
		float angle = root["angle"];
		float sg=root["gravity"];
		_remoteHydrometerReport(sg,angle);

	}else{
		    error = ErrorUnknownSource;
		    return false;
	}
	return true;
}

void  ExternalData::userSetGravity(float gravity,float tilt){
	
	if(tilt < 0){
		if(_formulaKeeper.addGravity(gravity)){
			_deriveFormula();
		}
	}else{
		_formulaKeeper.addPoint(tilt,gravity);			// update formula
		_deriveFormula();
	}

	_setGravity(gravity);
	
	if(! _cfg->calbybpl){
		brewLogger.addGravity(gravity);
	}

}

void  ExternalData::_remoteHydrometerReport(float gravity,float tilt){
	//gravit might be plato or SG.
	// 
	_lastUpdate = TimeKeeper.getTimeSeconds();
	_tiltAngle = tilt;

		// calculate SG by polynomial
		// problem here, "tilt angle" stored as multiplied by 16 to be stored as an uint16_t
		// the "tilt" values of "Tilt" is in fact SG, like 1.001. The least 2 digits will be truncated.	
	if(_cfg->gravityDeviceType == GravityDeviceTilt) brewLogger.addTiltAngle(tilt * 100);
	else brewLogger.addTiltAngle(tilt);

	_formulaKeeper.setTilt(tilt,TimeKeeper.getLocalTimeSeconds());

	if(_cfg->calbybpl){
		if( _formulaValid){
			float calculated=_calculateGravity(tilt);
			brewLogger.addGravity(calculated);
			_setGravity(calculated);
		}
	}else{
		float corrected = gravity + _cfg->offset;
		brewLogger.addGravity(corrected);
		_setGravity(corrected);
	}
}

void  ExternalData::_deriveFormula(void){
	// only Tilt can have one point calibration.
	//

	// if there are 2+ pairs of data, we can run regression
	// however, if there is ONLY one. It's just offset.
	// What if User set to use Plato when Tilt and Pill reports SG(1.001)?
	// Seconds, the "saved" data from iSpindel and Pill is tilt angle, which can't be used directly
	if(_formulaKeeper.getFormula(_cfg->coefficients)){

		_formulaValid = true;

		for(int i=0;i< _formulaKeeper.numberOfPoints();i++){
			_cfg->calPoints[i].calsg =_cfg->usePlato? PlatoToSetting(_formulaKeeper._calGravities[i]):SGToSetting(_formulaKeeper._calGravities[i]);
			_cfg->calPoints[i].raw =(_cfg->gravityDeviceType == GravityDeviceTilt)?
				SGToSetting(_formulaKeeper._calTilts[i]):AngleToSetting(_formulaKeeper._calTilts[i]);

		}
		_cfg->numCalPoints = _formulaKeeper.numberOfPoints();

		theSettings.save();

		brewLogger.addCalibrateData();

		DBG_PRINTF("New Formula from %d points:",_cfg->numCalPoints);
			DBG_PRINT(_cfg->coefficients[0],8);
			DBG_PRINT(",");
			DBG_PRINT(_cfg->coefficients[1],8);
			DBG_PRINT(",");
			DBG_PRINT(_cfg->coefficients[2],8);
			DBG_PRINT(",");
			DBG_PRINT(_cfg->coefficients[3],8);
		DBG_PRINTF("\n");
	}
}


const char* ExternalData::getDeviceName(void){
	if(_cfg->gravityDeviceType == GravityDeviceIspindel){
		return (_ispindelName)? _ispindelName:"iSpindel";
	}else if(_cfg->gravityDeviceType == GravityDeviceTilt){
		return "Tilt";
	}else if(_cfg->gravityDeviceType == GravityDevicePill){
		return "Pill";
	}else{
		return "unkown";
	}
}