#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include "espconfig.h"
#include "WiFiSetup.h"



void WiFiSetup::begin(char const *ssid,const char *passwd)
{

	WiFiManager wifiManager;
	#if SerialDebug != true
	wifiManager.setDebugOutput(false);
	#endif
    //reset saved settings
    //wifiManager.resetSettings();
        
    //set custom ip for portal
    //and goes into a blocking loop awaiting configuration
    wifiManager.autoConnect(ssid,passwd);
    //or use this for auto generated name ESP + ChipID    
}

void WiFiSetup::beginAP(char const *ssid,const char *passwd)
{

	WiFiManager wifiManager;

	#if SerialDebug != true
	wifiManager.setDebugOutput(false);
	#endif

    //reset saved settings
    //wifiManager.resetSettings();
        
    //set custom ip for portal
    //and goes into a blocking loop awaiting configuration
    wifiManager.startConfigPortal(ssid,passwd);
    //or use this for auto generated name ESP + ChipID    
}






