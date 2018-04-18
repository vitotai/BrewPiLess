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

class AutoCapControl
{
public:

    AutoCapControl(void){}

    bool isCapOn(void){ return AutoCapControl::capper->isActive();}
    bool autoCapOn(uint32_t current, float gravity);

    void begin(void);
    void capManualSet(bool capped);
    void capAtTime(uint32_t now);
    void catOnGravity(float sg);
    
    uint32_t targetTime(void){return _settings->condition.targetTime;}
    float    targetGravity(void){return _settings->condition.targetGravity;}
    uint8_t mode(void){return _settings->autoCapMode;}

    static Actuator* capper;
private:
    AutoCapSettings *_settings;

    void saveConfig(void);
};

extern AutoCapControl autoCapControl;
#endif