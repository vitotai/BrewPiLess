#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#endif
#include <ArduinoOTA.h>
#include <FS.h>

#if defined(ESP32)
#include <AsyncTCP.h>
#else
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
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
#if AUTO_CAP
#include "AutoCapControl.h"
#endif

#include "TimeKeeper.h"
#include "mystrlib.h"

#include "GravityTracker.h"
#include "BrewKeeper.h"
#ifdef ENABLE_LOGGING
#include "DataLogger.h"
#endif

extern "C" {
#if defined(ESP32)
//#include "lwip/apps/sntp.h"
#else
#include <sntp.h>
#endif
}

#include "BPLSettings.h"

#include "ESPUpdateServer.h"
#include "WiFiSetup.h"

#include "BrewPiProxy.h"

#include "BrewLogger.h"

#include "ExternalData.h"

#if SupportMqttRemoteControl
#include "MqttRemoteControl.h"
#endif

#if EnableParasiteTempControl
#include "ParasiteTempController.h"
#endif

#if SupportPressureTransducer
#include "PressureMonitor.h"
#endif

#if SupportTiltHydrometer
#include "TiltListener.h"
#endif


#if UseLittleFS
#include <LittleFS.h>
#else
#if defined(ESP32)
#include <SPIFFS.h>
#endif
#endif

#if EnableDHTSensorSupport
#include "HumidityControl.h"
#endif

//WebSocket seems to be unstable, at least on iPhone.
//Go back to ServerSide Event.
#define UseWebSocket true
#define UseServerSideEvent false
#define ResponseAppleCNA true
#define CaptivePortalTimeout 180

#ifndef LegacyEspAsyncLibraries
#define LegacyEspAsyncLibraries false
#endif
/**************************************************************************************/
/* Start of Configuration 															  */
/**************************************************************************************/


#define JSON_BUFFER_SIZE 1024



#define WS_PATH 		"/ws"
#define SSE_PATH 		"/getline"

#define POLLING_PATH 	"/getline_p"
#define PUTLINE_PATH	"/putline"

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


#define BEER_PROFILE_PATH       "/tschedule"

#define GETSTATUS_PATH "/getstatus"
#define DEFAULT_INDEX_FILE     "index.htm"

#if EnableParasiteTempControl
#define ParasiteTempControlPath "/ptc"
#endif

#define GRAVITY_PATH       "/gravity"
#define GravityDeviceConfigPath "/gdc"
#define GravityFormulaPath "/coeff"

#if SupportTiltHydrometer
#define TiltCommandPath "/tcmd"
#endif


#if AUTO_CAP
#define CAPPER_PATH "/cap"
#endif

#define WIFI_SCAN_PATH "/wifiscan"
#define WIFI_CONNECT_PATH "/wificon"
#define WIFI_DISC_PATH "/wifidisc"

#define MQTT_PATH "/mqtt"
#if SupportPressureTransducer
#define PRESSURE_PATH "/psi"
#endif

#define HUMIDITY_CONTROL_PATH "/rh"

const char *public_list[]={
"/bwf.js",
"/brewing.json"
};

const char *nocache_list[]={
"/brewing.json",
"/brewpi.cfg"
};

#if UseLittleFS
FS& FileSystem = LittleFS;
#else
FS& FileSystem =SPIFFS;
#endif

//*******************************************

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
		else if(filename.endsWith(".jgz")) return "application/javascript";
		return "text/plain";
	  }

GravityTracker gravityTracker;

AsyncWebServer *webServer;

BrewPiProxy brewPi;
BrewKeeper brewKeeper([](const char* str){ brewPi.putLine(str);});
#ifdef ENABLE_LOGGING
DataLogger dataLogger;
#endif


#if UseServerSideEvent == true
AsyncEventSource sse(SSE_PATH);
#endif

extern const uint8_t* getEmbeddedFile(const char* filename,bool &gzip, unsigned int &size);

void requestRestart(bool disc);
void tiltScanResult(String& result);

void initTime(bool apmode)
{
/*	if(apmode){
		DBG_PRINTF("initTime in ap mode\n");
		TimeKeeper.begin();
	}else{
		DBG_PRINTF("connect to Time server\n");
		TimeKeeper.begin((char*)"time.google.com",(char*)"pool.ntp.org",(char*)"time.windows.com");
	}
*/
	TimeKeeper.begin((char*)"time.google.com",(char*)"pool.ntp.org",(char*)"time.windows.com");

}
#if AUTO_CAP
void capStatusReport(void);
#endif
class BrewPiWebHandler: public AsyncWebHandler
{
	void handleFileList(AsyncWebServerRequest *request) {
		if(request->hasParam("dir",true)){
        	String path = request->getParam("dir",true)->value();
			#if defined(ESP32)
			File dir = FileSystem.open(path);
          	String output = "[";
			if(dir.isDirectory()){
				File entry = dir.openNextFile();
          		while(entry){
            		
            		if (output != "[") output += ',';
            		bool isDir = false;
            		output += "{\"type\":\"";
            		output += (isDir)?"dir":"file";
            		output += "\",\"name\":\"";
					#if UseLittleFS
					output += String(entry.name());
					#else
            		output += String(entry.name()).substring(1);
					#endif
            		output += "\"}";
            		entry.close();
					entry = dir.openNextFile();
          		}
			  }
          	output += "]";
          	request->send(200, "text/json", output);
          	output = String();


			#else
          	Dir dir = FileSystem.openDir(path);
          	path = String();
          	String output = "[";
          	while(dir.next()){
            	File entry = dir.openFile("r");
            	if (output != "[") output += ',';
            		bool isDir = false;
            		output += "{\"type\":\"";
            		output += (isDir)?"dir":"file";
            		output += "\",\"name\":\"";
					#if UseLittleFS
            		output += String(entry.name());
					#else
            		output += String(entry.name()).substring(1);
					#endif
            		output += "\"}";
            		entry.close();
          	}
          	output += "]";
          	request->send(200, "text/json", output);
          	output = String();
			#endif
        }
        else
          request->send(400);
	}

	void handleFileDelete(AsyncWebServerRequest *request){
		if(request->hasParam("path", true)){
			#if !defined(ESP32)
        	ESP.wdtDisable(); 
			#endif
			FileSystem.remove(request->getParam("path", true)->value()); 
			#if !defined(ESP32)
			ESP.wdtEnable(10);
			#endif
            request->send(200, "", "DELETE: "+request->getParam("path", true)->value());
        } else
          request->send(404);
    }

