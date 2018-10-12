#ifndef DataLogger_H
#define DataLogger_H
#include "BPLSettings.h"
#define RETRY_TIME 5
#define MAX_RETRY_NUMBER 3

#define ServiceGenericHttp 0
#define ServiceNonNullJson 1

class DataLogger
{
public:
    DataLogger(void):_lastUpdate(0){ _loggingInfo= theSettings.remoteLogInfo(); }

    // web interface
	void loop(time_t now);

protected:
	void sendData(void);

	size_t nonNullJson(char *buffer,size_t size);

 	size_t dataSprintf(char *buffer,const char *format);
 	size_t printFloat(char* buffer,float value,int precision,bool valid);

	RemoteLoggingInformation *_loggingInfo;
	
	time_t _lastUpdate;
};

#endif
