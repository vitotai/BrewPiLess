#ifndef BleTiltListener_H
#define BleTiltListener_H
#include <Arduino.h>
#include <functional>

#include "Config.h"
#if SupportPillHydrometer

#include "BleListener.h"

class PillHydrometerInfo: public BleHydrometerDevice{
public:
    PillHydrometerInfo(){}
    BleHydrometerDevice* duplicate(void){
        PillHydrometerInfo* info= new PillHydrometerInfo();

        info->gravity = this->gravity;
        info->temperature = this->temperature;
        info->rssi = this->rssi;
        info->temperature = this->temperature;
        info->accX = this->accX;
        info->accY = this->accY;
        info->accZ = this->accZ;
        info->battery = this->battery;
        memcpy(info->mac,this->mac,6);
        return info;
    }
    float gravity;
    float temperature;
    uint16_t accX;
    uint16_t accY;
    uint16_t accZ;
    float battery;
    int rssi;    
    uint8_t mac[6];
};


typedef std::function<void(PillHydrometerInfo&)> PillDataHandler;


class PillListener:public BleDeviceListener {
public:
    PillListener():_dataAvailableHandler(NULL){}

    void listen(uint8_t macAddr[6],PillDataHandler onData);
    // callbacks
    bool identifyDevice(NimBLEAdvertisedDevice*);
protected:
    PillDataHandler _dataAvailableHandler;
    uint8_t _macAddr[6];
    PillHydrometerInfo _info;
};

class PillScanner:public BleDeviceScanner {
public:
    PillScanner(){}
    // callbacks
    BleHydrometerDevice* getDevice(NimBLEAdvertisedDevice*);
protected:
    PillHydrometerInfo _info;
};



#endif

#endif