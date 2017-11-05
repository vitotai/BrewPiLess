#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "espconfig.h"
//{ brewpi
#include <OneWire.h>

#include "Ticks.h"
#include "Display.h"
#include "TempControl.h"
#include "PiLink.h"
#include "Menu.h"
#include "Pins.h"
#include "RotaryEncoder.h"
#include "Buzzer.h"
#include "TempSensor.h"
#include "TempSensorMock.h"
#include "TempSensorExternal.h"
#include "Ticks.h"
#include "Sensor.h"
#include "SettingsManager.h"
#include "EepromFormat.h"

#if BREWPI_SIMULATE
#include "Simulator.h"
#endif

//}brewpi

#include "espconfig.h"
#include "TimeKeeper.h"
#include "mystrlib.h"

#include "GravityTracker.h"
#include "BrewKeeper.h"
#ifdef ENABLE_LOGGING
#include "DataLogger.h"
#endif

extern "C" {
#include <sntp.h>
}


#include "ESPUpdateServer.h"
#include "WiFiSetup.h"

#include "BrewPiProxy.h"

#include "BrewLogger.h"

#include "ExternalData.h"

//WebSocket seems to be unstable, at least on iPhone.
//Go back to ServerSide Event.
#define UseWebSocket false
#define UseServerSideEvent true
#define ResponseAppleCNA true
#define CaptivePortalTimeout 180
/**************************************************************************************/
/* Start of Configuration 															  */
/**************************************************************************************/
static const char DefaultConfiguration[] PROGMEM =
R"END(
{"name":"brewpiless",
"user":"brewpiless",
"pass":"brewpiless",
"title":"brewpiless",
"protect":0,
"ap":0
}
)END";

static const char configFormat[] PROGMEM =
R"END(
{"name":"%s",
"user":"%s",
"pass":"%s",
"title":"%s",
"protect":%d,
"ap":%d
}
)END";


#define MAX_CONFIG_LEN 1024
#define JSON_BUFFER_SIZE 1024



#define PROFILE_FILENAME "/brewing.json"
#define CONFIG_FILENAME "/brewpi.cfg"

#define WS_PATH 		"/websocket"
#define SSE_PATH 		"/getline"

#define POLLING_PATH 	"/getline_p"
#define PUTLINE_PATH	"/putline"
#define CONTROL_CC_PATH	"/tcc"

#ifdef ENABLE_LOGGING
#define LOGGING_PATH	"/log"
#endif

#define LOGLIST_PATH  "/loglist.php"
#define CHART_DATA_PATH "/chart.php"

#define CONFIG_PATH		"/config"
#define TIME_PATH       "/time"
#define RESETWIFI_PATH       "/erasewifisetting"

#define FPUTS_PATH       "/fputs"
#define FLIST_PATH       "/list"
#define DELETE_PATH       "/rm"

#define CHART_LIB_PATH       "/dygraph-combined.js"

#define GRAVITY_PATH       "/gravity"

#define GETSTATUS_PATH "/getstatus"

#define DEFAULT_INDEX_FILE     "index.htm"

const char *public_list[]={
"/bwf.js",
"/brewing.json"
};

const char *nocache_list[]={
"/brewing.json",
"/brewpi.cfg"
};
//*******************************************

ExternalData externalData;

GravityTracker gravityTracker;

bool passwordLcd;
bool stationApMode;
char username[32];
char password[32];
char hostnetworkname[32];
char titlelabel[32];

AsyncWebServer server(80);
BrewPiProxy brewPi;
BrewKeeper brewKeeper([](const char* str){ brewPi.putLine(str);});
#ifdef ENABLE_LOGGING
DataLogger dataLogger;
#endif

BrewLogger brewLogger;

#if UseServerSideEvent == true
AsyncEventSource sse(SSE_PATH);
#endif

// use in sprintf, put into PROGMEM complicates it.
const char confightml[] PROGMEM =R"END(
<html><head><title>Configuration</title></head><body>
<form action="" method="post">
<table>
<tr><td>Host/Network Name</td><td><input name="name" type="text" size="12" maxlength="16" value="%s"></td></tr>
<tr><td>User Name</td><td><input name="user" type="text" size="12" maxlength="16" value="%s"></td></tr>
<tr><td>Password</td><td><input name="pass" type="password" size="12" maxlength="16" value="%s"></td></tr>
<tr><td>Always need password</td><td><input type="checkbox" name="protect" value="yes" %s></td></tr>
<tr><td>Always softAP</td><td><input type="checkbox" name="ap" value="yes" %s></td></tr>
<tr><td>Save Change</td><td><input type="submit" name="submit"></input></td></tr>
</table></form></body></html>)END";

extern const uint8_t* getEmbeddedFile(const char* filename,bool &gzip, unsigned int &size);

void requestRestart(bool disc);

void initTime(bool apmode)
{
	if(apmode){
		TimeKeeper.begin();
	}else{
		TimeKeeper.begin((char*)"time.nist.gov",(char*)"time.windows.com",(char*)"de.pool.ntp.org");
	}
}

