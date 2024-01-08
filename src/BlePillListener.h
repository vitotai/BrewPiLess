#ifndef BlePillListener_H
#define BlePillListener_H
#include <Arduino.h>
#include <functional>

#include "Config.h"

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


typedef std::function<void(PillHydrometerInfo*)> PillDataHandler;


class PillListener:public BleDeviceListener {
public:
    PillListener():_dataAvailableHandler(NULL){}

    void listen(uint8_t macAddr[6],PillDataHandler onData);
    // callbacks
    bool identifyDevice(NimBLEAdvertisedDevice*);
    void setMac(uint8_t mac[6]){ memcpy(_macAddr,mac,6); }
protected:
    PillDataHandler _dataAvailableHandler;
    uint8_t _macAddr[6];
    PillHydrometerInfo _info;
};

class PillScanner:public BleDeviceScanner {
public:
    PillScanner(){}
    // callbacks
    BleHydrometerDevice* checkDevice(NimBLEAdvertisedDevice*);
};

extern PillScanner pillScanner;
#endif