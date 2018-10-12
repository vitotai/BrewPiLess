#ifndef BPLSettings_H
#define BPLSettings_H
#include <FS.h>
#include <time.h>
//*****************************************************
// 156 bytes
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
// 12
typedef struct _TimeInformation{
    uint32_t savedTime;
    uint32_t timezoneoffset;
    uint8_t _padding[4];
} TimeInformation;

//*****************************************************
// gravity device
//  36
typedef struct _GravityDeviceConfiguration{
    float ispindelCoefficients[4];
    float   lpfBeta;
	uint32_t  numberCalPoints;
    
    uint8_t  ispindelEnable;
    uint8_t  ispindelTempCal;
    uint8_t  calculateGravity;
    uint8_t  ispindelCalibrationBaseTemp;

	uint8_t  stableThreshold;
	uint8_t  usePlato;
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
#define ScheduleDayToTime(d) ((uint32_t)(d) * 864) //((uint32_t)((float)(d)/100.0 * 86400))
#define ScheduleTempFromJson(t) ((int16_t)((t)*100.0))
#define ScheduleTempToJson(t) ((float)(t)/100.0)

#define ScheduleTemp(t) ((float)t/100.0)
typedef int16_t Gravity;

#define INVALID_GRAVITY -1
#define IsGravityValid(g) ((g)>0)

#define PointToGravity(p) (1000+(Gravity)((p)+0.5))

#define PlatoToGravity(p) ((uint16_t)((p)*10.0 + 0.5))
#define SGToGravity(p) ((uint16_t)((p)*1000.0 + 0.5))
#define GravityToSG(g) (((float)(g) / 1000.0))
#define GravityToPlato(g) (((float)(g) / 10.0))

// 12
typedef struct _ScheduleStep{
    int16_t   temp;
    uint16_t  days;
    union GravityT{
        uint16_t  sg;
        uint16_t  attenuation;
    } gravity;
    struct _StableT{
        uint8_t  stableTime;
        uint8_t  stablePoint;
    } stable;
    uint8_t  attSpecified;
    char     condition;
    uint8_t _padding[2];
} ScheduleStep; // 12bytes
// 12 * 7 +12 = 96
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
	uint16_t  OGPoints;
	uint8_t  currentStep;
    uint8_t _padding[5];
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
	uint8_t service;
    uint8_t _padding[2];
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

//*****************************************************
// Parasite temp control
typedef struct _ParasiteTempControlSettings{
    float setTemp;
    float maxIdleTemp;
    uint32_t minCoolingTime;
    uint32_t minIdleTime;
    uint8_t _padding[4];
}ParasiteTempControlSettings;

//####################################################
// whole structure
struct Settings{
    SystemConfiguration syscfg; //0:156
    TimeInformation  timeinfo; //156:12
    GravityDeviceConfiguration gdc;  //168:36
    BeerTempSchedule tempSchedule; // 204:96
    BrewStatus  brewStatus; // 300:20
    FileIndexes  logFileIndexes; // 320:316
    RemoteLoggingInformation remoteLogginInfo; // 636: 444
    AutoCapSettings autoCapSettings; // 1080: 12
    ParasiteTempControlSettings parasiteTempControlSettings; //1092: 20
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
    //ParasiteTempControlSettings
    ParasiteTempControlSettings *parasiteTempControlSettings(void){ return &_data.parasiteTempControlSettings;}
    bool dejsonParasiteTempControlSettings(String json);
    String jsonParasiteTempControlSettings(bool enabled);
protected:
    Settings _data;

    void    setDefault(void);

    void defaultSystemConfiguration(void);
    void defaultTimeInformation(void);
    void defaultGravityConfig(void);
    void defaultBeerProfile(void);
    void defaultLogFileIndexes(void);
    void defaultRemoteLogging(void);
    void defaultAutoCapSettings(void);
    void defaultParasiteTempControlSettings(void);
};

extern BPLSettings theSettings;
#endif //BPLSettings_H