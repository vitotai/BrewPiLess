#ifndef WiFiSetup_H
#define WiFiSetup_H

#include <DNSServer.h>

#define WiFiStateUnknown 255
#define WiFiStateConnected 0
#define WiFiStateModeChangePending 1
#define WiFiStateConnecting 2
#define WiFiStateDisconnected 3
#define WiFiStateDisconnectPending 4
#define WiFiStateChangeConnectPending 5
#define WiFiStateConnectionRecovering 6
#define WiFiStateConnectionRecoveringPending 7


#define WiFiScanStateNone 0
#define WiFiScanStatePending 1
#define WiFiScanStateScanning 2


#define TIME_WAIT_TO_CONNECT 20000
#define TIME_RECONNECT_TIMEOUT 20000
#define    DNS_PORT  53

#if defined(ESP32) 
//WiFiMode
typedef wifi_mode_t WiFiMode;
#endif

typedef std::function<void(bool apmode,IPAddress ip, IPAddress gw, IPAddress mask)> NetcfgHandler;

class WiFiSetupClass
{
public:
	WiFiSetupClass():_wifiState(WiFiStateUnknown),_wifiScanState(WiFiScanStateNone),_switchToAp(true),_autoReconnect(true),
		 _maxReconnect(5),_eventHandler(NULL),_targetSSID(NULL),_targetPass(NULL),_ip(INADDR_NONE),_gw(INADDR_NONE),_nm(INADDR_NONE){}

	void begin(WiFiMode mode, char const *ssid,const char *passwd=NULL,char const* targetSSID=NULL,const char *targetPass=NULL);
	void setMode(WiFiMode mode);
	void staConfig(IPAddress ip=(uint32_t)0x00000000,IPAddress gw=(uint32_t)0x00000000, IPAddress nm=(uint32_t)0x00000000, IPAddress dns=(uint32_t)0x00000000);

	void onEvent(std::function<void(const char*)> handler){ _eventHandler = handler;}

	bool stayConnected(void);
	bool isApMode(void);

	void setMaxReconnect(unsigned int reconnect){_maxReconnect=reconnect;}
	void setSwitchToApWhenDisconnected(bool toAp){  _switchToAp= toAp; }
	void setAutoReconnect(bool reconnect){ _autoReconnect=reconnect; }

	String scanWifi(void);
	bool requestScanWifi(void);
	bool connect(char const *ssid,const char *passwd=NULL,IPAddress ip=(uint32_t)0x00000000,IPAddress gw=(uint32_t)0x00000000, IPAddress nm=(uint32_t)0x00000000,IPAddress dns=(uint32_t)0x00000000);
	bool disconnect(void);

	bool isConnected(void);
	String status(void);
private:
	WiFiMode _mode;
	byte _wifiState;
	byte _wifiScanState;
	byte _reconnectAttempt;
	bool _switchToAp;
	bool _autoReconnect;

	unsigned int _maxReconnect;
	unsigned int _reconnect;

	unsigned long _time;
	std::function<void(const char*)> _eventHandler;
	
	std::unique_ptr<DNSServer>        dnsServer;

	const char *_apName;
	const char *_apPassword;

	const char *_targetSSID;
	const char *_targetPass;
	IPAddress _ip;
	IPAddress _gw;
	IPAddress _nm;
	IPAddress _dns;

	void setupApService(void);
	void enterBackupApMode();
	void onStatus();
	void createNetwork();
};

extern WiFiSetupClass WiFiSetup;
#endif
