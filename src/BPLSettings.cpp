#include <ArduinoJson.h>
#include <time.h>
#include <string.h>
#include <IPAddress.h>
#include <FS.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#if UseLittleFS
#include <LittleFS.h>
#else
#include <SPIFFS.h>
#endif
#include <rom/spi_flash.h>
#endif

#include "Config.h"
#include "BPLSettings.h"
#include "BrewLogger.h"


extern FS& FileSystem;

BPLSettings theSettings;

#define BPLSettingFileName "/bpl.cfg"


#ifndef WL_MAC_ADDR_LENGTH
#define WL_MAC_ADDR_LENGTH 6
#endif

void BPLSettings::preFormat(void){
	brewLogger.onFormatFS();
}

void BPLSettings::postFormat(void){
	save();
}


void BPLSettings::load()
{
	DBG_PRINTF("syscfg:%d, timeinfo:%d, gdc:%d, \
		tempSchedule:%d, brewStatus:%d, logFileIndexes:%d, \
		remoteLogginInfo:%d, autoCapSettings:%d, parasiteTempControlSettings:%d\n",\
		 offsetof(Settings,syscfg),offsetof(Settings,timeinfo),offsetof(Settings,gdc),
		 offsetof(Settings,tempSchedule),offsetof(Settings,brewStatus),offsetof(Settings,logFileIndexes),
		 offsetof(Settings,remoteLogginInfo),offsetof(Settings,autoCapSettings),
		 offsetof(Settings,parasiteTempControlSettings));

	fs::File f = FileSystem.open(BPLSettingFileName, "r");
	if(!f){
		setDefault();
		return;
	}
	f.read((uint8_t*)&_data,sizeof(_data));
	f.close();
	// check invalid value, and correct
	SystemConfiguration *cfg=  systemConfiguration();
	if( *( cfg->hostnetworkname) == '\0'
		|| cfg->wifiMode ==0){
			setDefault();
			DBG_PRINTF("invalid system configuration!\n");
	}
		
}

void BPLSettings::save()
{
	fs::File f = FileSystem.open(BPLSettingFileName, "w");
    if(!f){
		DBG_PRINTF("error open configuratoin file\n");
		return;
	}
    f.write((uint8_t*)&_data,sizeof(_data));
    f.close();
}


void BPLSettings::setDefault(void)
{
	// clear. to be safe
	memset((char*)&_data,'\0',sizeof(_data));
	// 
	defaultSystemConfiguration();
    defaultTimeInformation();
    defaultGravityConfig();
    defaultBeerProfile();
    defaultLogFileIndexes();
    defaultRemoteLogging();
    defaultAutoCapSettings();

#if EnableParasiteTempControl
    defaultParasiteTempControlSettings();
#endif
#if EnableDHTSensorSupport
	defaultHumidityControlSettings();
#endif
}

void BPLSettings::defaultTimeInformation(void){}
void BPLSettings::defaultAutoCapSettings(void){}

void BPLSettings::defaultLogFileIndexes(void){}

//***************************************************************
// system configuration
#define  KeyLcdBackLight "aoff"
#define  KeyPageTitle "title"
#define  KeyHostName "name"
#define  KeyPort     "port"
#define  KeyUsername  "user"
#define  KeyPassword  "pass"
#define  KeyProtect   "protect"
#define  KeyWifi      "wifi"
#define  KeyIpAddress "ip"
#define  KeyGateway   "gw"
#define  KeyNetmask    "mask"
#define  KeyDNS "dns"

#define KeyFlashChipId "fid"
#define KeyFlashRealSize "frsize"
#define KeyFlashAssignedSize "fsize"
#define KeyFileSystemSize "fs"
#define KeyMacAddress "mac"
#define KeyBuildTime "date"

extern IPAddress scanIP(const char *str);

SystemConfiguration* BPLSettings::systemConfiguration(){
    return &_data.syscfg;
}
    // decode json
static void stringNcopy(char *dst,const char *src,size_t n){
	if(strlen(src) < n){
		strcpy(dst,src);
	}else{
		strncpy(dst,src,n-1);
		dst[n-1]='\0';
	}
}


void BPLSettings::defaultSystemConfiguration(void){
    SystemConfiguration *syscfg=&_data.syscfg;

    stringNcopy(syscfg->titlelabel,DEFAULT_PAGE_TITLE,32);
    stringNcopy(syscfg->hostnetworkname,DEFAULT_HOSTNAME,32);
    stringNcopy(syscfg->username,DEFAULT_USERNAME,32);
    stringNcopy(syscfg->password,DEFAULT_PASSWORD,32);

    syscfg->port = 80;
    syscfg->passwordLcd = false;
    syscfg->wifiMode = WIFI_AP_STA;
    syscfg->backlite = 0;
    syscfg->ip = (uint32_t) IPAddress(0,0,0,0);
    syscfg->gw = (uint32_t) IPAddress(0,0,0,0);
    syscfg->netmask = (uint32_t) IPAddress(0,0,0,0);
    syscfg->dns = (uint32_t) IPAddress(0,0,0,0);
}

