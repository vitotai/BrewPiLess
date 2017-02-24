#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include "espconfig.h"
#include "WiFiSetup.h"

WiFiSetupClass WiFiSetup;

void WiFiSetupClass::setupNetwork(void)
{
	WiFi.mode(WIFI_AP_STA);
	if (_apPassword != NULL) {
   		WiFi.softAP(_apName, _apPassword);
  	} else {
    	WiFi.softAP(_apName);
 	}						
}

void WiFiSetupClass::enterApMode(void)
{

	WiFi.disconnect();
	DBG_PRINTF("AP Mode\n");
    _apMode=true;
	
	dnsServer.reset(new DNSServer());
	dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
	dnsServer->start(DNS_PORT, "*", WiFi.softAPIP());
	delay(500);
}
static bool _apEntered=false;

void WiFiSetupClass::startWiFiManager(bool portal)
{
	WiFiManager wifiManager;
	#if SerialDebug != true
	wifiManager.setDebugOutput(false);
	#endif
    //reset saved settings
    //wifiManager.resetSettings();
    if(_apTimeout !=0)
	    wifiManager.setTimeout(_apTimeout);
    //set custom ip for portal
    //and goes into a blocking loop awaiting configuration
    wifiManager.setAPCallback([](WiFiManager*){ _apEntered=true;});

    bool connected;
    if(portal){
    	connected=wifiManager.startConfigPortal(_apName,_apPassword);
    }else{
    	connected=wifiManager.autoConnect(_apName,_apPassword);
    }
    if(!connected)	// not connected. setup AP mode
    	enterApMode();
    else{
    	// onced it enter AP mode, tcp_bind() lf lwip will return failure.
    	// thereore, restart the system.
    	if(_apEntered) ESP.restart();
    }
}

void WiFiSetupClass::begin(char const *ssid,const char *passwd)
{
	_apName=ssid;
	_apPassword=passwd;
	startWiFiManager(false);
}

void WiFiSetupClass::beginAP(char const *ssid,const char *passwd)
{
	_apName=ssid;
	_apPassword=passwd;
    startWiFiManager(true);
}


void WiFiSetupClass::stayConnected(void)
{
	if(_apMode){
		dnsServer->processNextRequest();
	}else{
 		if(WiFi.status() != WL_CONNECTED)
 		{
 			if(_wifiState==WiFiStateConnected)
 			{
				_time=millis();
				_wifiState = WiFiStateWaitToConnect;
				DBG_PRINTF("Lost Network. Wait to connect.\n");
			}
			else if(_wifiState==WiFiStateWaitToConnect)
			{
				if((millis() - _time) > TIME_WAIT_TO_CONNECT)
				{
					WiFi.begin();
					_time=millis();
					_wifiState = WiFiStateConnecting;
					DBG_PRINTF("Reconnect...\n");
				}
			}
			else if(_wifiState==WiFiStateConnecting)
			{
				if((millis() - _time) > TIME_RECONNECT_TIMEOUT){
					_time=millis();
					_wifiState = WiFiStateWaitToConnect;
					_reconnect++;
					DBG_PRINTF("Reconnect fail\n");

					if(_maxReconnect !=0 && _reconnect>=_maxReconnect){
						DBG_PRINTF("Fail to reconnect. Setup AP mode.\n");

						setupNetwork();
 						enterApMode();
					}
				}
			}
 		}
 		else
 		{
 			_wifiState=WiFiStateConnected;
 			_reconnect=0;
  		}
	}
}































































































