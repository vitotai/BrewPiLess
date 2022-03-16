#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <ESPmDNS.h>
#endif

//needed for library
#include <DNSServer.h>
#include <FS.h>
#include "Config.h"
#include "WiFiSetup.h"

WiFiSetupClass WiFiSetup;

#define TimeForRecoveringNetwork 8000
#define TimeWaitToRecoverNetwork 60000

#if SerialDebug == true
#define DebugOut(a) DebugPort.print(a)
#define DBG_PRINTF(...) DebugPort.printf(__VA_ARGS__)
#define DBG_PRINTLN(a) DebugPort.println(a)
#else
#define DebugOut(a)
#define DBG_PRINTF(...)
#define DBG_PRINTLN(a) 
#endif

#if SerialDebug
#define wifi_info(a)	DBG_PRINTF("%s,SSID:\"%s\" pass:\"%s\" IP:%s, gw:%s, dns:%s\n",(a),WiFi.SSID().c_str(),WiFi.psk().c_str(),WiFi.localIP().toString().c_str(),WiFi.gatewayIP().toString().c_str(),WiFi.dnsIP().toString().c_str())
#else
#define wifi_info(a)
#endif

void WiFiSetupClass::staConfig(IPAddress ip,IPAddress gw, IPAddress nm,IPAddress dns){
	_ip=ip;
	_gw=gw;
	_nm=nm;
	_dns=dns;
}

void WiFiSetupClass::setMode(WiFiMode mode){
	DBG_PRINTF("WiFi mode from:%d to %d\n",_mode,mode);	

	if(mode == _mode) return;
	_mode = mode;
	_wifiState = WiFiStateModeChangePending;
}

void WiFiSetupClass::enterBackupApMode(void)
{
	WiFi.mode(WIFI_AP_STA);
	createNetwork();
	setupApService();
}

void WiFiSetupClass::createNetwork(){
	if(strlen(_apPassword)>=8)
		WiFi.softAP(_apName, _apPassword);
	else
		WiFi.softAP(_apName);
	
	DBG_PRINTF("\ncreate network:%s pass:%s\n",_apName, _apPassword);
}

void WiFiSetupClass::setupApService(void)
{
	dnsServer.reset(new DNSServer());
	dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
	dnsServer->start(DNS_PORT, "*", WiFi.softAPIP());
	delay(500);
}
bool WiFiSetupClass::isApMode(){
	return WiFi.getMode() == WIFI_AP;
}

void WiFiSetupClass::begin(WiFiMode mode, char const *ssid,const char *passwd,char const* targetSSID,const char *targetPass)
{
	wifi_info("begin:");
	
	if(targetSSID && targetSSID[0]){
		if(_targetSSID) free((void*)_targetSSID);
		_targetSSID=strdup(targetSSID);
	}
	if(targetPass && targetPass[0]){
		if(_targetPass) free((void*)_targetPass);
		_targetPass=strdup(targetPass);
	}

	_mode= mode;
	WiFiMode mode2use = (_mode == WIFI_OFF)? WIFI_AP_STA:_mode;
	
	DBG_PRINTF("\nSaved SSID:\"%s\" targetSSID:%s\n",WiFi.SSID().c_str(),targetSSID? targetSSID:"NULL");
	DBG_PRINTF("\nAP mode:%d, used;%d autoReconect:%d\n",mode,mode2use,WiFi.getAutoReconnect());

	if( (mode2use == WIFI_STA || mode2use == WIFI_AP_STA) 
		 && _targetSSID == NULL 
		 && (WiFi.SSID() == "[Your SSID]" || WiFi.SSID() == "" || WiFi.SSID() == NULL)){
			DBG_PRINTF("Invalid SSID!");
			mode2use = WIFI_AP;
	}
	_apName=(ssid == NULL || *ssid=='\0')? DEFAULT_HOSTNAME:ssid;	
	_apPassword=(passwd !=NULL && *passwd=='\0')? NULL:passwd;

	WiFi.setAutoConnect(true);
	WiFi.mode(mode2use);
	// start AP
	if( mode2use == WIFI_AP || mode2use == WIFI_AP_STA){
		createNetwork();
		setupApService();
	}

	if( mode2use == WIFI_STA || mode2use == WIFI_AP_STA){
		if(_ip !=INADDR_NONE && _ip !=IPAddress(0,0,0,0)){
				DBG_PRINTF("Config IP:%s, _gw:%s, _nm:%s\n",_ip.toString().c_str(),_gw.toString().c_str(),_nm.toString().c_str());
				WiFi.config(_ip,_gw,_nm,_dns);
		}else{
			// the weird printout of "[NO IP]" implies that explicitly specification of DHCP 
			// might be necessary.
			DBG_PRINTF("Dynamic IP\n");
			WiFi.config( IPAddress(0,0,0,0),IPAddress(0,0,0,0), IPAddress(0,0,0,0));
		}
		WiFi.setAutoReconnect(true);		
		
		wl_status_t status;
		if(targetSSID) status= WiFi.begin(targetSSID,targetPass);
		else status=WiFi.begin();
		DBG_PRINTF("WiFi.begin() return:%d\n",status);
		(void) status;
		_time=millis();
	}
	_wifiState=(mode2use == WIFI_AP)? WiFiStateDisconnected:WiFiStateConnectionRecovering;
}

