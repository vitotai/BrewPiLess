#ifndef AutoCapControl_H
#define AutoCapControl_H

#include "Brewpi.h"
#include "Actuator.h"
#include "BPLSettings.h"

#define AutoCapModeNone 0
#define AutoCapModeManualOpen 1
#define AutoCapModeManualClose 2
#define AutoCapModeTime 3
#define AutoCapModeGravity 4


#define CapStatusInactive 0
#define CapStatusActive  1
#define CapStatusUnknown 2
typedef uint8_t CapStatus;

class AutoCapControl
{
public:

    AutoCapControl(void):_capStatus(CapStatusUnknown){}

    void begin(void);

    bool isCapOn(void);
    bool isPhysicalCapOn(void);
    void setPhysicalCapOn(bool on);

    bool autoCapOn(uint32_t current, float gravity);
    void capManualSet(bool capped);
    void capAtTime(uint32_t now);
    void catOnGravity(float sg);
    
    uint32_t targetTime(void){return _settings->condition.targetTime;}
    float    targetGravity(void){return _settings->condition.targetGravity;}
    uint8_t mode(void);

    static Actuator* capper;

private:
    AutoCapSettings *_settings;
    CapStatus _capStatus;

    void setCapOn(bool on);

    void saveConfig(void);
};

extern AutoCapControl autoCapControl;
#endif