bool BPLSettings::dejsonSystemConfiguration(String json){

	SystemConfiguration *syscfg=&_data.syscfg;

    const int BUFFER_SIZE = JSON_OBJECT_SIZE(15);
	
	#if ARDUINOJSON_VERSION_MAJOR == 6
	DynamicJsonDocument root(BUFFER_SIZE +json.length());

	auto error = deserializeJson(root,json);

	if(!error)

	#else

 	StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(json.c_str());

    if (root.success())
	#endif
	{
        stringNcopy(syscfg->titlelabel,root[KeyPageTitle],32);
        stringNcopy(syscfg->hostnetworkname,root[KeyHostName],32);
        stringNcopy(syscfg->username,root[KeyUsername],32);
        stringNcopy(syscfg->password,root[KeyPassword],32);

        syscfg->port = root[KeyPort];
        syscfg->passwordLcd = root[KeyProtect];
        syscfg->wifiMode = root[KeyWifi];
        syscfg->backlite = root[KeyLcdBackLight];
		return true;
    }
	return false;
}
    // encod json
String BPLSettings::jsonSystemConfiguration(void){
	#if ARDUINOJSON_VERSION_MAJOR == 6
	DynamicJsonDocument root(1024);
	#else
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
	#endif

    SystemConfiguration *syscfg=&_data.syscfg;

    root[KeyPageTitle]=syscfg->titlelabel;
    root[KeyHostName]= syscfg->hostnetworkname;
    root[KeyUsername]= syscfg->username;
    root[KeyPassword]= syscfg->password;

    root[KeyPort]=   syscfg->port;
    root[KeyProtect]=    syscfg->passwordLcd;
    root[KeyWifi] =   syscfg->wifiMode;
    root[KeyLcdBackLight] =   syscfg->backlite;

    root[KeyIpAddress]= IPAddress(syscfg->ip).toString();
    root[KeyGateway]= IPAddress(syscfg->gw).toString();
    root[KeyNetmask]= IPAddress(syscfg->netmask).toString();
	root[KeyDNS] = IPAddress(syscfg->dns).toString();

// system info
#if ESP32
	root[KeyFlashChipId]=g_rom_flashchip.device_id;
	root[KeyFlashRealSize]=g_rom_flashchip.chip_size;
	#if UseLittleFS
	#error "ESP32 doesn't support LittleFS by default"
	#endif
	root[KeyFileSystemSize]=SPIFFS.totalBytes();
#else
	root[KeyFlashChipId]=ESP.getFlashChipId();
	root[KeyFlashRealSize]=ESP.getFlashChipRealSize();

	FSInfo fs_info;
	FileSystem.info(fs_info);
	root[KeyFileSystemSize]=fs_info.totalBytes;

#endif

	root[KeyFlashAssignedSize]=ESP.getFlashChipSize();
	root[KeyBuildTime] = __DATE__ __TIME__;
	uint8_t mac[WL_MAC_ADDR_LENGTH];
	WiFi.macAddress(mac);
	JsonArray data = root.createNestedArray(KeyMacAddress);
	
	for(int i=0;i<WL_MAC_ADDR_LENGTH;i++){
		data.add(mac[i]);
	}

    String ret;
	#if ARDUINOJSON_VERSION_MAJOR == 6
	serializeJson(root,ret);
	#else
    root.printTo(ret);
	#endif

    return ret;
}
   
//***************************************************************
// gravity device configuration

#define KeyGravityDeviceType "dev"
#define KeyTempCorrection "tc"
#define KeyCorrectionTemp "ctemp"
#define KeyCalculateGravity "cbpl"
#define KeyCoefficientA0 "a0"
#define KeyCoefficientA1 "a1"
#define KeyCoefficientA2 "a2"
#define KeyCoefficientA3 "a3"
#define KeyLPFBeta "lpc"
#define KeyStableGravityThreshold "stpt"
#define KeyNumberCalPoints "npt"
#define KeyUsePlato "plato"
#define KeyTiltColor "color"
#define KeyTiltCalibrationPoints "tcpts"
#define KeyTiltCoefficients "tiltcoe"
 bool BPLSettings::dejsonGravityConfig(char* json)
{
		#if ARDUINOJSON_VERSION_MAJOR == 6
		StaticJsonDocument<JSON_OBJECT_SIZE(16) + 256> root;
		auto error = deserializeJson(root,json);
		if (error)

		#else
    	const int BUFFER_SIZE = JSON_OBJECT_SIZE(16);
		StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(json);
		if (!root.success())
		#endif
		{
  			DBG_PRINTF("Invalid JSON config\n");
  			return false;
		}
        GravityDeviceConfiguration *gdc = &_data.gdc;

		gdc->gravityDeviceType=root[KeyGravityDeviceType];
		gdc->ispindelTempCal = root[KeyTempCorrection];

//		if(gdc->ispindelTempCal){
		    gdc->ispindelCalibrationBaseTemp =
                (root.containsKey(KeyCorrectionTemp))? root[KeyCorrectionTemp]:20;
//		}
		gdc->calculateGravity=root[KeyCalculateGravity];
		gdc->ispindelCoefficients[0]=root[KeyCoefficientA0];
		gdc->ispindelCoefficients[1]=root[KeyCoefficientA1];
		gdc->ispindelCoefficients[2]=root[KeyCoefficientA2];
		gdc->ispindelCoefficients[3]=root[KeyCoefficientA3];
        gdc->lpfBeta =root[KeyLPFBeta];
        gdc->stableThreshold=root[KeyStableGravityThreshold];
		gdc->numberCalPoints=root[KeyNumberCalPoints];
		gdc->usePlato = root.containsKey(KeyUsePlato)? root[KeyUsePlato]:0;
		

		#if SupportTiltHydrometer
		TiltConfiguration *tcfg = & _data.tiltConfiguration;
		tcfg->tiltColor = root[KeyTiltColor];
		
		JsonArray calpts = root[KeyTiltCalibrationPoints].as<JsonArray>();
		int i=0;
		for(JsonVariant v : calpts) {
			JsonArray point=v.as<JsonArray>();
			tcfg->calibrationPoints[i].rawsg= point[0].as<int>();
			tcfg->calibrationPoints[i].calsg= point[1].as<int>();
			i++;
		}
		tcfg->numCalPoints = calpts.size();
		JsonArray tcoe= root[KeyTiltCoefficients].as<JsonArray>();
		for(i=0;i<4;i++) tcfg->coefficients[i] = tcoe[i];
		#endif

		// debug
		#if SerialDebug
		Serial.print("\nCoefficient:");
		for(int i=0;i<4;i++){
		    Serial.print(gdc->ispindelCoefficients[i],10);
		    Serial.print(", ");
		}
		Serial.println("");
		#endif
	return true;
}

