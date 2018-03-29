#include <FS.h>
#include <ArduinoJson.h>
#include "ParasiteTempController.h"
#include "DeviceManager.h"

#include "espconfig.h"
#define MIN_TEM_DIFF 0.5
#define MIN_COOL_TIME 180 //seconds
#define MIN_IDLE_TIME 180 //seconds
#define SensorDisconnectToleranceTime 10000

#define MAX_CONFIG_LEN 200
#define CONFIG_FILENAME "/paratc.cfg"

#define EnableKey "enabled"
#define SetTempKey "temp"
#define TrigerTempKey "stemp"
#define MinCoolKey "mincool"
#define MinIdleKey "minidle"
#define PinListKey "pins"
#define PinListAvail "avail"

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
    _minCoolingTime = 300000;
    _minIdleTime = 300000;
    
    _validSetting = false;
  	
    File config=SPIFFS.open(CONFIG_FILENAME,"r+");
	
    if(config){
    	char configBuf[MAX_CONFIG_LEN];

        size_t len=config.readBytes(configBuf,MAX_CONFIG_LEN);
	    configBuf[len]='\0';

        if(_parseJson(configBuf)){
            _validSetting = true;        
            _setCooling(false);
        }
    }
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
        if(disconnect || ( rawTemp != INVALID_TEMP &&  _currentTemp <= _setTemp)){
            if(now - _lastSwitchedTime > _minCoolingTime){
                _setCooling(false);
            }
        }
    }else{
        if(disconnect) return; // do nothing if temperature unable

        if(rawTemp != INVALID_TEMP && _currentTemp > _maxIdleTemp){
            if(now - _lastSwitchedTime > _minIdleTime){
                _setCooling(true);
            }
        }
    }
}

bool ParasiteTempController::_parseJson(const char* json){
    DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(10));
	JsonObject& root = jsonBuffer.parseObject(json);
	if(!root.success()
		|| !root.containsKey(SetTempKey)
		|| !root.containsKey(TrigerTempKey)
		|| !root.containsKey(MinCoolKey)
		|| !root.containsKey(MinIdleKey)){
            return false;
        }
    // sanity check?

    float n_setTemp = root[SetTempKey];
    float n_maxIdleTemp = root[TrigerTempKey];
    if((n_setTemp + MIN_TEM_DIFF) > n_maxIdleTemp) return false;
  
    uint32_t n_mincool=root[MinCoolKey] ;
    if(n_mincool < MIN_COOL_TIME ) return false;

    uint32_t n_minidle= root[MinIdleKey];
    if(n_mincool < MIN_IDLE_TIME ) return false;

    // the value is valid now.
    _minIdleTime = n_minidle * 1000;
    _minCoolingTime = n_mincool * 1000;
    _setTemp = n_setTemp;
    _maxIdleTemp = n_maxIdleTemp;

    return true;
}

String ParasiteTempController::getSettings(){
    // using string operation for simpler action?
    const int BUFFER_SIZE = 2*JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(9);
    StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root[EnableKey]= (cooler != &defaultActuator);
    
    root[SetTempKey]=_setTemp ;
    root[TrigerTempKey] =_maxIdleTemp;
    root[MinCoolKey] =_minCoolingTime /  1000;
    root[MinIdleKey]= _minIdleTime / 1000;
    String output;
    root.printTo(output);
    return output;
}

bool ParasiteTempController::updateSettings(String json){
    // create a buffer. ArduinoJson will modify the data
    bool ret=true;
    if(json.length() >= MAX_CONFIG_LEN ) return false;
    char configBuf[MAX_CONFIG_LEN];
    strcpy(configBuf,json.c_str());
//    bool oldSettingValid= _validSetting;
    if(!_parseJson(configBuf)){
        _validSetting = false;
        ret=false;
        DBG_PRINTF("Invalid config\n");
    }else{
    	File newconfig=SPIFFS.open(CONFIG_FILENAME,"w+");
        if(newconfig){
    	    newconfig.print(json);
	        newconfig.close();
        }else{
             DBG_PRINTF("erro write config\n");
        }
    }
//    if(oldSettingValid  && !_validSetting){
      // stop cooling anyway
        _setCooling(false);
//    }
    return ret;
}