bool WiFiSetupClass::connect(char const *ssid,const char *passwd,IPAddress ip,IPAddress gw, IPAddress nm, IPAddress dns){
	DBG_PRINTF("Connect to %s pass:%s, ip=%s\n",ssid, passwd,ip.toString().c_str());

	if(_targetSSID) free((void*)_targetSSID);
	_targetSSID=strdup(ssid);
	if(_targetPass) free((void*)_targetPass);
	_targetPass=(passwd)? strdup(passwd):NULL;

	_ip=ip;
	_gw=gw;
	_nm=nm;
	_dns=dns;

	_wifiState = WiFiStateChangeConnectPending;
	// mode change implicitly in AP mode.
	if(_mode == WIFI_AP){
		_mode = WIFI_AP_STA;
	}
	return true;
}

bool WiFiSetupClass::disconnect(void){
	DBG_PRINTF("Disconnect Request\n");
	_wifiState = WiFiStateDisconnectPending;
	return true;
}

bool WiFiSetupClass::isConnected(void){
	return WiFi.status() == WL_CONNECTED;
}

void WiFiSetupClass::onStatus(){
	if(_eventHandler){
		_eventHandler(status().c_str());
	}
}

String WiFiSetupClass::status(void){
	String ret;
	ret  = String("{\"md\":") + String(_mode) + String(",\"con\":") + String((WiFi.status() == WL_CONNECTED)? 1:0);

	if(_mode != WIFI_AP){
		ret += String(",\"ssid\":\"") + WiFi.SSID() 
			 + String("\",\"ip\":\"") + WiFi.localIP().toString()
			 + String("\",\"gw\":\"") + WiFi.gatewayIP().toString()
			 + String("\",\"nm\":\"") + WiFi.subnetMask().toString() + String("\"");
	}

	ret += String("}");
	return ret;
}