	void handleFilePuts(AsyncWebServerRequest *request){
		if(request->hasParam("path", true)
			&& request->hasParam("content", true)){
			#if !defined(ESP32)
        	ESP.wdtDisable();
			#endif
    		String file=request->getParam("path", true)->value();
    		File fh= FileSystem.open(file, "w");
    		if(!fh){
    			request->send(500);
    			return;
    		}
    		String c=request->getParam("content", true)->value();
      		fh.print(c.c_str());
      		fh.close();
			#if !defined(ESP32)
        	ESP.wdtEnable(10);
			#endif
            request->send(200,"application/json","{}");
            DBG_PRINTF("fputs path=%s\n",file.c_str());
        } else
          request->send(404);
    }

    bool fileExists(String path)
    {
	    if(FileSystem.exists(path)) return true;
	    bool dum;
	    unsigned int dum2;

	    if(getEmbeddedFile(path.c_str(),dum,dum2)) return true;
		if(path.endsWith(CHART_LIB_PATH) && FileSystem.exists(CHART_LIB_PATH)) return true;
		// safari workaround.
		if(path.endsWith(".js")){
			String pathWithJgz = path.substring(0,path.lastIndexOf('.')) + ".jgz";
			 //DBG_PRINTF("checking with:%s\n",pathWithJgz.c_str());
			 if(FileSystem.exists(pathWithJgz)) return true;
		}
		String pathWithGz = path + ".gz";
		if(FileSystem.exists(pathWithGz)) return true;
		return false;
    }

	void sendProgmem(AsyncWebServerRequest *request,const char* html,String contentType)
	{
		AsyncWebServerResponse *response = request->beginResponse(contentType,
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
			if(FileSystem.exists(pathWithJgz)){
				AsyncWebServerResponse * response = request->beginResponse(FileSystem, pathWithJgz,"application/javascript");
				response->addHeader("Content-Encoding", "gzip");
				response->addHeader("Cache-Control","max-age=2592000");
				request->send(response);

				return;
			}
		}
		String pathWithGz = path + String(".gz");
		if(FileSystem.exists(pathWithGz)){
#if 0
			AsyncWebServerResponse * response = request->beginResponse(FileSystem, pathWithGz,getContentType(path));
			// AsyncFileResonse will add "content-disposion" header, result in "download" of Safari, instead of "render" 
#else
			File file=FileSystem.open(pathWithGz,"r");
			if(!file){
				request->send(500);
				return;
			}
			AsyncWebServerResponse * response = request->beginResponse(file, path,getContentType(path));
#endif
//			response->addHeader("Content-Encoding", "gzip");
			response->addHeader("Cache-Control","max-age=2592000");
			request->send(response);
			return;
		}
		  
		if(FileSystem.exists(path)){
			//request->send(FileSystem, path);
			bool nocache=false;
			for(byte i=0;i< sizeof(nocache_list)/sizeof(const char*);i++){
				if(path.equals(nocache_list[i])){
						nocache=true;
						break;
					}
			}


			AsyncWebServerResponse *response = request->beginResponse(FileSystem, path);
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
				#if defined(ESP32)
                AsyncWebServerResponse *response = request->beginResponse("text/html", size,[=](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
					 //Write up to "maxLen" bytes into "buffer" and return the amount written.
  					//index equals the amount of bytes that have been already sent
  					//You will not be asked for more bytes once the content length has been reached.
  					//Keep in mind that you can not delay or yield waiting for more data!
  					//Send what you currently have and you will be asked for more again
  					memcpy(buffer,file + index, maxLen);
					return maxLen;
				});

				#else
                AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", file, size);
				#endif
                response->addHeader("Content-Encoding", "gzip");
                request->send(response);
			}else sendProgmem(request,(const char*)file,getContentType(path));
		}
	}	  
public:
	BrewPiWebHandler(void){}

#if LegacyEspAsyncLibraries != true
	virtual bool isRequestHandlerTrivial() override final {return false;}
