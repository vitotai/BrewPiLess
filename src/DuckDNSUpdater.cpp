#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <HTTPClient.h>
#endif

#include "Config.h"
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
	if((unsigned long)(millis() - _lastUpdateMillis) < UPDATE_INTERVAL_MS){
		return;
	}
	_lastUpdateMillis = millis();

	sendUpdateRequest();
}

void DuckDNSUpdaterClass::sendUpdateRequest(void)
{
	if(DUCKDNS_DOMAIN[0] == '\0' || DUCKDNS_TOKEN[0] == '\0'){
		return; // not configured, stay silent
	}

	if(WiFi.status() != WL_CONNECTED){
		DBG_PRINTF("DuckDNS: WiFi not connected, skipping this cycle\n");
		return; // just wait for the next 30-minute window, no retry loop
	}

	WiFiClient wifiClient;
	HTTPClient http;

	if(!http.begin(wifiClient, DUCKDNS_UPDATE_URL)){
		DBG_PRINTF("DuckDNS: http.begin() failed\n");
		http.end();
		return;
	}

	int code = http.GET();
	if(code > 0){
		DBG_PRINTF("DuckDNS: update result code=%d\n", code);
	}else{
		DBG_PRINTF("DuckDNS: update request failed: %s\n", http.errorToString(code).c_str());
	}

	http.end();
}
