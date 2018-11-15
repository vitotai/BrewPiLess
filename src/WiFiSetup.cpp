#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include "config.h"
#include "WiFiSetup.h"

WiFiSetupClass WiFiSetup;

#define TimeForRescueAPMode 60000

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

void WiFiSetupClass::staConfig(IPAddress ip,IPAddress gw, IPAddress nm){
	_ip=ip;
	_gw=gw;
	_nm=nm;
}

void WiFiSetupClass::setMode(WiFiMode mode){
	_mode = mode;
}

void WiFiSetupClass::enterApMode(void)
{
	WiFi.mode(WIFI_AP);
	_apMode=true;
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
	_mode=mode;
	_apName=ssid;
	_apPassword=passwd;
	WiFi.setAutoConnect(_autoReconnect);
	WiFi.mode(_mode);

	if( _mode == WIFI_AP){
		DBG_PRINTF("\nAP mode\n");
		_apMode=true;
		WiFi.softAP(_apName, _apPassword);

	}else{
		if(_mode == WIFI_AP_STA){
			DBG_PRINTF("\nAP_STA mode\n");
			WiFi.softAP(_apName, _apPassword);
		}else{
			DBG_PRINTF("\nSTA mode\n");
		}
		_apMode=false;
		if(_ip !=INADDR_NONE){
			WiFi.config(_ip,_gw,_nm);
		}
		WiFi.begin();
		_time=millis();
	}
	setupApService();
	DBG_PRINTF("\ncreate network:%s pass:%s\n",ssid, passwd);
}

bool WiFiSetupClass::connect(char const *ssid,const char *passwd,IPAddress ip,IPAddress gw, IPAddress nm){
	DBG_PRINTF("Connect to %s pass:%s, ip=%s\n",ssid, passwd,ip.toString().c_str());

	if(_targetSSID) free((void*)_targetSSID);
	_targetSSID=strdup(ssid);
	if(_targetPass) free((void*)_targetPass);
	_targetPass=(passwd)? strdup(passwd):NULL;

	_ip=ip;
	_gw=gw;
	_nm=nm;

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
	}else{
		if(_wifiState==WiFiStateChangeConnectPending){
			DBG_PRINTF("Change Connect\n");
			//if(WiFi.status() == WL_CONNECTED){
			WiFi.disconnect();
			WiFi.mode(_mode);
			//DBG_PRINTF("Disconnect\n");
			//}
			if(_ip != INADDR_NONE){
				WiFi.config(_ip,_gw,_nm);
			}
			WiFi.begin(_targetSSID,_targetPass);
			_reconnect =0;
			_wifiState = WiFiStateConnecting;

			wifi_info("**try:");
			_time=millis();

		}else if(_wifiState==WiFiStateDisconnectPending){
			WiFi.disconnect();
			WiFi.mode(WIFI_OFF);
			DBG_PRINTF("Enter AP Mode\n");
    		_apMode=true;
			WiFi.mode(WIFI_AP);	
			_wifiState =WiFiStateDisconnected;
			return true;
			
		}else if(WiFi.status() != WL_CONNECTED){
 			if(_wifiState==WiFiStateConnected)
 			{
				wifi_info("**disc:");

				_time=millis();
				DBG_PRINTF("Lost Network. auto reconnect %d\n",_autoReconnect);
				_wifiState = WiFiStateConnectionRecovering;
				return true;
			}else if (_wifiState==WiFiStateConnectionRecovering){
				// if sta mode, turn on softAP
				if(_time > TimeForRescueAPMode){
					_wifiState =WiFiStateDisconnected;
					if(_mode == WIFI_STA){
						// create a wifi
						WiFi.mode(WIFI_AP_STA);
						WiFi.softAP(_apName, _apPassword);						
					}
				}
			}
 		}
 		else // connected
 		{
 			byte oldState=_wifiState;
 			_wifiState=WiFiStateConnected;
 			_reconnect=0;
 			if(oldState != _wifiState){
				onConnected();
				return true;
			}
  		}
	}
	
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