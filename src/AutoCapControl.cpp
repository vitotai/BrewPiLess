#include <FS.h>
#include <ArduinoJson.h>
#include "AutoCapControl.h"
#include "Config.h"
#include "mystrlib.h"

#define MAX_CONFIGDATA_SIZE 256

extern ValueActuator defaultActuator;
Actuator* AutoCapControl::capper = &defaultActuator;


AutoCapControl autoCapControl;

uint8_t AutoCapControl::mode(void)
{
    if(AutoCapControl::capper != &defaultActuator)
        return _settings->autoCapMode;
    return AutoCapModeNone;
}

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

bool isPhysicalCapOn(void){ 
    if( AutoCapControl::capper == &defaultActuator ) return false;
    return AutoCapControl::capper->isActive(); 
}

void AutoCapControl::setPhysicalCapOn(bool on){
    if( AutoCapControl::capper != &defaultActuator ){
        if(AutoCapControl::capper->isActive() != on){
            DBG_PRINTF("capper CAP:%d\n",on);
            AutoCapControl::capper->setActive(on);
        }
    }
}

bool AutoCapControl::isCapOn(void){ 
    return _capStatus == CapStatusActive;
}

void AutoCapControl::setCapOn(bool on){
    if( AutoCapControl::capper != &defaultActuator ){
            CapStatus status = on? CapStatusActive:CapStatusInactive;
            if(status != _capStatus){
                _capStatus =status;
                setPhysicalCapOn(on);
            }
    }
}

// one thing to keep in mind: the actuactor might be assigned after power on
// before the actuactor is assigned, the status is "unknown", and
// the action to check and set capping status should be done.

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

        if( _capStatus != CapStatusActive) setCapOn(true);

    }else if (_settings->autoCapMode == AutoCapModeManualOpen){

        if(_capStatus != CapStatusInactive) setCapOn(false);

    }else if(_settings->autoCapMode == AutoCapModeTime){
        
        if(current > _settings->condition.targetTime){
            if(_capStatus != CapStatusActive){
                DBG_PRINTF("times up, capped. act:%d\n",_capStatus);
                setCapOn(true);
                return true;
            }
        }else if(_capStatus != CapStatusInactive)
            setCapOn(false);

    }else if(_settings->autoCapMode == AutoCapModeGravity){

        if((gravity <= _settings->condition.targetGravity)){
            if(_capStatus != CapStatusActive){
                DBG_PRINTF("gravity meet, capped.\n");
                setCapOn(true);
                return true;
            }
        }else if(_capStatus != CapStatusInactive)
            setCapOn(false);
    }

    return false;
}