class BrewPiWebHandler: public AsyncWebHandler
{
	void handleFileList(AsyncWebServerRequest *request) {
		if(request->hasParam("dir",true)){
        	String path = request->getParam("dir",true)->value();
          	Dir dir = SPIFFS.openDir(path);
          	path = String();
          	String output = "[";
          	while(dir.next()){
            	File entry = dir.openFile("r");
            	if (output != "[") output += ',';
            		bool isDir = false;
            		output += "{\"type\":\"";
            		output += (isDir)?"dir":"file";
            		output += "\",\"name\":\"";
            		output += String(entry.name()).substring(1);
            		output += "\"}";
            		entry.close();
          	}
          	output += "]";
          	request->send(200, "text/json", output);
          	output = String();
        }
        else
          request->send(400);
	}

	void handleFileDelete(AsyncWebServerRequest *request){
		if(request->hasParam("path", true)){
        	ESP.wdtDisable(); SPIFFS.remove(request->getParam("path", true)->value()); ESP.wdtEnable(10);
            request->send(200, "", "DELETE: "+request->getParam("path", true)->value());
        } else
          request->send(404);
    }

	void handleFilePuts(AsyncWebServerRequest *request){
		if(request->hasParam("path", true)
			&& request->hasParam("content", true)){
        	ESP.wdtDisable();
    		String file=request->getParam("path", true)->value();
    		File fh= SPIFFS.open(file, "w");
    		if(!fh){
    			request->send(500);
    			return;
    		}
    		String c=request->getParam("content", true)->value();
      		fh.print(c.c_str());
      		fh.close();
        	ESP.wdtEnable(10);
            request->send(200);
            DBG_PRINTF("fputs path=%s\n",file.c_str());
            if(file == PROFILE_FILENAME){
	            DBG_PRINTF("reload file\n");
            	brewKeeper.reloadProfile();
            }
        } else
          request->send(404);
    }

    bool fileExists(String path)
    {
	    if(SPIFFS.exists(path)) return true;
	    bool dum;
	    unsigned int dum2;

	    if(getEmbeddedFile(path.c_str(),dum,dum2)) return true;
		if(path.endsWith(CHART_LIB_PATH) && SPIFFS.exists(CHART_LIB_PATH)) return true;
		// safari workaround.
		if(path.endsWith(".js")){
			String pathWithJgz = path.substring(0,path.lastIndexOf('.')) + ".jgz";
			 //DBG_PRINTF("checking with:%s\n",pathWithJgz.c_str());
			 if(SPIFFS.exists(pathWithJgz)) return true;
		}
		String pathWithGz = path + ".gz";
		if(SPIFFS.exists(pathWithGz)) return true;
		return false;
    }

	void sendProgmem(AsyncWebServerRequest *request,const char* html)
	{
		AsyncWebServerResponse *response = request->beginResponse(String("text/html"),
  			strlen_P(html),
  			[=](uint8_t *buffer, size_t maxLen, size_t alreadySent) -> size_t {
    			if (strlen_P(html+alreadySent)>maxLen) {
	      		memcpy_P((char*)buffer, html+alreadySent, maxLen);
    	  		return maxLen;
    		}
    		// Ok, last chunk
    		memcpy_P((char*)buffer, html+alreadySent, strlen_P(html+alreadySent));
    		return strlen_P(html+alreadySent); // Return from here to end of indexhtml
 	 	});
 	 	response->addHeader("Cache-Control","max-age=2592000");
		request->send(response);
	}

	void sendFile(AsyncWebServerRequest *request,String path)
	{
		//workaround for safari
		if(path.endsWith(".js")){
			String pathWithJgz = path.substring(0,path.lastIndexOf('.')) + ".jgz";
			if(SPIFFS.exists(pathWithJgz)){
				AsyncWebServerResponse * response = request->beginResponse(SPIFFS, pathWithJgz,"application/javascript");
				response->addHeader("Content-Encoding", "gzip");
				response->addHeader("Cache-Control","max-age=2592000");
//				response->addHeader("Content-Type","application/javascript");
				request->send(response);

				return;
			}
		}
		String pathWithGz = path + ".gz";
		if(SPIFFS.exists(pathWithGz)){
			AsyncWebServerResponse * response = request->beginResponse(SPIFFS, pathWithGz,getContentType(path));
			response->addHeader("Content-Encoding", "gzip");
			response->addHeader("Cache-Control","max-age=2592000");
//			response->addHeader("Content-Type",getContentType(path));
			request->send(response);
			return;
		}
		  
		if(SPIFFS.exists(path)){
			//request->send(SPIFFS, path);
			bool nocache=false;
			for(byte i=0;i< sizeof(nocache_list)/sizeof(const char*);i++){
				if(path.equals(nocache_list[i])){
						nocache=true;
						break;
					}
			}


			AsyncWebServerResponse *response = request->beginResponse(SPIFFS, path);
			if(nocache)
				response->addHeader("Cache-Control","no-cache");
			else
				response->addHeader("Cache-Control","max-age=2592000");
			request->send(response);
			return;
		}
		//else, embedded html file
		bool gzip;
		uint32_t size;
		const uint8_t* file=getEmbeddedFile(path.c_str(),gzip,size);
		if(file){
			DBG_PRINTF("using embedded file:%s\n",path.c_str());
			if(gzip){
                AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", file, size);
                response->addHeader("Content-Encoding", "gzip");
                request->send(response);
			}else sendProgmem(request,(const char*)file);
		}
	}
	String getContentType(String filename){
		if(filename.endsWith(".htm")) return "text/html";
		else if(filename.endsWith(".html")) return "text/html";
		else if(filename.endsWith(".css")) return "text/css";
		else if(filename.endsWith(".js")) return "application/javascript";
		else if(filename.endsWith(".png")) return "image/png";
		else if(filename.endsWith(".gif")) return "image/gif";
		else if(filename.endsWith(".jpg")) return "image/jpeg";
		else if(filename.endsWith(".ico")) return "image/x-icon";
		else if(filename.endsWith(".xml")) return "text/xml";
		else if(filename.endsWith(".pdf")) return "application/x-pdf";
		else if(filename.endsWith(".zip")) return "application/x-zip";
		else if(filename.endsWith(".gz")) return "application/x-gzip";
		return "text/plain";
	  }
	  
public:
	BrewPiWebHandler(void){}

