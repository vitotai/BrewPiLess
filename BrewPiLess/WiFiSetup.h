#ifndef WiFiSetup_H
#define WiFiSetup_H

#include <DNSServer.h>

#define WiFiStateConnected 0
#define WiFiStateWaitToConnect 1
#define WiFiStateConnecting 2

#define TIME_WAIT_TO_CONNECT 20000
#define TIME_RECONNECT_TIMEOUT 20000
#define    DNS_PORT  53

class WiFiSetupClass
{
public:
	WiFiSetupClass(){_wifiState=WiFiStateConnected;_apMode=false; _maxReconnect=0; _apStaMode=false;}

    void preInit(void);
	void begin(void){begin("BrewPiLess");}
	void begin(char const *ssid,const char *passwd=NULL);
	void beginAP(char const *ssid,const char *passwd=NULL);

	void stayConnected(void);
	bool isApMode(void) {return _apMode;}

	void setTimeout(unsigned long timeout){ _apTimeout=timeout;}	
	void setMaxReconnect(unsigned int reconnect){_maxReconnect=reconnect;}
	void setApStation(bool apsta){ _apStaMode = apsta; }
private:
	unsigned int _maxReconnect;
	unsigned int _reconnect;
	
	unsigned long _time;
	byte _wifiState;
	bool _apMode;
	bool _apStaMode;
	
	std::unique_ptr<DNSServer>        dnsServer;

	const char *_apName;
	const char *_apPassword;
	unsigned long _apTimeout;

	void setupNetwork(void);
	void enterApMode(void);
	void startWiFiManager(bool portal);
};

extern WiFiSetupClass WiFiSetup;
#endif





























































