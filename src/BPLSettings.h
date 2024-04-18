#ifndef BPLSettings_H
#define BPLSettings_H
#include <FS.h>
#include <time.h>
#include "Config.h"
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
    uint32_t dns;
    uint8_t  displayMode;
    uint8_t glycolChilling;
    uint8_t _padding[2];
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
#define GravityDeviceNone 0
#define GravityDeviceIspindel 1
#define GravityDeviceTilt 2
#define GravityDevicePill 3
#define MaxDeviceTypeNumber 3


#define SGFromSetting(a)  ((float)(a) /10000.0)
#define SGToSetting(a)   ((uint16_t)(((a) * 10000.0) + 0.5))

#define PlatoFromSetting(a)  ((float)(a) /100.0)
#define PlatoToSetting(a)   ((uint16_t)(((a) * 100.0)+0.5))

#define AngleFromSetting(a)  ((float)(a) /100.0)
#define AngleToSetting(a)   ((uint16_t)(((a) * 100.0) + 0.5))
#define MaxNumberCalibrationPoints 10

typedef  struct _CalibrationPoint{
        uint16_t raw;
        uint16_t calsg;
} CalibrationPoint;


typedef struct _GravityDeviceConfiguration{
    float   coefficients[4];
    float   lpfBeta;
	float   offset;
    
    uint8_t  gravityDeviceType;
    uint8_t  calbybpl;
    uint8_t  numCalPoints;

	uint8_t  stableThreshold;
	uint8_t  usePlato;
    uint8_t  _padding[6];
    CalibrationPoint calPoints[MaxNumberCalibrationPoints];
    uint8_t  _unused2;
}GravityDeviceConfiguration;


#if SupportBleHydrometer

typedef struct _TiltConfiguratoin{
    uint8_t  tiltColor;
    uint8_t  _padding[10];
} TiltConfiguration;

typedef struct _PillConfiguratoin{
    uint8_t  macAddress[6];
    uint8_t  _padding[5];
} PillConfiguration;

typedef union _BleHydrometerConfiguration{
    TiltConfiguration tilt;
    PillConfiguration pill;
}BleHydrometerConfiguration;

#endif
//*****************************************************
// Beer Profile

// 
#define InvalidStableThreshold 0xFF
#define MaximumSteps 10
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
// 12 * 10 +12 = 132
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
    uint8_t writeOnBufferFull;
    uint8_t _padding[7];
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

//*****************************************************
// MQtt remote control
// too many strings. fixed allocation wastes too much.
// server, user, pass, 4x path = 128 * 7 
// Furthermore, ArduinoJson will modify the "source" buffer.
// so additional buffer is neede to decode.
// So let's store the strings in  a compact way 
#if SupportMqttRemoteControl

#define MqttModeOff 0
#define MqttModeControl 1
#define MqttModeLogging 2
#define MqttModeBothControlLoggging 3

#define MqttReportIndividual 0
#define MqttReportJson 1

#define MqttSettingStringSpace 320
typedef struct _MqttRemoteControlSettings{
    uint16_t port;
    uint8_t  mode;
    uint8_t  reportFormat;

    uint16_t  serverOffset;
    uint16_t  usernameOffset;
    uint16_t  passwordOffset;
    uint16_t  modePathOffset;
    uint16_t  beerSetPathOffset;
    uint16_t  capControlPathOffset;
    uint16_t  ptcPathOffset;
    uint16_t  fridgeSetPathOffset;

    uint16_t  reportBasePathOffset;
    uint16_t  reportPeriod;
    uint8_t   _padding2[2];

    uint8_t   _strings[MqttSettingStringSpace];
}MqttRemoteControlSettings;
#endif

//*****************************************************
// Pressure Sensor
#if SupportPressureTransducer
#define PMModeOff 0
#define PMModeMonitor 1
#define PMModeControl 2

#define TransducerADC_Internal 0
#define TransducerADC_ADS1115 1

typedef struct _PressureMonitorSettings{
    float fa;
    int16_t fb;
    uint8_t mode;
    uint8_t psi;
    uint8_t adc_type;
    uint8_t adc_gain;
    uint8_t _padding[7];
}PressureMonitorSettings;
#endif