#endif

	void handleRequest(AsyncWebServerRequest *request){
		SystemConfiguration *syscfg=theSettings.systemConfiguration();

		#if UseServerSideEvent == true
	 	if(request->method() == HTTP_GET && request->url() == POLLING_PATH) {
	 		char *line=brewPi.getLastLine();
	 		if(line[0]!=0) request->send(200, "text/plain", line);
	 		else request->send(200, "text/plain;", "");
	 	}
		 else if(request->method() == HTTP_POST && request->url() == PUTLINE_PATH){
	 		String data=request->getParam("data", true, false)->value();
	 		//DBG_PRINTF("putline:%s\n",data.c_str());

			if(data.startsWith("j") && !request->authenticate((const char*)syscfg->username,(const char*) syscfg->password))
		        return request->requestAuthentication();

	 		brewPi.putLine(data.c_str());
	 		request->send(200,"application/json","{}");
	 	}
		else
		#endif

		#if SupportMqttRemoteControl
		if(request->method() == HTTP_GET && request->url() == MQTT_PATH){
			request->send(200,"application/json",theSettings.jsonMqttRemoteControlSettings());
	 	}else if(request->method() == HTTP_POST && request->url() == MQTT_PATH){
	 	    if(!request->authenticate(syscfg->username, syscfg->password))
	        return request->requestAuthentication();

			if(request->hasParam("data", true)){
				if(theSettings.dejsonMqttRemoteControlSettings(request->getParam("data", true)->value())){
					theSettings.save();
					request->send(200,"application/json","{}");
					mqttRemoteControl.reset();
				}else{
  					request->send(500);
					DBG_PRINTF("json format error\n");
  					return;
				}
			}else{
	  			request->send(400);
				DBG_PRINTF("no data in post\n");
  			}

		}else 
		#endif
		if(request->method() == HTTP_GET && request->url() == CONFIG_PATH){
			if(!request->authenticate(syscfg->username, syscfg->password)) return request->requestAuthentication();
			if(request->hasParam("cfg"))
				request->send(200,"application/json",theSettings.jsonSystemConfiguration());
			else 
				request->redirect(request->url() + ".htm");
	 	}else if(request->method() == HTTP_POST && request->url() == CONFIG_PATH){
	 	    if(!request->authenticate(syscfg->username, syscfg->password))
	        return request->requestAuthentication();

			if(request->hasParam("data", true)){
				uint8_t oldMode = theSettings.systemConfiguration()->wifiMode;
				DBG_PRINTF("config to save: %s\n",request->getParam("data", true)->value().c_str());
				if(theSettings.dejsonSystemConfiguration(request->getParam("data", true)->value())){
					theSettings.save();
					DBG_PRINTF("config saved: %s\n",theSettings.systemConfiguration()->hostnetworkname);
					request->send(200,"application/json","{}");
					display.setAutoOffPeriod(theSettings.systemConfiguration()->backlite);

					if(oldMode !=  theSettings.systemConfiguration()->wifiMode){
						DBG_PRINTF("change from %d to %d\n",oldMode,theSettings.systemConfiguration()->wifiMode);
						WiFiSetup.setMode((WiFiMode) (theSettings.systemConfiguration()->wifiMode));
					}

					if(!request->hasParam("nb")){
						requestRestart(false);
					}
				}else{
  					request->send(500);
					DBG_PRINTF("json format error\n");
  					return;
  				}			
			}else{
	  			request->send(400);
				DBG_PRINTF("no data in post\n");
  			}
	 	}else if(request->method() == HTTP_GET &&  request->url() == TIME_PATH){
			AsyncResponseStream *response = request->beginResponseStream("application/json");
			response->printf("{\"t\":\"%s\",\"e\":%lu,\"o\":%d}",TimeKeeper.getDateTimeStr(),TimeKeeper.getTimeSeconds(),TimeKeeper.getTimezoneOffset());
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
			request->send(200,"application/json","{}");
			 
		}else if(request->method() == HTTP_GET &&  request->url() == RESETWIFI_PATH){
	 	    if(!request->authenticate(syscfg->username, syscfg->password))
	        return request->requestAuthentication();
		 	request->send(200,"text/html","Done, restarting..");
			#if SaveWiFiConfiguration
			WiFiConfiguration *wifiCon=theSettings.getWifiConfiguration();
			wifiCon->ssid[0]='\0';
			wifiCon->pass[0]='\0';
			theSettings.save();
			#endif
			requestRestart(true);
	 	}else if(request->method() == HTTP_POST &&  request->url() == FLIST_PATH){
	 	    if(!request->authenticate(syscfg->username, syscfg->password))
	        return request->requestAuthentication();

			handleFileList(request);
	 	}else if(request->method() == HTTP_DELETE &&  request->url() == DELETE_PATH){
	 	    if(!request->authenticate(syscfg->username, syscfg->password))
	        return request->requestAuthentication();

			handleFileDelete(request);
	 	}else if(request->method() == HTTP_POST &&  request->url() == FPUTS_PATH){
	 	    if(!request->authenticate(syscfg->username, syscfg->password))
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
		}
	 	#ifdef ENABLE_LOGGING
	 	else if (request->url() == LOGGING_PATH){
	 		if(request->method() == HTTP_POST){
				if(!request->authenticate(syscfg->username, syscfg->password)) return request->requestAuthentication();
				if(request->hasParam("data", true)){
		    		if(theSettings.dejsonRemoteLogging(request->getParam("data", true)->value())){
		    			request->send(200,"application/json","{}");
						theSettings.save();
					}else{
						request->send(401);
					}
        		} else{
        		  request->send(404);
    			}
	 		}else{
				if(request->hasParam("data")){
					request->send(200,"application/json",theSettings.jsonRemoteLogging());
				}else{
					request->redirect(request->url() + ".htm");
				} 
	 		}
		 }
	 	#endif
		#if EnableParasiteTempControl
		else if(request->url() == ParasiteTempControlPath){
			if(request->method() == HTTP_POST){
				if(request->hasParam("c", true)){
		    		String content=request->getParam("c", true)->value();
					if(parasiteTempController.updateSettings(content))
			            request->send(200,"application/json","{}");
					else 
						request->send(400);	
        		} else
          			request->send(404);
	 		}else{
				String status=parasiteTempController.getSettings();
				request->send(200,"application/json",status);
	 		}
		}
		#endif
		#if AUTO_CAP
		else if(request->url() == CAPPER_PATH){
	 	    if(!request->authenticate(syscfg->username, syscfg->password))
	        return request->requestAuthentication();
			// auto cap.
			if(request->hasParam("psi")){
				theSettings.pressureMonitorSettings()->psi=request->getParam("psi")->value().toInt();
				DBG_PRINTF("set pressure:%d",theSettings.pressureMonitorSettings()->psi);
				theSettings.save();
				PressureMonitor.configChanged();
			}
			bool response=true;
			if(request->hasParam("cap")){
				AsyncWebParameter* value = request->getParam("cap");
				autoCapControl.capManualSet(value->value().toInt()!=0);
				// manual
			}else if(request->hasParam("at")){
				// time
				AsyncWebParameter* value = request->getParam("at");
				autoCapControl.capAtTime(value->value().toInt());
				
			}else if(request->hasParam("sg")){
				// gravity
				AsyncWebParameter* value = request->getParam("sg");
				autoCapControl.catOnGravity(value->value().toFloat());
			}else{
				request->send(400);
				response=false;
			}
			if(response) request->send(200,"application/json","{}");
			capStatusReport();
		}
		#endif
		#if SupportPressureTransducer
		else if(request->url() == PRESSURE_PATH){
	 	    if(!request->authenticate(syscfg->username, syscfg->password))
	        return request->requestAuthentication();

			if(request->method() == HTTP_GET){
				if(request->hasParam("r")){
					int reading=PressureMonitor.currentAdcReading();
					request->send(200,"application/json",String("{\"a0\":")+String(reading)+String("}"));
				}else{
					request->send(200,"application/json",theSettings.jsonPressureMonitorSettings());
				}
			}else{
				// post
				if(!request->authenticate(syscfg->username, syscfg->password)) return request->requestAuthentication();

				if(request->hasParam("data",true)){					
					if(theSettings.dejsonPressureMonitorSettings(request->getParam("data",true)->value())){
						theSettings.save();
						PressureMonitor.configChanged();
						request->send(200,"application/json","{}");
					}else
						DBG_PRINTF("invalid Json\n");
						request->send(402);
				}else{
					DBG_PRINTF("no data\n");
					request->send(401);
				}
			}
		}
		#endif
		#if EnableDHTSensorSupport
		else if(request->url() == HUMIDITY_CONTROL_PATH){
			if(request->hasParam("m",true) &&  request->hasParam("t",true) ){
				uint8_t mode=(uint8_t) request->getParam("m",true)->value().toInt();
				uint8_t target=(uint8_t) request->getParam("t",true)->value().toInt();
				humidityControl.setMode(mode);
				humidityControl.setTarget(target);
				theSettings.save();
				request->send(200,"application/json","{}");
			}else{
				request->send(404);
				DBG_PRINTF("missing parameter:m =%d, t=%d\n",request->hasParam("m",true), request->hasParam("t",true) );
			}
		}
		#endif
		else if(request->url() == BEER_PROFILE_PATH){
			if(request->method() == HTTP_GET){
				request->send(200,"application/json",theSettings.jsonBeerProfile());
			}else{ //if(request->method() == HTTP_POST){

				if(!request->authenticate(syscfg->username, syscfg->password)) return request->requestAuthentication();

				if(request->hasParam("data",true)){
					if(theSettings.dejsonBeerProfile(request->getParam("data",true)->value())){
						theSettings.save();
						brewKeeper.profileUpdated();
						request->send(200,"application/json","{}");
					}else
						request->send(402);
				}else{
					request->send(401);
				}
			}
		}else if(request->method() == HTTP_GET){

			String path=request->url();
	 		if(path.endsWith("/")) path +=DEFAULT_INDEX_FILE;
	 		else if(path.endsWith(CHART_LIB_PATH)) path = CHART_LIB_PATH;

	 		if(request->url().equals("/")){
				SystemConfiguration *syscfg=theSettings.systemConfiguration();
		 		if(!syscfg->passwordLcd){
		 			sendFile(request,path); //request->send(FileSystem, path);
		 			return;
		 		}
		 	}
			/*
			bool auth=true;

			for(byte i=0;i< sizeof(public_list)/sizeof(const char*);i++){
				if(path.equals(public_list[i])){
						auth=false;
						break;
					}
			}
			*/
	 	    if(syscfg->passwordLcd && !request->authenticate(syscfg->username, syscfg->password))
	        return request->requestAuthentication();

	 		sendFile(request,path); //request->send(FileSystem, path);
		}
	 }

	bool canHandle(AsyncWebServerRequest *request){
	 	if(request->method() == HTTP_GET){
	 		if( request->url() == CONFIG_PATH || request->url() == TIME_PATH
			#if UseServerSideEvent == true
			|| request->url() == POLLING_PATH 
			#endif
			 || request->url() == RESETWIFI_PATH  
			 || request->url() == GETSTATUS_PATH
			 || request->url() == BEER_PROFILE_PATH
			 || request->url() == MQTT_PATH
	 		#ifdef ENABLE_LOGGING
	 		|| request->url() == LOGGING_PATH
	 		#endif
			 #if EnableParasiteTempControl
			 || request->url() == ParasiteTempControlPath
			 #endif
			#if AUTO_CAP
			 || request->url() == CAPPER_PATH
			#endif
			#if SupportPressureTransducer
			|| request->url() == PRESSURE_PATH
			#endif
	 		){
	 			return true;
			}else{
				// get file
				String path=request->url();
	 			if(path.endsWith("/")) path +=DEFAULT_INDEX_FILE;
	 			//DBG_PRINTF("request:%s\n",path.c_str());
				if(fileExists(path)) return true; //if(FileSystem.exists(path)) return true;
				//DBG_PRINTF("request:%s not found\n",path.c_str());
			}
	 	}else if(request->method() == HTTP_DELETE && request->url() == DELETE_PATH){
				return true;
	 	}else if(request->method() == HTTP_POST){
	 		if(
				#if UseServerSideEvent == true
				 request->url() == PUTLINE_PATH || 
				#endif
			 request->url() == CONFIG_PATH
	 			|| request->url() ==  FPUTS_PATH || request->url() == FLIST_PATH
	 			|| request->url() == TIME_PATH
				|| request->url() == BEER_PROFILE_PATH
				|| request->url() == MQTT_PATH
	 			#ifdef ENABLE_LOGGING
	 			|| request->url() == LOGGING_PATH
	 			#endif
				#if EnableParasiteTempControl
			 	|| request->url() == ParasiteTempControlPath
			 	#endif
				#if SupportPressureTransducer
				|| request->url() == PRESSURE_PATH
				#endif
				#if EnableDHTSensorSupport
				|| request->url() == HUMIDITY_CONTROL_PATH
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


#if AUTO_CAP

void capControlStatusJson(JsonObject& obj){
	
	uint8_t mode=autoCapControl.mode();
	obj["m"] = mode;
	obj["c"] = autoCapControl.isCapOn()? 1:0;

	if(mode == AutoCapModeGravity){
		obj["g"]=autoCapControl.targetGravity();
	}else if (mode ==AutoCapModeTime){
		obj["t"]=autoCapControl.targetTime();
	}

#if SupportPressureTransducer
	PressureMonitorSettings* ps=theSettings.pressureMonitorSettings();
	if(ps->mode == PMModeControl){
		obj["pm"]=2;
		obj["psi"]= ps->psi;
	}
#endif

}

void stringAvailable(const char*);
void capStatusReport(void)
{

	DynamicJsonDocument doc(1024);
	JsonObject cap = doc.createNestedObject("cap");
	capControlStatusJson(cap);

	String out="A:";
	serializeJson(doc,out);
	stringAvailable(out.c_str());
}
#endif

void greeting(std::function<void(const char*)> sendFunc)
{
	char buf[512];
	// gravity related info., starting from "G"
	if(externalData.gravityDeviceEnabled()){
		externalData.sseNotify(buf);
		sendFunc(buf);
	} 

	// misc informatoin, including

	// RSSI && 
	const char *logname= brewLogger.currentLog();
	if(logname == NULL) logname="";
	SystemConfiguration *syscfg= theSettings.systemConfiguration();

	DynamicJsonDocument doc(1024);

	doc["nn"] = String(syscfg->titlelabel);
	doc["ver"] =  String(BPL_VERSION);
	doc["rssi"]= WiFi.RSSI();
	doc["tm"] = TimeKeeper.getTimeSeconds();
	doc["off"]= TimeKeeper.getTimezoneOffset();
	doc["log"] = String(logname);

#if AUTO_CAP
	JsonObject cap = doc.createNestedObject("cap");
	capControlStatusJson(cap);

#endif		 
#if EnableParasiteTempControl
	
	doc["ptc"]= serialized(parasiteTempController.getSettings());
#endif

#if EnableDHTSensorSupport
	JsonObject hum = doc.createNestedObject("rh");
	hum["m"] = humidityControl.mode();
	hum["t"] = humidityControl.targetRH();
#endif

	String out="A:";
	serializeJson(doc,out);

	sendFunc(out.c_str());

	// beer profile:
	String profile=String("B:") + theSettings.jsonBeerProfile();
	sendFunc(profile.c_str());
	//network status:

	String nwstatus=String("W:") + WiFiSetup.status();
	sendFunc(nwstatus.c_str());

}

#define GreetingInMainLoop 1



#if UseWebSocket == true

class AWSClient{
	uint32_t _clientId;
public:

	AWSClient(AsyncWebSocketClient* client){
		_clientId=client->id();
	}
	void text(AsyncWebSocket *websocket,const char* msg){
		websocket->text(_clientId,msg);
	}
	uint32_t clientId(void){ return _clientId; }

};

AsyncWebSocket ws(WS_PATH);

#if GreetingInMainLoop
AsyncWebSocketClient * _lastWSclient=NULL;
void sayHelloWS()
{
	if(! _lastWSclient) return;
	
	greeting([=](const char* msg){
			_lastWSclient->text(msg);
	});
	
	_lastWSclient = NULL;
}

#endif

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
	if(type == WS_EVT_CONNECT){
    	DBG_PRINTF("ws[%s][%u] connect\n", server->url(), client->id());
    	//client->printf("Hello Client %u :)", client->id());
    	client->ping();
		#if GreetingInMainLoop
		_lastWSclient = client;
		#else
		greeting([=](const char* msg){
			client->text(msg);
		});
		#endif
  	} else if(type == WS_EVT_DISCONNECT){
    	DBG_PRINTF("ws[%s] disconnect: %u\n", server->url(), client->id());
  	} else if(type == WS_EVT_ERROR){
    	DBG_PRINTF("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  	} else if(type == WS_EVT_PONG){
    	DBG_PRINTF("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
  	} else if(type == WS_EVT_DATA){
    	AwsFrameInfo * info = (AwsFrameInfo*)arg;
		
//		DBG_PRINTF("ws[%u] message[%u]:", client->id(), info->len);

//    	String msg = "";
    	if(info->final && info->index == 0 && info->len == len){
      		//the whole message is in a single frame and we got all of it's data

	        for(size_t i=0; i < info->len; i++) {
        	  //msg += (char) data[i];
        	  brewPi.write(data[i]);
        	}
//		    DBG_PRINTF("%s\n",msg.c_str());

		} else {
      		//message is comprised of multiple frames or the frame is split into multiple packets
/*      		if(info->index == 0){
        		if(info->num == 0)
        		DBG_PRINTF("ws[%u] frame[%u] start[%u]\n", client->id(), info->num, info->len);
      		}*/

//      		DBG_PRINTF("ws[%u] frame [%lu - %lu]: ", client->id(), info->num, info->index, info->index + len);

	        for(size_t i=0; i < info->len; i++) {
    	    	//msg += (char) data[i];
    	    	brewPi.write(data[i]);
        	}

      		//DBG_PRINTF("%s\n",msg.c_str());

			if((info->index + len) == info->len){
//				DBG_PRINTF("ws[%u] frame[%u] end[%lu]\n", client->id(), info->num, info->len);
//        		if(info->final){
//        			DBG_PRINTF("ws[%s][%u] %s-message end\n",  client->id());
//        		}
      		}
      	}
    }
}
#endif //#if UseWebSocket == true

void stringAvailable(const char *str)
{
	//DBG_PRINTF("BroadCast:%s\n",str);

#if UseWebSocket == true
	ws.textAll(str);
#endif

#if UseServerSideEvent == true
	sse.send(str);
#endif
}

void notifyLogStatus(void)
{
	externalData.waitFormula();
	const char *logname= brewLogger.currentLog();
	String logstr=(logname)? String(logname):String("");
	String status=String("A:{\"reload\":\"chart\", \"log\":\"") +  logstr + String("\"}");
	stringAvailable(status.c_str());
}

void reportRssi(void)
{
//	char buf[512];

	uint8_t mode, state;
	char unit;
	float beerSet, beerTemp, fridgeTemp, fridgeSet, roomTemp;
	float min,max;
	char statusLine[21];
	brewPi.getTemperatureSetting(&unit,&min,&max);
	brewPi.getAllStatus(&state, &mode, &beerTemp, &beerSet, &fridgeTemp, &fridgeSet, &roomTemp);
	display.getLine(3,statusLine);

	DynamicJsonDocument doc(1024);
	doc["rssi"]= WiFi.RSSI();
	doc["st"] = state;
	doc["md"] = String((char)mode);
	doc["bt"] = (int)(beerTemp*100);
	doc["bs"] = (int)(beerSet*100);
	doc["ft"] = (int)(fridgeTemp*100);
	doc["fs"] = (int)(fridgeSet*100);
	doc["rt"] = (int)(roomTemp*100);
	doc["sl"] = statusLine;
	doc["tu"] = String(unit);


#if EnableParasiteTempControl
	doc["ptc"] = String(parasiteTempController.getMode());
	doc["pt"] = parasiteTempController.getTimeElapsed();
	doc["ptctp"] = parasiteTempController.getTemp();
	doc["ptclo"] = parasiteTempController.getLowerBound();
	doc["ptcup"] = parasiteTempController.getUpperBound();
#endif

#if SupportPressureTransducer
	doc["pm"] = PressureMonitor.mode();
	doc["psi"] = (int) PressureMonitor.currentPsi();
#endif

#if EnableDHTSensorSupport
	if (humidityControl.sensorInstalled()){
		doc["h"]= humidityControl.humidity();
	}
#endif


	JsonObject G = doc.createNestedObject("G");
	G["u"] = externalData.lastUpdate();
	G["t"] = (int)(externalData.auxTemp() * 100);
	G["r"] = externalData.rssi();
	G["g"] = (int)(externalData.gravity() * 1000);


	String out="A:";
	serializeJson(doc,out);

	stringAvailable(out.c_str());
}


#if UseServerSideEvent
#if GreetingInMainLoop 

AsyncEventSourceClient *_lastClient=NULL;

void sayHelloSSE()
{
	if(! _lastClient) return;

	DBG_PRINTF("SSE Connect\n");
	greeting([=](const char* msg){
		_lastClient->send(msg);
	});
	_lastClient = NULL;
}

void onClientConnected(AsyncEventSourceClient *client)
{
	_lastClient = client;
}

#else
void onClientConnected(AsyncEventSourceClient *client){
	DBG_PRINTF("SSE Connect\n");
	greeting([=](const char* msg){
		client->send(msg);
	});
}
#endif
#endif

#if GreetingInMainLoop 
void sayHello()
{
#if UseServerSideEvent
	sayHelloSSE();
#endif 

#if UseWebSocket == true
	sayHelloWS();
#endif
}
#endif 

#define MAX_DATA_SIZE 256

class LogHandler:public AsyncWebHandler
{
public:

	void handleRequest(AsyncWebServerRequest *request){
/*		if( request->url() == IGNORE_MASK_PATH){
			if(request->hasParam("m")){
				uint32_t mask= request->getParam("m")->value().toInt();
				brewLogger.addIgnoredCalPointMask(mask);
				request->send(200,"application/json","{}");
			}else{
				request->send(404);
			}
		}else */
		if( request->url() == LOGLIST_PATH){
			if(request->hasParam("dl")){
				int index=request->getParam("dl")->value().toInt();
				char buf[36];
				brewLogger.getFilePath(buf,index);
				if(FileSystem.exists(buf)){
					request->send(FileSystem,buf,"application/octet-stream",true);
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
				bool cal=false;
				float tiltwater, hydroreading;
				if(request->hasParam("tw") && request->hasParam("hr")){
					cal=true;
					tiltwater=request->getParam("tw")->value().toFloat();
					hydroreading=request->getParam("hr")->value().toFloat();
				}

				if(brewLogger.startSession(filename.c_str(),cal)){
					if(cal){
						brewLogger.addTiltInWater(tiltwater,hydroreading);
						externalData.setCalibrating(true);
						DBG_PRINTF("Start BrweNCal log\n");
					}

					brewLogger.addCorrectionTemperature(externalData.hydrometerCalibration());

					request->send(200,"application/json","{}");
					notifyLogStatus();
				}else
					request->send(404);
			}else if(request->hasParam("stop")){
				DBG_PRINTF("Stop logging\n");
				brewLogger.endSession();
				externalData.setCalibrating(false);
				request->send(200,"application/json","{}");
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
	 	if(request->url() == CHART_DATA_PATH || request->url() ==LOGLIST_PATH
		  /*|| request->url() == IGNORE_MASK_PATH */) return true;
	 	return false;
	}
};
LogHandler logHandler;


class ExternalDataHandler:public AsyncWebHandler
{
private:
	char _buffer[MAX_DATA_SIZE+2];
	char *_data;

	size_t _dataLength;
	bool   _error;

	void processGravity(AsyncWebServerRequest *request,char data[],size_t length){
		if(length ==0) return request->send(500);;
		SystemConfiguration *syscfg=theSettings.systemConfiguration();
        uint8_t error;
		if(externalData.processGravityReport(data,length,request->authenticate(syscfg->username,syscfg->password),error)){
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
		externalData.loadConfig();
	}

	bool canHandle(AsyncWebServerRequest *request){
		DBG_PRINTF("req: %s\n", request->url().c_str());
	 	if(request->url() == GRAVITY_PATH	) return true;
	 	if(request->url() == GravityDeviceConfigPath) return true;
	 	if(request->url() == GravityFormulaPath) return true;
#if	SupportTiltHydrometer
	 	if(request->url() == TiltCommandPath) return true;
#endif

	 	return false;
	}

	void handleRequest(AsyncWebServerRequest *request){
#if	SupportTiltHydrometer
	 	if(request->url() == TiltCommandPath){
			 if(request->hasParam("scan")){
				 DBG_PRINTF("scan BLE\n");
				 tiltListener.scan([](int count,TiltHydrometerInfo *tilts){
					String ret="{\"tilts\":[";
						 for(int i=0;i<count;i++){
							 ret += String("{\"c\":")+ String(tilts[i].color) +
							 		String(",\"r\":")+ String(tilts[i].rssi) +
									String(",\"g\":")+ String(tilts[i].gravity) +
									String(",\"t\":")+ String(tilts[i].temperature) +
									((i==count-1)? String("}"): String("},"));
						 }
					ret += "]}";
					tiltScanResult(ret);
				 });
				 request->send(200);
			 }
			 return;
		 }
#endif

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
		if(request->url() == GravityFormulaPath){
			if(request->hasParam("a0") && request->hasParam("a1") 
				&& request->hasParam("a2") && request->hasParam("a3")
				&& request->hasParam("pt")){
				float coeff[4];
				coeff[0]=request->getParam("a0")->value().toFloat();
				coeff[1]=request->getParam("a1")->value().toFloat();
				coeff[2]=request->getParam("a2")->value().toFloat();
				coeff[3]=request->getParam("a3")->value().toFloat();
				uint32_t npt=(uint32_t) request->getParam("pt")->value().toInt();
				externalData.formula(coeff,npt);

				brewLogger.addIgnoredCalPointMask(npt & 0xFFFFFF);
  				
				request->send(200,"application/json","{}");
			}else{
				DBG_PRINTF("Invalid parameter\n");
  				request->send(400);
			}

			return;
		}
		// config
		if(request->method() == HTTP_POST){
  			if(externalData.processconfig(_data)){
		  		request->send(200,"application/json","{}");
			}else{
				request->send(400);
			}
		}//else{
			// get
		if(request->hasParam("data")){
			request->send(200,"application/json",theSettings.jsonGravityConfig());
		}else{
			// get the HTML
			request->redirect(request->url() + ".htm");
		    //request->send_P(200, "text/html", externalData.html());
		}
	}

	virtual void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)override final{
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
#if LegacyEspAsyncLibraries != true	
	virtual bool isRequestHandlerTrivial() override final {return false;}
#endif
};
ExternalDataHandler externalDataHandler;

IPAddress scanIP(char const *str)
	{
    	// DBG_PRINTF("Scan IP length=%d :\"%s\"\n",len,buffer);
    	// this doesn't work. the last byte always 0: ip.fromString(buffer);

    	int Parts[4] = {0,0,0,0};
    	int Part = 0;
		char* ptr=(char*)str;
    	for ( ; *ptr; ptr++)
    	{
	    char c = *ptr;
	    if ( c == '.' )
	    {
		    Part++;
		    continue;
	    }
	    Parts[Part] *= 10;
	    Parts[Part] += c - '0';
    	}

    	IPAddress sip( Parts[0], Parts[1], Parts[2], Parts[3] );
    	return sip;
	}

class NetworkConfig:public AsyncWebHandler
{
public:
	NetworkConfig(){}

	void handleRequest(AsyncWebServerRequest *request){
		if(request->url() == WIFI_SCAN_PATH) handleNetworkScan(request);
		else if(request->url() == WIFI_CONNECT_PATH) handleNetworkConnect(request);
		else if(request->url() == WIFI_DISC_PATH) handleNetworkDisconnect(request);
	}

	void handleNetworkScan(AsyncWebServerRequest *request){
		if(WiFiSetup.requestScanWifi())
			request->send(200,"application/json","{}");
		else 
			request->send(403);
	}

	void handleNetworkDisconnect(AsyncWebServerRequest *request){
		theSettings.systemConfiguration()->wifiMode=WIFI_AP;
		WiFiSetup.setMode(WIFI_AP);

		request->send(200,"application/json","{}");
	}

	
	void handleNetworkConnect(AsyncWebServerRequest *request){

		if(!request->hasParam("nw",true)){
			request->send(400);
			return;
		}
		
		SystemConfiguration *syscfg=theSettings.systemConfiguration();
		

			String ssid=request->getParam("nw",true)->value();
			const char *pass=NULL;
			if(request->hasParam("pass",true)){
				pass = request->getParam("pass",true)->value().c_str();
			}
			
			if(syscfg->wifiMode == WIFI_AP){
				// change to WIFI_STA mode
				syscfg->wifiMode = WIFI_AP_STA;
			}

			if(request->hasParam("ip",true) && request->hasParam("gw",true) && request->hasParam("nm",true)){
				DBG_PRINTF("static IP\n");
				IPAddress ip=scanIP(request->getParam("ip",true)->value().c_str());
				IPAddress gw=scanIP(request->getParam("gw",true)->value().c_str());
				IPAddress nm=scanIP(request->getParam("nm",true)->value().c_str());
				
				IPAddress dns=request->hasParam("dns",true)? scanIP(request->getParam("dns",true)->value().c_str()):IPAddress(0,0,0,0);

				WiFiSetup.connect(ssid.c_str(),pass, 
							ip,
							gw,
							nm,
							dns
				);
				// save to config
				syscfg->ip = ip;
				syscfg->gw = gw;
				syscfg->netmask = nm;
				syscfg->dns = dns;
			}else{
				WiFiSetup.staConfig();
				
				WiFiSetup.connect(ssid.c_str(),pass);
				DBG_PRINTF("dynamic IP\n");
				syscfg->ip = IPAddress(0,0,0,0);
				syscfg->gw = IPAddress(0,0,0,0);
				syscfg->netmask = IPAddress(0,0,0,0);
				syscfg->dns = IPAddress(0,0,0,0);
			}
			theSettings.save();

		#ifdef SaveWiFiConfiguration
		DBG_PRINTF("SSID:%s\n",ssid.c_str());
		theSettings.setWiFiConfiguration(ssid.c_str(),pass);
		#endif
		//MDNS.notifyAPChange();		
		theSettings.save();

		request->send(200,"application/json","{}");
	}

	bool canHandle(AsyncWebServerRequest *request){
		if(request->url() == WIFI_SCAN_PATH) return true; 
		else if(request->url() == WIFI_CONNECT_PATH) return true;
		else if(request->url() == WIFI_DISC_PATH) return true;

	 	return false;
	}

	#if !LegacyEspAsyncLibraries
	virtual bool isRequestHandlerTrivial() override final {return false;}
	#endif
};

NetworkConfig networkConfig;

void wiFiEvent(const char* msg){
	char *buff=(char*)malloc(strlen(msg) +3);
	sprintf(buff,"W:%s",msg);
	stringAvailable(buff);
	free(buff);

	if(WiFi.status() == WL_CONNECTED){
		DBG_PRINTF("channel:%d, BSSID:%s\n",WiFi.channel(),WiFi.BSSIDstr().c_str());
		if(! TimeKeeper.isSynchronized())TimeKeeper.updateTime();
	}
}

void tiltScanResult(String& result){
	String report="T:" + result;
	stringAvailable(report.c_str());
}
//{brewpi


// global class objects static and defined in class cpp and h files

// instantiate and configure the sensors, actuators and controllers we want to use


/* Configure the counter and delay timer. The actual type of these will vary depending upon the environment.
* They are non-virtual to keep code size minimal, so typedefs and preprocessing are used to select the actual compile-time type used. */
TicksImpl ticks = TicksImpl(TICKS_IMPL_CONFIG);
DelayImpl wait = DelayImpl(DELAY_IMPL_CONFIG);

DisplayType realDisplay;
DisplayType DISPLAY_REF display = realDisplay;

ValueActuator alarmActuator;

#ifdef ESP8266_WiFi


WiFiServer server(23);
WiFiClient serverClient;
#endif
void handleReset()
{
#if defined(ESP8266) || defined(ESP32)
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

#if FS_EEPROM
	eepromAccess.begin();
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
		buzzer.setActive(alarmActuator.isActive() && !buzzer.isActive());
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

typedef enum _StatusLineDisplayItem{
StatusLineDisplayIP=0,
StatusLineDisplayTime
}StatusLineDisplayItem;

#define DisplayTimeDuration 8
#define DisplayIPDuration 3

class StatusLine{
protected:
	static time_t _displayTime;
	static time_t _switchTime;
	static StatusLineDisplayItem _displaying;

	static void _printTime(time_t now){
		struct tm t;
		if(_displayTime == now) return;
		_displayTime = now;
		makeTime(TimeKeeper.getLocalTimeSeconds(),t);
		char buf[21];
		sprintf(buf,"%d/%02d/%02d %02d:%02d:%02d",t.tm_year,t.tm_mon,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
		display.printStatus(buf);
	}
	static void _printIP(void){
		
		IPAddress ip =(WiFiSetup.isApMode())? WiFi.softAPIP():WiFi.localIP();
		char buf[21];
		sprintf(buf,"IP:%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
		display.printStatus(buf);
	}

public:
	StatusLine(){
	}


	static void loop(time_t now){	
		if(now == _displayTime) return;

		if(_displaying == StatusLineDisplayIP){
			if(now - _switchTime > DisplayIPDuration){
				_printTime(now);
				_switchTime = now;
				_displaying = StatusLineDisplayTime;
			}
		}else if(_displaying == StatusLineDisplayTime){
			if(now - _switchTime > DisplayTimeDuration){
				_printIP();
				_switchTime = now;
				_displaying = StatusLineDisplayIP;
			}else _printTime(now);
		}
	}
};
time_t StatusLine::_displayTime;
time_t  StatusLine::_switchTime;
StatusLineDisplayItem StatusLine::_displaying = StatusLineDisplayTime;


StatusLine statusLine;
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
#if defined(ESP32)
  	if(!SPIFFS.begin(true)){
#else
	if(!FileSystem.begin()){
#endif
  		// TO DO: what to do?
  		DBG_PRINTF("FileSystem.begin() failed!\n");
  	}else{
  		DBG_PRINTF("FileSystem.begin() Success.\n");
  	}


#ifdef EARLY_DISPLAY
	DBG_PRINTF("Init LCD...\n");
	display.init();
	display.printAt_P(1,0,PSTR("Initialize WiFi"));
	display.updateBacklight();
	DBG_PRINTF("LCD Initialized..\n");
#endif


	// try open configuration
	theSettings.load();

	SystemConfiguration *syscfg=theSettings.systemConfiguration();
	
	display.setAutoOffPeriod(syscfg->backlite);
	
	#ifdef ENABLE_LOGGING
//  	dataLogger.loadConfig();
  	#endif


	//1. Start WiFi
	DBG_PRINTF("Starting WiFi...\n");
	WiFiMode wifiMode= (WiFiMode) syscfg->wifiMode;
	WiFiSetup.staConfig(IPAddress(syscfg->ip),IPAddress(syscfg->gw),IPAddress(syscfg->netmask),IPAddress(syscfg->dns));
	WiFiSetup.onEvent(wiFiEvent);
#ifdef SaveWiFiConfiguration
	WiFiConfiguration *wifiCon=theSettings.getWifiConfiguration();

	if(strlen(syscfg->hostnetworkname)>0)
		WiFiSetup.begin(wifiMode,syscfg->hostnetworkname,syscfg->password,
					wifiCon->ssid[0]? wifiCon->ssid:NULL,
					wifiCon->pass[0]? wifiCon->pass:NULL);
	else // something wrong with the file
		WiFiSetup.begin(wifiMode,DEFAULT_HOSTNAME,DEFAULT_PASSWORD);
#else
	if(strlen(syscfg->hostnetworkname)>0)
		WiFiSetup.begin(wifiMode,syscfg->hostnetworkname,syscfg->password);
	else // something wrong with the file
		WiFiSetup.begin(wifiMode,DEFAULT_HOSTNAME,DEFAULT_PASSWORD);
#endif

  	DBG_PRINTF("WiFi Done!\n");

	// get time
	initTime(WiFiSetup.isApMode());

	if (!MDNS.begin(syscfg->hostnetworkname)) {
			DBG_PRINTF("Error setting mDNS responder\n");
	}else{
		MDNS.addService("http", "tcp", 80);
	}

	// TODO: SSDP responder


	//3. setup Web Server
	webServer=new AsyncWebServer(syscfg->port);
	// start WEB update pages.
#if (DEVELOPMENT_OTA == true) || (DEVELOPMENT_FILEMANAGER == true)
	ESPUpdateServer_setup(syscfg->username,syscfg->password);
#endif

	//3.1 Normal serving pages
	//3.1.1 status report through SSE

#if ResponseAppleCNA == true
	webServer->addHandler(&appleCNAHandler);
#endif

#if UseWebSocket == true
	ws.onEvent(onWsEvent);
	webServer->addHandler(&ws);
#endif

#if UseServerSideEvent == true
	sse.onConnect(onClientConnected);
	webServer->addHandler(&sse);
#endif

	webServer->addHandler(&brewPiWebHandler);

	webServer->addHandler(&logHandler);

	externalDataHandler.loadConfig();
	webServer->addHandler(&externalDataHandler);

	webServer->addHandler(&networkConfig);
	//3.1.2 file system is part of the serving pages
	//server.serveStatic("/", file system, "/","public, max-age=259200"); // 3 days

#if defined(ESP32)
	webServer->on("/fs",[](AsyncWebServerRequest *request){
		request->send(200,"","totalBytes:" +String(SPIFFS.totalBytes()) +
		" usedBytes:" + String(SPIFFS.usedBytes()) +
		" heap:"+String(ESP.getFreeHeap()));
		//testSPIFFS();
	});
#else
	webServer->on("/fs",[](AsyncWebServerRequest *request){
		FSInfo fs_info;
		FileSystem.info(fs_info);
		request->send(200,"","totalBytes:" +String(fs_info.totalBytes) +
		" usedBytes:" + String(fs_info.usedBytes)+" blockSize:" + String(fs_info.blockSize)
		+" pageSize:" + String(fs_info.pageSize)
		+" freesketch:" + String(ESP.getFreeSketchSpace())
		+" heap:"+String(ESP.getFreeHeap()));
		//testSPIFFS();
	});
#endif
	// 404 NOT found.
  	//called when the url is not defined here
	webServer->onNotFound([](AsyncWebServerRequest *request){
		request->send(404);
	});

	//4. start Web server
	webServer->begin();
	DBG_PRINTF("HTTP server started\n");
#if SupportPressureTransducer
	PressureMonitor.begin();
#endif
	// 5. try to connnect Arduino
	brewpi_setup();
  	brewPi.begin(stringAvailable);
	//make sure externalData  is initialized.
	if(brewLogger.begin()){
		// resume, update calibrating information to external data
		externalData.setCalibrating(brewLogger.isCalibrating());
		DBG_PRINTF("Start BrweNCal log:%d\n",brewLogger.isCalibrating());
	}
	
	brewKeeper.begin();

	#if AUTO_CAP
	//Note: necessary to call after brewpi_setup() so that device has been installed.
	autoCapControl.begin();
	#endif

#if SupportTiltHydrometer
	tiltListener.begin();
#endif

#if EnableParasiteTempControl
	parasiteTempController.init();
#endif


#ifdef STATUS_LINE
	statusLine.loop(0);
#endif
#ifdef EMIWorkaround
	_lcdReinitTime = millis();
#endif

#if SupportMqttRemoteControl
	//mqtt
	mqttRemoteControl.begin();
#endif

	#if EnableDHTSensorSupport
	humidityControl.begin();
	#endif


}

uint32_t _rssiReportTime;
#define RssiReportPeriod 5

void loop(void){
//{brewpi
#if BREWPI_SIMULATE
	simulateLoop();
#else
	brewpiLoop();
#endif
//}brewpi
#ifdef ESP8266
	MDNS.update();
#endif
#if EnableParasiteTempControl
	parasiteTempController.run();
#endif

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
	statusLine.loop(now);
#endif
	if( (now - _rssiReportTime) > RssiReportPeriod){
		_rssiReportTime =now;
		reportRssi();
	}

  	brewKeeper.keep(now);

  	brewPi.loop();

 	brewLogger.loop();

#if SupportMqttRemoteControl
	mqttRemoteControl.loop();
#endif

 	#ifdef ENABLE_LOGGING

 	dataLogger.loop(now);
 	#endif
	
	#if AUTO_CAP
	if(autoCapControl.autoCapOn(now,externalData.gravity(true))){
		capStatusReport();
	}
	#endif
	
	#if SupportPressureTransducer
	PressureMonitor.loop();
	#endif
	
	#if SupportTiltHydrometer
	tiltListener.loop();
	#endif

	#if EnableDHTSensorSupport
	humidityControl.loop();
	#endif

	#if GreetingInMainLoop
	sayHello();
	#endif

	if(!IS_RESTARTING){
		WiFiSetup.stayConnected();
	}

  	if(_systemState ==SystemStateRestartPending){
	  	_time=millis();
	  	_systemState =SystemStateWaitRestart;
  	}else if(_systemState ==SystemStateWaitRestart){
  		if((millis() - _time) > TIME_RESTART_TIMEOUT){
  			if(_disconnectBeforeRestart){
  				WiFi.disconnect();
  				WiFiSetup.setAutoReconnect(false);
  				delay(1000);
  			}
  			ESP.restart();
  		}
  	}
}
