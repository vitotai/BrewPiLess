#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include "Config.h"
#include "WiFiSetup.h"

WiFiSetupClass WiFiSetup;

#define TimeForRescueAPMode 60000
#define TimeForRecoverNetwork 120000

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
#define wifi_info(a)	DBG_PRINTF("%s,SSID:%s pass:%s IP:%s, gw:%s\n",(a),WiFi.SSID().c_str(),WiFi.psk().c_str(),WiFi.localIP().toString().c_str(),WiFi.gatewayIP().toString().c_str())
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
	DBG_PRINTF("WiFi mode from:%d to %d\n",_mode,_mode);	

	if(mode == _mode) return;
	_mode = mode;
	_wifiState = WiFiStateModeChangePending;
}

void WiFiSetupClass::enterBackupApMode(void)
{
	WiFi.mode(WIFI_AP_STA);
	createNetwork();
}

void WiFiSetupClass::createNetwork(){
	if(strlen(_apPassword)>=8)
		WiFi.softAP(_apName, _apPassword);
	else
		WiFi.softAP(_apName);
}

void WiFiSetupClass::setupApService(void)
{
	dnsServer.reset(new DNSServer());
	dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
	dnsServer->start(DNS_PORT, "*", WiFi.softAPIP());
	delay(500);
}


void WiFiSetupClass::begin(WiFiMode mode, char const *ssid,const char *passwd)
{
	wifi_info("begin:");
	

	_mode= (mode==WIFI_OFF)? WIFI_AP_STA:mode;


	DBG_PRINTF("\nSaved SSID:\"%s\"\n",WiFi.SSID().c_str());
	DBG_PRINTF("\nAP mode:%d, used;%d\n",mode,_mode);
	if(WiFi.SSID() == "[Your SSID]"){
			DBG_PRINTF("Invalid SSID!");
			_mode = WIFI_AP;
	}
	_apName=(ssid == NULL || *ssid=='\0')? DEFAULT_HOSTNAME:ssid;
	
	_apPassword=(passwd !=NULL && *passwd=='\0')? NULL:passwd;

	// let the underlined library do the reconnection jobs.
	WiFi.setAutoConnect(_autoReconnect);

	WiFi.mode(_mode);
	// start AP
	if( _mode == WIFI_AP || _mode == WIFI_AP_STA){
		_apMode=true;
		createNetwork();
		setupApService();
	}

	if( _mode == WIFI_STA || _mode == WIFI_AP_STA){
		_apMode=false;
		if(_ip !=INADDR_NONE){
				WiFi.config(_ip,_gw,_nm);
		}else{
			// the weird printout of "[NO IP]" implies that explicitly specification of DHCP 
			// might be necessary.
			WiFi.config(INADDR_NONE,INADDR_NONE,INADDR_NONE);
		}
		
		WiFi.setAutoReconnect(true);
		WiFi.begin();
		
		_time=millis();
	}
	DBG_PRINTF("\ncreate network:%s pass:%s\n",_apName, passwd);
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
	_apMode =false;
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

void WiFiSetupClass::onConnected(){
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
	if(_apMode){
		dnsServer->processNextRequest();
		return true;
	}
	
	if(_wifiState==WiFiStateChangeConnectPending){
			DBG_PRINTF("Change Connect\n");
			//if(WiFi.status() == WL_CONNECTED){
			WiFi.disconnect();
			//DBG_PRINTF("Disconnect\n");
			//}
			if(_ip != INADDR_NONE){
				WiFi.config(_ip,_gw,_nm,_dns);
			}
			WiFi.begin(_targetSSID,_targetPass);
			_reconnect =0;
			_wifiState = WiFiStateConnecting;

			wifi_info("**try:");
			_time=millis();

	}else if(_wifiState==WiFiStateDisconnectPending){
			WiFi.disconnect();
			DBG_PRINTF("Enter AP Mode\n");
    		_apMode=true;
			_wifiState =WiFiStateDisconnected;
			return true;
	}else if(_wifiState==WiFiStateModeChangePending){
			WiFiMode mode= WiFi.getMode();

			if(mode == WIFI_AP_STA){
				if( _mode == WIFI_AP){
					//WiFi.disconnect();
					_wifiState =WiFiStateDisconnected;
				}else if (_mode == WIFI_STA){
				}
				WiFi.mode(_mode);

			}else if(mode == WIFI_STA){
				if( _mode == WIFI_AP_STA){
					WiFi.mode(_mode);
					createNetwork();
				}else if (_mode == WIFI_AP){
					//WiFi.disconnect();
					_wifiState =WiFiStateDisconnected;
					WiFi.mode(_mode);
				}

			}else if(mode == WIFI_AP){
				if(_mode == WIFI_AP_STA) WiFi.mode(_mode);
				WiFi.begin();
				_wifiState =WiFiStateConnectionRecovering;
				_time=millis();

				if(_mode == WIFI_STA){
					if(WiFi.SSID() == NULL || WiFi.SSID() == "")
						WiFi.mode(WIFI_AP_STA);
					// just keep WIFI_AP_Mode in case Network isn't specified
				}
			}

	}else if(WiFi.status() != WL_CONNECTED){
 			if(_wifiState==WiFiStateConnected)
 			{
				wifi_info("**disc:");

				_time=millis();
				DBG_PRINTF("Lost Network. auto reconnect %d\n",_autoReconnect);
				_wifiState = WiFiStateConnectionRecovering;
				return true;
			}else if (_wifiState==WiFiStateConnectionRecovering){
				// if sta mode, turn on AP mode
				if(millis() - _time > TimeForRescueAPMode){
					DBG_PRINTF("Stop recovering\n");
					_time = millis();
					_wifiState =WiFiStateDisconnected;
					WiFi.setAutoConnect(false);
					if(_mode == WIFI_STA){
						// create a wifi
						WiFi.mode(WIFI_AP_STA);
						createNetwork();
					} // _mode == WIFI_STA
				} // millis() - _time > TimeForRescueAPMode
			} else if(_wifiState==WiFiStateDisconnected){ // _wifiState == WiFiStateConnectionRecovering
				if( millis() -  _time  > TimeForRecoverNetwork){
  					DBG_PRINTF("Start recovering\n");
						WiFi.setAutoConnect(true);
						_wifiState = WiFiStateConnectionRecovering;
						_time = millis();
				}
		  }
 	} // WiFi.status() != WL_CONNECTED 
 	else // connected
 	{
 			byte oldState=_wifiState;
 			_wifiState=WiFiStateConnected;
 			_reconnect=0;
 			if(oldState != _wifiState){
				if(WiFi.getMode() != _mode){
					WiFi.mode(_mode);
				}
				onConnected();
				return true;
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
			String(",\"enc\":") +  String((WiFi.encryptionType(indices[i]) != ENC_TYPE_NONE)? "1":"0")
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
	return rst;
}