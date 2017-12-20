#include "ParasiteTempController.h"

#define MAX_CONFIG_LEN 200
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
}

void ParasiteTempController::init(){
    // init.
  	File config=SPIFFS.open(CONFIG_FILENAME,"r+");
	
    if(config){
    	configBuf[MAX_CONFIG_LEN];

        size_t len=config.readBytes(configBuf,MAX_CONFIG_LEN);
	    configBuf[len]='\0';
        if(_parseJson(configBuf)){
            if(_enabled) _setCooling(false);
        }else{
            _enabled = false;
        }
    }else {
         _enabled = false;
    }
}

void ParasiteTempController::run(){
    if(!_enabled) return;
    _minCoolingTime = 300000;
    _minIdleTime = 300000;

    uint32_t now=millis();
    // read temperature.
    temperature rawTemp= tempControl.getRoomTemp();
    float temp =temperatureFloatValue(rawTemp);

    if(_cooling){
        if(temp <= _setTemp || temp == INVALID_TEMP_FLOAT){
            if(now - _lastSwitchedTime > _minCoolingTime){
                _setCooling(false);
            }
        }
    }else{
        if(temp == INVALID_TEMP_FLOAT) return; // do nothing if temperature unable

        if(temp > _maxIdleTemp){
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
    _enabled = root[EnableKey];
    _actuatorPin = root[PinKey];
    _invertedActuator = root[InvertedKey];
    _setTemp = root[SetTempKey];
    _maxIdleTemp = root[TrigerTempKey];
    _minCoolingTime = root[MinCoolKey] * 1000;
    _minIdleTime = root[MinIdleKey] * 1000;
    // sanity check?

    return true;
}
static const uint8_t _HardwarePinList[]={coolingPin, heatingPin, doorPin};

String ParasiteTempController::getSettings(){
    // using string operation for simpler action?
    const int BUFFER_SIZE = 2*JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(9);
    StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root[EnableKey]=_enabled;
    root[PinKey]=  _actuatorPin ;
    root[InvertedKey]=_invertedActuator;
    root[SetTempKey]=_setTemp ;
    root[TrigerTempKey]_maxIdleTemp;
    root[MinCoolKey] =_minCoolingTime /  1000;
    root[MinIdleKey]= _minIdleTime / 1000;
    JsonArray& pins = root.createNestedArray(PinListKey);
    JsonArray& avails = root.createNestedArray(PinListAvail);
    for(int i=0;i< sizeof(_HardwarePinList)/sizeof(uint8_t);i++){
        pins.add(_HardwarePinList[i]);
        // check available
        bool available=_checkPinAvailable(_HardwarePinList[i]);
        avails.add(available? 1:0);
    }
}

bool ParasiteTempController::updateSettings(String json){
    // create a buffer. ArduinoJson will modify the data
    if(json.length() >= MAX_CONFIG_LEN ) return false;
    char configBuf[MAX_CONFIG_LEN];
    strcpy(configBuf,json.c_str());
    return _parseJson(configBuf);
}

bool ParasiteTempController::_checkPinAvailable(bool pin){
    return true;
}