	void handleRequest(AsyncWebServerRequest *request){
	 	if(request->method() == HTTP_GET && request->url() == POLLING_PATH) {
	 		char *line=brewPi.getLastLine();
	 		if(line[0]!=0) request->send(200, "text/plain", line);
	 		else request->send(200, "text/plain;", "");
	 	}else if(request->method() == HTTP_POST && request->url() == PUTLINE_PATH){
	 		String data=request->getParam("data", true, false)->value();
	 		//DBG_PRINTF("putline:%s\n",data.c_str());

			if(data.startsWith("j") && !request->authenticate(username, password))
		        return request->requestAuthentication();

	 		brewPi.putLine(data.c_str());
	 		request->send(200);
	 	}else if(request->method() == HTTP_GET && request->url() == CONTROL_CC_PATH){
	 		char unit;
	 		float minTemp,maxTemp;
	 		brewPi.getTemperatureSetting(&unit,&minTemp,&maxTemp);
	 		String json=String("{\"tempSetMin\":") + String(minTemp)
	 			+ String(",\"tempSetMax\":") + String(maxTemp)
	 			+ String(",\"tempFormat\":\"") + String(unit)  +String("\"}");
	 		request->send(200,"application/json",json);

	 	}else if(request->method() == HTTP_GET && request->url() == CONFIG_PATH){
			request->redirect(request->url() + ".htm");
	 	}else if(request->method() == HTTP_POST && request->url() == CONFIG_PATH){
	 	    if(!request->authenticate(username, password))
	        return request->requestAuthentication();

			if(request->hasParam("name", true)
					&& request->hasParam("user", true)
					&& request->hasParam("title", true)					
					&& request->hasParam("pass", true)){
  				AsyncWebParameter* name = request->getParam("name", true);
  				AsyncWebParameter* user = request->getParam("user", true);
				AsyncWebParameter* pass = request->getParam("pass", true);
				AsyncWebParameter* title = request->getParam("title", true);

  				File config=SPIFFS.open(CONFIG_FILENAME,"w+");

  				if(!config){
  					request->send(500);
  					return;
  				}

  				int protect =request->getParam("protect", true)->value().toInt();
  				int ap = request->getParam("ap", true)->value().toInt();
  				DBG_PRINTF("STA_AP mode? %d\n",ap);

  				int size=strlen_P(configFormat);
  				char *fmt=(char*) malloc(size +1);
  				if(!fmt){
  					request->send(500);
					DBG_PRINTF("!!Alloc error\n");
  					return;
  				}
  				strcpy_P(fmt,configFormat);

  				config.printf(fmt,name->value().c_str(),
  											user->value().c_str(),
											  pass->value().c_str(),
											  title->value().c_str(),
  											protect,ap);
  				free(fmt);
  				config.flush();
  				config.close();
				request->send(200);
				requestRestart(false);
  			}else{
	  			request->send(400);
  			}
	 	}else if(request->method() == HTTP_GET &&  request->url() == TIME_PATH){
			AsyncResponseStream *response = request->beginResponseStream("application/json");
			response->printf("{\"t\":\"%s\",\"e\":%lu,\"o\":%ld}",TimeKeeper.getDateTimeStr(),TimeKeeper.getTimeSeconds(),TimeKeeper.getTimezoneOffset());
			request->send(response);
		}else if(request->method() == HTTP_POST &&  request->url() == TIME_PATH){
			if(request->hasParam("time", true)){
				  AsyncWebParameter* tvalue = request->getParam("time", true);
				  time_t time=(time_t)tvalue->value().toInt();
  				DBG_PRINTF("Set Time:%lu from:%s\n",time,tvalue->value().c_str());
	 			TimeKeeper.setCurrentTime(time);
			 }
			 if(request->hasParam("off", true)){
				AsyncWebParameter* tvalue = request->getParam("off", true);
				DBG_PRINTF("Set timezone:%ld\n",tvalue->value().toInt());
			   TimeKeeper.setTimezoneOffset(tvalue->value().toInt());
		    }		   
			request->send(200, "text/plain;", "");
			 
		}else if(request->method() == HTTP_GET &&  request->url() == RESETWIFI_PATH){
	 	    if(!request->authenticate(username, password))
	        return request->requestAuthentication();
		 	request->send(200,"text/html","Done, restarting..");
			requestRestart(true);
	 	}else if(request->method() == HTTP_POST &&  request->url() == FLIST_PATH){
	 	    if(!request->authenticate(username, password))
	        return request->requestAuthentication();

			handleFileList(request);
	 	}else if(request->method() == HTTP_DELETE &&  request->url() == DELETE_PATH){
	 	    if(!request->authenticate(username, password))
	        return request->requestAuthentication();

			handleFileDelete(request);
	 	}else if(request->method() == HTTP_POST &&  request->url() == FPUTS_PATH){
	 	    if(!request->authenticate(username, password))
	        return request->requestAuthentication();

			handleFilePuts(request);
		}else if(request->method() == HTTP_GET && request->url() == GETSTATUS_PATH){
			uint8_t mode, state;
			float beerSet, beerTemp, fridgeTemp, fridgeSet, roomTemp;
			brewPi.getAllStatus(&state, &mode, &beerTemp, &beerSet, &fridgeTemp, &fridgeSet, &roomTemp);
			#define TEMPorNull(a) (IS_FLOAT_TEMP_VALID(a)?  String(a):String("null"))
			String json=String("{\"mode\":\"") + String((char) mode)
			+ String("\",\"state\":") + String(state)
			+ String(",\"beerSet\":") + TEMPorNull(beerSet)
			+ String(",\"beerTemp\":") + TEMPorNull(beerTemp)
			+ String(",\"fridgeSet\":") + TEMPorNull(fridgeSet)
			+ String(",\"fridgeTemp\":") + TEMPorNull(fridgeTemp)
			+ String(",\"roomTemp\":") + TEMPorNull(roomTemp)
			+String("}");
			request->send(200,"application/json",json);

	 	#ifdef ENABLE_LOGGING
	 	}else if (request->url() == LOGGING_PATH){
	 		if(request->method() == HTTP_POST){
		 		dataLogger.updateSetting(request);
	 		}else{
	 			dataLogger.getSettings(request);
	 		}
	 	#endif
	 	}else if(request->method() == HTTP_GET){

			String path=request->url();
	 		if(path.endsWith("/")) path +=DEFAULT_INDEX_FILE;
	 		else if(path.endsWith(CHART_LIB_PATH)) path = CHART_LIB_PATH;

	 		if(request->url().equals("/")){
		 		if(!passwordLcd){
		 			sendFile(request,path); //request->send(SPIFFS, path);
		 			return;
		 		}
		 	}
			bool auth=true;

			for(byte i=0;i< sizeof(public_list)/sizeof(const char*);i++){
				if(path.equals(public_list[i])){
						auth=false;
						break;
					}
			}

	 	    if(auth && !request->authenticate(username, password))
	        return request->requestAuthentication();

	 		sendFile(request,path); //request->send(SPIFFS, path);
		}
	 }

