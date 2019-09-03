#ifndef PressureMonitor_H
#define PressureMonitor_H


#include "Brewpi.h"
#include "Actuator.h"
#include "BPLSettings.h"
#if PressureViaADS1115
#include <Adafruit_ADS1015.h>
#endif
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
    void setTargetPsi(uint8_t psi);
    uint8_t getTargetPsi(void);
    void configChanged(void);
protected:
    PressureMonitorSettings *_settings;
    uint32_t _time;
    float _currentPsi;
#if PressureViaADS1115
    Adafruit_ADS1115 *_ads;
#endif

    void _readPressure();
};

extern PressureMonitorClass PressureMonitor;
#endif

#endif