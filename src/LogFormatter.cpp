#include <ArduinoJson.h>
#include "LogFormatter.h"
#include "mystrlib.h"
#include "DataLogger.h"
#include "Config.h"
#include "TemperatureFormats.h"
#include "BrewPiProxy.h"
#include "ExternalData.h"
#include "BPLSettings.h"
#if SupportPressureTransducer
#include "PressureMonitor.h"
#endif
#if EnableHumidityControlSupport
#include "HumidityControl.h"
#endif
extern BrewPiProxy brewPi;



static char modeInInteger(char mode){
	char modevalue;
	if(mode == 'p') modevalue = '3';
	else if(mode == 'b') modevalue = '2';
	else if(mode == 'f') modevalue = '1';
	else modevalue = '0';
	return modevalue;
}

size_t printFloat(char* buffer,float value,int precision,bool valid,const char* invalidstr)
{
	if(valid){
		return sprintFloat(buffer,value,precision);
	}else{
        strcpy(buffer,invalidstr);
        return strlen(invalidstr);
	}
}

size_t dataSprintf(char *buffer,const char *format,const char* invalid)
{
	int i=0;
	size_t d=0;
	for(i=0;i< (int) strlen(format);i++){
		char ch=format[i];
		if( ch == '%'){
			i++;
			ch=format[i];
			if(ch == '%'){
				buffer[d++]=ch;
			}else if(ch == 'b'){
				float  beerTemp = brewPi.getBeerTemp();

				d += printFloat(buffer+d,beerTemp,1,IS_FLOAT_TEMP_VALID(beerTemp),invalid);
			}else if(ch == 'B'){
				float  beerSet = brewPi.getBeerSet();
				d += printFloat(buffer+d,beerSet,1,IS_FLOAT_TEMP_VALID(beerSet),invalid);
			}else if(ch == 'f'){
				float fridgeTemp = brewPi.getFridgeTemp();
				d += printFloat(buffer+d,fridgeTemp,1,IS_FLOAT_TEMP_VALID(fridgeTemp),invalid);
			}else if(ch == 'F'){
				float fridgeSet = brewPi.getFridgeSet();
				d += printFloat(buffer+d,fridgeSet,1,IS_FLOAT_TEMP_VALID(fridgeSet),invalid);
			}else if(ch == 'r'){
				float  roomTemp = brewPi.getRoomTemp();
				d += printFloat(buffer+d,roomTemp,1,IS_FLOAT_TEMP_VALID(roomTemp),invalid);
			}else if(ch == 'g'){
				float sg=externalData.gravity();
				d += printFloat(buffer+d,sg,4,IsGravityValid(sg),invalid);
			}else if(ch == 'p'){
				float sg=externalData.plato();
				d += printFloat(buffer+d,sg,2,IsGravityValid(sg),invalid);
			}
			else if(ch == 'P'){
			#if SupportPressureTransducer
				d += printFloat(buffer+d,PressureMonitor.currentPsi(),1,PressureMonitor.isCurrentPsiValid(),invalid);
			#else
		        strcpy(buffer+d,invalid);
        		d+= strlen(invalid);
			#endif

			}
			else if(ch == 'v'){
				float vol=externalData.deviceVoltage();
				d += printFloat(buffer+d,vol,1,IsVoltageValid(vol),invalid);
			}else if(ch == 'a'){
				float at=externalData.auxTemp();
				d += printFloat(buffer+d,at,1,IS_FLOAT_TEMP_VALID(at),invalid);
			}else if(ch == 't'){
				float tilt=externalData.tiltValue();
				d += printFloat(buffer+d,tilt,2,true,invalid);
			}else if(ch == 'u'){
				d += sprintInt(buffer+d, externalData.lastUpdate());
			}else if(ch == 'U'){
				char unit = brewPi.getUnit();
				*(buffer+d)= unit;
				d++;
			}else if(ch == 'm'){
				*(buffer+d)= modeInInteger(brewPi.getMode());
				d++;
			}else if(ch == 'M'){
				*(buffer+d)= brewPi.getMode();
				d++;
			}else if(ch == 's'){
				*(buffer+d)= '0' + brewPi.getState();
				d++;
			}else if(ch == 'H'){
				strcpy(buffer+d,theSettings.systemConfiguration()->hostnetworkname);
				d += strlen(theSettings.systemConfiguration()->hostnetworkname);
			}else if(ch == 'h'){
				#if EnableHumidityControlSupport
				 d += printFloat(buffer+d,(float)humidityControl.humidity(),0,humidityControl.isHumidityValid(),invalid);
				#else
		        strcpy(buffer+d,invalid);
        		d+= strlen(invalid);
				#endif
			}else if(ch == 'E'){
				#if EnableHumidityControlSupport
				 d += printFloat(buffer+d,(float)humidityControl.roomHumidity(),0,humidityControl.isRoomSensorInstalled(),invalid);
				#else
		        strcpy(buffer+d,invalid);
        		d+= strlen(invalid);
				#endif
			}else{				
				// wrong format
				//return 0; ignored
			}
		}else{
			buffer[d++]=ch;
		}
	}// for each char

	buffer[d]='\0';
	return d;
}

