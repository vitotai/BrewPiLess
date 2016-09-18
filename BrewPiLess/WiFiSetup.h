#ifndef WiFiSetup_H
#define WiFiSetup_H

#include <DNSServer.h>

#define WiFiStateConnected 0
#define WiFiStateWaitToConnect 1
#define WiFiStateConnecting 2

#define TIME_WAIT_TO_CONNECT 10000
#define TIME_RECONNECT_TIMEOUT 10000
#define    DNS_PORT  53

class WiFiSetupClass
{
public:
	WiFiSetupClass(){_wifiState=WiFiStateConnected;_apMode=false;};

	void begin(void){begin("BrewPiLess");}
	void begin(char const *ssid,const char *passwd=NULL);
	void beginAP(char const *ssid,const char *passwd=NULL);

	void stayConnected(void);
	bool isApMode(void) {return _apMode;}
private:
	unsigned long _time;
	byte _wifiState;
	bool _apMode;
	std::unique_ptr<DNSServer>        dnsServer;
	void setupAp(void);
};

extern WiFiSetupClass WiFiSetup;
#endif







