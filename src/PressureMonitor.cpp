#include <ESP8266WiFi.h>
#include "PressureMonitor.h"
#include "AutoCapControl.h"
#include "BrewLogger.h"

#if SupportPressureTransducer
#define MinimumMonitorTime 10000
#define MinimumControlCheckTime 1000

#if FilterPressureReading
#define LowPassFilterParameter 0.15
#endif

PressureMonitorClass PressureMonitor;

#define MULTIPLE_READ_NUMBER 3

int PressureMonitorClass::currentAdcReading(){
    float reading=0;
    
    #if 0
    for(int i=0;i<MULTIPLE_READ_NUMBER;i++)
        reading +=(float) analogRead(A0);
    reading = reading / MULTIPLE_READ_NUMBER;
    #endif

    reading =(float) system_adc_read();

    return (int)reading;
}

void PressureMonitorClass::_readPressure(){
    float reading=0;

    float reading;
    system_soft_wdt_stop();
    ets_intr_lock( ); 
//    noInterrupts();
    reading =(float) system_adc_read();
//    interrupts();
    ets_intr_unlock(); 
    system_soft_wdt_restart();

     //analogRead(A0);

    float psi = (reading - _settings->fb) * _settings->fa;
    #if FilterPressureReading
    _currentPsi = _currentPsi + LowPassFilterParameter *(psi - _currentPsi);
    DBG_PRINTF("ADC:%d  PSIx10:%d currentx10:%d\n",(int)reading,(int)(psi*10),(int)_currentPsi*10);
    #else
    _currentPsi = psi
    DBG_PRINTF("ADC:%d  PSIx10:%d\n",(int)reading,(int)(psi*10));
    #endif
}

PressureMonitorClass::PressureMonitorClass(){
    _currentPsi = 0;
    _settings=theSettings.pressureMonitorSettings();

    wifi_set_sleep_type(NONE_SLEEP_T);
}

void PressureMonitorClass::setTargetPsi(uint8_t psi){
    _settings->psi =psi;
    theSettings.save();
}

uint8_t PressureMonitorClass::getTargetPsi(void){
    if(_settings->mode != PMModeControl) return 0;
    return _settings->psi;
}

void PressureMonitorClass::configChanged(void){
        
}

void PressureMonitorClass::loop(){
    if(_settings->mode == PMModeOff) return;
    uint32_t current= millis();
    if(_settings->mode == PMModeMonitor
            && (current - _time) > MinimumMonitorTime ){
        // read pressure, and report
        _time=current;
        _readPressure();

    }else if(_settings->mode == PMModeControl
            && (current - _time) > MinimumControlCheckTime ){
        
        _time=current;
        _readPressure();
        if(_currentPsi >0){
            // only when capping status is ON
            if(autoCapControl.isCapOn() && _settings->psi > 0){
                if( _currentPsi > _settings->psi){
                    // open
                    autoCapControl.setPhysicalCapOn(false);
                }else{
                    // close
                    autoCapControl.setPhysicalCapOn(true);
                }
            }
        }
    }
}

#endif