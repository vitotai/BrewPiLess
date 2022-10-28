
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
//#include <WiFiClientSecureBearSSL.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <HTTPClient.h>
#include <AsyncTCP.h>
#include <WiFiClientSecure.h>
#endif

#include <ESPAsyncWebServer.h>
#include "DataLogger.h"
#include "Config.h"
#include "LogFormatter.h"
#if SupportPressureTransducer
#include "PressureMonitor.h"
#endif

#define GSLOG_JSON_BUFFER_SIZE 256
#define MAX_GSLOG_CONFIG_LEN 1024



void DataLogger::reportNow(void)
{
	_lastUpdate=0;
}


void DataLogger::loop(time_t now)
{
	if(!_loggingInfo->enabled) return;

	if((now - _lastUpdate) < _loggingInfo->period) return;

	sendData();
	_lastUpdate=now;
}


#define BUFFERSIZE 512

void DataLogger::sendData(void)
{
	char data[BUFFERSIZE];
	int len=0;

	if(_loggingInfo->service == ServiceNonNullJson){
		len = nonNullJson(data,BUFFERSIZE);
	}else{

		if(_loggingInfo->service == ServiceHTTPNullString){
			len =dataSprintf(data,_loggingInfo->format,"\"\"");
		}else{
			len =dataSprintf(data,_loggingInfo->format,"null");
		}
	}

	if(len==0){
		DBG_PRINTF("Invalid format\n");
		return;
	}

	DBG_PRINTF("url=\"%s\"\n",_loggingInfo->url);
	DBG_PRINTF("data= %d, \"%s\"\n",len,data);

	int code;

#if ESP32	
	WiFiClient wifiClient;
	WiFiClientSecure sClient;
	bool isHttps=false;

	if( strncasecmp(_loggingInfo->url,"https",5) ==0){
//		sClient.setBufferSizes(1024, 1024);
//		sClient.setInsecure();
		isHttps=true;
	}	
#else
	WiFiClient wifiClient;
#endif

	HTTPClient _http;
  	_http.setUserAgent(F("ESP8266"));

	DBG_PRINTF("[HTTP] %d...\n",_loggingInfo->method);
	DBG_PRINTF("Content-Type:\"%s\"\n", _loggingInfo->contentType);
	if(_loggingInfo->method == mHTTP_POST
		|| _loggingInfo->method== mHTTP_PUT ){
		// post
		#if ESP32	
		if(isHttps) _http.begin(sClient,_loggingInfo->url);
		else 
		#endif
		_http.begin(wifiClient,_loggingInfo->url);

 		if(_loggingInfo->contentType && _loggingInfo->contentType[0]!='\0'){
  			_http.addHeader("Content-Type", _loggingInfo->contentType);
 		}else{
  			_http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  		}
    // start connection and send HTTP header
    	code = _http.sendRequest((_loggingInfo->method == mHTTP_POST)? "POST":"PUT",(uint8_t*)data,len);
    }else{
		#if ESP32	
		if(isHttps) _http.begin(sClient,String(_loggingInfo->url) + String("?") + String(data));
		else
		#endif
 		_http.begin(wifiClient,String(_loggingInfo->url) + String("?") + String(data));
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
