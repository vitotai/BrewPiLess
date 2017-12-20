
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
    bool _checkPinAvailable();
    void _markPinNotAvailable(uint8_t pinNr);
    bool _validPin(uint8_t pinNr);
};

extern ParasiteTempController parasiteTempController;