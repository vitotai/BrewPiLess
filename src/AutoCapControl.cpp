#include "AutoCapControl.h"

extern ValueActuator defaultActuator;
Actuator* AutoCapControl::capper = &defaultActuator;


AutoCapControl autoCapControl;

void AutoCapControl::setCapState(bool capped)
{
    capper->setActive(capped);
}

void AutoCapControl::autoCapOn(uint32_t current, float gravity)
{
    if(_autoCapMode == AutoCapModeNone) return;

    if(_autoCapMode == AutoCapModeTime){
        if(current > _targetTime){
            DBG_PRINTF("times up, capped.\n");
            capper->setActive(true);
        }
    }else if(_autoCapMode == AutoCapModeGravity){
        if(gravity <= _targetGravity){
            DBG_PRINTF("gravity meet, capped.\n");
            capper->setActive(true);
        }
    }
}
