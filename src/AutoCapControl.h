#ifndef AutoCapControl_H
#define AutoCapControl_H

#include "Brewpi.h"
#include "Actuator.h"

typedef enum _AutoCapMode{
    AutoCapModeNone=0,
    AutoCapModeManualOpen=1,
    AutoCapModeManualClose=2,
    AutoCapModeTime=3,
    AutoCapModeGravity=4
}AutoCapMode;

class AutoCapControl
{
public:

    AutoCapControl(void){
        _autoCapMode=  AutoCapModeNone;
    }

    bool isCapOn(void){ return AutoCapControl::capper->isActive();}
    bool autoCapOn(uint32_t current, float gravity);

    void begin(void);
    void capManualSet(bool capped);
    void capAtTime(uint32_t now);
    void catOnGravity(float sg);
    
    uint32_t targetTime(void){return _targetTime;}
    float    targetGravity(void){return _targetGravity;}
    AutoCapMode mode(void){return _autoCapMode;}

    static Actuator* capper;
private:
    AutoCapMode _autoCapMode;
    uint32_t _targetTime;
    float _targetGravity;

    void saveConfig(void);
};

extern AutoCapControl autoCapControl;
#endif