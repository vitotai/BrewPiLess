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

#define GSLOG_JSON_BUFFER_SIZE 256
#define MAX_GSLOG_CONFIG_LEN 1024
#define GSLogConfigFile "/gslog.cfg"


void DataLogger::loop(time_t now,void (*getTemp)(float *pBeerTemp,float *pBeerSet,float *pFridgeTemp, float *pFridgeSet))
{
	if(!_enabled) return;
	
	if((now - _lastUpdate) < _period) return;
	
	float beerTemp,beerSet,fridgeTemp,fridgeSet;
	(*getTemp)(&beerTemp,&beerSet,&fridgeTemp,&fridgeSet);
	
	if(IS_FLOAT_TEMP_VALID(beerTemp)){
		sendData(beerTemp,beerSet,fridgeTemp,fridgeSet);
		_lastUpdate=now;
		_retry =0;
	}else{
		DBG_PRINTF("Invalid Temp, retry:%d\n",_retry);
		// star retry
		if(_retry > MAX_RETRY_NUMBER){
			sendData(beerTemp,beerSet,fridgeTemp,fridgeSet);
			_lastUpdate=now;
			_retry =0;
		}else{
			_lastUpdate=now - _period + RETRY_TIME; 
			_retry ++;
		}
	}
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

void DataLogger::sendData(float beerTemp,float beerSet,float fridgeTemp, float fridgeSet)
{
	char data[512];
	int len=0;
	int n;
	len  =copyTemp(data,_btname,beerTemp,false);
	len +=copyTemp(data+len,_bsname,beerSet,len!=0);
	len +=copyTemp(data+len,_ftname,fridgeTemp,len!=0);
	len +=copyTemp(data+len,_fsname,fridgeSet,len!=0);
	if(len==0) return;

	if(_extra!=NULL){
		data[len++]='&';
		strcpy(data+len,_extra);
		len += strlen(_extra);
	}
	data[len]='\0';

	DBG_PRINTF("data= %d, \"%s\"\n",len,data);

	int code;
	HTTPClient _http;
  	_http.setUserAgent(F("ESP8266"));

	if(strcmp(_method,"POST")==0){
		// post
	 
 		_http.begin(_url);
  		_http.addHeader("Content-Type", "application/x-www-form-urlencoded");
		DBG_PRINTF("[HTTP] POST...\n");
    // start connection and send HTTP header
    	code = _http.POST((uint8_t*)data,len);
    }else{
 		_http.begin(String(_url) + String("?") + String(data));

		DBG_PRINTF("[HTTP] GET...\n");
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
//    String output=_http.getString();
//    DBG_PRINTF("output:\n%s\n",output.c_str());

}


bool DataLogger::processJson(char* jsonstring)
{
	DynamicJsonBuffer jsonBuffer(GSLOG_JSON_BUFFER_SIZE);
	JsonObject& root = jsonBuffer.parseObject(jsonstring);
	if(!root.success()
		|| !root.containsKey("enabled")
		|| !root.containsKey("url")
		|| !root.containsKey("bs")
		|| !root.containsKey("bt")
		|| !root.containsKey("fs")
		|| !root.containsKey("ft")
		|| !root.containsKey("extra")
		|| !root.containsKey("method")
		|| !root.containsKey("period")){
		_enabled=false;
		return false;
	}
	_enabled= root["enabled"];
	_period = root["period"];

	const char *url=root["url"];
	const char *method=root["method"];
	const char *extra=root["extra"];
	const char *bsname=root["bs"]; 
	const char *btname=root["bt"]; 
	const char *fsname=root["fs"]; 
	const char *ftname=root["ft"]; 

	if(url == NULL || method==NULL || strcmp(url,"") ==0 || strcmp(method,"") ==0){
		_enabled=false;
		return false;
	}
	#define COPYSTRING(a) if(_##a) free(_##a); _##a = (a==NULL || strcmp(a,"") ==0)? NULL:strdup(a)

	COPYSTRING(url);
	COPYSTRING(method);
	COPYSTRING(extra);
	COPYSTRING(bsname);
	COPYSTRING(btname);
	COPYSTRING(fsname);
	COPYSTRING(ftname);
  	
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

static const char LogConfigHtml[] PROGMEM =
R"END(
<html>
<head>
<title>Logging Setting</title>
<script>var logurl="/log";function s_ajax(a){var d=new XMLHttpRequest();d.onreadystatechange=function(){if(d.readyState==4){if(d.status==200){a.success(d.responseText)}else{d.onerror(d.status)}}};d.ontimeout=function(){if(typeof a.timeout!="undefined"){a.timeout()}else{d.onerror(-1)}},d.onerror=function(b){if(typeof a.fail!="undefined"){a.fail(b)}};d.open(a.m,a.url,true);if(typeof a.data!="undefined"){d.setRequestHeader("Content-Type",(typeof a.mime!="undefined")?a.mime:"application/x-www-form-urlencoded");d.send(a.data)}else{d.send()}}function checkurl(a){if(a.value.trim().startsWith("https")){alert("HTTPS is not supported")}}function mothod(b){var a;if(b.id=="m_get"){a="m_post"}else{a="m_get"}document.getElementById(a).checked=!b.checked}function update(){var a={};a.enabled=document.getElementById("enabled").checked;a.url=encodeURIComponent(document.getElementById("url").value);a.bs=document.getElementById("bs").value;a.bt=document.getElementById("bt").value;a.fs=document.getElementById("fs").value;a.ft=document.getElementById("ft").value;a.extra=encodeURIComponent(document.getElementById("extra").value);a.period=document.getElementById("period").value;a.method=(document.getElementById("m_post").checked)?"POST":"GET";s_ajax({url:logurl,m:"POST",data:"data="+JSON.stringify(a),success:function(b){alert("done")},fail:function(b){alert("failed:"+e)}})}function load(){s_ajax({url:logurl+"?data=1",m:"GET",success:function(b){var a=JSON.parse(b);document.getElementById("enabled").checked=a.enabled;document.getElementById((a.method=="POST")?"m_post":"m_get").checked=true;document.getElementById("url").value=(a.url===undefined)?"":a.url;document.getElementById("bt").value=(a.bt===undefined)?"":a.bt;document.getElementById("bs").value=(a.bs===undefined)?"":a.bs;document.getElementById("ft").value=(a.ft===undefined)?"":a.ft;document.getElementById("fs").value=(a.fs===undefined)?"":a.fs;document.getElementById("extra").value=(a.extra===undefined)?"":a.extra;document.getElementById("period").value=(a.period===undefined)?300:a.period},fail:function(a){alert("error :"+a)}})};</script>
</head>
<body onload="load()">
<form>
<table>
<tr><th>Enabled:</th><td><input type="checkbox" id="enabled" value="yes"></td></tr>
<tr><th>Method:</th><td><input type="checkbox" id="m_get" name="method" value="GET" onchange="mothod(this)">Get <input type="checkbox" id="m_post" name="method" value="POST" onchange="mothod(this)">Post </td></tr>
<tr><th>URL:</th><td><input type="text" id="url" size="64" placeholder="input link" onchange="checkurl(this)"></td></tr>
<tr><th>Beer Temperature Name:</th><td><input type="text" id="bt" size="20"></td></tr>
<tr><th>Beer Set Temperature Name:</th><td><input type="text" id="bs" size="20"></td></tr>
<tr><th>Fridge Temperature Name:</th><td><input type="text" id="ft" size="20"></td></tr>
<tr><th>Fridge Set Temperature Name:</th><td><input type="text" id="fs" size="20"></td></tr>
<tr><th>Extra parameter:</th><td><input type="text" id="extra" size="56"></td></tr>
<tr><th>Log time period:</th><td><input type="text" id="period" size="4">Seconds</td></tr>
<tr><th></th><td><button type="button" onclick="update()">Update</button></td></tr>
</table>
</form>
</body>
</html>
)END";

void DataLogger::getSettings(AsyncWebServerRequest *request)
{
	if(request->hasParam("data")){
		if(SPIFFS.exists(GSLogConfigFile))
			request->send(SPIFFS, GSLogConfigFile);
		else
			request->send(200,"application/json","{}");
	}else{
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
	}
}