bool WiFiSetupClass::stayConnected(void)
{
	if(WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA){
		dnsServer->processNextRequest();
//		if(_mode == WIFI_AP) return true;
	}
	
	if(_wifiState==WiFiStateChangeConnectPending){
			DBG_PRINTF("Change Connect\n");
			//if(WiFi.status() == WL_CONNECTED){
			WiFi.disconnect();
			//DBG_PRINTF("Disconnect\n");
			//}
			WiFiMode mode= WiFi.getMode();
			if(mode == WIFI_AP){
				WiFi.mode(WIFI_AP_STA);
				#if ESP8266
				MDNS.notifyAPChange();
				#endif
			}

			if(_ip != INADDR_NONE){
				WiFi.config(_ip,_gw,_nm,_dns);
			}
			if(_targetSSID)
				WiFi.begin(_targetSSID,_targetPass);
			else
				WiFi.begin();
			_reconnect =0;
			_wifiState = WiFiStateConnectionRecovering;

			wifi_info("**try:");
			_time=millis();

	}else if(_wifiState==WiFiStateDisconnectPending){
			WiFi.disconnect();
			DBG_PRINTF("Enter AP Mode\n");
			_wifiState =WiFiStateDisconnected;
			return true;
	}else if(_wifiState==WiFiStateModeChangePending){
			WiFiMode mode= WiFi.getMode();

			if(mode == WIFI_AP_STA){
				if( _mode == WIFI_AP){
					DBG_PRINTF("Change from AP_STA to AP\n");
					WiFi.disconnect();
					_wifiState =WiFiStateDisconnected;
				}else if (_mode == WIFI_STA){
				}
				WiFi.mode(_mode);
				#if ESP8266
				MDNS.notifyAPChange();
				#endif


			}else if(mode == WIFI_STA){
				if( _mode == WIFI_AP_STA){
					WiFi.mode(_mode);
				#if ESP8266
				MDNS.notifyAPChange();
				#endif

					createNetwork();
					setupApService();
				}else if (_mode == WIFI_AP){
					//WiFi.disconnect();
					_wifiState =WiFiStateDisconnected;
					WiFi.mode(_mode);
					#if ESP8266
					MDNS.notifyAPChange();
					#endif

				}

			}else if(mode == WIFI_AP){
				if(_mode == WIFI_AP_STA){
					WiFi.mode(_mode);
					#if ESP8266
					MDNS.notifyAPChange();
					#endif
				}
				WiFi.begin();
				_wifiState =WiFiStateConnectionRecovering;
				_time=millis();

				if(_mode == WIFI_STA){
					if(WiFi.SSID() == NULL || WiFi.SSID() == ""){
						WiFi.mode(WIFI_AP_STA);
						#if ESP8266
						MDNS.notifyAPChange();
						#endif
					}
					// just keep WIFI_AP_Mode in case Network isn't specified
				}
			}

	}else if(WiFi.status() != WL_CONNECTED){
 			if(_wifiState==WiFiStateConnected)
 			{
				wifi_info("**disc:");
				if(_mode != WIFI_AP){
					onStatus();
					DBG_PRINTF("Lost Network.WiFi.status()= %d\n",WiFi.status());
					_wifiState = WiFiStateConnectionRecovering;
					if(_targetSSID) WiFi.begin(_targetSSID,_targetPass);
					else WiFi.begin();
					WiFi.setAutoReconnect(true);
					_time=millis();
				}
			}else if (_wifiState==WiFiStateConnectionRecovering){
				// if sta mode, turn on AP mode
				if(millis() - _time > TimeForRecoveringNetwork){
					DBG_PRINTF("Stop recovering\n");
					// WiFi.disconnect();
					// enter AP mode, or the underlying WiFi stack would keep searching and block
					//  connections to AP mode.

					WiFi.setAutoReconnect(false);
					// WiFi.mode(WIFI_AP);
					
					if(_mode == WIFI_STA && WiFi.getMode() == WIFI_STA){
						// create a wifi
						enterBackupApMode();
					} // _mode == WIFI_STA

					_time = millis();
					_wifiState =WiFiStateDisconnected;
				} // millis() - _time > TimeForRecoveringNetwork
			} else if(_wifiState==WiFiStateDisconnected){
				if(_mode == WIFI_AP){

				 // Can't "return" here, if it returns here, the "canning networking will never run."
				 //  // don't try to restore network, since there is none to be rediscover
				}else{
				// in AP_STA or STA mode
					if( millis() -  _time  > TimeWaitToRecoverNetwork){
  						DBG_PRINTF("Start recovering, wifi.status=%d\n",WiFi.status());
						// WiFi.mode(WIFI_AP_STA);
						DBG_PRINTF("retry SSID:%s\n",_targetSSID? _targetSSID:"NULL");
						if(_targetSSID) WiFi.begin(_targetSSID,_targetPass);
						else WiFi.begin();

						WiFi.setAutoReconnect(true);

						_wifiState = WiFiStateConnectionRecovering;
						_time = millis();
					}
				}
		    }
 	} // WiFi.status() != WL_CONNECTED 
 	else // connected
 	{
		 if(_mode == WIFI_AP){
			 DBG_PRINTF("Connected in AP_mode\n");
		 }else{
 			byte oldState=_wifiState;
 			_wifiState=WiFiStateConnected;
 			_reconnect=0;
 			if(oldState != _wifiState){
				
				if(WiFi.getMode() != _mode){
					DBG_PRINTF("Change mode to desired:%d from %d\n",_mode,WiFi.getMode());
					WiFi.mode(_mode);
					#if ESP8266
					MDNS.notifyAPChange();
					#endif
				}
				
				wifi_info("WiFi Connected:");
				onStatus();
			}
		 }
  } // end of connected

	
	if(_wifiScanState == WiFiScanStatePending){
		String nets=scanWifi();
		_wifiScanState = WiFiScanStateNone;
		if(_eventHandler) _eventHandler(nets.c_str());
	}

	return false;
}

