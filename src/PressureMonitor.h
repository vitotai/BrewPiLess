#ifndef PressureMonitor_H
#define PressureMonitor_H


#include "Brewpi.h"
#include "Actuator.h"
#include "BPLSettings.h"

#if SupportPressureTransducer

typedef uint8_t PMMode;

class PressureMonitorClass{
public:
    PressureMonitorClass();
    float currentPsi(){return _currentPsi;}
    bool isCurrentPsiValid(){return _currentPsi > -1; }
    int currentAdcReading();
    PMMode mode(){return _settings->mode; };    
    void loop();
protected:
    PressureMonitorSettings *_settings;
    uint32_t _time;
    float _currentPsi;
    void _readPressure();
};

extern PressureMonitorClass PressureMonitor;
#endif

#endif