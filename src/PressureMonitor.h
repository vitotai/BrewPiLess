#ifndef PressureMonitor_H
#define PressureMonitor_H

#if ESP32
#include "driver/adc.h"
#include "esp_adc_cal.h"
#endif

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
    void begin(void);
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
    uint8_t _adcType;
    uint32_t _time;
    float _currentPsi;
#if PressureViaADS1115
    Adafruit_ADS1115 *_ads;
#endif

    void _readPressure(void);

    void _initInternalAdc(void);
    void _deinitInternalAdc(void);
    int  _readInternalAdc(void);
#if PressureViaADS1115
    void _initExternalAdc(void);
    void _deinitExternalAdc(void);
    int  _readExternalAdc(void);
#endif

#if ESP32
    esp_adc_cal_characteristics_t *_adcCharacter;
#endif
};

extern PressureMonitorClass PressureMonitor;
#endif

#endif