bool WiFiSetupClass::requestScanWifi(void) {
	if(_wifiScanState == WiFiScanStateNone){
		_wifiScanState = WiFiScanStatePending;
		return true;
	}
	return false;
}

String WiFiSetupClass::scanWifi(void) {
	bool apmode=false;
	if(WiFi.getMode() == WIFI_AP){
		apmode=true;
		WiFi.mode(WIFI_AP_STA);
		#if ESP8266
		MDNS.notifyAPChange();
		#endif
	}

	String rst="{\"list\":[";
	
	DBG_PRINTF("Scan Networks...\n");
	int n = WiFi.scanNetworks();
    DBG_PRINTF("Scan done");
    if (n == 0) {
    	DBG_PRINTF("No networks found");
    } else {
      	//sort networks by RSSI
      	int indices[n];
      	for (int i = 0; i < n; i++) {
        	indices[i] = i;
    	}
      	// bubble sort
      	for (int i = 0; i < n; i++) {
        	for (int j = i + 1; j < n; j++) {
          		if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
            		std::swap(indices[i], indices[j]);
          		}	
        	}
      	}

	    // remove duplicates ( must be RSSI sorted )
        String cssid;
        for (int i = 0; i < n; i++) {
        	if (indices[i] == -1) continue;
          	cssid = WiFi.SSID(indices[i]);
          	for (int j = i + 1; j < n; j++) {
            	if (cssid == WiFi.SSID(indices[j])) {
              		DBG_PRINTF("DUP AP: ");
					DBG_PRINTF(WiFi.SSID(indices[j]).c_str());
              		indices[j] = -1; // set dup aps to index -1
            	}
          	}
        }
		
      	//display networks in page
		bool comma=false; // i==0 might not the "first", might be duplicated.
      	for (int i = 0; i < n; i++) {
        	if (indices[i] == -1) continue; // skip dups
        	DBG_PRINTLN(WiFi.SSID(indices[i]));
	        DBG_PRINTLN(WiFi.RSSI(indices[i]));
        	//int quality = getRSSIasQuality(WiFi.RSSI(indices[i]));
			String item=String("{\"ssid\":\"") + WiFi.SSID(indices[i]) + 
			String("\",\"rssi\":") + WiFi.RSSI(indices[i]) +
			String(",\"enc\":") + 
			#if defined(ESP32)
			 String((WiFi.encryptionType(indices[i]) != WIFI_AUTH_OPEN)? "1":"0")
			#else
			 String((WiFi.encryptionType(indices[i]) != ENC_TYPE_NONE)? "1":"0")
			#endif
			+ String("}");
			if(comma){
				rst += ",";	
			}else{
				comma=true;
			}
			rst += item;
      	}
    }
	rst += "]}";
	DBG_PRINTF("scan result:%s\n",rst.c_str());
	if(apmode){
		WiFi.mode(WIFI_AP);
		#if ESP8266
		MDNS.notifyAPChange();
		#endif
	}

	return rst;
}
