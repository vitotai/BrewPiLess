#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <FS.h>
#include "espconfig.h"
#include "WiFiSetup.h"

#define IPConfigFileName "ip.cfg"

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

bool startSetupPortal(WiFiManager& wifiManager,const char* ssid,const char*pass)
{
    WiFiManagerParameter ipAddress("staticip", "Static IP", "", 16);
    WiFiManagerParameter gateway("gateway", "Gateway",  "", 16);
    WiFiManagerParameter netmask("netmask", "Net Mask", "255.255.255.0", 16);

    wifiManager.addParameter(&ipAddress);
    wifiManager.addParameter(&gateway);
    wifiManager.addParameter(&netmask);

  	bool connected= wifiManager.startConfigPortal(ssid,pass);
    if(connected){
        //SAVE configuration.
        File fh= SPIFFS.open(IPConfigFileName, "w");
        if(!fh){
            DBG_PRINTF("Error opening file!!\n");
    	    return connected;
        }
        fh.println(ipAddress.getValue());
        fh.println(gateway.getValue());
        fh.println(netmask.getValue());
        fh.close();
    
        DBG_PRINTF("Save IP:%s, GW:%s, SM:%s\n",ipAddress.getValue(),gateway.getValue(),netmask.getValue());
    }
  	return connected;
}
void scanIP(File& fh,IPAddress& ip)
{
    char buffer[20];
    size_t len=fh.readBytesUntil('\n', buffer, 20);
    buffer[len]='\0';
    // DBG_PRINTF("Scan IP length=%d :\"%s\"\n",len,buffer);
    // this doesn't work. the last byte always 0: ip.fromString(buffer);
    
    int Parts[4] = {0,0,0,0};
    int Part = 0;
    for ( int i=0; i<len-1; i++ )
    {
	    char c = buffer[i];
	    if ( c == '.' )
	    {
		    Part++;
		    continue;
	    }
	    Parts[Part] *= 10;
	    Parts[Part] += c - '0';
    }
    
    IPAddress sip( Parts[0], Parts[1], Parts[2], Parts[3] );    
    ip = sip;
    //Serial.print("result:");
    //Serial.println(ip);
}

void WiFiSetupClass::preInit(void)
{
        File fh= SPIFFS.open(IPConfigFileName, "r");
        if(fh){
            DBG_PRINTF("Static IP Setting Exists:\n");

            IPAddress ip;
            scanIP(fh,ip);
            
            if(ip == (uint32_t)0){
                DBG_PRINTF("Invalid IP: 0.0.0\n");
            }else{
                IPAddress gateway;
                IPAddress mask;
                scanIP(fh,gateway);
                scanIP(fh,mask);
                                
                WiFi.config(ip, gateway, mask);
                
            }
            fh.close();

        }
}

void WiFiSetupClass::startWiFiManager(bool portal)
{
	DBG_PRINTF("AP SSID:%s  pass:%s\n",_apName,_apPassword);
	
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

  	WiFi.hostname(_apName);

    if(portal){
        startSetupPortal(wifiManager,_apName,_apPassword);
    }else{
 		
 		if( _apStaMode){
	    	WiFi.mode(WIFI_AP_STA);
	    }else{
	    	WiFi.mode(WIFI_STA);
		}

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
 		// timeout or not
		if(WiFi.status()==WL_CONNECTED){
			connected=true;
			if( _apStaMode){
				// start a AP.
				DBG_PRINTF("Start SoftAP\n");
				WiFi.softAP(_apName,_apPassword);
			}
		}else{
	    	connected=startSetupPortal(wifiManager,_apName,_apPassword);
	    }
    }
    if(!connected){	// not connected. setup AP mode
    	enterApMode();
    }else{
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





























































