#include <time.h>
#include <Arduino.h>
#include <FS.h>
#include "Config.h"
#include "BPLSettings.h"
extern "C" {
#if !defined(ESP32)
#include <sntp.h>
#endif
}
#include "TimeKeeper.h"

#define TIME_SAVE_FILENAME "/time.saved"
#define TIME_SAVING_PERIOD 3600

#define RESYNC_TIME 43200000UL
// time gap in seconds from 01.01.1900 (NTP time) to 01.01.1970 (UNIX time)
#define DIFF1900TO1970 2208988800UL

TimeKeeperClass TimeKeeper;

void TimeKeeperClass::setCurrentTime(time_t now)
{
	_referenceSeconds=now;
  	_referenceSystemTime = millis();
	_lastSaved=_referenceSeconds;
	saveTime(now);
}

void TimeKeeperClass::begin(void)
{
	_online = false;

	_referenceSeconds=loadTime();
	_referenceSeconds += 300; // add 5 minutes.
  	_referenceSystemTime = millis();
	_lastSaved=_referenceSeconds;
	DBG_PRINTF("Load saved time:%ld\n",_referenceSeconds);
}

void TimeKeeperClass::begin(char* server1,char* server2,char* server3)
{
	_online=true;
#ifdef ESP32
	if(! server1) configTime(0,0,"time.nist.gov");
	else configTime(0,0,server1,server2,server3);

#else
  	if(server1) sntp_setservername(0,server1);
  	else sntp_setservername(0,(char*)"time.nist.gov");
  	if(server2) sntp_setservername(1,server2);
  	if(server3) sntp_setservername(2,server3);
  	sntp_set_timezone(0);
	sntp_init();
#endif
  	time_t secs=0;
	int trial;
	for(trial=0;trial< 50;trial++)
  	{
		#ifdef ESP32
		time(&secs);
		#else
    	secs = sntp_get_current_timestamp();
		#endif
    	if(secs) break;
    	delay(200);
  	}
	if(secs ==0){
		secs=loadTime() + 300;
		DBG_PRINTF("failed to connect NTP, load time:%ld\n",secs);
	}
  	_referenceSystemTime = millis();
  	_referenceSeconds = secs;
  	_lastSaved=_referenceSeconds;
}

time_t TimeKeeperClass::getTimeSeconds(void) // get Epoch time
{
	unsigned long diff=millis() -  _referenceSystemTime;

	if(diff > RESYNC_TIME){
		if( _online){
			time_t newtime;
			#ifdef ESP32
			time(&newtime);
			#else
			 newtime=sntp_get_current_timestamp();
			#endif
			if(newtime){
  				_referenceSystemTime = millis();
	  			_referenceSeconds = newtime;
	  			diff=0;
			}
		}else{
			// just add up
  			_referenceSystemTime = millis();
	  		_referenceSeconds = _referenceSeconds + diff/1000;
	  		diff=0;
		}
	}
	time_t now= _referenceSeconds + diff/1000;

	if(	(now - _lastSaved) > TIME_SAVING_PERIOD){
		saveTime(now);
		_lastSaved=now;
	}
	return now;
}

static char _dateTimeStrBuff[24];

const char* TimeKeeperClass::getDateTimeStr(void)
{
	time_t current=getTimeSeconds();
	tm *t= localtime(&current);

  //2016-07-01T05:22:33Z
	sprintf(_dateTimeStrBuff,"%d-%02d-%02dT%02d:%02d:%02dZ",
		t->tm_year+1900,t->tm_mon +1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
	return _dateTimeStrBuff;
}

void TimeKeeperClass::saveTime(time_t t)
{
	(theSettings.timeInformation())->savedTime = t;
	theSettings.save();
}

time_t TimeKeeperClass::loadTime(void)
{
	return (theSettings.timeInformation())->savedTime;
}

void TimeKeeperClass::setTimezoneOffset(int32_t offset){
	(theSettings.timeInformation())->timezoneoffset = offset;
}
int32_t TimeKeeperClass::getTimezoneOffset(void){
	return (theSettings.timeInformation())->timezoneoffset;
}
time_t TimeKeeperClass::getLocalTimeSeconds(void){ 
	return  getTimeSeconds() + (theSettings.timeInformation())->timezoneoffset;
}
