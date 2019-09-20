#if ESP8266
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include "Brewpi.h"
#include "Actuator.h"
#include "BPLSettings.h"
#include "AutoCapControl.h"
#include "BrewLogger.h"

#include "PressureMonitor.h"

#if SupportPressureTransducer
#define MinimumMonitorTime 10000
#define MinimumControlCheckTime 1000

#if FilterPressureReading
#define LowPassFilterParameter 0.15
#endif

PressureMonitorClass PressureMonitor;

#define MULTIPLE_READ_NUMBER 3

int PressureMonitorClass::currentAdcReading(){

    int reading=0;
    #if PressureViaADS1115
    if(_settings->adc_type == TransducerADC_ADS1115){
        if(_ads)
           reading= _ads->readADC_SingleEnded(ADS1115_Transducer_ADC_NO);
    }else 
    #endif
    {
//        system_soft_wdt_stop();
//        ets_intr_lock( ); 
//      noInterrupts();
#ifdef ESP8266
        reading = system_adc_read();
#endif
//      interrupts();
//        ets_intr_unlock(); 
//        system_soft_wdt_restart();
    }

    return reading;
}

void PressureMonitorClass::_readPressure(){
<<<<<<< HEAD
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
=======
    float reading =(float) currentAdcReading();
>>>>>>> doc

    float psi = (reading - _settings->fb) * _settings->fa;
    #if FilterPressureReading
    _currentPsi = _currentPsi + LowPassFilterParameter *(psi - _currentPsi);
    DBG_PRINTF("ADC:%d  PSIx10:%d currentx10:%d\n",(int)reading,(int)(psi*10),(int)_currentPsi*10);
    #else
    _currentPsi = psi;
    DBG_PRINTF("ADC:%d  PSIx10:%d\n",(int)reading,(int)(psi*10));
    #endif
}

PressureMonitorClass::PressureMonitorClass(){
    _currentPsi = 0;
    _settings=theSettings.pressureMonitorSettings();
#if ESP8266
    wifi_set_sleep_type(NONE_SLEEP_T);
#endif
    #if PressureViaADS1115
    if(_settings->adc_type == TransducerADC_ADS1115){
        _ads = new Adafruit_ADS1115(ADS1115_ADDRESS);
        _ads->begin();
    }else{
        _ads =(Adafruit_ADS1115*) NULL;
    }
    #endif
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
    #if PressureViaADS1115
    if(_settings->adc_type == TransducerADC_ADS1115 && _ads == NULL){
        _ads = new Adafruit_ADS1115(ADS1115_ADDRESS);
        _ads->begin();
    }else if (_settings->adc_type != TransducerADC_ADS1115 && _ads != NULL){
        delete _ads;
        _ads =(Adafruit_ADS1115*) NULL;
    }
    #endif
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