String BPLSettings::jsonGravityConfig(void){
		// save to file

		#if ARDUINOJSON_VERSION_MAJOR == 6
		DynamicJsonDocument root(1024);
		#else
    	const int BUFFER_SIZE = JSON_OBJECT_SIZE(16);
		StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();
		#endif

        GravityDeviceConfiguration *gdc = &_data.gdc;

		root[KeyGravityDeviceType] = gdc->gravityDeviceType;
		root[KeyTempCorrection] = gdc->ispindelTempCal;

		root[KeyCorrectionTemp] = gdc->ispindelCalibrationBaseTemp;
		root[KeyCalculateGravity] = gdc->calculateGravity;
		root[KeyLPFBeta] =gdc->lpfBeta;
		root[KeyStableGravityThreshold] = gdc->stableThreshold;

		root[KeyCoefficientA0]=gdc->ispindelCoefficients[0];
		root[KeyCoefficientA1]=gdc->ispindelCoefficients[1];
		root[KeyCoefficientA2]=gdc->ispindelCoefficients[2];
		root[KeyCoefficientA3]=gdc->ispindelCoefficients[3];
		root[KeyNumberCalPoints] = gdc->numberCalPoints;
		root[KeyUsePlato] = gdc->usePlato;

		#if SupportTiltHydrometer
		TiltConfiguration *tcfg = & _data.tiltConfiguration;

		root[KeyTiltColor]	=	tcfg->tiltColor;
		if(tcfg->numCalPoints > 0){
			JsonArray points = root.createNestedArray(KeyTiltCalibrationPoints);
			for(int i=0;i< tcfg->numCalPoints;i++){
				JsonArray point= points.createNestedArray();
				point.add(tcfg->calibrationPoints[i].rawsg);
				point.add(tcfg->calibrationPoints[i].calsg);
			}
		}
		#endif

	 String ret;

	#if ARDUINOJSON_VERSION_MAJOR == 6
	serializeJson(root,ret);
	#else
    root.printTo(ret);
	#endif
    return ret;
}	

void BPLSettings::defaultGravityConfig(void)
{
	GravityDeviceConfiguration *gdc = &_data.gdc;

	//gdc->gravityDeviceType=GravityDeviceNone;
	//gdc->ispindelTempCal =0;

	//gdc->calculateGravity= 0;
    gdc->lpfBeta = 0.1;
    gdc->stableThreshold=1;
	//gdc->numberCalPoints=0;
	#if SupportTiltHydrometer

	TiltConfiguration *tc = & _data.tiltConfiguration;

	tc->numCalPoints = 0;
	tc->coefficients[0] = tc->coefficients[2] =  tc->coefficients[3] = 0.0; 
	tc->coefficients[1] =  1.0;

	#endif
}
  
