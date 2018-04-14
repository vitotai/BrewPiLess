#ifndef BPLSettings_H
#define BPLSettings_H
#include <FS.h>
#include <time.h>
//*****************************************************
// System confiuration
/*R"END(
{"name":"brewpiless",
"user":"brewpiless",
"pass":"brewpiless",
"title":"brewpiless",
"protect":0,
"wifi":1,
"ip":"0.0.0.0",
"gw":"0.0.0.0",
"mask":"255.255.255.0",
"port":80}
)END";
*/
typedef struct _SystemConfiguration{
    char  username[32];
    char  password[32];
    char  hostnetworkname[32];
    char  titlelabel[32];
    uint32_t  backlite;
    uint32_t  ip;
    uint32_t  gw;
    uint32_t  netmask;
    uint16_t  port;
    uint8_t passwordLcd;
    uint8_t wifiMode;
    uint8_t _padding[8];
}SystemConfiguration;

//*****************************************************
// time information
typedef struct _TimeInformation{
    uint32_t savedTime;
    uint32_t timezoneoffset;
    uint8_t _padding[4];
} TimeInformation;

//*****************************************************
// gravity device
typedef struct _GravityDeviceConfiguration{
    float ispindelCoefficients[4];
    float   lpfBeta;
    uint8_t  ispindelEnable;
    uint8_t  ispindelTempCal;
    uint8_t  calculateGravity;
    uint8_t  ispindelCalibrationBaseTemp;
	uint8_t  stableThreshold;
	uint8_t  numberCalPoints;

    uint8_t _padding[6];
}GravityDeviceConfiguration;

//*****************************************************
// Beer Profile

// 
#define InvalidStableThreshold 0xFF
#define MaximumSteps 7
// datys are encoded by *100
#define ScheduleDayFromJson(d)  ((uint16_t) ((d) * 100.0))
#define ScheduleDayToJson(d)  ((float)(d)/100.0)
#define ScheduleDayToTime(d) ((uint32_t)((float)(d)/100.0 * 86400))
#define ScheduleTempFromJson(t) ((int16_t)((t)*100.0))
#define ScheduleTempToJson(t) ((float)(t)/100.0)

#define ScheduleTemp(t) ((float)t/100.0)
typedef int16_t Gravity;

#define INVALID_GRAVITY -1
#define IsGravityValid(g) ((g)>0)
#define FloatToGravity(f) ((Gravity)((f) * 1000.0 +0.5))
#define GravityToFloat(g) (((float)(g) / 1000.0))
#define PointToGravity(p) (1000+(Gravity)((p)+0.5))

typedef struct _ScheduleStep{
    int16_t   temp;
    uint16_t  days;
    union GravityT{
        uint16_t  sg;
        uint16_t  attenuation;
        struct _StableT{
            uint8_t  stableTime;
            uint8_t  stablePoint;
        } stable;
    } gravity;
    uint8_t  attSpecified;
    char     condition;
    uint8_t _padding[3];
} ScheduleStep; // 12bytes

typedef struct _BeerTempSchedule{
	ScheduleStep steps[MaximumSteps];
	time_t   startDay;
    uint8_t  numberOfSteps;
    char     unit;
    uint8_t  _padding[6];
} BeerTempSchedule;

typedef struct _BrewStatus{
	time_t   startingDate;
	time_t   timeEnterCurrentStep;
	time_t   currentStepDuration;
	uint8_t  OGPoints;
	uint8_t  currentStep;
    uint8_t _padding[6];
}BrewStatus;

//*****************************************************
// Local logging
#define MAX_LOG_FILE_NUMBER 10

#define MaximumLogFileName 24

typedef struct _FileIndexEntry{
	char name[MaximumLogFileName];
	unsigned long time;
} FileIndexEntry;

typedef struct _FileIndexes
{
	FileIndexEntry files[MAX_LOG_FILE_NUMBER];
	char logname[MaximumLogFileName];
	unsigned long starttime;
    uint8_t _padding[8];
} FileIndexes;

//*****************************************************
// Remote logging
#define MaximumContentTypeLength 48
#define MaximumUrlLength 128
#define MaximumFormatLength 256
#define mHTTP_GET 0 
#define mHTTP_POST 1
#define mHTTP_PUT 2 

typedef struct _RemoteLoggingInformation{
	char url[MaximumUrlLength];
	char format[MaximumFormatLength];
	char contentType[MaximumContentTypeLength];
	time_t period;
	uint8_t method;
	uint8_t enabled;
    uint8_t _padding[3];
} RemoteLoggingInformation;


//*****************************************************
// Auto Cap

typedef struct _AutoCapSettings{
    union _condition{
        uint32_t targetTime;
        float    targetGravity;
    }condition;
    uint8_t autoCapMode;
    uint8_t _padding[7];
} AutoCapSettings;

//####################################################
// whole structure
struct Settings{
    SystemConfiguration syscfg;
    TimeInformation  timeinfo;
    GravityDeviceConfiguration gdc;
    BeerTempSchedule tempSchedule;
    BrewStatus  brewStatus;
    FileIndexes  logFileIndexes;
    RemoteLoggingInformation remoteLogginInfo;
    AutoCapSettings autoCapSettings;
};

class BPLSettings
{
public:
    BPLSettings(){}

    void load();
    void save();
    // system configuration
    SystemConfiguration* systemConfiguration();
    bool dejsonSystemConfiguration(String json);
    String jsonSystemConfiguration(void);
    // time info
    TimeInformation* timeInformation(void){ return &_data.timeinfo;}
    // gravity device
    GravityDeviceConfiguration* GravityConfig(void){return &_data.gdc; }
    bool dejsonGravityConfig(char* json);
    String jsonGravityConfig(void);
    // beer profile
    BeerTempSchedule* beerTempSchedule(void){ return &_data.tempSchedule;}
    BrewStatus* brewStatus(void){ return &_data.brewStatus;}
    bool dejsonBeerProfile(String json);
    String jsonBeerProfile(void);

    // local log
    FileIndexes* logFileIndexes(void){ return &_data.logFileIndexes; }

    // Remote logging
    RemoteLoggingInformation *remoteLogInfo(void){return &_data.remoteLogginInfo;}
    bool dejsonRemoteLogging(String json);
    String jsonRemoteLogging(void);

    // autocap
    AutoCapSettings *autoCapSettings(void){ return &_data.autoCapSettings;}
protected:
    Settings _data;
};

extern BPLSettings theSettings;
#endif //BPLSettings_H