#ifdef SaveWiFiConfiguration
typedef struct _WiFiConfiguration{
    char ssid[33];
    char pass[33];
    char _padding[30];
} WiFiConfiguration;
#endif

//####################################################
/* HumidityControl */
// 28 bytes

typedef struct _HumidityControlSettings{
    uint8_t  mode;
    uint8_t  target;
    uint8_t  idleLow;
    uint8_t  idleHigh;
    uint8_t  humidifyingTargetHigh;
    uint8_t  dehumidifyingTargetLow;
    uint8_t  _reserved0;
    uint8_t  _reserved1;

    uint16_t minHumidifyingIdleTime;
    uint16_t minHumidifyingRunningTime;
    uint16_t minDehumidifyingIdleTime;
    uint16_t minDehumidifyingRunningTime;
    uint16_t minDeadTime;
    uint8_t  _reserved2;
    uint8_t  _reserved3;

    uint8_t  _reserved4[8];
} HumidityControlSettings;


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

#if SupportPressureTransducer
    PressureMonitorSettings pressureMonitorSettings; // 16
#endif
#if SupportMqttRemoteControl
    MqttRemoteControlSettings mqttRemoteControlSettings;
#endif
#ifdef SaveWiFiConfiguration
    WiFiConfiguration wifiConfiguration;
#endif
#if SupportBleHydrometer
    BleHydrometerConfiguration bleHydrometerConfiguration;
#endif
#if EnableHumidityControlSupport
    HumidityControlSettings humidityControl;
#endif
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

    void preFormat(void);
    void postFormat(void);
    
#if SupportPressureTransducer
    //pressure monitor
    PressureMonitorSettings *pressureMonitorSettings(){return &_data.pressureMonitorSettings;}
    bool dejsonPressureMonitorSettings(String json);
    String jsonPressureMonitorSettings(void);
#endif

#if SupportMqttRemoteControl
    MqttRemoteControlSettings *mqttRemoteControlSettings(void){ return & _data.mqttRemoteControlSettings;}
    bool dejsonMqttRemoteControlSettings(String json);
    String jsonMqttRemoteControlSettings(void);
#endif

#ifdef SaveWiFiConfiguration
    WiFiConfiguration *getWifiConfiguration(void){ return &_data.wifiConfiguration;}
    void setWiFiConfiguration(const char* ssid,const char* pass){
        if(ssid) strcpy(_data.wifiConfiguration.ssid,ssid);
        else _data.wifiConfiguration.ssid[0]='\0';
        if(pass) strcpy(_data.wifiConfiguration.pass,pass);
        else _data.wifiConfiguration.pass[0]='\0';
    }
#endif

#if SupportTiltHydrometer
    TiltConfiguration* tiltConfiguration(void){ return & _data.bleHydrometerConfiguration.tilt;}
#endif
#if SupportPillHydrometer
    PillConfiguration* pillConfiguration(void){ return & _data.bleHydrometerConfiguration.pill;}
#endif


#if EnableHumidityControlSupport
    HumidityControlSettings* humidityControlSettings(void){ return & _data.humidityControl;}
#endif

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
#if EanbleParasiteTempControl   
    void defaultParasiteTempControlSettings(void);
#endif
#if EnableHumidityControlSupport
    void defaultHumidityControlSettings(void);
#endif
    void defaultMqttSetting(void);

    bool systemConfigurationSanity(void);
    bool timeInformationSanity(void);
    bool gravityConfigSantiy(void);
    bool beerProfileSanity(void);
    bool logFileIndexesSanity(void);
    bool remoteLoggingSanity(void);
    bool autoCapSettingsSanity(void);
#if EanbleParasiteTempControl   
    bool parasiteTempControlSettingsSanity(void);
#endif
#if EnableHumidityControlSupport
    bool humidityControlSettingsSanity(void);
#endif

    bool mqttSettingSanity(void);

};

extern BPLSettings theSettings;
#endif //BPLSettings_H
