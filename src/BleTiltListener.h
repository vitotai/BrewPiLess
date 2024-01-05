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
        return info;
    }
    uint16_t gravity;
    uint16_t temperature;
    int rssi;
    TiltColor color;
};


typedef std::function<void(TiltHydrometerInfo&)> TiltDataHandler;


class TiltListener:public BleDeviceScanner {
public:
    TiltListener():_scanCompleteHandler(NULL),_dataAvailableHandler(NULL),_targetColor(TiltColorInvalid){}

    void scan(void (*scanCompleteHandler)(std::vector<BleHydrometerDevice*>)); // callback result.
    void listen(TiltColor color,TiltDataHandler onData);
    // callbacks
    BleHydrometerDevice* identifyDevice(NimBLEAdvertisedDevice*);
    void scanDone(std::vector<BleHydrometerDevice*> foundDevices);
protected:
    bool _parseTiltInfoFromAdvertise(NimBLEAdvertisedDevice* advertisedDevice,TiltHydrometerInfo& tiltInfo);
    
    void (*_scanCompleteHandler)(std::vector<BleHydrometerDevice*>);
    TiltDataHandler _dataAvailableHandler;
    TiltHydrometerInfo _tiltInfo;
    TiltColor _targetColor;
};

extern TiltListener tiltListener;

#endif

#endif