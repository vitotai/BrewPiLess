#ifndef BlePillListener_H
#define BlePillListener_H
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
        info->macAddress=this->macAddress;
        info->rssi = this->rssi;
        return info;
    }
    float gravity;
    float temperature;
    int16_t accX;
    int16_t accY;
    int16_t accZ;
    float battery;
};


typedef std::function<void(PillHydrometerInfo*)> PillDataHandler;


class PillListener:public BleDeviceListener {
public:
    PillListener(uint8_t mac[6]):_dataAvailableHandler(NULL){memcpy(_macAddress,mac,6);}

    void listen(PillDataHandler onData);
    // callbacks
    bool onDeviceFound(NimBLEAdvertisedDevice*);
    void setMac(uint8_t mac[6]){
            //NimBLEAddress nmac(mac);
            //_mac = nmac;
            memcpy(_macAddress,mac,6);
        }
protected:
    PillDataHandler _dataAvailableHandler;
    PillHydrometerInfo _info;
    //NimBLEAddress _mac;
    //onstructor NimBleAddress(uint8_t[6]) will reverse the order of the byte
    uint8_t _macAddress[6];
};

class PillScanner:public BleDeviceListener {
public:
    PillScanner(){}
    // callbacks
    
    void scan(PillDataHandler onData);
    void stopScan(void);
    bool onDeviceFound(NimBLEAdvertisedDevice*);
protected:
    PillDataHandler _dataAvailableHandler;
};

extern PillScanner pillScanner;
#endif
#endif