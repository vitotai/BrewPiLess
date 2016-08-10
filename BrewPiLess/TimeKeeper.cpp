#include <time.h>
#include <Arduino.h>

extern "C" {
#include <sntp.h>
}
#include "TimeKeeper.h"

#define RESYNC_TIME 43200000UL
// time gap in seconds from 01.01.1900 (NTP time) to 01.01.1970 (UNIX time)
#define DIFF1900TO1970 2208988800UL

TimeKeeperClass TimeKeeper;


void TimeKeeperClass::begin(char* server1,char* server2,char* server3)
{
	sntp_init();
  	if(server1) sntp_setservername(0,server1);
  	else sntp_setservername(0,"time.nist.gov");
  	if(server2) sntp_setservername(1,server2);
  	if(server3) sntp_setservername(2,server3);
  	sntp_set_timezone(0);
  	unsigned long secs=0;

  	while (/* true */ 1)
  	{
    	secs = sntp_get_current_timestamp();
    	if(secs) break;
    	delay(200);
  	}

  	_referenceSystemTime = millis();
  	_referenceSeconds = secs;
}

time_t TimeKeeperClass::getTimeSeconds(void) // get Epoch time
{
	unsigned long diff=millis() -  _referenceSystemTime;
	
	if(diff > RESYNC_TIME)
	{
		unsigned long newtime=sntp_get_current_timestamp();
		if(newtime){
  			_referenceSystemTime = millis();
	  		_referenceSeconds = newtime;
	  		diff=0;
		}
	}
	return  _referenceSeconds + diff/1000;
}

static char _dateTimeStrBuff[24];

const char* TimeKeeperClass::getDateTimeStr(void)
{
	time_t current=getTimeSeconds();
	tm *t= localtime(&current);
  
  //2016-07-01T05:22:33Z
	sprintf(_dateTimeStrBuff,"%d-%02d-%02dT%02d:%02d:%02dZ",
		t->tm_year+1900,t->tm_mon,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
	return _dateTimeStrBuff;
}

