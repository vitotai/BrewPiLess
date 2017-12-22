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
#define PinKey "pin"
#define InvertedKey "inverted"
#define SetTempKey "temp"
#define TrigerTempKey "stemp"
#define MinCoolKey "mincool"
#define MinIdleKey "minidle"
#define PinListKey "pins"
#define PinListAvail "avail"

ParasiteTempController parasiteTempController;

void ParasiteTempController::_setCooling(bool cool){

    digitalWrite(_actuatorPin, cool ^ _invertedActuator ? HIGH : LOW);
    _lastSwitchedTime = millis();
    _cooling=cool;
    DBG_PRINTF("Turn cooling %d!\n",cool);
}

char ParasiteTempController::getMode(){
    if(!_enabled) return 'o';
    if(_cooling) return 'c';
    else return 'i';
}

uint32_t ParasiteTempController::getTimeElapsed(){
    if(!_enabled) return 0;
    return (millis() - _lastSwitchedTime)/1000;
}

void ParasiteTempController::init(){
    // init.
    _cooling=false;
    _minCoolingTime = 300000;
    _minIdleTime = 300000;

    _checkPinAvailable();

  	File config=SPIFFS.open(CONFIG_FILENAME,"r+");
	
    if(config){
    	char configBuf[MAX_CONFIG_LEN];

        size_t len=config.readBytes(configBuf,MAX_CONFIG_LEN);
	    configBuf[len]='\0';
        if(_parseJson(configBuf)){
            // set 
            if(_enabled){
                pinMode(_actuatorPin, OUTPUT);
                _setCooling(false);
            }
        }else{
            _enabled = false;
        }
    }else {
         _enabled = false;
    }
}

void ParasiteTempController::run(){
    if(!_enabled) return;

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

    if(_cooling){
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
	    || !root.containsKey(EnableKey)
		|| !root.containsKey(PinKey)
		|| !root.containsKey(InvertedKey)
		|| !root.containsKey(SetTempKey)
		|| !root.containsKey(TrigerTempKey)
		|| !root.containsKey(MinCoolKey)
		|| !root.containsKey(MinIdleKey)){
            return false;
        }
    // sanity check?

    bool n_enabled = root[EnableKey];
    bool n_invertedActuator = root[InvertedKey];

    uint8_t n_pin = root[PinKey];
    if(!_validPin(n_pin)) return false;

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
    _actuatorPin = n_pin;
    _invertedActuator = n_invertedActuator;
    _enabled = n_enabled;

    return true;
}
static const uint8_t _HardwarePinList[]={coolingPin, heatingPin, doorPin};
static bool _HardwarePinAvailable[]={true, true, true};

String ParasiteTempController::getSettings(){
    // using string operation for simpler action?
    const int BUFFER_SIZE = 2*JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(9);
    StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root[EnableKey]=_enabled;
    root[PinKey]=  _actuatorPin ;
    root[InvertedKey]=_invertedActuator;
    root[SetTempKey]=_setTemp ;
    root[TrigerTempKey] =_maxIdleTemp;
    root[MinCoolKey] =_minCoolingTime /  1000;
    root[MinIdleKey]= _minIdleTime / 1000;
    JsonArray& pins = root.createNestedArray(PinListKey);
    JsonArray& avails = root.createNestedArray(PinListAvail);
    for(int i=0;i< sizeof(_HardwarePinList)/sizeof(uint8_t);i++){
        pins.add(_HardwarePinList[i]);
        // check available
        bool available=_HardwarePinAvailable[i];
        avails.add(available? 1:0);
    }
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
    bool saved= _enabled;
    if(!_parseJson(configBuf)){
        _enabled = false;
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
    if(saved  && !_enabled){
        _setCooling(false);
    }else if(_enabled)
        pinMode(_actuatorPin, OUTPUT);
    return ret;
}

bool ParasiteTempController::_checkPinAvailable(){
	DeviceConfig deviceConfig;
	for (uint8_t index = 0; eepromManager.fetchDevice(deviceConfig, index); index++)
	{
		if (deviceManager.isDeviceValid(deviceConfig, deviceConfig, index)){
            // check device configuration
            if(deviceConfig.deviceHardware == DEVICE_HARDWARE_PIN){
                _markPinNotAvailable(deviceConfig.hw.pinNr);
                DBG_PRINTF("Pin Nr %d alocated.\n",deviceConfig.hw.pinNr);
            }
        }
	}    
}
void ParasiteTempController::_markPinNotAvailable(uint8_t pinNr)
{
    for(int i=0;i< sizeof(_HardwarePinList)/sizeof(uint8_t);i++){
        if(_HardwarePinList[i] == pinNr) _HardwarePinAvailable[i]=false;
    }
}
 bool ParasiteTempController::_validPin(uint8_t pinNr){
    for(int i=0;i< sizeof(_HardwarePinList)/sizeof(uint8_t);i++){
        if(_HardwarePinList[i] == pinNr && _HardwarePinAvailable[i]) return true;
    }
    DBG_PRINTF("Pin Nr %d Not available.\n",pinNr);
    return false;
 }