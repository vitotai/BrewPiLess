
#pragma once

#include "TempControl.h"


class ParasiteTempController
{
public:
    ParasiteTempController(){}
    void run();
    void init();

    String getSettings();
    bool updateSettings(String json);
    
    char getMode();
    uint32_t getTimeElapsed();

    int getTemp(){ return(int)(_currentTemp * 100.0);}
    int getLowerBound(){return(int)(_setTemp * 100.0);}
    int getUpperBound(){return(int)(_maxIdleTemp * 100.0);}

    static Actuator *cooler;

protected:
    bool _validSetting;
    float _setTemp;
    float _maxIdleTemp;

    float _currentTemp;

    uint32_t _minCoolingTime;
    uint32_t _minIdleTime;
    uint32_t _lastSwitchedTime;

    uint32_t _lastSensorValidTime;

    void _setCooling(bool cool);
    bool _parseJson(const char* json);
};

extern ParasiteTempController parasiteTempController;