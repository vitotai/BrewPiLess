#include <FS.h>
#include <ArduinoJson.h>
#include "ParasiteTempController.h"
#include "DeviceManager.h"

#include "Config.h"
#define MIN_TEM_DIFF 0.5
#define MIN_COOL_TIME 180000 //ms
#define MIN_IDLE_TIME 180000 //ms
#define SensorDisconnectToleranceTime 10000

ParasiteTempController parasiteTempController;

extern ValueActuator defaultActuator;
Actuator* ParasiteTempController::cooler = &defaultActuator;


void ParasiteTempController::_setCooling(bool cool){
    if(ParasiteTempController::cooler == &defaultActuator) return;
    cooler->setActive(cool);
    _lastSwitchedTime = millis();
    DBG_PRINTF("Turn cooling %d!\n",cool);
}

char ParasiteTempController::getMode(){
    if(!_validSetting || cooler == &defaultActuator) return 'o';
    if(cooler->isActive()) return 'c';
    else return 'i';
}

uint32_t ParasiteTempController::getTimeElapsed(){
    if(!_validSetting || cooler == &defaultActuator) return 0;
    return (millis() - _lastSwitchedTime)/1000;
}

void ParasiteTempController::init(){
    // init.
    _settings = theSettings.parasiteTempControlSettings();
   _validSetting= checkSettings();
}

void ParasiteTempController::run(){
    if(!_validSetting || cooler == &defaultActuator) return;

    uint32_t now=millis();
    // read temperature.
    temperature rawTemp= tempControl.getRoomTemp();

    bool disconnect=false;

    if(rawTemp == INVALID_TEMP ){
        if(now - _lastSensorValidTime >= SensorDisconnectToleranceTime){
            disconnect =true;
            //moving the disconnect time in case the time roundup
            _lastSensorValidTime = now - SensorDisconnectToleranceTime;
        }
    }else _lastSensorValidTime = now;

    _currentTemp =temperatureFloatValue(rawTemp);

    if(cooler->isActive()){
        if(disconnect || ( rawTemp != INVALID_TEMP &&  _currentTemp <= _settings->setTemp)){
            if(now - _lastSwitchedTime > _settings->minCoolingTime){
                _setCooling(false);
            }
        }
    }else{
        if(disconnect) return; // do nothing if temperature unable

        if(rawTemp != INVALID_TEMP && _currentTemp > _settings->maxIdleTemp){
            if(now - _lastSwitchedTime > _settings->minIdleTime){
                _setCooling(true);
            }
        }
    }
}


bool ParasiteTempController::updateSettings(String json){
    bool ret=theSettings.dejsonParasiteTempControlSettings(json);
    theSettings.save();
    if(ret)_validSetting=checkSettings();
    _setCooling(false);
    return ret;
}
String ParasiteTempController::getSettings(void)
{
   return theSettings.jsonParasiteTempControlSettings(cooler != &defaultActuator);
}
bool ParasiteTempController::checkSettings(void)
{
    if((_settings->setTemp + MIN_TEM_DIFF) > _settings->maxIdleTemp) return false;
    if(_settings->minCoolingTime < MIN_COOL_TIME ) return false;
    if(_settings->minIdleTime < MIN_IDLE_TIME ) return false;
    return true;
}

void ParasiteTempController::setTemperatureRange(float lower,float upper)
{
    _settings->setTemp = lower;
    _settings->maxIdleTemp = upper;
}
