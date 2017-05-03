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
static const char LogConfigHtml[] PROGMEM =
R"END(
<html>
<head>
<title>Logging Setting</title>
<script>/*<![CDATA[*/var logurl="log";function s_ajax(a){var d=new XMLHttpRequest();d.onreadystatechange=function(){if(d.readyState==4){if(d.status==200){a.success(d.responseText)}else{d.onerror(d.status)}}};d.ontimeout=function(){if(typeof a.timeout!="undefined"){a.timeout()}else{d.onerror(-1)}},d.onerror=function(b){if(typeof a.fail!="undefined"){a.fail(b)}};d.open(a.m,a.url,true);if(typeof a.data!="undefined"){d.setRequestHeader("Content-Type",(typeof a.mime!="undefined")?a.mime:"application/x-www-form-urlencoded");d.send(a.data)}else{d.send()}}var EI=function(a){return document.getElementById(a)};Number.prototype.format=function(h,a,f,g){var d="\\d(?=(\\d{"+(a||3)+"})+"+(h>0?"\\D":"$")+")",b=this.toFixed(Math.max(0,~~h));return(g?b.replace(".",g):b).replace(new RegExp(d,"g"),"$&"+(f||","))};String.prototype.escapeJSON=function(){return this.replace(/[\\]/g,"\\\\").replace(/[\"]/g,'\\"').replace(/[\/]/g,"\\/").replace(/[\b]/g,"\\b").replace(/[\f]/g,"\\f").replace(/[\n]/g,"\\n").replace(/[\r]/g,"\\r").replace(/[\t]/g,"\\t")};var logs={url:"loglist.php",rmurl:"loglist.php?rm=",starturl:"loglist.php?start=",stopurl:"loglist.php?stop=1",dlurl:"loglist.php?dl=",vurl:"viewlog.htm?dl=",ll:[],fs:{},logging:false,vname:function(a){if(a==""){return false}if(a.match(/[\W]/g)){return false}return true},dupname:function(b){var a=false;this.ll.forEach(function(c){if(b==c.name){a=true}});return a},fsinfo:function(b,a){EI("fssize").innerHTML=b.format(0,3,",");EI("fsused").innerHTML=a.format(0,3,",");EI("fsfree").innerHTML=(b-a).format(0,3,",")},slog:function(){var b=this;if(b.logging){if(confirm("Stop current logging?")){var c=EI("logname").value.trim();s_ajax({url:b.stopurl+c,m:"GET",success:function(f){location.reload()},fail:function(f){alert("Failed to stop for:"+f)}})}}else{if(b.ll.length>=10){alert("Too many logs. Delete some before creating new.");return}if((b.fs.size-b.fs.used)<=b.fs.block*2){alert("Not enough free space!");return}var a=EI("logname").value.trim();if(b.vname(a)===false){alert("Invalid file name, no special characters allowed.");return}if(b.dupname(a)){alert("Duplicated name.");return}if(confirm("Start new logging?")){s_ajax({url:b.starturl+a,m:"GET",success:function(f){location.reload()},fail:function(f){alert("Failed to start for:"+f)}})}}},recording:function(f,b){this.logging=true;var c=new Date(b*1000);EI("logtitle").innerHTML="Recording since <b>"+c.toLocaleString()+"</b> ";var a=EI("logname");a.value=f;a.disabled=true;EI("logbutton").innerHTML="STOP Logging"},stop:function(){this.logging=false;EI("logtitle").innerHTML="New Log Name:";var a=EI("logname");a.value="";a.disabled=false;EI("logbutton").innerHTML="Start Logging"},view:function(a){window.open(this.vurl+a)},rm:function(b){var a=this;if(confirm("Delete the log "+a.ll[b].name)){console.log("rm "+a.ll[b].name);s_ajax({url:a.rmurl+b,m:"GET",success:function(f){var c=JSON.parse(f);a.fs=c;a.fsinfo(c.size,c.used);a.ll.splice(b,1);a.list(a.ll)},fail:function(c){alert("Failed to delete for:"+c)}})}},dl:function(a){window.open(this.dlurl+a)},list:function(b){var a=EI("loglist").querySelector("tbody");var d;while(d=a.querySelector("tr:nth-of-type(2)")){a.removeChild(d)}var c=this;var f=c.row;b.forEach(function(k,g){var j=k.name;var h=new Date(k.time*1000);var l=f.cloneNode(true);l.querySelector(".logid").innerHTML=j;l.querySelector(".logdate").innerHTML=h.toLocaleString();l.querySelector(".dlbutton").onclick=function(){c.dl(g)};l.querySelector(".viewbutton").onclick=function(){c.view(g)};l.querySelector(".rmbutton").onclick=function(){c.rm(g)};a.appendChild(l)})},init:function(){var a=this;EI("logbutton").onclick=function(){a.slog()};a.row=EI("loglist").querySelector("tr:nth-of-type(2)");a.row.parentNode.removeChild(a.row);s_ajax({url:a.url,m:"GET",success:function(c){var b=JSON.parse(c);a.fs=b.fs;if(b.rec){a.recording(b.log,b.start)}a.ll=b.list;a.list(b.list);a.fsinfo(b.fs.size,b.fs.used)},fail:function(b){alert("failed:"+e)}})},};function checkurl(a){if(a.value.trim().startsWith("https")){alert("HTTPS is not supported")}}function checkformat(a){if(a.value.length>256){a.value=t.value.substring(0,256)}EI("fmthint").innerHTML=""+a.value.length+"/256"}function mothod(d){var a=document.querySelectorAll('input[name$="method"]');for(var b=0;b<a.length;b++){if(a[b].id!=d.id){a[b].checked=false}}window.selectedMethod=d.value}function update(){if(typeof window.selectedMethod=="undefined"){alert("select Method!");return}var b=EI("format").value.trim();if(window.selectedMethod=="GET"){var c=new RegExp("s","g");if(c.exec(b)){alert("space is not allowed");return}}var a={};a.enabled=EI("enabled").checked;a.url=EI("url").value.trim();a.format=encodeURIComponent(b.escapeJSON());a.period=EI("period").value;a.method=(EI("m_post").checked)?"POST":"GET";a.type=EI("data-type").value.trim();s_ajax({url:logurl,m:"POST",data:"data="+JSON.stringify(a),success:function(f){alert("done")},fail:function(f){alert("failed:"+e)}})}function load(){s_ajax({url:logurl+"?data=1",m:"GET",success:function(b){var a=JSON.parse(b);EI("enabled").checked=a.enabled;window.selectedMethod=a.method;EI("m_"+a.method.toLowerCase()).checked=true;EI("url").value=(a.url===undefined)?"":a.url;EI("data-type").value=(a.type===undefined)?"":a.type;EI("format").value=(a.format===undefined)?"":a.format;checkformat(EI("format"));EI("period").value=(a.period===undefined)?300:a.period}});logs.init()}function showformat(a){var b=EI("formatlist");var c=a.getBoundingClientRect();b.style.display="block";b.style.left=(c.left)+"px";b.style.top=(c.top+100)+"px"}function hideformat(){EI("formatlist").style.display="none"};/*]]>*/</script>
<style>#loglist td,#loglist tr,#loglist th,#loglist{border:1px solid black}fieldset{margin:10px}#fsinfo{margin:10px}#formatlist{display:none;position:absolute;border:1px solid whtie;background:lightgray}#formatlist table,#formatlist td,#formatlist th{border:1px solid black;border-collapse:collapse}</style>
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
<tr><th></th><td><span onmouseover="showformat(this)" onmouseout="hideformat()"><u>Notations...</u></span></td></tr>
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
<div id="formatlist">
<table>
<tr><th>%b</th><td>beer temp</td></tr>
<tr><th>%B</th><td>beer setting</td></tr>
<tr><th>%f</th><td>fridge temp</td></tr>
<tr><th>%F</th><td>fridge setting</td></tr>
<tr><th>%r</th><td>room temp</td></tr>
<tr><th>%g</th><td>gravity</td></tr>
<tr><th>%a</th><td>Aux temp.</td></tr>
<tr><th>%v</th><td>device voltage</td></tr>
<tr><th>%u</th><td>Unix timestamp of last gravity update</td></tr>
</table>
</div>
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


















































































































































































































































































































