//***************************************************************
// Beer profile

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
// got from https://github.com/PaulStoffregen/Time
void makeTime(time_t timeInput, struct tm &tm){
// break the given time_t into time components
// this is a more compact version of the C library localtime function
// note that year is offset from 1970 !!!

  uint8_t year;
  uint8_t month, monthLength;
  uint32_t time;
  unsigned long days;

  time = (uint32_t)timeInput;
  tm.tm_sec = time % 60;
  time /= 60; // now it is minutes
  tm.tm_min = time % 60;
  time /= 60; // now it is hours
  tm.tm_hour = time % 24;
  time /= 24; // now it is days
  tm.tm_wday = ((time + 4) % 7) + 1;  // Sunday is day 1

  year = 0;
  days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
  }
  tm.tm_year = year +1970; // year is offset from 1970

  days -= LEAP_YEAR(year) ? 366 : 365;
  time  -= days; // now it is days in this year, starting at 0

  days=0;
  month=0;
  monthLength=0;
  for (month=0; month<12; month++) {
    if (month==1) { // february
      if (LEAP_YEAR(year)) {
        monthLength=29;
      } else {
        monthLength=28;
      }
    } else {
      monthLength = monthDays[month];
    }

    if (time >= monthLength) {
      time -= monthLength;
    } else {
        break;
    }
  }
  tm.tm_mon = month + 1;  // jan is month 1
  tm.tm_mday = time + 1;     // day of month
}

 void BPLSettings::defaultBeerProfile(void)
 {
	BeerTempSchedule *tempSchedule = & _data.tempSchedule;
	tempSchedule->unit = 'C';
	tempSchedule->numberOfSteps =1;
	ScheduleStep *step = &tempSchedule->steps[0];
	step->condition = 't';
	step->days = ScheduleDayFromJson(7);
	step->temp = ScheduleTempFromJson(20);
 }

