#ifndef BPLSettings_H
#define BPLSettings_H
#include <FS.h>

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

// time information
typedef struct _TimeInformation{
    uint32_t savedTime;
    uint32_t timezoneoffset;
    uint8_t _padding[4];
} TimeInformation;

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

// whoe structure
struct Settings{
    SystemConfiguration syscfg;
    TimeInformation  timeinfo;
    GravityDeviceConfiguration gdc;
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

protected:
    Settings _data;
};

extern BPLSettings theSettings;
#endif //BPLSettings_H