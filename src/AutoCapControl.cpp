#include "FS.h"
#include "ArduinoJson.h"
#include "AutoCapControl.h"
#include "Config.h"
#include "mystrlib.h"

#define MAX_CONFIGDATA_SIZE 256

extern ValueActuator defaultActuator;
Actuator* AutoCapControl::capper = &defaultActuator;


AutoCapControl autoCapControl;


void AutoCapControl::begin(void)
{
    _settings = theSettings.autoCapSettings();

   // check config
    if (_settings->autoCapMode == AutoCapModeManualClose){
        if(AutoCapControl::capper != &defaultActuator)
            AutoCapControl::capper->setActive(true);

    }else if( _settings->autoCapMode == AutoCapModeManualOpen){
        if(AutoCapControl::capper != &defaultActuator)
            AutoCapControl::capper->setActive(false);
    }
}

void AutoCapControl::saveConfig(void)
{
    theSettings.save();
}
void AutoCapControl::capAtTime(uint32_t now)
{
    _settings->autoCapMode = AutoCapModeTime;
    _settings->condition.targetTime = now;
    saveConfig();
}

void AutoCapControl::catOnGravity(float sg)
{
    _settings->autoCapMode = AutoCapModeGravity;
    _settings->condition.targetGravity = sg;
    saveConfig();
}

void AutoCapControl::capManualSet(bool capped)
{
     _settings->autoCapMode =capped? AutoCapModeManualClose:AutoCapModeManualOpen;

    if(AutoCapControl::capper != &defaultActuator)
        capper->setActive(capped);
    saveConfig();
}

bool AutoCapControl::autoCapOn(uint32_t current, float gravity)
{
    if( AutoCapControl::capper == &defaultActuator ) return false;

    if(_settings->autoCapMode == AutoCapModeNone){
        // asigned. auto change to open
        // not necessary for it is check at first statement
        //    if( AutoCapControl::capper != &defaultActuator ) 
        _settings->autoCapMode = AutoCapModeManualOpen;
        return true;
    }else if(_settings->autoCapMode == AutoCapModeManualClose ){
        if(!AutoCapControl::capper->isActive())
            AutoCapControl::capper->setActive(true);
    }else if (_settings->autoCapMode == AutoCapModeManualOpen){
        if(AutoCapControl::capper->isActive())
            AutoCapControl::capper->setActive(false);
    }else if(_settings->autoCapMode == AutoCapModeTime){
        if(current > _settings->condition.targetTime){
            if(!AutoCapControl::capper->isActive()){
                DBG_PRINTF("times up, capped. act:%d\n",AutoCapControl::capper->isActive());
                AutoCapControl::capper->setActive(true);
                return true;
            }
        }else if(AutoCapControl::capper->isActive())
            AutoCapControl::capper->setActive(false);
    }else if(_settings->autoCapMode == AutoCapModeGravity){
        if((gravity > 0.8 && gravity < 1.25)
            && (gravity <= _settings->condition.targetGravity)){
            if(!AutoCapControl::capper->isActive()){
                DBG_PRINTF("gravity meet, capped.\n");
                AutoCapControl::capper->setActive(true);
                return true;
            }
        }if(AutoCapControl::capper->isActive())
            AutoCapControl::capper->setActive(false);
    }

    return false;
}
