#ifndef WiFiSetup_H
#define WiFiSetup_H

#include <DNSServer.h>

#define WiFiStateConnected 0
#define WiFiStateWaitToConnect 1
#define WiFiStateConnecting 2

#define TIME_WAIT_TO_CONNECT 20000
#define TIME_RECONNECT_TIMEOUT 20000
#define    DNS_PORT  53

typedef std::function<void(bool apmode,IPAddress ip, IPAddress gw, IPAddress mask)> NetcfgHandler;

class WiFiSetupClass
{
public:
	WiFiSetupClass():_configHandler(NULL){_wifiState=WiFiStateConnected;_apMode=false; _maxReconnect=0;}

//    void preInit(void);
	void setNetwork(WiFiMode mode,IPAddress ip,IPAddress gw,IPAddress mask);

	void begin(void){begin("brewpiless");}
	void begin(char const *ssid,const char *passwd=NULL);
	void beginAP(char const *ssid,const char *passwd=NULL);

	bool settingChanged(NetcfgHandler func){_configHandler=func; }

	void stayConnected(void);
	bool isApMode(void) {return _apMode;}

	void setTimeout(unsigned long timeout){ _apTimeout=timeout;}
	void setMaxReconnect(unsigned int reconnect){_maxReconnect=reconnect;}
private:
	unsigned int _maxReconnect;
	unsigned int _reconnect;

	unsigned long _time;
	byte _wifiState;
	bool _apMode; // current in AP mode. 
	WiFiMode _desiredWiFiMode;
	IPAddress _staticIP;
	IPAddress _staticGateway;
	IPAddress _staticMask;

	std::unique_ptr<DNSServer>        dnsServer;

	const char *_apName;
	const char *_apPassword;
	unsigned long _apTimeout;

	void enterApMode(void);
	void startWiFiManager(bool portal);
	bool startSetupPortal(void);

	NetcfgHandler _configHandler;
};

extern WiFiSetupClass WiFiSetup;
#endif