	bool canHandle(AsyncWebServerRequest *request){
	 	if(request->method() == HTTP_GET){
	 		if(request->url() == POLLING_PATH || request->url() == CONFIG_PATH || request->url() == TIME_PATH
			 || request->url() == RESETWIFI_PATH || request->url() == CONTROL_CC_PATH
			 || request->url() == GETSTATUS_PATH
	 		#ifdef ENABLE_LOGGING
	 		|| request->url() == LOGGING_PATH
	 		#endif
	 		){
	 			return true;
			}else{
				// get file
				String path=request->url();
	 			if(path.endsWith("/")) path +=DEFAULT_INDEX_FILE;
	 			//DBG_PRINTF("request:%s\n",path.c_str());
				if(fileExists(path)) return true; //if(SPIFFS.exists(path)) return true;
				//DBG_PRINTF("request:%s not found\n",path.c_str());
			}
	 	}else if(request->method() == HTTP_DELETE && request->url() == DELETE_PATH){
				return true;
	 	}else if(request->method() == HTTP_POST){
	 		if(request->url() == PUTLINE_PATH || request->url() == CONFIG_PATH
	 			|| request->url() ==  FPUTS_PATH || request->url() == FLIST_PATH
	 			|| request->url() == TIME_PATH
	 			#ifdef ENABLE_LOGGING
	 			|| request->url() == LOGGING_PATH
	 			#endif
	 			)
	 			return true;
		}
		return false;
	 }
};

BrewPiWebHandler brewPiWebHandler;

#if ResponseAppleCNA == true

class AppleCNAHandler: public AsyncWebHandler
{
public:
	AppleCNAHandler(){}
	void handleRequest(AsyncWebServerRequest *request){
		request->send(200, "text/html", "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>");
	}
	bool canHandle(AsyncWebServerRequest *request){
		String host=request->host();
		//DBG_PRINTF("Request host:");
		//DBG_PRINTF(host.c_str());
		//DBG_PRINTF("\n");
  		if(host.indexOf(String("apple")) >=0
  		|| host.indexOf(String("itools")) >=0
  		|| host.indexOf(String("ibook")) >=0
  		|| host.indexOf(String("airport")) >=0
  		|| host.indexOf(String("thinkdifferent")) >=0
  		|| host.indexOf(String("akamai")) >=0 ){
  			return true;
  		}
  		return false;
	}
};



AppleCNAHandler appleCNAHandler;
#endif //#if ResponseAppleCNA == true

