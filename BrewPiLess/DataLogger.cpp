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

	len =dataSprintf(data,_format,beerTemp,beerSet,fridgeTemp,fridgeSet);
	
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

}

int DataLogger::dataSprintf(char *buffer,const char *format,float beerTemp,float beerSet,float fridgeTemp,float fridgeSet)
{
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
				if(IS_FLOAT_TEMP_VALID(beerTemp)){
					d += sprintFloat(buffer+d,beerTemp,1);
				}else{
					strcpy(buffer+d,"null");
					d += 4;
				}
			}else if(ch == 'B'){
				if(IS_FLOAT_TEMP_VALID(beerSet)){
					d += sprintFloat(buffer+d,beerSet,1);
				}else{
					strcpy(buffer+d,"null");
					d += 4;
				}
			}else if(ch == 'f'){
				if(IS_FLOAT_TEMP_VALID(fridgeTemp)){
					d += sprintFloat(buffer+d,fridgeTemp,1);
				}else{
					strcpy(buffer+d,"null");
					d += 4;
				}
			}else if(ch == 'F'){
				if(IS_FLOAT_TEMP_VALID(fridgeSet)){
					d += sprintFloat(buffer+d,fridgeSet,1);
				}else{
					strcpy(buffer+d,"null");
					d += 4;
				}
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
static const char LogConfigHtml[] PROGMEM =
R"END(
<html>
<head>
<title>Logging Setting</title>
<script>/*<![CDATA[*/
var logurl="log";function s_ajax(b){var c=new XMLHttpRequest();c.onreadystatechange=function(){if(c.readyState==4){if(c.status==200){b.success(c.responseText)}else{c.onerror(c.status)}}};c.ontimeout=function(){if(typeof b["timeout"]!="undefined")b.timeout();else c.onerror(-1)},c.onerror=function(a){if(typeof b["fail"]!="undefined")b.fail(a)};c.open(b.m,b.url,true);if(typeof b["data"]!="undefined"){c.setRequestHeader("Content-Type",(typeof b["mime"]!="undefined")?b["mime"]:"application/x-www-form-urlencoded");c.send(b.data)}else c.send()}var EI=function(i){return document.getElementById(i)};Number.prototype.format=function(n,x,s,c){var a='\\d(?=(\\d{'+(x||3)+'})+'+(n>0?'\\D':'$')+')',num=this.toFixed(Math.max(0,~~n));return(c?num.replace('.',c):num).replace(new RegExp(a,'g'),'$&'+(s||','))};String.prototype.escapeJSON=function(){return this.replace(/[\\]/g,'\\\\').replace(/[\"]/g,'\\\"').replace(/[\/]/g,'\\/').replace(/[\b]/g,'\\b').replace(/[\f]/g,'\\f').replace(/[\n]/g,'\\n').replace(/[\r]/g,'\\r').replace(/[\t]/g,'\\t')};var logs={url:"loglist.php",rmurl:"loglist.php?rm=",starturl:"loglist.php?start=",stopurl:"loglist.php?stop=1",dlurl:"loglist.php?dl=",vurl:"viewlog.htm?dl=",ll:[],fs:{},logging:false,vname:function(a){if(a=="")return false;if(a.match(/[\W]/g))return false;return true},dupname:function(b){var c=false;this.ll.forEach(function(a){if(b==a.name)c=true});return c},fsinfo:function(s,u){EI("fssize").innerHTML=s.format(0,3,',');EI("fsused").innerHTML=u.format(0,3,',');EI("fsfree").innerHTML=(s-u).format(0,3,',')},slog:function(){var t=this;if(t.logging){if(confirm("Stop current logging?")){var n=EI("logname").value.trim();s_ajax({url:t.stopurl+n,m:"GET",success:function(d){location.reload()},fail:function(d){alert("Failed to stop for:"+d)}})}}else{if(t.ll.length>=10){alert("Too many logs. Delete some before creating new.");return}if((t.fs.size-t.fs.used)<=t.fs.block*2){alert("Not enough free space!");return}var a=EI("logname").value.trim();if(t.vname(a)===false){alert("Invalid file name, no special characters allowed.");return}if(t.dupname(a)){alert("Duplicated name.");return}if(confirm("Start new logging?")){s_ajax({url:t.starturl+a,m:"GET",success:function(d){location.reload()},fail:function(d){alert("Failed to start for:"+d)}})}}},recording:function(n,t){this.logging=true;var d=new Date(t*1000);EI("logtitle").innerHTML="Recording since <b>"+d.toLocaleString()+"</b> ";var l=EI("logname");l.value=n;l.disabled=true;EI("logbutton").innerHTML="STOP Logging"},stop:function(){this.logging=false;EI("logtitle").innerHTML="New Log Name:";var l=EI("logname");l.value="";l.disabled=false;EI("logbutton").innerHTML="Start Logging"},view:function(n){window.open(this.vurl+n)},rm:function(n){var t=this;if(confirm("Delete the log "+t.ll[n].name)){console.log("rm "+t.ll[n].name);s_ajax({url:t.rmurl+n,m:"GET",success:function(d){var r=JSON.parse(d);t.fs=r;t.fsinfo(r.size,r.used);t.ll.splice(n,1);t.list(t.ll)},fail:function(d){alert("Failed to delete for:"+d)}})}},dl:function(n){window.open(this.dlurl+n)},list:function(l){var e=EI("loglist").querySelector("tbody");var f;while(f=e.querySelector("tr:nth-of-type(2)"))e.removeChild(f);var t=this;var g=t.row;l.forEach(function(i,a){var b=i.name;var c=new Date(i.time*1000);var d=g.cloneNode(true);d.querySelector(".logid").innerHTML=b;d.querySelector(".logdate").innerHTML=c.toLocaleString();d.querySelector(".dlbutton").onclick=function(){t.dl(a)};d.querySelector(".viewbutton").onclick=function(){t.view(a)};d.querySelector(".rmbutton").onclick=function(){t.rm(a)};e.appendChild(d)})},init:function(){var t=this;EI("logbutton").onclick=function(){t.slog()};t.row=EI("loglist").querySelector("tr:nth-of-type(2)");t.row.parentNode.removeChild(t.row);s_ajax({url:t.url,m:"GET",success:function(d){var r=JSON.parse(d);t.fs=r.fs;if(r.rec)t.recording(r.log,r.start);t.ll=r.list;t.list(r.list);t.fsinfo(r.fs.size,r.fs.used)},fail:function(d){alert("failed:"+e)}})},};function checkurl(t){if(t.value.trim().startsWith("https")){alert("HTTPS is not supported")}}function checkformat(a){if(a.value.length>256){a.value=t.value.substring(0,256)}EI("fmthint").innerHTML=""+a.value.length+"/256"}function mothod(c){var a=document.querySelectorAll('input[name$="method"]');for(var i=0;i<a.length;i++){if(a[i].id!=c.id)a[i].checked=false}window.selectedMethod=c.value}function update(){if(typeof window.selectedMethod=="undefined"){alert("select Method!");return}var a=EI("format").value.trim();if(window.selectedMethod=="GET"){var b=new RegExp("\s","g");if(b.exec(a)){alert("space is not allowed");return}}var r={};r.enabled=EI("enabled").checked;r.url=EI("url").value.trim();r.format=encodeURIComponent(a.escapeJSON());r.period=EI("period").value;r.method=(EI("m_post").checked)?"POST":"GET";r.type=EI("data-type").value.trim();s_ajax({url:logurl,m:"POST",data:"data="+JSON.stringify(r),success:function(d){alert("done")},fail:function(d){alert("failed:"+e)}})}function load(){s_ajax({url:logurl+"?data=1",m:"GET",success:function(d){var r=JSON.parse(d);EI("enabled").checked=r.enabled;window.selectedMethod=r.method;EI("m_"+r.method.toLowerCase()).checked=true;EI("url").value=(r.url===undefined)?"":r.url;EI("data-type").value=(r.type===undefined)?"":r.type;EI("format").value=(r.format===undefined)?"":r.format;checkformat(EI("format"));EI("period").value=(r.period===undefined)?300:r.period}});logs.init()}/*]]>*/</script>
<style>#loglist td,#loglist tr,#loglist th,#loglist{border:1px solid black}fieldset{margin:10px}#fsinfo{margin:10px}</style>
</head>
<body onload="load()">
<fieldset>
<legend>Remote Log</legend>
<form>
<table>
<tr><th>Enabled:</th><td><input type="checkbox" id="enabled" value="yes"></td></tr>
<tr><th>Method:</th><td><input type="checkbox" id="m_get" name="method" value="GET" onchange="mothod(this)">Get
<input type="checkbox" id="m_post" name="method" value="POST" onchange="mothod(this)">Post
<input type="checkbox" id="m_put" name="method" value="PUT" onchange="mothod(this)">Put </td></tr>
<tr><th>URL:</th><td><input type="text" id="url" size="128" placeholder="input link" onchange="checkurl(this)"></td></tr>
<tr><th></th><td>JSON:"application/json", Form Type:"application/x-www-form-urlencoded"</td></tr>
<tr><th>Data Type:</th><td><input type="text" id="data-type" size="42" placeholder="Content-Type"</td></tr>
<tr><th></th><td>%b:beer temp, %B:beer setting,%f:fridge temp, %F:fridge setting</td></tr>
<tr><th>Format:</th><td><textarea id="format" rows="4" cols="64" oninput="checkformat(this)"></textarea></td></tr>
<tr><th></th><td>Characters:<span id="fmthint"></span></td></tr>
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














































































