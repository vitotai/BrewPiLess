#ifndef AutoCapControl_H
#define AutoCapControl_H

#include "Brewpi.h"
#include "Actuator.h"

typedef enum _AutoCapMode{
    AutoCapModeNone=0,
    AutoCapModeTime,
    AutoCapModeGravity,
}AutoCapMode;

class AutoCapControl
{
public:

    AutoCapControl(void){
        _autoCapMode=  AutoCapModeNone;
    }

    bool capState(void){ return capper->isActive();}

    void setCapState(bool capped);
    void autoCapOn(uint32_t current, float gravity);

    static Actuator* capper;
private:
    AutoCapMode _autoCapMode;
    uint32_t _targetTime;
    float _targetGravity;
};

extern AutoCapControl autoCapControl;
#endif