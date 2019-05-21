#ifndef DataLogger_H
#define DataLogger_H
#include "BPLSettings.h"
#define RETRY_TIME 5
#define MAX_RETRY_NUMBER 3

#define ServiceGenericHttp 0
#define ServiceNonNullJson 1
#define ServiceHTTPNullString 2

class DataLogger
{
public:
    DataLogger(void):_lastUpdate(0){ _loggingInfo= theSettings.remoteLogInfo(); }

    // web interface
	void loop(time_t now);
	void reportNow(void);

protected:
	void sendData(void);

	RemoteLoggingInformation *_loggingInfo;
	
	time_t _lastUpdate;
};
extern DataLogger dataLogger;

#endif
