#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <HTTPClient.h>
#endif

#include "Config.h"
#include "BPLSettings.h"
#include "DuckDNSUpdater.h"

// Concatenated once by the preprocessor at compile time, so no runtime
// String concatenation is needed to build the request URL. Plain HTTP is
// used on purpose (DuckDNS explicitly supports it) to avoid the extra
// RAM/CPU cost of a TLS handshake on every update.
#define DUCKDNS_UPDATE_URL \
	"http://www.duckdns.org/update?domains=" DUCKDNS_DOMAIN "&token=" DUCKDNS_TOKEN "&ip="

DuckDNSUpdaterClass duckDNSUpdater;

void DuckDNSUpdaterClass::update(void)
{
	// Unsigned subtraction: still correct once millis() wraps around
	// after ~49 days, no separate overflow-handling branch needed.
	SystemConfiguration *settings = theSettings.systemConfiguration();
	if(!settings->duckdnsEnabled || settings->duckdnsInterval == 0){
		return;
	}
	unsigned long interval = (unsigned long)settings->duckdnsInterval * 60UL * 1000UL;
	if((unsigned long)(millis() - _lastUpdateMillis) < interval){
		return;
	}
	_lastUpdateMillis = millis();

	sendUpdateRequest();
}

void DuckDNSUpdaterClass::sendUpdateRequest(void)
{
	SystemConfiguration *settings = theSettings.systemConfiguration();
	if(settings->duckdnsDomain[0] == '\0' || settings->duckdnsToken[0] == '\0'){
		return; // not configured, stay silent
	}

	if(WiFi.status() != WL_CONNECTED){
		DBG_PRINTF("DuckDNS: WiFi not connected, skipping this cycle\n");
		return; // just wait for the next 30-minute window, no retry loop
	}

	WiFiClient wifiClient;
	HTTPClient http;
	String url = String("http://www.duckdns.org/update?domains=") + settings->duckdnsDomain
		+ "&token=" + settings->duckdnsToken + "&ip=";

	if(!http.begin(wifiClient, url)){
		DBG_PRINTF("DuckDNS: http.begin() failed\n");
		http.end();
		return;
	}

	http.setConnectTimeout(1000);
	http.setTimeout(1000);
	int code = http.GET();
	if(code > 0){
		DBG_PRINTF("DuckDNS: update result code=%d\n", code);
	}else{
		DBG_PRINTF("DuckDNS: update request failed: %s\n", http.errorToString(code).c_str());
	}

	http.end();
}
