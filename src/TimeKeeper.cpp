#include <time.h>
#include <Arduino.h>
#include <FS.h>
#include "Config.h"
#include "BPLSettings.h"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

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
	_referenceEpoc=now;
  	_referenceSystemTime = millis();
	_lastSaved=_referenceEpoc;
	saveTime(now);
}

void TimeKeeperClass::begin(void)
{

	_referenceEpoc=loadTime();
	_referenceEpoc += 300; // add 5 minutes.
  	_referenceSystemTime = millis();
	_lastSaved=_referenceEpoc;
	DBG_PRINTF("Load saved time:%ld\n",(long)_referenceEpoc);
}

time_t TimeKeeperClass::_queryServer(void){
	time_t secs=0;

	int trial;
	for(trial=0;trial< 20;trial++)
  	{
		#ifdef ESP32
		time(&secs);
		#else
    	secs = sntp_get_current_timestamp();
		#endif
		DBG_PRINTF("Time from NTP :%ld, %d\n",(long)secs,trial);
    	if(secs > 1546265623){ 
			_ntpSynced=true;
			break;
		}
    	delay(750);
  	}
	return secs;
}

void TimeKeeperClass::updateTime(void){
	time_t secs=_queryServer();
	if(secs > 1546265623){
  		_referenceSystemTime = millis();
  		_referenceEpoc = secs;
	}
}

void TimeKeeperClass::begin(char* server1,char* server2,char* server3)
{
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
	if(WiFi.status() == WL_CONNECTED){
		secs=_queryServer();
	}
	if(secs < 1546265623){
		secs=loadTime() + 30;
		DBG_PRINTF("failed to connect NTP, load time:%ld\n",(long)secs);
	}
	_referenceSystemTime = millis();
  	_referenceEpoc = secs;

  	_lastSaved=_referenceEpoc;
}

time_t TimeKeeperClass::getTimeSeconds(void) // get Epoch time
{
	unsigned long diff=millis() -  _referenceSystemTime;

	if(diff > RESYNC_TIME){
		if( WiFi.status() == WL_CONNECTED){
			//updateTime();
			time_t secs=_queryServer();
			if(secs > 1546265623){
  				_referenceSystemTime = millis();
  				_referenceEpoc = secs;
	  			diff=0;
			}else{
				_referenceSystemTime = millis();
		  		_referenceEpoc = _referenceEpoc + diff/1000;
		  		diff=0;
			}
		}else{
			// just add up
  			_referenceSystemTime = millis();
	  		_referenceEpoc = _referenceEpoc + diff/1000;
	  		diff=0;
		}
	}
	time_t now= _referenceEpoc + diff/1000;

	if(	(now - _lastSaved) > TIME_SAVING_PERIOD){
		saveTime(now);
		_lastSaved=now;
	}
	return now;
}

static char _dateTimeStrBuff[64];

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
