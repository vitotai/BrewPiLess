#ifndef WiFiSetup_H
#define WiFiSetup_H

class WiFiSetup
{
public:
	static void begin(void){WiFiSetup::begin("BrewManiac");}
	static void begin(char const *ssid,const char *passwd=NULL);
	static void beginAP(char const *ssid,const char *passwd=NULL);
};

#endif





