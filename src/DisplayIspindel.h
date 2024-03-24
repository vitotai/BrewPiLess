#pragma once
#include "SharedLcd.h"

class DisplayIspindel: public SharedLcdDisplay{
public:
    DisplayIspindel(){
        _gravity=0.0;
        _temperature=0.0;
        _battery=0.0;
        _tilt=0.0;
        _lastSeen=0;
        _wifiStrength=-120;
        _unit='C';
    }
    // sharing parts
    void onShow(){}
    void onHide(){}
    void redraw();
    void loop();
    // linked list 
    void updateInfo(float gravity,float temperature,char unit,float battery,float tilt,int8_t wifiStrength);
protected:
    OledDisplay *_display;
    uint32_t _lastUpdated;
    float _gravity;
    float _temperature;
    float _battery;
    float _tilt;
    time_t _lastSeen;
    int8_t _wifiStrength;
    char _unit;
    
    void _showSignalAt(int16_t x, int16_t y,int8_t strength);
    void _showFixedParts();
    void _showUpdates();

    void _showGravity();
    void _showTemperature();
    void _showBattery();
    void _showTilt();
    void _showLastSeen();
    void _showIp();
    void _drawFloatAt(int16_t x, int16_t y, float value, uint8_t space, uint8_t precision, uint16_t fontWidth,uint16_t fontHeight);
   
};

extern DisplayIspindel displayIspindel;