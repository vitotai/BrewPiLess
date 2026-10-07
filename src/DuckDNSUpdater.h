#ifndef DuckDNSUpdater_H
#define DuckDNSUpdater_H

// DuckDNS account is hardcoded per project owner's request (no UI/settings
// entry for this). Domain does not need the ".duckdns.org" suffix per the
// DuckDNS API, but passing the full name works too.
#define DUCKDNS_DOMAIN "******.duckdns.org" //write the domain name here
#define DUCKDNS_TOKEN  "*********************************" //write the token between "" those

class DuckDNSUpdaterClass
{
public:
	DuckDNSUpdaterClass(void):_lastUpdateMillis(0){}

	// Call from the main loop(). Only performs the actual (synchronous,
	// but brief) HTTP GET once every UPDATE_INTERVAL_MS; every other
	// call is just a millis() comparison and returns immediately.
	void update(void);

private:
	// 30 minutes. millis()-based, overflow-safe via unsigned subtraction.
	static const unsigned long UPDATE_INTERVAL_MS = 30UL * 60UL * 1000UL;

	void sendUpdateRequest(void);

	unsigned long _lastUpdateMillis;
};

extern DuckDNSUpdaterClass duckDNSUpdater;

#endif
