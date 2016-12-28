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
<script>/*<![CDATA[*/var logurl="log.php";function s_ajax(a){var d=new XMLHttpRequest();d.onreadystatechange=function(){if(d.readyState==4){if(d.status==200){a.success(d.responseText)}else{d.onerror(d.status)}}};d.ontimeout=function(){if(typeof a.timeout!="undefined"){a.timeout()}else{d.onerror(-1)}},d.onerror=function(b){if(typeof a.fail!="undefined"){a.fail(b)}};d.open(a.m,a.url,true);if(typeof a.data!="undefined"){d.setRequestHeader("Content-Type",(typeof a.mime!="undefined")?a.mime:"application/x-www-form-urlencoded");d.send(a.data)}else{d.send()}}var EI=function(a){return document.getElementById(a)};Number.prototype.format=function(i,b,g,h){var d="\\d(?=(\\d{"+(b||3)+"})+"+(i>0?"\\D":"$")+")",f=this.toFixed(Math.max(0,~~i));return(h?f.replace(".",h):f).replace(new RegExp(d,"g"),"$&"+(g||","))};var logs={url:"loglist.php",rmurl:"loglist.php?rm=",starturl:"loglist.php?start=",stopurl:"loglist.php?stop=1",dlurl:"loglist.php?dl=",vurl:"viewlog.htm?dl=",ll:[],fs:{},logging:false,vname:function(b){if(b==""){return false}if(b.match(/[\W]/g)){return false}return true},dupname:function(a){var d=false;this.ll.forEach(function(b){if(a==b.name){d=true}});return d},fsinfo:function(b,a){EI("fssize").innerHTML=b.format(0,3,",");EI("fsused").innerHTML=a.format(0,3,",");EI("fsfree").innerHTML=(b-a).format(0,3,",")},slog:function(){var c=this;if(c.logging){if(confirm("Stop current logging?")){var d=EI("logname").value.trim();s_ajax({url:c.stopurl+d,m:"GET",success:function(a){location.reload()},fail:function(a){alert("Failed to stop for:"+a)}})}}else{if(c.ll.length>=10){alert("Too many logs. Delete some before creating new.");return}if((c.fs.size-c.fs.used)<=c.fs.block*2){alert("Not enough free space!");return}var b=EI("logname").value.trim();if(c.vname(b)===false){alert("Invalid file name, no special characters allowed.");return}if(c.dupname(b)){alert("Duplicated name.");return}if(confirm("Start new logging?")){s_ajax({url:c.starturl+b,m:"GET",success:function(a){location.reload()},fail:function(a){alert("Failed to start for:"+a)}})}}},recording:function(f,b){this.logging=true;var c=new Date(b*1000);EI("logtitle").innerHTML="Recording since <b>"+c.toLocaleString()+"</b> ";var a=EI("logname");a.value=f;a.disabled=true;EI("logbutton").innerHTML="STOP Logging"},stop:function(){this.logging=false;EI("logtitle").innerHTML="New Log Name:";var a=EI("logname");a.value="";a.disabled=false;EI("logbutton").innerHTML="Start Logging"},view:function(a){window.open(this.vurl+a)},rm:function(b){var a=this;if(confirm("Delete the log "+a.ll[b].name)){console.log("rm "+a.ll[b].name);s_ajax({url:a.rmurl+b,m:"GET",success:function(f){var c=JSON.parse(f);a.fs=c;a.fsinfo(c.size,c.used);a.ll.splice(b,1);a.list(a.ll)},fail:function(c){alert("Failed to delete for:"+c)}})}},dl:function(a){window.open(this.dlurl+a)},list:function(a){var h=EI("loglist").querySelector("tbody");var d;while(d=h.querySelector("tr:nth-of-type(2)")){h.removeChild(d)}var b=this;var c=b.row;a.forEach(function(j,g){var f=j.name;var l=new Date(j.time*1000);var k=c.cloneNode(true);k.querySelector(".logid").innerHTML=f;k.querySelector(".logdate").innerHTML=l.toLocaleString();k.querySelector(".dlbutton").onclick=function(){b.dl(g)};k.querySelector(".viewbutton").onclick=function(){b.view(g)};k.querySelector(".rmbutton").onclick=function(){b.rm(g)};h.appendChild(k)})},init:function(){var a=this;EI("logbutton").onclick=function(){a.slog()};a.row=EI("loglist").querySelector("tr:nth-of-type(2)");a.row.parentNode.removeChild(a.row);s_ajax({url:a.url,m:"GET",success:function(c){var b=JSON.parse(c);a.fs=b.fs;if(b.rec){a.recording(b.log,b.start)}a.ll=b.list;a.list(b.list);a.fsinfo(b.fs.size,b.fs.used)},fail:function(b){alert("failed:"+e)}})},};function checkurl(a){if(a.value.trim().startsWith("https")){alert("HTTPS is not supported")}}function mothod(b){var a;if(b.id=="m_get"){a="m_post"}else{a="m_get"}EI(a).checked=!b.checked}function update(){var a={};a.enabled=EI("enabled").checked;a.url=encodeURIComponent(EI("url").value);a.bs=EI("bs").value;a.bt=EI("bt").value;a.fs=EI("fs").value;a.ft=EI("ft").value;a.extra=encodeURIComponent(EI("extra").value);a.period=EI("period").value;a.method=(EI("m_post").checked)?"POST":"GET";s_ajax({url:logurl,m:"POST",data:"data="+JSON.stringify(a),success:function(b){alert("done")},fail:function(b){alert("failed:"+e)}})}function load(){s_ajax({url:logurl+"?data=1",m:"GET",success:function(b){var a=JSON.parse(b);EI("enabled").checked=a.enabled;EI((a.method=="POST")?"m_post":"m_get").checked=true;EI("url").value=(a.url===undefined)?"":a.url;EI("bt").value=(a.bt===undefined)?"":a.bt;EI("bs").value=(a.bs===undefined)?"":a.bs;EI("ft").value=(a.ft===undefined)?"":a.ft;EI("fs").value=(a.fs===undefined)?"":a.fs;EI("extra").value=(a.extra===undefined)?"":a.extra;EI("period").value=(a.period===undefined)?300:a.period}});logs.init()};/*]]>*/</script>
<style>#loglist td,#loglist tr,#loglist th,#loglist{border:1px solid black}fieldset{margin:10px}#fsinfo{margin:10px}</style>
</head>
<body onload="load()">
<fieldset>
<legend>Remote Log</legend>
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
</fieldset>
<fieldset>
<legend>Local Log</legend>
<span id="logtitle">New Log Name:</span><input type="text" id="logname" size="24" maxlength="24"></input> <button id="logbutton">Start Log</button>
<div id="fsinfo">
Free Space: <span id="fsfree">0</span> Bytes, Used Space: <span id="fsused">0</span> Bytes, Total Space: <span id="fssize">0</span> Bytes
</div>
<table id="loglist">
<tr><th style="width:30%">Log</th><th style="width:40%">Date</th><th>Action</th></tr>
<tr><td class="logid"></td><td class="logdate"></td><td><button class="dlbutton">Download</button><button class="viewbutton">View</button><button class="rmbutton">Delete</button></td></tr>
</table>
</fieldset>
</body>
</html>
)END";

/*
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
*/

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



