bool BPLSettings::dejsonBeerProfile(String json)
{
	const int PROFILE_JSON_BUFFER_SIZE = JSON_ARRAY_SIZE(15) + 7*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 3*JSON_OBJECT_SIZE(4) + 5*JSON_OBJECT_SIZE(6);
	#if ARDUINOJSON_VERSION_MAJOR == 6
	DynamicJsonDocument root(PROFILE_JSON_BUFFER_SIZE + json.length());
	auto error = deserializeJson(root,json);
	if(error)
	#else
	DynamicJsonBuffer jsonBuffer(PROFILE_JSON_BUFFER_SIZE);
	JsonObject& root = jsonBuffer.parseObject(json.c_str());

	if(!root.success())
	#endif
	{
		DBG_PRINTF("JSON parsing failed json size:%d\n",PROFILE_JSON_BUFFER_SIZE);
		return false;
	}
	if(!root.containsKey("s")
		|| !root.containsKey("u")
		|| !root.containsKey("t")){
		DBG_PRINTF("JSON file not include necessary fields\n");
		return false;
	}
	#if ARDUINOJSON_VERSION_MAJOR == 6
	if (!root.as<JsonVariant>()["t"].is<JsonArray>()){
	#else
	if (!root["t"].is<JsonArray&>()){
	#endif
		DBG_PRINTF("JSON t is not array\n");		
		return false;
	}
	BeerTempSchedule *tempSchedule = & _data.tempSchedule;
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

	tempSchedule->startDay= tm_to_timet(&tmStart);
	#if ARDUINOJSON_VERSION_MAJOR == 6
	JsonArray schedule = root["t"];
	#else
	JsonArray& schedule = root["t"];
	#endif
	tempSchedule->numberOfSteps=schedule.size();
	if(tempSchedule->numberOfSteps > MaximumSteps) tempSchedule->numberOfSteps=MaximumSteps;

	for(int i=0;i< tempSchedule->numberOfSteps ;i++){
		ScheduleStep *step = &tempSchedule->steps[i];
		#if ARDUINOJSON_VERSION_MAJOR == 6
		JsonObject	 entry= schedule[i];
		#else
		JsonObject&	 entry= schedule[i];
		#endif
		//{"c":"g","d":6,"t":12,"g":1.026},{"c":"r","d":1}
		const char* constr= entry["c"];
		step->condition = *constr;
		step->days = ScheduleDayFromJson(entry["d"].as<float>());

		DBG_PRINTF("%d ,type:%c time:",i,step->condition );
		DBG_PRINT(step->days);

		if(step->condition != 'r'){ // all but not ramping
			float temp=entry["t"];
			step->temp= ScheduleTempFromJson(temp);

			if(entry.containsKey("g")){
    			if(entry["g"].is<const char*>()){
					step->attSpecified=false;
    			    const char* attStr=entry["g"];
    			    float att=atof(attStr);
    			    if( strchr ( attStr, '%' ) > 0){
	    			    DBG_PRINTF(" att:%s sg:%d ",attStr,step->gravity.sg);
						step->attSpecified=true;
						step->gravity.attenuation =(uint8_t) att;
                    }
    			}
				if(! step->attSpecified){
    			    float fsg= entry["g"];
					if(_data.gdc.usePlato){
	    		    	step->gravity.sg = PlatoToGravity(fsg);
					}else{
	    		    	step->gravity.sg = SGToGravity(fsg);
					}
    			    DBG_PRINTF(" sg:%d",step->gravity.sg);
    		    }
			}

			if(entry.containsKey("s")){
    			int st= entry["s"];
	    		step->stable.stableTime =st;
	    		step->stable.stablePoint=(entry.containsKey("x"))? entry["x"]:_data.gdc.stableThreshold;

    			DBG_PRINTF("Stable :%d@%d",step->stable.stablePoint,step->stable.stableTime);
			}

			DBG_PRINT(" temp:");
			DBG_PRINT(step->temp);
		}
		DBG_PRINTF("\n");
	}

	// unit
	const char *uintStr=root["u"];
	tempSchedule->unit=  *uintStr;

	DBG_PRINTF("Load finished, st:%ld, unit:%c, _numberOfSteps:%d\n",tempSchedule->startDay,
	tempSchedule->unit,tempSchedule->numberOfSteps);

	return true;
}

String BPLSettings::jsonBeerProfile(void)
{

	#if ARDUINOJSON_VERSION_MAJOR == 6
		DynamicJsonDocument root(1024);
	#else
 	const int PROFILE_JSON_BUFFER_SIZE = JSON_ARRAY_SIZE(15) + 7*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 3*JSON_OBJECT_SIZE(4) + 5*JSON_OBJECT_SIZE(6);
	DynamicJsonBuffer jsonBuffer(PROFILE_JSON_BUFFER_SIZE);
	JsonObject& root = jsonBuffer.createObject();
	#endif

	BeerTempSchedule *tempSchedule = & _data.tempSchedule;

	//start date
	//ISO time:
	//2016-07-01T05:22:33.351Z
	struct tm * ptm;
	ptm = localtime(& tempSchedule->startDay);
	char timeBuf[32];
	sprintf(timeBuf,"%d-%02d-%02dT%02d:%02d:%02d.0Z",ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,
		ptm->tm_hour,ptm->tm_min,ptm->tm_sec);
	root["s"]=timeBuf;
	// unit, unfortunatly, no "char" type in JSON. "char" will be integer.
	//root["u"]=(char)tempSchedule->unit;
	char unitBuffer[4];
	sprintf(unitBuffer,"%c",tempSchedule->unit);
	root["u"]=unitBuffer;
	#if ARDUINOJSON_VERSION_MAJOR == 6
	JsonArray steps=root.createNestedArray("t");
	#else
	JsonArray& steps=root.createNestedArray("t");
	#endif

	char conditionBuf[MaximumSteps][4];
	char pertages[MaximumSteps][16];
	int  pertageIndex=0;

	for(int i=0;i< tempSchedule->numberOfSteps;i++){
		ScheduleStep *s_step= & tempSchedule->steps[i];
		#if ARDUINOJSON_VERSION_MAJOR == 6
		JsonObject jstep = steps.createNestedObject();
		#else
		JsonObject& jstep = steps.createNestedObject();
		#endif
		// condition
		//jstep["c"] =(char) s_step->condition;
		sprintf(conditionBuf[i],"%c", s_step->condition);
		jstep["c"] = conditionBuf[i];
		// days
		jstep["d"] =ScheduleDayToJson(s_step->days);
		// temp.
		if( s_step->condition != 'r' ){ // not rampping
			jstep["t"]= ScheduleTempToJson(s_step->temp);
		}
		/*
                   <option value="t">Time</option>
                   <option value="g">SG</option>
                   <option value="s">Stable</option>
                   <option value="a">Time & SG</option>
                   <option value="o">Time OR SG</option>
                   <option value="u">Time OR Stable</option>
                   <option value="v">Time & Stable</option>
                    <option value="b">SG OR Stable</option>
                    <option value="x">SG & Stable</option>
                    <option value="w">ALL</option>
                    <option value="e">Either</option>
		*/
		if(strchr("gaobxwe",s_step->condition)){ 
			// gravity
			if(s_step->attSpecified){
				
				sprintf(pertages[pertageIndex],"%d%%",s_step->gravity.attenuation);
				jstep["g"]= pertages[pertageIndex];
				pertageIndex++;
			}else{
				DBG_PRINTF("  sg:%d \n",s_step->gravity.sg);
				if(_data.gdc.usePlato){
					jstep["g"]= GravityToPlato(s_step->gravity.sg);
					#if SerialDebug
					Serial.print("SG:");
					Serial.println(GravityToPlato(s_step->gravity.sg));
					#endif
				}else{
					jstep["g"]= GravityToSG(s_step->gravity.sg);
				}
			}
		}
		if(strchr("suvbxwe",s_step->condition)){ 
			// stable.
			jstep["s"]= s_step->stable.stableTime;
			jstep["x"]= s_step->stable.stablePoint;
		}
	}// end of for
	
	String ret;
	#if ARDUINOJSON_VERSION_MAJOR == 6
	serializeJson(root,ret);
	#else
    root.printTo(ret);
	#endif

    return ret;
}

//***************************************************************
// Remote data logging
void BPLSettings::defaultRemoteLogging(void)
{
	// OK for all zero
}

bool BPLSettings::dejsonRemoteLogging(String json)
{

	RemoteLoggingInformation *logInfo = remoteLogInfo();
	logInfo->enabled=false;

#if ARDUINOJSON_VERSION_MAJOR == 6
	DynamicJsonDocument root(JSON_OBJECT_SIZE(10)+ json.length());
	auto error = deserializeJson(root,json);
	if(error
#else
	const int GSLOG_JSON_BUFFER_SIZE = JSON_OBJECT_SIZE(10);
	DynamicJsonBuffer jsonBuffer(GSLOG_JSON_BUFFER_SIZE);
	JsonObject& root = jsonBuffer.parseObject(json.c_str());

	if(!root.success()
#endif	
		|| !root.containsKey("enabled")
		|| !root.containsKey("format")
		|| !root.containsKey("url")
		|| !root.containsKey("type")
		|| !root.containsKey("method")
		|| !root.containsKey("period")){
#if ARDUINOJSON_VERSION_MAJOR == 6	
		DBG_PRINTF("dejsonRemoteLogging error:%s",error.c_str());
#endif
		return false;
	}

	const char *url=root["url"];
	const char *method=root["method"];
	const char *format=root["format"];
	const char *contentType=root["type"];

	bool enabled= root["enabled"];
	uint32_t period = root["period"];
		
	if(url == NULL || method==NULL || format==NULL 
		|| strcmp(url,"") ==0 || strcmp(method,"") ==0 || strcmp(format,"") ==0){	
		return false;
	}

	if(strlen(url) >=MaximumUrlLength) return false;
	if(strlen(format) >= MaximumFormatLength) return false;
	if(strlen(contentType)>=MaximumContentTypeLength) return false;

	if(strcmp(method,"GET") ==0) logInfo->method = mHTTP_GET;
	else if(strcmp(method,"POST") ==0) logInfo->method = mHTTP_POST;
	else if(strcmp(method,"PUT") ==0)  logInfo->method = mHTTP_PUT;
	else return false;
	strcpy(logInfo->url,url);
	strcpy(logInfo->format,format);
	strcpy(logInfo->contentType,contentType);
	logInfo->period = period;
	logInfo->enabled = enabled;
	logInfo->service = root.containsKey("service")? root["service"]:0;

  	return true;
}

String BPLSettings::jsonRemoteLogging(void)
{
	
#if ARDUINOJSON_VERSION_MAJOR == 6
	DynamicJsonDocument root(512);
#else
    const int GSLOG_JSON_BUFFER_SIZE = JSON_OBJECT_SIZE(10);
	DynamicJsonBuffer jsonBuffer(GSLOG_JSON_BUFFER_SIZE);
	JsonObject& root = jsonBuffer.createObject();
#endif

	RemoteLoggingInformation *logInfo = remoteLogInfo();
	root["enabled"] = logInfo->enabled;
	root["period"] = logInfo->period;
	root["service"] = logInfo->service;

	if(logInfo->method ==mHTTP_GET) root["method"]="GET";
	else if(logInfo->method ==mHTTP_PUT) root["method"]="PUT";
	else if(logInfo->method ==mHTTP_POST) root["method"]="POST";

	root["url"]=(logInfo->url)? logInfo->url:"";
	root["format"]=(logInfo->format)? logInfo->format:"";
	root["type"]=(logInfo->contentType)? logInfo->contentType:"";

	String ret;
	#if ARDUINOJSON_VERSION_MAJOR == 6
	serializeJson(root,ret);
	#else
    root.printTo(ret);
	#endif
    return ret;
}

//***************************************************************
// parasite control
#if EnableParasiteTempControl
#define EnableKey "enabled"
#define SetTempKey "temp"
#define TrigerTempKey "stemp"
#define MinCoolKey "mincool"
#define MinIdleKey "minidle"


void BPLSettings::defaultParasiteTempControlSettings(void)
{
	ParasiteTempControlSettings *ps=parasiteTempControlSettings();
    ps->minIdleTime = 300 * 1000;
    ps->minCoolingTime = 300 * 1000;
    ps->setTemp = 0;
    ps->maxIdleTemp = 4;
}

bool BPLSettings::dejsonParasiteTempControlSettings(String json){
#if ARDUINOJSON_VERSION_MAJOR == 6
	DynamicJsonDocument root(JSON_OBJECT_SIZE(10) + json.length());
	auto error = deserializeJson(root,json);
	if(error
#else
    DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(10));
	JsonObject& root = jsonBuffer.parseObject(json.c_str());
	if(!root.success()
#endif
		|| !root.containsKey(SetTempKey)
		|| !root.containsKey(TrigerTempKey)
		|| !root.containsKey(MinCoolKey)
		|| !root.containsKey(MinIdleKey)){
            return false;
        }
	ParasiteTempControlSettings *ps=parasiteTempControlSettings();

    float n_setTemp = root[SetTempKey];
    float n_maxIdleTemp = root[TrigerTempKey];
    uint32_t n_mincool=root[MinCoolKey] ;
    uint32_t n_minidle= root[MinIdleKey];

    ps->minIdleTime = n_minidle * 1000;
    ps->minCoolingTime = n_mincool * 1000;
    ps->setTemp = n_setTemp;
    ps->maxIdleTemp = n_maxIdleTemp;

    return true;
}

String BPLSettings::jsonParasiteTempControlSettings(bool enabled){
    // using string operation for simpler action?
    const int BUFFER_SIZE = 2*JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(9);

	#if ARDUINOJSON_VERSION_MAJOR == 6
		DynamicJsonDocument root(BUFFER_SIZE + 512);
	#else

    StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
	#endif

    root[EnableKey]= enabled;
    
	ParasiteTempControlSettings *ps=parasiteTempControlSettings();

    root[SetTempKey]=ps->setTemp ;
    root[TrigerTempKey] =ps->maxIdleTemp;
    root[MinCoolKey] = ps->minCoolingTime /  1000;
    root[MinIdleKey]=  ps->minIdleTime / 1000;
    String ret;
	#if ARDUINOJSON_VERSION_MAJOR == 6
	serializeJson(root,ret);
	#else
    root.printTo(ret);
	#endif
    return ret;
}


#endif
//***************************************************************
// pressure control
#if SupportPressureTransducer
#define PressureMonitorModeKey "mode"
#define ConversionAKey "a"
#define ConversionBKey "b"
#define AdcTypeKey "adc"
#define GainKey "gn"

bool BPLSettings::dejsonPressureMonitorSettings(String json){

#if ARDUINOJSON_VERSION_MAJOR == 6
	DynamicJsonDocument root(JSON_OBJECT_SIZE(10) + json.length());
	auto error = deserializeJson(root,json);
	if(error
#else

    DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(10));
	JsonObject& root = jsonBuffer.parseObject(json.c_str());
	DBG_PRINTF("PM json:\"%s\"\n",json.c_str());
	DBG_PRINTF("success:%d\n",root.success());
	if(!root.success()
#endif
		|| !root.containsKey(PressureMonitorModeKey)
		|| !root.containsKey(ConversionAKey)
		|| !root.containsKey(AdcTypeKey)
		|| !root.containsKey(ConversionBKey)){
            return false;
        }
	PressureMonitorSettings *settings=pressureMonitorSettings();
	settings->mode = root[PressureMonitorModeKey];
	settings->fa = root[ConversionAKey];
	settings->fb = root[ConversionBKey];
	settings->adc_type = root[AdcTypeKey];
	settings->adc_gain = root[GainKey];
	return true;
}

String BPLSettings::jsonPressureMonitorSettings(void){
    const int BUFFER_SIZE = JSON_OBJECT_SIZE(9);

	#if ARDUINOJSON_VERSION_MAJOR == 6
	
	DynamicJsonDocument root(BUFFER_SIZE +512);
	
	#else

    StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
	#endif

	PressureMonitorSettings *settings=pressureMonitorSettings();
	root[PressureMonitorModeKey]=settings->mode;
	root[ConversionAKey]=settings->fa;
	root[ConversionBKey]=settings->fb;
	root[AdcTypeKey] = settings->adc_type;
	root[GainKey] =	settings->adc_gain;

    String ret;
	#if ARDUINOJSON_VERSION_MAJOR == 6
	serializeJson(root,ret);
	#else
    root.printTo(ret);
	#endif
    return ret;

}
#endif
//***************************************************************
// MQTT control
#if SupportMqttRemoteControl
#define EnableRemoteControlKey "enabled"
#define ServerAddressKey "server"
#define ServerPort "port"
#define MqttUsernameKey "user"
#define MqttPasswordKey "pass"

#define ModePathKey "mode"
#define BeerSetPathKey "beerset"
#define FridgeSetPathKey "fridgeset"
#define PtcPathKey "ptc"
#define CapPathKey "cap"

#define MqttLoggingKey "log"
#define MqttLogPeriodKey "period"
#define ReportBasePathKey "base"
#define MqttReportFormatKey "format"

String BPLSettings::jsonMqttRemoteControlSettings(void){

	#if ARDUINOJSON_VERSION_MAJOR == 6
		DynamicJsonDocument root(1024);
	#else
    const int BUFFER_SIZE = JSON_OBJECT_SIZE(12);

    DynamicJsonBuffer jsonBuffer(BUFFER_SIZE);
    JsonObject& root = jsonBuffer.createObject();
	#endif
	MqttRemoteControlSettings *settings=mqttRemoteControlSettings();
	
	root[EnableRemoteControlKey] = (settings->mode == MqttModeControl || settings->mode == MqttModeBothControlLoggging)? 1:0;
	root[ServerPort] =  settings->port;

	root[MqttLoggingKey] =  (settings->mode == MqttModeLogging || settings->mode == MqttModeBothControlLoggging)? 1:0;
	root[MqttLogPeriodKey] = settings->reportPeriod;
	root[MqttReportFormatKey] = settings->reportFormat;

	if(settings->reportBasePathOffset){
		char* base=(char*) (settings->_strings + settings->reportBasePathOffset);
		DBG_PRINTF("base path:%s offset:%d\n",base, settings->reportBasePathOffset);
		root[ReportBasePathKey] =base;
	}

	if(settings->modePathOffset){
		char* modepath=(char*) (settings->_strings + settings->modePathOffset);
		DBG_PRINTF("mode path:%s offset:%d\n",modepath, settings->modePathOffset);
		root[ModePathKey] =modepath;
	}

	if(settings->beerSetPathOffset){
		char* setpath=(char*) (settings->_strings + settings->beerSetPathOffset);
		DBG_PRINTF("beerSet path:%s offset:%d\n",setpath, settings->beerSetPathOffset);
		root[BeerSetPathKey] = setpath;
	}

	if(settings->fridgeSetPathOffset){
		char* setpath=(char*) (settings->_strings + settings->fridgeSetPathOffset);
		DBG_PRINTF("fridgeSet path:%s offset:%d\n",setpath, settings->fridgeSetPathOffset);
		root[FridgeSetPathKey] = setpath;
	}


#if	EnableParasiteTempControl
	if(settings->ptcPathOffset){
		root[PtcPathKey] = settings->_strings + settings->ptcPathOffset;
	}
#endif

#if Auto_CAP
	if(settings->capControlPathOffset){
		root[CapPathKey] = settings->_strings + settings->capControlPathOffset;
	}
#endif

	if(settings->serverOffset){
		root[ServerAddressKey] =(char*) (settings->_strings + settings->serverOffset);
	}

	if(settings->usernameOffset){
		root[MqttUsernameKey] =(char*)(settings->_strings + settings->usernameOffset);
	}

	if(settings->passwordOffset){
		root[MqttPasswordKey] =(char*) (settings->_strings + settings->passwordOffset);
	}

    String ret;
	#if ARDUINOJSON_VERSION_MAJOR == 6
	serializeJson(root,ret);
	#else
    root.printTo(ret);
	#endif

	DBG_PRINTF("json:--\n%s\n--\n",ret.c_str());
    return ret;

}
#if ARDUINOJSON_VERSION_MAJOR == 6
static char *copyIfExist(JsonDocument root,const char* key,uint16_t &offset,char* ptr,char* base){
#else
static char *copyIfExist(JsonObject& root,const char* key,uint16_t &offset,char* ptr,char* base){
#endif
	if(root.containsKey(key)){
		const char* str=root[key];
		size_t length = strlen(str) +1;
		if(length==1){
			offset =0;
			return ptr;
		} 


		if(ptr - base  +length > MqttSettingStringSpace ) return NULL;
		strcpy(ptr,str);
		offset = (uint16_t)(ptr - base);

		size_t rto4= (length & 0x3)? ((length & ~0x3) + 4):length;
		ptr += rto4;

		DBG_PRINTF("mqtt set:%s offset:%d, length:%d, ptr inc:%d\n",key,offset,length,rto4);
	}

	return ptr;
}

bool BPLSettings::dejsonMqttRemoteControlSettings(String json){

#if ARDUINOJSON_VERSION_MAJOR == 6
	DynamicJsonDocument root(JSON_OBJECT_SIZE(15) +json.length());
	auto error = deserializeJson(root,json);
	if(error
#else

    DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(10));
	JsonObject& root = jsonBuffer.parseObject(json.c_str());
	if(!root.success()
#endif
		|| !root.containsKey(EnableRemoteControlKey)
		|| !root.containsKey(ServerPort)){

#if ARDUINOJSON_VERSION_MAJOR == 6	
		DBG_PRINTF("dejsonMqttRemoteControlSettings error:%s",error.c_str());
#endif

        return false;
    }
	MqttRemoteControlSettings *settings=mqttRemoteControlSettings();

	memset((char*)settings,'\0',sizeof(MqttRemoteControlSettings));

	bool rc=root[EnableRemoteControlKey];
	if(!root.containsKey(MqttLoggingKey)){
		settings->mode= rc? MqttModeControl:MqttModeOff;
		// everything else is "cleared" by memset to zero
	}else{
		bool log=root[MqttLoggingKey];
		settings->mode= (rc && log)? MqttModeBothControlLoggging:
						( rc? MqttModeControl:
							( log? MqttModeLogging:MqttModeOff));		
		settings->reportPeriod=root[MqttLogPeriodKey];
		settings->reportFormat=root[MqttReportFormatKey];
	}



	settings->port=root[ServerPort];

	char *base=(char*) settings->_strings;
	char *ptr=base +4;

	if(!(ptr=copyIfExist(root,ServerAddressKey,settings->serverOffset,ptr,base))) return false;
	if(!(ptr=copyIfExist(root,MqttUsernameKey,settings->usernameOffset,ptr,base))) return false;
	if(!(ptr=copyIfExist(root,MqttPasswordKey,settings->passwordOffset,ptr,base))) return false;
	if(!(ptr=copyIfExist(root,ModePathKey,settings->modePathOffset,ptr,base))) return false;
	if(!(ptr=copyIfExist(root,BeerSetPathKey,settings->beerSetPathOffset,ptr,base))) return false;
	if(!(ptr=copyIfExist(root,FridgeSetPathKey,settings->fridgeSetPathOffset,ptr,base))) return false;

	if(!(ptr=copyIfExist(root,ReportBasePathKey,settings->reportBasePathOffset,ptr,base))) return false;

	#if	EnableParasiteTempControl
	if(!(ptr=copyIfExist(root,PtcPathKey,settings->ptcPathOffset,ptr,base))) return false;
	#endif

	#if Auto_CAP
	if(!(ptr=copyIfExist(root,CapPathKey,settings->capControlPathOffset,ptr,base))) return false;
	#endif

	return true;
}

#endif



#if EnableDHTSensorSupport

void BPLSettings::defaultHumidityControlSettings(void){

}

#endif
