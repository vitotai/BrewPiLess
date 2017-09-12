#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "mystrlib.h"
#include "DataLogger.h"
#include "espconfig.h"
#include "TemperatureFormats.h"
#include "BrewPiProxy.h"
#include "ExternalData.h"
extern BrewPiProxy brewPi;

#define GSLOG_JSON_BUFFER_SIZE 256
#define MAX_GSLOG_CONFIG_LEN 1024
#define GSLogConfigFile "/gslog.cfg"


int DataLogger::printFloat(char* buffer,float value,int precision,bool valid)
{
	if(valid){
		return sprintFloat(buffer,value,precision);
	}else{
		strcpy(buffer,"null");
		return 4;
	}
}

int DataLogger::dataSprintf(char *buffer,const char *format)
{
	uint8_t state, mode;
	float beerSet,fridgeSet;
	float beerTemp,fridgeTemp,roomTemp;

	brewPi.getAllStatus(&state,&mode,& beerTemp,& beerSet,& fridgeTemp,& fridgeSet,& roomTemp);

	int i=0;
	int d=0;
	for(i=0;i< strlen(format);i++){
		char ch=format[i];
		if( ch == '%'){
			i++;
			ch=format[i];
			if(ch == '%'){
				buffer[d++]=ch;
			}else if(ch == 'b'){
				d += printFloat(buffer+d,beerTemp,1,IS_FLOAT_TEMP_VALID(beerTemp));
			}else if(ch == 'B'){
				d += printFloat(buffer+d,beerSet,1,IS_FLOAT_TEMP_VALID(beerSet));
			}else if(ch == 'f'){
				d += printFloat(buffer+d,fridgeTemp,1,IS_FLOAT_TEMP_VALID(fridgeTemp));
			}else if(ch == 'F'){
				d += printFloat(buffer+d,fridgeSet,1,IS_FLOAT_TEMP_VALID(fridgeSet));
			}else if(ch == 'r'){
				d += printFloat(buffer+d,roomTemp,1,IS_FLOAT_TEMP_VALID(roomTemp));
			}else if(ch == 'g'){
				float sg=externalData.gravity();
				d += printFloat(buffer+d,sg,3,IsGravityValid(sg));
			}else if(ch == 'v'){
				float vol=externalData.deviceVoltage();
				d += printFloat(buffer+d,vol,1,IsVoltageValid(vol));
			}else if(ch == 'a'){
				float at=externalData.auxTemp();
				d += printFloat(buffer+d,at,1,IS_FLOAT_TEMP_VALID(at));
			}else if(ch == 'u'){
				d += sprintInt(buffer+d, externalData.lastUpdate());
			}else{
				// wrong format
				return 0;
			}
		}else{
			buffer[d++]=ch;
		}
	}// for each char

	buffer[d]='\0';
	return d;
}

void DataLogger::loop(time_t now)
{
	if(!_enabled) return;

	if((now - _lastUpdate) < _period) return;

	sendData();
	_lastUpdate=now;
}

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

void DataLogger::sendData(void)
{
	char data[512];
	int len=0;

	len =dataSprintf(data,_format);

	if(len==0){
		DBG_PRINTF("Invalid format\n");
		return;
	}

	DBG_PRINTF("url=%s\n",_url);
	DBG_PRINTF("data= %d, \"%s\"\n",len,data);

	int code;
	HTTPClient _http;
  	_http.setUserAgent(F("ESP8266"));

	DBG_PRINTF("[HTTP] %s...\n",_method);
	if(strcmp(_method,"POST")==0
		|| strcmp(_method,"PUT")==0 ){
		// post

 		_http.begin(_url);
 		if(_contentType){
  			_http.addHeader("Content-Type", _contentType);
 		}else{
  			_http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  		}
    // start connection and send HTTP header
    	code = _http.sendRequest(_method,(uint8_t*)data,len);
    }else{
 		_http.begin(String(_url) + String("?") + String(data));

    	code = _http.GET();
    }

    if(code <= 0) {
        DBG_PRINTF("HTTP error: %s\n", _http.errorToString(code).c_str());
        _http.end();
        return;
    }
      // HTTP header has been send and Server response header has been handled
    DBG_PRINTF("[HTTP] result code: %d\n", code);
    if(code == HTTP_CODE_OK){

    }else if((code / 100) == 3 && _http.hasHeader("Location")){
      String location=_http.header("Location");
      DBG_PRINTF("redirect:%s\n",location.c_str());
    }else{
      DBG_PRINTF("error, unhandled code:%d",code);
    }
    String output=_http.getString();
    DBG_PRINTF("output:\n%s\n",output.c_str());
	_http.end();
}



