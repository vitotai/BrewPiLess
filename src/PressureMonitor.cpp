#include "PressureMonitor.h"
#include "AutoCapControl.h"

#if SupportPressureTransducer
#define MinimumMonitorTime 10000
#define MinimumControlCheckTime 1000

#define LowPassFilterParameter 0.15


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
#if 0    
    float reading;
    float rsum=0;
    float max=0;
    float min=1024;
    for(int i=0;i<MULTIPLE_READ_NUMBER +2;i++){
        reading =(float) analogRead(A0);
        if( reading > max) max=reading;
        if( reading < min) min = reading;
        rsum += reading;
    }

    reading = (rsum  -max -min) / MULTIPLE_READ_NUMBER;
    float psi = (reading - _settings->fb) * _settings->fa;
    DBG_PRINTF("ADC:%d max:%d, min:%d,  PSIx10:%d\n",(int)reading,(int)max, int(min),(int)(psi*10));

    if(psi < 0 || psi > 60) return;
    _currentPsi = _currentPsi + LowPassFilterParameter *(psi - _currentPsi);
#endif

    float reading;
    reading =(float) analogRead(A0);
    float psi = (reading - _settings->fb) * _settings->fa;
    _currentPsi = _currentPsi + LowPassFilterParameter *(psi - _currentPsi);
    DBG_PRINTF("ADC:%d  PSIx10:%d currentx10:%d\n",(int)reading,(int)(psi*10),(int)_currentPsi*10);
}

PressureMonitorClass::PressureMonitorClass(){
    _currentPsi = 0;
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