/*
int _copyName(char *buf,char *name,bool concate)
{
	char *ptr=buf;
	if(name ==NULL) return 0;
	if(concate){
		*ptr='&';
		ptr++;
	}
	int len=strlen(name);
	strcpy(ptr,name);
	ptr+=len;
	*ptr = '=';
	ptr++;
	return (ptr - buf);
}

int copyTemp(char* buf,char* name,float value, bool concate)
{
	int n;
	if((n = _copyName(buf,name,concate))!=0){
		if(IS_FLOAT_TEMP_VALID(value)){
			n += sprintFloat(buf + n ,value,2);
		}else{
			strcpy(buf + n,"null");
			n += 4;
		}

	}
	return n;
}
*/

size_t nonNullJson(char* buffer,size_t size)
{
	const int JSON_BUFFER_SIZE = JSON_OBJECT_SIZE(15);
	
	#if ARDUINOJSON_VERSION_MAJOR == 6
	DynamicJsonDocument root(JSON_BUFFER_SIZE +size);
	#else

	DynamicJsonBuffer jsonBuffer(JSON_BUFFER_SIZE);
	JsonObject& root = jsonBuffer.createObject();
	#endif

	uint8_t state, mode;
	float beerSet,fridgeSet;
	float beerTemp,fridgeTemp,roomTemp;

    state = brewPi.getState();
    mode = brewPi.getMode();
    beerTemp = brewPi.getBeerTemp();
    beerSet = brewPi.getBeerSet();
    fridgeTemp = brewPi.getFridgeTemp();
    fridgeSet = brewPi.getFridgeSet();
    roomTemp = brewPi.getRoomTemp();


	root[KeyState] = state;

	if(IS_FLOAT_TEMP_VALID(beerTemp)) root[KeyBeerTemp] = beerTemp;
	if(IS_FLOAT_TEMP_VALID(beerSet)) root[KeyBeerSet] = beerSet;
	if(IS_FLOAT_TEMP_VALID(fridgeTemp)) root[KeyFridgeTemp] = fridgeTemp;
	if(IS_FLOAT_TEMP_VALID(fridgeSet)) root[KeyFridgeSet] = fridgeSet;
	if(IS_FLOAT_TEMP_VALID(roomTemp)) root[KeyRoomTemp] = roomTemp;

	root[KeyMode] =(int)( modeInInteger(mode) - '0');
	#if SupportPressureTransducer
	if(PressureMonitor.isCurrentPsiValid()) root[KeyPressure]= PressureMonitor.currentPsi();
	#endif

	#if EnableHumidityControlSupport
	if(humidityControl.isHumidityValid()) root[KeyFridgeHumidity] = humidityControl.humidity();
	if(humidityControl.isRoomSensorInstalled()){
		uint8_t rh = humidityControl.roomHumidity();
		if(rh<100) root[KeyRoomHumidity] =rh;
	}
	#endif

	float sg=externalData.gravity();
	if(IsGravityValid(sg)){
		root[KeyGravity] = sg;
		root[KeyPlato] = externalData.plato();
	}

	// Hydrometer data
	float vol=externalData.deviceVoltage();
	if(IsVoltageValid(vol)) root[KeyVoltage] = vol;
	float at=externalData.auxTemp();
	if(IS_FLOAT_TEMP_VALID(at)) root[KeyAuxTemp] = at;
	
	float tilt=externalData.tiltValue();
	if(tilt >0)	root[KeyTilt]=tilt;
	
	int16_t rssi=externalData.rssi();
	if(IsRssiValid(rssi)) root[KeyWirelessHydrometerRssi]=rssi;
	const char *hname=externalData.getDeviceName();
	if(hname) root[KeyWirelessHydrometerName] = hname;

	#if ARDUINOJSON_VERSION_MAJOR == 6
	return	serializeJson(root,buffer,size);
	#else
	return root.printTo(buffer,size);
	#endif
}