#if UseWebSocket == true
AsyncWebSocket ws(WS_PATH);
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
	if(type == WS_EVT_CONNECT){
    	DBG_PRINTF("ws[%s][%u] connect\n", server->url(), client->id());
    	//client->printf("Hello Client %u :)", client->id());
    	client->ping();
  	} else if(type == WS_EVT_DISCONNECT){
    	DBG_PRINTF("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
  	} else if(type == WS_EVT_ERROR){
    	DBG_PRINTF("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  	} else if(type == WS_EVT_PONG){
    	DBG_PRINTF("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
  	} else if(type == WS_EVT_DATA){
    	AwsFrameInfo * info = (AwsFrameInfo*)arg;
    	String msg = "";
    	if(info->final && info->index == 0 && info->len == len){
      		//the whole message is in a single frame and we got all of it's data
      		DBG_PRINTF("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);

	        for(size_t i=0; i < info->len; i++) {
        	  //msg += (char) data[i];
        	  brewPi.write(data[i]);
        	}
		    //DBG_PRINTF("%s\n",msg.c_str());

		} else {
      		//message is comprised of multiple frames or the frame is split into multiple packets
      		if(info->index == 0){
        		if(info->num == 0)
          		DBG_PRINTF("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
        		DBG_PRINTF("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      		}

      		DBG_PRINTF("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);

	        for(size_t i=0; i < info->len; i++) {
    	    	//msg += (char) data[i];
    	    	brewPi.write(data[i]);
        	}

      		//DBG_PRINTF("%s\n",msg.c_str());

			if((info->index + len) == info->len){
				DBG_PRINTF("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
        		if(info->final){
        			DBG_PRINTF("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
        		}
      		}
      	}
    }
}
#endif //#if UseWebSocket == true

void stringAvailable(const char *str)
{
	//DBG_PRINTF("BroadCast:%s\n",str);

#if UseWebSocket == true
	ws.textAll(str,strlen(str));
#endif

#if UseServerSideEvent == true
	sse.send(str);
#endif
}

void notifyLogStatus(void)
{
	stringAvailable("V:{\"reload\":\"chart\"}");
}

void reportRssi(void)
{
	char buf[32];
	sprintf(buf,"V:{\"rssi\":%d}",WiFi.RSSI());
	stringAvailable(buf);
}

void onClientConnected(AsyncEventSourceClient *client){
	DBG_PRINTF("SSE Connect\n");
	char buf[128];
	// gravity related info.
	if(externalData.iSpindelEnabled()){
		externalData.sseNotify(buf);
		client->send(buf);
	}
	// RSSI && 
	sprintf(buf,"V:{\"nn\":\"%s\",\"ver\":\"%s\",\"rssi\":%d,\"tm\":%lu,\"off\":%ld}"
		,titlelabel,BPL_VERSION,WiFi.RSSI(),TimeKeeper.getTimeSeconds(),TimeKeeper.getTimezoneOffset());
	client->send(buf);
}

#define MAX_DATA_SIZE 256

class LogHandler:public AsyncWebHandler
{
public:

	void handleRequest(AsyncWebServerRequest *request){
		if( request->url() == LOGLIST_PATH){
			if(request->hasParam("dl")){
				int index=request->getParam("dl")->value().toInt();
				char buf[36];
				brewLogger.getFilePath(buf,index);
				if(SPIFFS.exists(buf)){
					request->send(SPIFFS,buf,"application/octet-stream",true);
				}else{
					request->send(404);
				}
			}else if(request->hasParam("rm")){
				int index=request->getParam("rm")->value().toInt();
				DBG_PRINTF("Delete log file %d\n",index);
				brewLogger.rmLog(index);

				request->send(200,"application/json",brewLogger.fsinfo());
			}else if(request->hasParam("start")){
				String filename=request->getParam("start")->value();
				DBG_PRINTF("start logging:%s\n",filename.c_str());
				#if BREW_AND_CALIBRATION
				bool cal=externalData.isCalibrating();
				if(brewLogger.startSession(filename.c_str(),cal)){
					if(cal) brewLogger.addTiltInWater(externalData.titltInWater());
				#else
				if(brewLogger.startSession(filename.c_str())){
				#endif
					request->send(200);
					notifyLogStatus();
				}else
					request->send(404);
			}else if(request->hasParam("stop")){
				DBG_PRINTF("Stop logging\n");
				brewLogger.endSession();
				request->send(200);
				notifyLogStatus();
			}else{
				// default. list information
				String status=brewLogger.loggingStatus();
				request->send(200,"application/json",status);
			}
			return;
		} // end of logist path
		// charting

		int offset;
		if(request->hasParam("offset")){
			offset=request->getParam("offset")->value().toInt();
			//DBG_PRINTF("offset= %d\n",offset);
		}else{
			offset=0;
		}

		size_t index;
		bool indexValid;
		if(request->hasParam("index")){
			index=request->getParam("index")->value().toInt();
			//DBG_PRINTF("index= %d\n",index);
			indexValid=true;
		}else{
			indexValid=false;
		}

		if(!brewLogger.isLogging()){
			// volatile logging
			if(!indexValid){
				// client in Logging mode. force to reload
				offset=0;
				index =0;
			}
			size_t size=brewLogger.volatileDataAvailable(index,offset);
			size_t logoffset=brewLogger.volatileDataOffset();

			if(size >0){
				AsyncWebServerResponse *response = request->beginResponse("application/octet-stream", size,
						[](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
					return brewLogger.readVolatileData(buffer, maxLen,index);
				});
				response->addHeader("LogOffset",String(logoffset));
				request->send(response);
			}else{
				request->send(204);
			}
		}else{
			if(indexValid){
				// client in volatile Logging mode. force to reload
				offset=0;
			}

			size_t size=brewLogger.beginCopyAfter(offset);
			if(size >0){
				request->send("application/octet-stream", size, [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
					return brewLogger.read(buffer, maxLen,index);
				});
			}else{
				request->send(204);
			}
		}
	}

	LogHandler(){}
	bool canHandle(AsyncWebServerRequest *request){
	 	if(request->url() == CHART_DATA_PATH || request->url() ==LOGLIST_PATH) return true;
	 	return false;
	}
};
LogHandler logHandler;


#define GavityDeviceConfigFilename "/gdconfig"
#define GravityDeviceConfigPath "/gdc"

class ExternalDataHandler:public AsyncWebHandler
{
private:
	char _buffer[MAX_DATA_SIZE+2];
	char *_data;

	size_t _dataLength;
	bool   _error;

	void processGravity(AsyncWebServerRequest *request,char data[],size_t length){
		if(length ==0) return request->send(500);;

        uint8_t error;
		if(externalData.processJSON(data,length,request->authenticate(username, password),error)){
    		request->send(200,"application/json","{}");
		}else{
		    if(error == ErrorAuthenticateNeeded) return request->requestAuthentication();
		    else request->send(500);
		}
	}

public:

	ExternalDataHandler(){
    	_data = &(_buffer[2]);
    	_buffer[0]='G';
    	_buffer[1]=':';
	}

	void loadConfig(void){
	    char *buf=_data;
		File config=SPIFFS.open(GavityDeviceConfigFilename,"r+");
		if(config){
			size_t len=config.readBytes(buf,MAX_DATA_SIZE);
			buf[len]='\0';
			externalData.config(buf);
		}
		config.close();
	}

	bool canHandle(AsyncWebServerRequest *request){
		DBG_PRINTF("req: %s\n", request->url().c_str());
	 	if(request->url() == GRAVITY_PATH	) return true;
	 	if(request->url() == GravityDeviceConfigPath) return true;

	 	return false;
	}

	void handleRequest(AsyncWebServerRequest *request){
		if(request->url() == GRAVITY_PATH){
			if(request->method() != HTTP_POST){
				request->send(400);
				return;
			}
			stringAvailable(_buffer);
			processGravity(request,_data,_dataLength);
			// Process the name
			externalData.sseNotify(_data);
			stringAvailable(_data);
			return;
		}
		// config
		if(request->method() == HTTP_POST){
			// post

  			File config=SPIFFS.open(GavityDeviceConfigFilename,"w+");
  			if(!config){
  				request->send(500);
  				return;
  			}
  			config.printf(_data);
  			config.flush();
  			config.close();
  			externalData.config(_data);
  			request->send(200);

		}//else{
			// get
		if(request->hasParam("data")){
		    request->send(SPIFFS,GavityDeviceConfigFilename, "application/json");
		}else{
			// get the HTML
			request->redirect(request->url() + ".htm");
		    //request->send_P(200, "text/html", externalData.html());
		}
	}

	void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
		if(!index){
		    DBG_PRINTF("BodyStart-len:%d total: %u\n",len, total);
			_dataLength =0;
			_error=(total >= MAX_DATA_SIZE);
		}

		if(_error) return;
		for(size_t i=0; i< len; i++){
			//Serial.write(data[i]);
			_data[_dataLength ++] = data[i];
		}
		if(index + len >= total){
			_data[_dataLength]='\0';
			DBG_PRINTF("Body total%u data:%s\n", total,_data);
		}
	}
};
ExternalDataHandler externalDataHandler;

//{brewpi


// global class objects static and defined in class cpp and h files

// instantiate and configure the sensors, actuators and controllers we want to use


/* Configure the counter and delay timer. The actual type of these will vary depending upon the environment.
* They are non-virtual to keep code size minimal, so typedefs and preprocessing are used to select the actual compile-time type used. */
TicksImpl ticks = TicksImpl(TICKS_IMPL_CONFIG);
DelayImpl wait = DelayImpl(DELAY_IMPL_CONFIG);

DisplayType realDisplay;
DisplayType DISPLAY_REF display = realDisplay;

ValueActuator alarm;

#ifdef ESP8266_WiFi


WiFiServer server(23);
WiFiClient serverClient;
#endif
void handleReset()
{
#if defined(ESP8266)
	// The asm volatile method doesn't work on ESP8266. Instead, use ESP.restart
	ESP.restart();
#else
	// resetting using the watchdog timer (which is a full reset of all registers)
	// might not be compatible with old Arduino bootloaders. jumping to 0 is safer.
	asm volatile ("  jmp 0");
#endif
}


void brewpi_setup()
{

#if defined(ESP8266)
	// We need to initialize the EEPROM on ESP8266
	EEPROM.begin(MAX_EEPROM_SIZE_LIMIT);
	eepromAccess.set_manual_commit(false); // TODO - Move this where it should actually belong (a class constructor)
#endif

#if BREWPI_BUZZER
	buzzer.init();
	buzzer.beep(2, 500);
#endif

	piLink.init();

	logDebug("started");
	tempControl.init();
	settingsManager.loadSettings();

#if BREWPI_SIMULATE
	simulator.step();
	// initialize the filters with the assigned initial temp value
	tempControl.beerSensor->init();
	tempControl.fridgeSensor->init();
#endif
#ifdef EARLY_DISPLAY
	display.clear();
#else
	display.init();
#endif
	display.printStationaryText();
	display.printState();

	rotaryEncoder.init();

	logDebug("init complete");
}

void brewpiLoop(void)
{
	static unsigned long lastUpdate = 0;
	uint8_t oldState;

	if (ticks.millis() - lastUpdate >= (1000)) { //update settings every second
		lastUpdate = ticks.millis();

#if BREWPI_BUZZER
		buzzer.setActive(alarm.isActive() && !buzzer.isActive());
#endif

		tempControl.updateTemperatures();
		tempControl.detectPeaks();
		tempControl.updatePID();
		oldState = tempControl.getState();
		tempControl.updateState();
		if (oldState != tempControl.getState()) {
			piLink.printTemperatures(); // add a data point at every state transition
		}
		tempControl.updateOutputs();

#if BREWPI_MENU
		if (rotaryEncoder.pushed()) {
			rotaryEncoder.resetPushed();
			display.updateBacklight();
			menu.pickSettingToChange();
		}
#endif

		// update the lcd for the chamber being displayed
		display.printState();
		display.printAllTemperatures();
		display.printMode();
		display.updateBacklight();
	}

	//listen for incoming serial connections while waiting to update
#ifdef ESP8266_WiFi
	yield();
	connectClients();
	yield();
#endif
	piLink.receive();

}

//}brewpi


#ifdef STATUS_LINE
extern void makeTime(time_t timeInput, struct tm &tm);
time_t _displayTime;
#endif



#define SystemStateOperating 0
#define SystemStateRestartPending 1
#define SystemStateWaitRestart 2

#define TIME_RESTART_TIMEOUT 3000

bool _disconnectBeforeRestart;
static unsigned long _time;
byte _systemState=SystemStateOperating;
void requestRestart(bool disc)
{
	_disconnectBeforeRestart=disc;
	_systemState =SystemStateRestartPending;
}

#define IS_RESTARTING (_systemState!=SystemStateOperating)

#if WAKEUP_BUTTON
void isr_wakeupLcd(void) { display.resetBacklightTimer(); }
void initWakeupButton(void){
	pinMode(WakeupButtonPin, INPUT_PULLUP);
	attachInterrupt(WakeupButtonPin, isr_wakeupLcd, FALLING);
}
#endif //#ifdef WAKEUP_BUTTON

#ifdef EMIWorkaround
uint32_t _lcdReinitTime;
#define LCDReInitPeriod (10*60*1000)
#endif

void setup(void){

	#if SerialDebug == true
  	DebugPort.begin(115200);
  	DBG_PRINTF("\nSetup()\n");
  	DebugPort.setDebugOutput(true);
  	#endif

	//0.Initialize file system
	//start SPI Filesystem
  	if(!SPIFFS.begin()){
  		// TO DO: what to do?
  		DBG_PRINTF("SPIFFS.being() failed!\n");
  	}else{
  		DBG_PRINTF("SPIFFS.being() Success.\n");
  	}

    WiFiSetup.preInit();

#ifdef EARLY_DISPLAY
	DBG_PRINTF("Init LCD...\n");
	display.init();
	display.printAt_P(1,0,PSTR("Initialize WiFi"));
	display.updateBacklight();
	DBG_PRINTF("LCD Initialized..\n");
#endif


	// try open configuration
	char configBuf[MAX_CONFIG_LEN];

	File config=SPIFFS.open(CONFIG_FILENAME,"r+");

	DynamicJsonBuffer jsonBuffer(JSON_BUFFER_SIZE);

	if(config){
		size_t len=config.readBytes(configBuf,MAX_CONFIG_LEN);
		configBuf[len]='\0';
	}

	JsonObject& root = jsonBuffer.parseObject(configBuf);
	//const char* host;
	if(!config
			|| !root.success()
			|| !root.containsKey("name")
			|| !root.containsKey("user")
			|| !root.containsKey("pass"))
	{
		strcpy_P(configBuf,DefaultConfiguration);
		JsonObject& root = jsonBuffer.parseObject(configBuf);
  		strcpy(hostnetworkname,root["name"]);
  		strcpy(username,root["user"]);
	  	strcpy(password,root["pass"]);
	 	strcpy(titlelabel,root["title"]);
  		passwordLcd=(root.containsKey("protect"))? (bool)(root["protect"]):false;
		stationApMode=(root.containsKey("ap"))? (bool)(root["protect"]):false;

		File newconfig=SPIFFS.open(CONFIG_FILENAME,"w+");
		newconfig.write((const byte*)configBuf,sizeof(DefaultConfiguration));
		newconfig.close();
		DBG_PRINTF("Reset config\n");
	}else{
		config.close();
	  	strcpy(hostnetworkname,root["name"]);
  		strcpy(username,root["user"]);
		strcpy(password,root["pass"]);
		if(root.containsKey("title")) strcpy(titlelabel,root["title"]);
		else  strcpy(titlelabel,root["name"]);
  		passwordLcd=(root.containsKey("protect"))? (bool)(root["protect"]):false;
		stationApMode=(root.containsKey("ap"))? (bool)(root["ap"]):false;
		DBG_PRINTF("title:%s, name:%s, user:%s, pass:%s\n",titlelabel,hostnetworkname,username,password);
  	}
	DBG_PRINTF("STA_AP mode? %d\n",stationApMode);
	#ifdef ENABLE_LOGGING
  	dataLogger.loadConfig();
  	#endif


	//1. Start WiFi
	DBG_PRINTF("Starting WiFi...\n");
	WiFiSetup.setApStation(stationApMode);
	WiFiSetup.setTimeout(CaptivePortalTimeout);
	WiFiSetup.begin(hostnetworkname,password);

  	DBG_PRINTF("WiFi Done!\n");

	// get time
	initTime(WiFiSetup.isApMode());

	if (!MDNS.begin(hostnetworkname,WiFi.localIP())) {
			DBG_PRINTF("Error setting mDNS responder\n");
	}else{
		MDNS.addService("http", "tcp", 80);
	}

	// TODO: SSDP responder


	//3. setup Web Server

	// start WEB update pages.
#if (DEVELOPMENT_OTA == true) || (DEVELOPMENT_FILEMANAGER == true)
	ESPUpdateServer_setup(username,password);
#endif

	//3.1 Normal serving pages
	//3.1.1 status report through SSE

#if ResponseAppleCNA == true
	server.addHandler(&appleCNAHandler);
#endif

#if UseWebSocket == true
	ws.onEvent(onWsEvent);
  	server.addHandler(&ws);
#endif

#if UseServerSideEvent == true
	sse.onConnect(onClientConnected);
	server.addHandler(&sse);
#endif

	server.addHandler(&brewPiWebHandler);

	server.addHandler(&logHandler);

	externalDataHandler.loadConfig();
	server.addHandler(&externalDataHandler);

	//3.1.2 SPIFFS is part of the serving pages
	//server.serveStatic("/", SPIFFS, "/","public, max-age=259200"); // 3 days


	server.on("/fs",[](AsyncWebServerRequest *request){
		FSInfo fs_info;
		SPIFFS.info(fs_info);
		request->send(200,"","totalBytes:" +String(fs_info.totalBytes) +
		" usedBytes:" + String(fs_info.usedBytes)+" blockSize:" + String(fs_info.blockSize)
		+" pageSize:" + String(fs_info.pageSize)
		+" freesketch:" + String(ESP.getFreeSketchSpace())
		+" heap:"+String(ESP.getFreeHeap()));
		//testSPIFFS();
	});

	// 404 NOT found.
  	//called when the url is not defined here
	server.onNotFound([](AsyncWebServerRequest *request){
		request->send(404);
	});

	//4. start Web server
	server.begin();
	DBG_PRINTF("HTTP server started\n");


	// 5. try to connnect Arduino
	brewpi_setup();
  	brewPi.begin(stringAvailable);
	brewKeeper.setFile(PROFILE_FILENAME);

	brewLogger.begin();

#if WAKEUP_BUTTON
	initWakeupButton();
#endif


#ifdef STATUS_LINE
	// brewpi_setup will "clear" the screen.
	IPAddress ip =(WiFiSetup.isApMode())? WiFi.softAPIP():WiFi.localIP();
	char buf[21];
	sprintf(buf,"IP:%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
	display.printStatus(buf);
	_displayTime = TimeKeeper.getTimeSeconds() + 20;
#endif
#ifdef EMIWorkaround
	_lcdReinitTime = millis();
#endif
}

uint32_t _rssiReportTime;
#define RssiReportPeriod 10

void loop(void){
//{brewpi
#if BREWPI_SIMULATE
	simulateLoop();
#else
	brewpiLoop();
#endif
//}brewpi
#if (DEVELOPMENT_OTA == true) || (DEVELOPMENT_FILEMANAGER == true)
	ESPUpdateServer_loop();
#endif
	time_t now=TimeKeeper.getTimeSeconds();

#ifdef EMIWorkAround
	if( (millis() - _lcdReinitTime) > LCDReInitPeriod){
		_lcdReinitTime=millis();
		display.fresh();
	}
#endif

#ifdef STATUS_LINE
	if(_displayTime < now){
		_displayTime=now;

		struct tm t;
		makeTime(TimeKeeper.getLocalTimeSeconds(),t);
		char buf[21];
		sprintf(buf,"%d/%02d/%02d %02d:%02d:%02d",t.tm_year,t.tm_mon,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
		display.printStatus(buf);
	}
#endif
	if( (now - _rssiReportTime) > RssiReportPeriod){
		_rssiReportTime =now;
		reportRssi();
	}

  	brewKeeper.keep(now);

  	brewPi.loop();

 	brewLogger.loop();

 	#ifdef ENABLE_LOGGING

 	dataLogger.loop(now);
 	#endif

	if(!IS_RESTARTING){
		WiFiSetup.stayConnected();
		if(WiFiSetup.isApMode()) TimeKeeper.setInternetAccessibility(false);
	}

  	if(_systemState ==SystemStateRestartPending){
	  	_time=millis();
	  	_systemState =SystemStateWaitRestart;
  	}else if(_systemState ==SystemStateWaitRestart){
  		if((millis() - _time) > TIME_RESTART_TIMEOUT){
  			if(_disconnectBeforeRestart){
  				WiFi.disconnect();
  				delay(1000);
  			}
  			ESP.restart();
  		}
  	}
}
