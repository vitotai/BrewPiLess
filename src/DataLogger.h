#ifndef DataLogger_H
#define DataLogger_H
#include "BPLSettings.h"
#define RETRY_TIME 5
#define MAX_RETRY_NUMBER 3


class DataLogger
{
public:
    DataLogger(void):_lastUpdate(0){ _loggingInfo= theSettings.remoteLogInfo(); }

    // web interface
	void loop(time_t now);

protected:
	void sendData(void);
 	int dataSprintf(char *buffer,const char *format);
 	int printFloat(char* buffer,float value,int precision,bool valid);

	RemoteLoggingInformation *_loggingInfo;
	
	time_t _lastUpdate;
};

#endif
