#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <FS.h>
#include "Config.h"
#include "WiFiSetup.h"

#define IPConfigFileName "ip.cfg"

WiFiSetupClass WiFiSetup;

void WiFiSetupClass::enterApMode(void)
{

	WiFi.disconnect();
	WiFi.mode(WIFI_OFF); // fixes D1 mini not entering initial AP mode without hard reset

	DBG_PRINTF("AP Mode\n");
    _apMode=true;
	
	WiFi.mode(WIFI_AP);	// fixes Error setting mDNS responder
	if (_apPassword != NULL) {
   		WiFi.softAP(_apName, _apPassword);
	} else {
    	WiFi.softAP(_apName);
	}

	dnsServer.reset(new DNSServer());
	dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
	dnsServer->start(DNS_PORT, "*", WiFi.softAPIP());
	delay(500);
}

IPAddress scanIP(const char *str)
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

bool WiFiSetupClass::startSetupPortal(void)
{
	WiFiManager wifiManager;

	#if SerialDebug != true
	wifiManager.setDebugOutput(false);
	#endif
    //reset saved settings
    //wifiManager.resetSettings();
    if(_apTimeout !=0) wifiManager.setTimeout(_apTimeout);
    //set custom ip for portal
    //and goes into a blocking loop awaiting configuration
    WiFiManagerParameter ipAddress("staticip", "Static IP", _staticIP.toString().c_str(), 16);
    WiFiManagerParameter gateway("gateway", "Gateway", _staticGateway.toString().c_str(), 16);
    WiFiManagerParameter netmask("netmask", "Net Mask", _staticMask.toString().c_str(), 16);

    wifiManager.addParameter(&ipAddress);
    wifiManager.addParameter(&gateway);
    wifiManager.addParameter(&netmask);

  	bool connected= wifiManager.startConfigPortal(_apName,_apPassword);
    if(connected){
        //SAVE configuration.
        _staticIP=scanIP(ipAddress.getValue());
        _staticGateway=scanIP(gateway.getValue());
        _staticMask=scanIP(netmask.getValue());
        DBG_PRINTF("Save IP:%s, GW:%s, SM:%s\n",ipAddress.getValue(),gateway.getValue(),netmask.getValue());
    }
  	return wifiManager.softAPModeSelected();
}


void WiFiSetupClass::setNetwork(WiFiMode mode,IPAddress ip,IPAddress gw,IPAddress mask)
{
	_desiredWiFiMode = mode;
	_staticIP = ip;
	_staticGateway = gw;
	_staticMask = mask;
}

void WiFiSetupClass::startWiFiManager(bool portal)
{
	DBG_PRINTF("AP SSID:%s  pass:%s\n",_apName,_apPassword);
	bool _enterPortal=false;
	WiFi.hostname(_apName);
	bool _userApMode=false;
    if(portal){
		// force to start Portal
        _userApMode=startSetupPortal();
		_enterPortal=true;
    }else if(_desiredWiFiMode == WIFI_AP_STA || _desiredWiFiMode == WIFI_STA) {
	    // try to connect
		if(_staticIP != IPADDR_ANY){
			DBG_PRINTF("Fixed IP:%s, gw:%s, mask:%s\n",_staticIP.toString().c_str(),_staticGateway.toString().c_str(),_staticMask.toString().c_str());
			WiFi.config(_staticIP,_staticGateway,_staticMask);
			delay(100);
		}

		WiFi.mode(_desiredWiFiMode);
 		if( WiFi.status() == WL_CONNECTED){
 			DBG_PRINTF("Already connected!!! who did it?\n");
 		}else{
			WiFi.begin();
			bool timeout = false;
			int i=0;
			while(WiFi.status() != WL_CONNECTED && !timeout) {
        		delay(200);
    			timeout = i++ > 100;
   				DBG_PRINTF(".");
    		}
 		}
 		// Somehow, it connected or not.
		if(WiFi.status()==WL_CONNECTED){
			if( _desiredWiFiMode == WIFI_AP_STA){
				// start a AP.
				DBG_PRINTF("Start SoftAP\n");
				WiFi.softAP(_apName,_apPassword);
			}
		}else{
			// previous network not available. start portal
	    	_userApMode=startSetupPortal();
			_enterPortal=true;
	    }
    }else{ // AP MODE
		// run in AP mode directly
		enterApMode();
	}

 	if(_desiredWiFiMode == WIFI_AP_STA || _desiredWiFiMode == WIFI_STA) {
	    if(WiFi.status()!=WL_CONNECTED){	// not connected, timeout or user select AP mdoe
			if(_userApMode){
				// user select APmode
				if(_configHandler) _configHandler(true,_staticIP,_staticGateway,_staticMask);
				delay(200);
				ESP.restart();
			}else{
				DBG_PRINTF("Portal Timeout user action.\n");
			}
	    	enterApMode();
   		 }else{
    		// onced it enter AP mode, tcp_bind() lf lwip will return failure.
    		// thereore, restart the system.
    		if(_enterPortal){
				_configHandler(false,_staticIP,_staticGateway,_staticMask);
				delay(200);
				ESP.restart();
			} 
	    }
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
