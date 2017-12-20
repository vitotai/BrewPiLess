
#pragma once

#include "TempController.h"


class ParasiteTempController
{
public:
    ParasiteTempController:_enabled(false)(){}
    void run();
    void init();

    String getSettings();
    bool updateSettings(String json);
    
protected:
    bool _enabled;
    bool _cooling;

    float _setTemp;
    float _maxIdleTemp;

    uint8_t _actuatorPin;
    bool _invertedActuator;

    uint32_t _minCoolingTime;
    uint32_t _minIdleTime;
    uint32_t _lastSwitchedTime;

    void _setCooling(bool cool);
    bool _parseJson(const char* json);
    bool _checkPinAvailable(uint8_t pin);
};

extern ParasiteTempController parasiteTempController;