#include <FS.h>
#include <ArduinoJson.h>

#include <time.h>

#include "BrewKeeper.h"
#include "mystrlib.h"

#define DBG_PRINTF(...) // DebugPort.printf(__VA_ARGS__)
#define DBG_PRINT(...) //DebugPort.print(__VA_ARGS__)


#define OUT_OF_RANGE(a,b,c) ((((a) - (b)) > (c)) || (((a) - (b)) < -(c)))
#define IS_INVALID_CONTROL_TEMP(t) ((t)< -99.0)
#define INVALID_CONTROL_TEMP -100.0

void BrewKeeper::keep(time_t now,char unit,char mode,float beerSet)
{
	// run in loop()
	if (mode != 'p') return;
	
	if((now - _lastSetTemp) < MINIMUM_TEMPERATURE_SETTING_PERIOD) return;
		
	if((_profile.loaded()!=true) && (_filename!=NULL)){
		DBG_PRINTF("load profile.");
		_profile.load(_filename);
	}
	if(! _profile.loaded()) return;
	
	// check unit
	_profile.setUnit(unit);
	
	float temp=_profile.tempByTime(now);
	
	if(IS_INVALID_CONTROL_TEMP(temp)) return;
	if(OUT_OF_RANGE(temp,beerSet,MINIMUM_TEMPERATURE_STEP)){
		// set temp
		//_write("j{beerSet:" + temp + "}");
		char buff[36];
		strcpy(buff,"j{beerSet:");
		int len=strlen(buff);
		len+=sprintFloat(buff+len,temp,2);
		strcpy(buff+len,"}");

		//BPSerial.print(buff);
		_write(buff);
		_lastSetTemp= now;
	}
}

//**********************************************************************************
//class BrewProfile
//**********************************************************************************
#define F2C(d) (((d)-32)/1.8)
#define C2F(d) (((d)*1.8)+32)

void BrewProfile::_tempConvert(void)
{
	for(int i=0;i< _numberOfSteps;i++){
		_setTemps[i] = (_unit == 'C')? F2C(_setTemps[i]):C2F(_setTemps[i]);
	}	
}

void BrewProfile::setUnit(char unit)
{
	if(_unit == unit) return;
	_unit = unit;

	if(!_profileLoaded) return;
	_tempConvert();
}
time_t tm_to_timet(struct tm *tm_time);

bool BrewProfile::load(String filename)
{
	DBG_PRINTF("BrewProfile::load\n");


	if(!SPIFFS.exists(filename)){
		DBG_PRINTF("file:%s not exist\n",filename.c_str());
		return false;
	}
	File pf=SPIFFS.open(filename,"r");
	if(!pf){
		DBG_PRINTF("file open failed\n");
		return false;
	}
	char profileBuffer[MAX_PROFILE_LEN];
	size_t len=pf.readBytes(profileBuffer,MAX_PROFILE_LEN);
	profileBuffer[len]='\0';
	
	DynamicJsonBuffer jsonBuffer(PROFILE_JSON_BUFFER_SIZE);
	JsonObject& root = jsonBuffer.parseObject(profileBuffer);
	
	if(!root.success()){
		DBG_PRINTF("JSON parsing failed\n");
		return false;
	}
	if(!root.containsKey("s")
		|| !root.containsKey("u")
		|| !root.containsKey("t")){
		DBG_PRINTF("JSON file not include necessary fields\n");
		return false;
	}
	if (!root["t"].is<JsonArray&>()){
		DBG_PRINTF("JSON t is not array\n");
		return false;
	}
	// get starting time
	//ISO time:
	//2016-07-01T05:22:33.351Z
	//01234567890123456789
	tm tmStart;
	char buf[8];
	const char* sdutc=root["s"];

	#define GetValue(d,s,l) strncpy(buf,sdutc+s,l);buf[l]='\0';d=atoi(buf)
	GetValue(tmStart.tm_year,0,4); 
	tmStart.tm_year -= 1970; //1900;
	GetValue(tmStart.tm_mon,5,2);
//	tmStart.tm_mon -= 1;
	GetValue(tmStart.tm_mday,8,2);
	GetValue(tmStart.tm_hour,11,2);
	GetValue(tmStart.tm_min,14,2);
	GetValue(tmStart.tm_sec,17,2);
	
	DBG_PRINTF("%d/%d/%d %d:%d:%d\n",tmStart.tm_year,tmStart.tm_mon,tmStart.tm_mday, 
		tmStart.tm_hour,tmStart.tm_min,tmStart.tm_sec);
	
	//_startDay = mktime(&tmStart);

	_startDay= tm_to_timet(&tmStart);
	
	JsonArray& schedule = root["t"];
	_numberOfSteps=schedule.size();
	
	if(!_setTemps) free(_setTemps);
	_setTemps =(float*) malloc(sizeof(float) * _numberOfSteps);
	if(!_times) free(_times);
	_times = (time_t*)malloc(sizeof(time_t) * _numberOfSteps);
	int i=0;
	for(JsonArray::iterator it=schedule.begin(); it!=schedule.end(); ++it,i++){
		JsonObject&	 entry= *it;
		*(_setTemps + i) = entry["t"];
		float day= entry["d"];
		*(_times+i) =(time_t)(day * 86400.0) + _startDay;
		DBG_PRINTF("%d ,time:%ld temp:",i,*(_times+i) );
		DBG_PRINT(*(_setTemps + i));
		DBG_PRINTF("\n");
	}
	_profileLoaded=true;

	// unit
	const char *punit = root["u"];
	char unit = *punit;
	if(_unit != 'U' && _unit != unit){ // been set by controller
		_unit = unit;
		_tempConvert();
	}else
		_unit = unit;

	DBG_PRINTF("finished, st:%ld, unit:%c, _numberOfSteps:%d\n",_startDay,unit,_numberOfSteps);
	return true;
}

