#include "FS.h"
#include "ArduinoJson.h"
#include "AutoCapControl.h"
#include "espconfig.h"
#include "mystrlib.h"
#define CapConfigFileName "/capping"  // there seems to be too many config files.

#define MAX_CONFIGDATA_SIZE 256

extern ValueActuator defaultActuator;
Actuator* AutoCapControl::capper = &defaultActuator;


AutoCapControl autoCapControl;


void AutoCapControl::begin(void)
{
    char buf[MAX_CONFIGDATA_SIZE];
	File f=SPIFFS.open(CapConfigFileName,"r+");
	if(!f){
        return;
	}
    size_t len=f.readBytes(buf,MAX_CONFIGDATA_SIZE);
	buf[len]='\0';    
	f.close();

    // load config from file
    const int BUFFER_SIZE = JSON_OBJECT_SIZE(10);
	StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(buf);

	if (!root.success()
	    || !root.containsKey("s")){
  		DBG_PRINTF("Invalid JSON config\n");
  		return;
	}
    _autoCapMode=(AutoCapMode) root["s"].as<int>();

   // check config
    if (_autoCapMode == AutoCapModeManualClose){
        if(AutoCapControl::capper != &defaultActuator)
            AutoCapControl::capper->setActive(true);

    }else if( _autoCapMode == AutoCapModeManualOpen){
        if(AutoCapControl::capper != &defaultActuator)
            AutoCapControl::capper->setActive(false);
    }else if(_autoCapMode ==AutoCapModeTime){
        if(!root.containsKey("t")){
            DBG_PRINTF("Invalid JSON config\n");
            _autoCapMode == AutoCapModeNone;
      		return;
        }
        _targetTime = root["t"];
    }else if(_autoCapMode ==AutoCapModeGravity){
        if(!root.containsKey("g")){
            DBG_PRINTF("Invalid JSON config\n");
            _autoCapMode == AutoCapModeNone;
      		return;
        }
        _targetTime = root["g"];
    }
}

void AutoCapControl::saveConfig(void)
{
	File f=SPIFFS.open(CapConfigFileName,"w+");
    if(!f) return;
    if(_autoCapMode ==AutoCapModeNone 
        ||  _autoCapMode== AutoCapModeManualClose 
        || _autoCapMode ==AutoCapModeManualOpen){
        f.printf("{\"s\":%d}",_autoCapMode);
    }else if(_autoCapMode ==AutoCapModeTime){
        f.printf("{\"s\":%d, \"t\":%ld}",_autoCapMode,_targetTime);
    }else if(_autoCapMode ==AutoCapModeGravity){
        char buf[10];
        sprintFloat(buf,_targetGravity,2);
        f.printf("{\"s\":%d, \"g\":%s}",_autoCapMode,buf);
    }
	f.close();
}
void AutoCapControl::capAtTime(uint32_t now)
{
    _autoCapMode = AutoCapModeTime;
    _targetTime = now;
    saveConfig();
}

void AutoCapControl::catOnGravity(float sg)
{
    _autoCapMode = AutoCapModeGravity;
    _targetGravity = sg;
    saveConfig();
}

void AutoCapControl::capManualSet(bool capped)
{
     _autoCapMode =capped? AutoCapModeManualClose:AutoCapModeManualOpen;

    if(AutoCapControl::capper != &defaultActuator)
        capper->setActive(capped);
    saveConfig();
}

bool AutoCapControl::autoCapOn(uint32_t current, float gravity)
{
    if( AutoCapControl::capper == &defaultActuator ) return false;

    if(_autoCapMode == AutoCapModeNone){
        // asigned. auto change to open
        // not necessary for it is check at first statement
        //    if( AutoCapControl::capper != &defaultActuator ) 
        _autoCapMode = AutoCapModeManualOpen;
        return true;
    }else if(_autoCapMode == AutoCapModeManualClose ){
        if(!AutoCapControl::capper->isActive())
            AutoCapControl::capper->setActive(true);
    }else if (_autoCapMode == AutoCapModeManualOpen){
        if(AutoCapControl::capper->isActive())
            AutoCapControl::capper->setActive(false);
    }else if(_autoCapMode == AutoCapModeTime){
        if(current > _targetTime){
            if(!AutoCapControl::capper->isActive()){
                DBG_PRINTF("times up, capped. act:%d\n",AutoCapControl::capper->isActive());
                AutoCapControl::capper->setActive(true);
                return true;
            }
        }else if(AutoCapControl::capper->isActive())
            AutoCapControl::capper->setActive(false);
    }else if(_autoCapMode == AutoCapModeGravity){
        if(gravity <= _targetGravity){
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
