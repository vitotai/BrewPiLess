#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "GSLogger.h"
#include "espconfig.h"
#define GSLOG_JSON_BUFFER_SIZE 256
#define MAX_GSLOG_CONFIG_LEN 1024

//const char *GScriptId = "AKfycbyakVKf5sIrXz8fuxDBQAJzg1saqEYCI71eY-QaGqQQ-QziXclm";
//https://script.google.com/macros/s/AKfycbyakVKf5sIrXz8fuxDBQAJzg1saqEYCI71eY-QaGqQQ-QziXclm/exec
//const char* fingerprint = "39 6B 80 5A C1 F6 FD 8A 1E 2A AA 02 B8 99 98 32 2B 1F 10 83";

#define GSLogConfigFile "/gslog.cfg"

const char* fingerprint = "39:6B:80:5A:C1:F6:FD:8A:1E:2A:AA:02:B8:99:98:32:2B:1F:10:83";
// Write to Google Spreadsheet
//String data=String("bt=28.3&bs=18.0&ft=28.1&fs=1.0&ss=1Zq2vR8DL5Xr_95H6LiLrpHZqwIm9rbvMX84UeI6hhNU&st=bluemoon&pc=thisistest");

void GSLogger::loop(time_t now,void (*getTemp)(float *pBeerTemp,float *pBeerSet,float *pFridgeTemp, float *pFridgeSet))
{
	if(!_enabled) return;
	
	if((now - _lastUpdate) < _period) return;
	_lastUpdate=now;

	float beerTemp,beerSet,fridgeTemp,fridgeSet;
	(*getTemp)(&beerTemp,&beerSet,&fridgeTemp,&fridgeSet);
	
	String url = String("https://script.google.com/macros/s/") + String(_scriptid) + "/exec";
	
	String data=String("bt=") + String(beerTemp)
				+String("&bs=") + String(beerSet)
				+String("&ft=") + String(fridgeTemp)
				+String("&fs=") + String(fridgeSet)
				+String("&ss=") + String(_spreadsheetid)
				+String("&st=") + String(_sheetname)
				+String("&pc=") + String(_passcode);
	 
	HTTPClient _http;
  	_http.setUserAgent(F("ESP8266"));
 	_http.begin(url,fingerprint);

	DBG_PRINTF("[HTTPS] POST...\n");
    // start connection and send HTTP header
    int code = _http.POST(data);
    
    if(code <= 0) {
        DBG_PRINTF("HTTP error: %s\n", _http.errorToString(code).c_str());
        _http.end();
        return;
    }
      // HTTP header has been send and Server response header has been handled
    DBG_PRINTF("[HTTP] Post... code: %d\n", code);
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

bool GSLogger::processJson(char* jsonstring)
{
	DynamicJsonBuffer jsonBuffer(GSLOG_JSON_BUFFER_SIZE);
	JsonObject& root = jsonBuffer.parseObject(jsonstring);
	if(!root.success()
		|| !root.containsKey("enabled")
		|| !root.containsKey("app")
		|| !root.containsKey("pass")
		|| !root.containsKey("ssid")
		|| !root.containsKey("sheet")
		|| !root.containsKey("period")){
		_enabled=false;
		return false;		
	}
	_enabled= root["enabled"];
	_period = root["period"];
	const char *app=root["app"];
	const char *pass=root["pass"];
	const char *ssid=root["ssid"];
	const char *sheet=root["sheet"]; 
	if(app == NULL || pass==NULL || ssid== NULL || sheet==NULL
	|| strcmp(app,"") ==0 || strcmp(pass,"") ==0 || strcmp(ssid,"") ==0  || strcmp(sheet,"") ==0 ){
		_enabled=false;
		return false;
	}
	_scriptid=strdup(app);
    _spreadsheetid=strdup(ssid);
    _sheetname=strdup(sheet);
    _passcode=strdup(pass);
  	
  	return true;
}

void GSLogger::loadConfig(void)
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

void GSLogger::updateSetting(AsyncWebServerRequest *request)
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
<script>
var logurl="/log";
var script_prefix="https://script.google.com/macros/s/";
var sheet_prefix="https://docs.google.com/spreadsheets/d/"; 
function s_ajax(b){var c=new XMLHttpRequest();c.onreadystatechange=function(){if(c.readyState==4){if(c.status==200){b.success(c.responseText)}else{c.onerror(c.status)}}};c.ontimeout=function(){if(typeof b["timeout"]!="undefined")b.timeout();else c.onerror(-1)},c.onerror=function(a){if(typeof b["fail"]!="undefined")b.fail(a)};c.open(b.m,b.url,true);if(typeof b["data"]!="undefined"){c.setRequestHeader("Content-Type",(typeof b["mime"]!="undefined")?b["mime"]:"application/x-www-form-urlencoded");c.send(b.data)}else c.send()}

function checkid(t,p)
{
	if(t.value.trim().startsWith(p)){
		var i=t.value.trim().substr(p.length);
		t.value=i.substr(0, i.search("/"));
	}
}
function update()
{
	var r={};
	r.enabled= document.getElementById("enabled").checked;
	r.app=document.getElementById("appid").value;
	r.pass=document.getElementById("pass").value;
	r.ssid=document.getElementById("ssid").value;
	r.sheet=document.getElementById("sheet").value;
	r.period=document.getElementById("period").value;
	//console.log(JSON.stringify(r));
	 s_ajax({
 	url:logurl, m:"POST", data:"data="+ JSON.stringify(r),
 	success:function(d){alert("done");},
 	fail:function(d){alert("failed:"+e);}});

}
function load()
{
 s_ajax({
 	url:logurl + "?data=1", m:"GET",
 	success:function(d){
 	var r= JSON.parse(d);
	document.getElementById("enabled").checked = r.enabled;
	document.getElementById("appid").value = r.app;
	document.getElementById("pass").value = r.pass;
	document.getElementById("ssid").value = r.ssid;
	document.getElementById("sheet").value = r.sheet;
	document.getElementById("period").value = r.period;
 	},
 	fail:function(d){
 		alert("error :"+d);
 	}});
}

</script>
</head>
<body onload="load();">
<form>
<table>
<tr><th>Enabled:</th><td><input type="checkbox" id="enabled" value="yes"></td></tr>
<tr><th>Google Script APP Link:</th><td><input type="text" id="appid" size="64" placeholder="input link or id" onchange="checkid(this,script_prefix);"></td></tr>
<tr><th>Pass Code:</th><td><input type="text" id="pass" size="56" placeholder="input passcode"></td></tr>
<tr><th>Google Sheet Link:</th><td><input type="text" id="ssid" size="64" placeholder="input link or id" onchange="checkid(this,sheet_prefix);"></td></tr>
<tr><th>Sheet Label/Name:</th><td><input type="text" id="sheet" size="36"></td></tr>
<tr><th>Log time period:</th><td><input type="text" id="period" size="4">Seconds</td></tr>
<tr><th></th><td><button type="button"  onclick="update();">Update</button></td></tr>
</table>
</form>
</body>
</html>
)END";

void GSLogger::getSettings(AsyncWebServerRequest *request)
{
	if(request->hasParam("data")){
		if(SPIFFS.exists(GSLogConfigFile))
			request->send(SPIFFS, GSLogConfigFile);
		else
			request->send(200,"text/html","");
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

