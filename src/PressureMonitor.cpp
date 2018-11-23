#include "PressureMonitor.h"
#include "AutoCapControl.h"

#if SupportPressureTransducer
#define MinimumMonitorTime 60000
#define MinimumControlCheckTime 1000

PressureMonitorClass PressureMonitor;

#define MULTIPLE_READ_NUMBER 3

int PressureMonitorClass::currentAdcReading(){
    float reading=0;
    for(int i=0;i<MULTIPLE_READ_NUMBER;i++)
        reading +=(float) analogRead(A0);
    reading = reading / MULTIPLE_READ_NUMBER;

    return (int)reading;
}

void PressureMonitorClass::_readPressure(){
    float reading=0;
    for(int i=0;i<MULTIPLE_READ_NUMBER;i++)
        reading +=(float) analogRead(A0);
    reading = reading / MULTIPLE_READ_NUMBER;

    _currentPsi = (reading - _settings->fb) * _settings->fa;
    DBG_PRINTF("ADC:%d, PSIx10:%d\n",(int)reading,(int)(_currentPsi*10));
}

PressureMonitorClass::PressureMonitorClass(){
    _settings=theSettings.pressureMonitorSettings();
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