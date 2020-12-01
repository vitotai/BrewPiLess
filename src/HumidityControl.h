#ifndef HumidityControl_H
#define HumidityControl_H
#include "Config.h"
#include "Brewpi.h"
#include "Actuator.h"
#include "DHTSensor.h"

#define INVALID_HUMIDITY_VALUE 0xFF
#define MINIMUM_HUMIDITY_SENSOR_READ_PERIOD 10000
#define IsValidHumidityValue(a) ((a) <=100)

extern ValueActuator defaultActuator;

typedef struct _HumidityControlSettings{
    uint8_t  target;
    uint8_t  idleLow;
    uint8_t  idleHigh;

    uint8_t  humidifyingTargetHigh;
    uint8_t  dehumidifyingTargetLow;

    uint16_t minHumidifyingIdleTime;
    uint16_t minHumidifyingRunningTime;
    uint16_t minDehumidifyingIdleTime;
    uint16_t minDehumidifyingRunningTime;
    uint16_t minDeadTime;
} HumidityControlSettings;

typedef enum _HumidityControlState{
    HC_Idle,
    HC_Dehumidifying,
    HC_Humidifying,
    HC_WaitDehumidifying,
    HC_WaitHumidifying
}HumidityControlState;

#define ToSystemTick(a) (a)*1000

class HumidityControl{
public:
    static DHTSensor *dhtSensor;
	static Actuator* humidifier;
	static Actuator* dehumidifier;


    HumidityControl():_humidity(INVALID_HUMIDITY_VALUE),_state(HC_Idle),_prevState(HC_Idle){}    
    
    void updateState(){
        switch(_state){
            case HC_Idle:
                if(_humidity > _cfg.target +_cfg.idleHigh){
                    // start dehumidifying, if possible
                    if( dehumidifier != &defaultActuator){
                        if( (_prevState == HC_Dehumidifying && (_lastreadtime - _lastStateChangeTime > ToSystemTick(_cfg.minDehumidifyingIdleTime)))
                         || ( (_prevState == HC_Humidifying || _prevState == HC_Idle )&& (_lastreadtime - _lastStateChangeTime > ToSystemTick(_cfg.minDeadTime) )) ){

                             _prevState = _state;
                             _state = HC_Dehumidifying;
                             _lastStateChangeTime=_lastreadtime;
                             dehumidifier->setActive(true);
                        }
                    }
                }else  if(_humidity < _cfg.target - _cfg.idleLow){
                    // start humidifying
                    if( humidifier != &defaultActuator){
                        if( (_prevState == HC_Humidifying && (_lastreadtime - _lastStateChangeTime > ToSystemTick(_cfg.minHumidifyingIdleTime)))
                         || ( (_prevState == HC_Dehumidifying || _prevState == HC_Idle )&& (_lastreadtime - _lastStateChangeTime > ToSystemTick(_cfg.minDeadTime) )) ){

                             _prevState = _state;
                             _state = HC_Humidifying;
                             _lastStateChangeTime=_lastreadtime;
                             humidifier->setActive(true);
                        }
                    }
                }
                break;

            case HC_Dehumidifying:
                if(_humidity < _cfg.target - _cfg.dehumidifyingTargetLow){
                    if(_lastreadtime - _lastStateChangeTime > ToSystemTick(_cfg.minDehumidifyingRunningTime)){
                        _prevState = _state;
                        _state = HC_Idle;
                        _lastStateChangeTime=_lastreadtime;
                        dehumidifier->setActive(false);
                    }
                }
                break;

            case HC_Humidifying:
                if(_humidity > _cfg.target + _cfg.humidifyingTargetHigh){
                    if(_lastreadtime - _lastStateChangeTime > ToSystemTick(_cfg.minHumidifyingRunningTime)){
                        _prevState = _state;
                        _state = HC_Idle;
                        _lastStateChangeTime=_lastreadtime;
                        humidifier->setActive(false);
                    }
                }
                break;
        }
    }

    void loop(){
            if(dhtSensor == NULL) return;
            uint32_t currenttime = millis();
            if ((currenttime - _lastreadtime) > MINIMUM_HUMIDITY_SENSOR_READ_PERIOD){                
                _lastreadtime = currenttime;
                _humidity= dhtSensor->humidity();
                DBG_PRINTF("Humidity:%d\n",_humidity);

                if(IsValidHumidityValue(_humidity)){
                    updateState();
                }
            }
    }

    uint8_t humidity(){
        return _humidity;
    }
    bool isHumidityValid(){
        return  _humidity <=100; 
    }
    bool sensorInstalled(){
        return dhtSensor != NULL;
    }
private:
    uint8_t _humidity;

    HumidityControlState _state;
    HumidityControlState _prevState;    

    uint32_t _lastreadtime;
    uint32_t _lastStateChangeTime;

    HumidityControlSettings _cfg;
};

extern HumidityControl humidityControl;

#endif