float BrewProfile::tempByTime(unsigned long time)
{
//	DBG_PRINTF("now= st:%ld\n",time);
	
//	DBG_PRINTF("tempByTime:now:%ld, _startDay:%ld, last time:%ld\n",time,_startDay,*(_times+_numberOfSteps-1));
	
	if(time < _startDay) return INVALID_CONTROL_TEMP;
	if(time >= *(_times+_numberOfSteps-1)) return INVALID_CONTROL_TEMP;

	int i=1;
	while( time > *(_times+i)) i++;
	float prevTemp= *(_setTemps + i -1);
	float nextTemp= *(_setTemps + i);
	time_t prevDate = *(_times+i-1);
	time_t nextDate =*(_times+i);
	
	float interpolatedTemp = ((float)(time - prevDate) /(float)(nextDate - prevDate) * (nextTemp - prevTemp) + prevTemp);
    interpolatedTemp = roundf(interpolatedTemp*10.0)/10.0;
    return interpolatedTemp;
}


/*
 * Reconstitute "struct tm" elements into a time_t count value.
 * Note that the year argument is offset from 1970.
 */
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24UL)
#define DAYS_PER_WEEK (7UL)
#define SECS_PER_WEEK (SECS_PER_DAY * DAYS_PER_WEEK)
#define SECS_PER_YEAR (SECS_PER_WEEK * 52UL)
#define SECS_YR_2000  (946684800UL)	// The time_t value at the very start of Y2K.
static	const uint8_t monthDays[]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
#define LEAP_YEAR(Y)		 ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )
time_t tm_to_timet(struct tm *tm_time){	 
	
	int i;
	time_t seconds;

	seconds= tm_time->tm_year*(SECS_PER_DAY * 365);
	for (i = 0; i < tm_time->tm_year; i++) {
		if (LEAP_YEAR(i)) {
			seconds += SECS_PER_DAY;	// Add extra days for leap years.
		}
	}
	// Add the number of elapsed days for the given year. Months start from 1.
	for (i = 1; i < tm_time->tm_mon; i++) {
		if ( (i == 2) && LEAP_YEAR(tm_time->tm_year)) { 
			seconds += SECS_PER_DAY * 29;
		} else {
			seconds += SECS_PER_DAY * monthDays[i-1];	// "monthDay" array starts from 0.
		}
	}
	seconds+= (tm_time->tm_mday-1) * SECS_PER_DAY;		// Days...
	seconds+= tm_time->tm_hour * SECS_PER_HOUR;		// Hours...
	seconds+= tm_time->tm_min * SECS_PER_MIN;		// Minutes...
	seconds+= tm_time->tm_sec;				// ...and finally, Seconds.
	return (time_t)seconds; 
}




