#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include "espconfig.h"
#include "WiFiSetup.h"

WiFiSetupClass WiFiSetup;

void WiFiSetupClass::setupAp(void)
{
	DBG_PRINTF("AP Mode\n");
    _apMode=true;
	
	dnsServer.reset(new DNSServer());
	dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
	dnsServer->start(DNS_PORT, "*", WiFi.softAPIP());
}

void WiFiSetupClass::begin(char const *ssid,const char *passwd)
{

	WiFiManager wifiManager;
	#if SerialDebug != true
	wifiManager.setDebugOutput(false);
	#endif
    //reset saved settings
    //wifiManager.resetSettings();
        
    //set custom ip for portal
    //and goes into a blocking loop awaiting configuration
    if(!wifiManager.autoConnect(ssid,passwd)){
    	// not connected. setup AP mode
    	setupAp();
    }
}

void WiFiSetupClass::beginAP(char const *ssid,const char *passwd)
{

	WiFiManager wifiManager;

	#if SerialDebug != true
	wifiManager.setDebugOutput(false);
	#endif

    //reset saved settings
    //wifiManager.resetSettings();
        
    //set custom ip for portal
    //and goes into a blocking loop awaiting configuration
    if(!wifiManager.startConfigPortal(ssid,passwd)){
    	//or use this for auto generated name ESP + ChipID
    	setupAp();
    }    
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
			}
			else if(_wifiState==WiFiStateWaitToConnect)
			{
				if((millis() - _time) > TIME_WAIT_TO_CONNECT)
				{
					WiFi.begin();
					_time=millis();
					_wifiState = WiFiStateConnecting;
				}
			}
			else if(_wifiState==WiFiStateConnecting)
			{
				if((millis() - _time) > TIME_RECONNECT_TIMEOUT){
					_time=millis();
					_wifiState = WiFiStateWaitToConnect;
				}
			}
 		}
 		else
 		{
 			_wifiState=WiFiStateConnected;
  		}
	}
}

