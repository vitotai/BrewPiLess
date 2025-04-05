#ifndef BleTiltListener_H
#define BleTiltListener_H
#include <Arduino.h>
#include <functional>

#include "Config.h"
#if SupportTiltHydrometer

#include "BleListener.h"


typedef enum _TiltColor{
    TiltColorRed=0,
    TiltColorGreen,
    TiltColorBlack,
    TiltColorPurple,
    TiltColorOrange,
    TiltColorBlue,
    TiltColorYellow,
    TiltColorPink,
    MaxTiltNumber,
    TiltColorInvalid
} TiltColor;

class TiltHydrometerInfo: public BleHydrometerDevice {
public:
    TiltHydrometerInfo(){}
    BleHydrometerDevice* duplicate(void){
        TiltHydrometerInfo* info= new TiltHydrometerInfo();

        info->gravity = this->gravity;
        info->temperature = this->temperature;
        info->rssi = this->rssi;
        info->color = this->color;
        info->macAddress = this->macAddress;
        return info;
    }
    uint16_t gravity;
    uint16_t temperature;
    int rssi;
    TiltColor color;
};


typedef std::function<void(TiltHydrometerInfo*)> TiltDataHandler;


class TiltListener:public BleDeviceListener {
public:
    TiltListener():_dataAvailableHandler(NULL),_targetColor(TiltColorInvalid){}

    void listen(TiltColor color,TiltDataHandler onData);
    // callbacks
    bool onDeviceFound(NimBLEAdvertisedDevice*);
    void setColor(TiltColor color){ _targetColor = color; }
protected:
    TiltDataHandler _dataAvailableHandler;
    TiltHydrometerInfo _tiltInfo;
    TiltColor _targetColor;
};

class TiltScanner:public BleDeviceListener {
public:
    TiltScanner(){}
    void scan(TiltDataHandler onData);
    void stopScan(void);
    // callbacks
    bool onDeviceFound(NimBLEAdvertisedDevice*);
protected:
    TiltDataHandler _dataAvailableHandler;
};

extern TiltScanner tiltScanner;
#endif

#endif