bool DataLogger::processJson(char* jsonstring)
{
	DynamicJsonBuffer jsonBuffer(GSLOG_JSON_BUFFER_SIZE);
	JsonObject& root = jsonBuffer.parseObject(jsonstring);
	if(!root.success()
		|| !root.containsKey("enabled")
		|| !root.containsKey("format")
		|| !root.containsKey("method")
		|| !root.containsKey("period")){
		_enabled=false;
		return false;
	}
	_enabled= root["enabled"];
	_period = root["period"];

	const char *url=root["url"];
	const char *method=root["method"];
	const char *format=root["format"];
	const char *contentType=root["type"];

	if(url == NULL || method==NULL || format==NULL || strcmp(url,"") ==0 || strcmp(method,"") ==0 || strcmp(format,"") ==0){
		_enabled=false;
		return false;
	}
	#define COPYSTRING(a) if(_##a) free(_##a); _##a = (a==NULL || strcmp(a,"") ==0)? NULL:strdup(a)

	COPYSTRING(url);
	COPYSTRING(method);
	COPYSTRING(format);
  	COPYSTRING(contentType);

  	return true;
}

void DataLogger::loadConfig(void)
{
	File f=SPIFFS.open(GSLogConfigFile,"r+");
	char configBuf[MAX_GSLOG_CONFIG_LEN];

	if(f){
		size_t len=f.readBytes(configBuf,MAX_GSLOG_CONFIG_LEN);
		f.close();
		configBuf[len]='\0';
		processJson(configBuf);
	}else{
		_enabled=false;
	}
}

void DataLogger::updateSetting(AsyncWebServerRequest *request)
{
		if(request->hasParam("data", true)){
    		String c=request->getParam("data", true)->value();
			char *tbuf=strdup(c.c_str());
			if(!processJson(tbuf)){
		         request->send(404);
		         free(tbuf);
		         return;
		    }
		    free(tbuf);

        	ESP.wdtDisable();
    		File fh= SPIFFS.open(GSLogConfigFile, "w");
    		if(!fh){
    			request->send(500);
    			return;
    		}
	      	fh.print(c.c_str());
      		fh.close();
        	ESP.wdtEnable(10);
            request->send(200);
        } else{
          request->send(404);
    	}

}

void DataLogger::getSettings(AsyncWebServerRequest *request)
{
	if(request->hasParam("data")){
		if(SPIFFS.exists(GSLogConfigFile))
			request->send(SPIFFS, GSLogConfigFile);
		else
			request->send(200,"application/json","{}");
	}else{
		request->redirect(request->url() + ".htm");
		/*  use htm file
		AsyncWebServerResponse *response = request->beginResponse(String("text/html"),
  		strlen_P(LogConfigHtml),
  		[=](uint8_t *buffer, size_t maxLen, size_t alreadySent) -> size_t {
    		if (strlen_P(LogConfigHtml+alreadySent)>maxLen) {
      		// We have more to read than fits in maxLen Buffer
      		memcpy_P((char*)buffer, LogConfigHtml+alreadySent, maxLen);
      		return maxLen;
    	}
    	// Ok, last chunk
    	memcpy_P((char*)buffer, LogConfigHtml+alreadySent, strlen_P(LogConfigHtml+alreadySent));
    	return strlen_P(LogConfigHtml+alreadySent); // Return from here to end of indexhtml
 	 	}
		);
		request->send(response);
		*/
	}
}
