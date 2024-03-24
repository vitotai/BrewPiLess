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

#if SMART_DISPLAY
#include "SharedLcd.h"
#endif

#if SupportPressureTransducer
#define MinimumMonitorTime 10000
#define MinimumControlCheckTime 1000

#if FilterPressureReading
#define LowPassFilterParameter 0.3
#endif
#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   4          //Multisampling

PressureMonitorClass PressureMonitor;

// for esp32
#if ESP32

#ifndef ADC1_GPIO36_CHANNEL
#define ADC1_GPIO36_CHANNEL ADC1_CHANNEL_0
#endif

#define ConcateChanel(pin) ADC1_GPIO ## pin ## _CHANNEL
#define AdcChannelFromPinNr(pin) ConcateChanel(pin)

// Only ADC1 (pin 32~39) is allowed 
#if PressureAdcPin > 39 || PressureAdcPin < 32
#error "Only GPIO32 - GPIO 39 can be used as ADC Pin"
#endif

#endif

int PressureMonitorClass::_readInternalAdc(void){
 
#ifdef ESP8266
    return system_adc_read();

/*    uint32_t adc_reading = 0;
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        adc_reading += system_adc_read();
        delay(10);
    }
    return (int) (adc_reading /NO_OF_SAMPLES);
*/
#endif

#if ESP32
    //Multisampling
    uint32_t adc_reading = 0;
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        adc_reading += adc1_get_raw(AdcChannelFromPinNr(PressureAdcPin));
        delay(10);
    }
    adc_reading /= NO_OF_SAMPLES;
    //Convert adc_reading to voltage in mV
    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, _adcCharacter);
    DBG_PRINTF("Raw: %d\tVoltage: %dmV, converted:%d, analogRead:%d\n", adc_reading, voltage,(int) ((float)voltage / 3900 * 2048 ),analogRead(PressureAdcPin));
    return (int) ((float)voltage / 3900 * 4095 );
#endif

}

#if PressureViaADS1115
int PressureMonitorClass::_readExternalAdc(void){
    if(_ads){
        uint16_t val= _ads->readADC_SingleEnded(ADS1115_Transducer_ADC_NO);
        DBG_PRINTF("ADS1115 return:%d\n",val);
        return val;
    } 
    return 0;
}
#endif

int PressureMonitorClass::currentAdcReading(void){

#if PressureViaADS1115
    if(_adcType == TransducerADC_ADS1115){
        return _readExternalAdc();
    }else 
#endif
    {
        return _readInternalAdc();
    }
}

void PressureMonitorClass::_readPressure(void){
    float reading =(float) currentAdcReading();

    float psi = (reading - _settings->fb) * _settings->fa;
    #if FilterPressureReading
    _currentPsi = _currentPsi + LowPassFilterParameter *(psi - _currentPsi);
    DBG_PRINTF("ADC:%d  PSIx10:%d currentx10:%d\n",(int)reading,(int)(psi*10),(int)(_currentPsi*10));
    #else
    _currentPsi = psi;
    DBG_PRINTF("ADC:%d  PSIx10:%d\n",(int)reading,(int)(psi*10));
    #endif

    #if SMART_DISPLAY
    smartDisplay.pressureData(psi);
    #endif
}

PressureMonitorClass::PressureMonitorClass(void){}

void PressureMonitorClass::_initInternalAdc(void){
#if ESP32
    if(_adcCharacter) return;
    //Characterize ADC at particular atten
    _adcCharacter =(esp_adc_cal_characteristics_t*) calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, _adcCharacter);
    //Check type of calibration value used to characterize ADC
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        DBG_PRINTF("ESP32 ADC CAL:eFuse Vref");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        DBG_PRINTF("ESP32 ADC CAL:Two Point");
    } else {
        DBG_PRINTF("ESP32 ADC CAL:Default");
    }
    pinMode(PressureAdcPin, INPUT);

    adc1_config_width(ADC_WIDTH_BIT_11); // 12 bits seems too jittery, 11 bits work just fine.
    adc1_config_channel_atten(AdcChannelFromPinNr(PressureAdcPin),ADC_ATTEN_DB_11); // 3.9v scale
#endif

}
#if PressureViaADS1115
void PressureMonitorClass::_initExternalAdc(void){
    if(!_ads){
        _ads = new Adafruit_ADS1115(ADS1115_ADDRESS);
        _ads->begin();
        _ads->setGain((adsGain_t)( _settings->adc_gain << 9 )); 
    }    
}
#endif

void PressureMonitorClass::_deinitInternalAdc(void){
#if ESP32
    if(!_adcCharacter) return;
    free(_adcCharacter);
    _adcCharacter=NULL;
#endif    
}

#if PressureViaADS1115
void PressureMonitorClass::_deinitExternalAdc(void){
    if(_ads){
        delete _ads;
        _ads =(Adafruit_ADS1115*) NULL;
    }
}
#endif


void PressureMonitorClass::begin(void){

    _currentPsi = 0;
    _settings=theSettings.pressureMonitorSettings();
    _ads =(Adafruit_ADS1115*) NULL;
    #if ESP32
    _adcCharacter = NULL;
    #endif

 #if ESP8266
    // move or NOT?
    wifi_set_sleep_type(NONE_SLEEP_T);
#endif

    #if PressureViaADS1115

    _adcType = _settings->adc_type; // to avoid race condition.

    if(_adcType == TransducerADC_ADS1115){
        _initExternalAdc();

    }else{
        _initInternalAdc();
    }
    #else
    _initInternalAdc();
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
    if(_settings->adc_type != _adcType){
        if(_settings->adc_type == TransducerADC_ADS1115){
            _initExternalAdc();
            _deinitInternalAdc();
        }else{
            _initInternalAdc();
            _deinitExternalAdc();
        }
        _adcType = _settings->adc_type; 
    }
    // gain might change
    if(_adcType == TransducerADC_ADS1115 ){
        _ads->setGain((adsGain_t)( _settings->adc_gain << 9 )); 
    }
    #endif
}

void PressureMonitorClass::loop(){
    if(_settings->mode == PMModeOff) return;
    uint32_t current= millis();
    if(_settings->mode == PMModeMonitor
            && (current - _time) > MinimumMonitorTime ){
        // read pressure, and report
        _time = current;
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