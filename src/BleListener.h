#ifndef BleListener_H
#define BleListener_H
#include <Arduino.h>
#include <functional>

#include "Config.h"
#if ESP32
#include <NimBLEDevice.h>
#include <NimBLEAdvertisedDevice.h>
#include "NimBLEEddystoneURL.h"
#include "NimBLEEddystoneTLM.h"
#include "NimBLEBeacon.h"



class BleHydrometerDevice{
public:
    BleHydrometerDevice(){}
    virtual BleHydrometerDevice* duplicate(void)=0;

    int rssi;   
    NimBLEAddress macAddress;
};

typedef std::function<void(BleHydrometerDevice*)> BleHydrometerDataHandler;
typedef std::function<void(std::vector<BleHydrometerDevice*>)> BleHydrometerScanResultHandler;

class BleDeviceListener{
public:
    BleDeviceListener(void):_listening(false){}
    // start to scan periodic
    void startListen(void);
    void stopListen(void);
    // 
    virtual bool onDeviceFound(NimBLEAdvertisedDevice*)=0;
protected:
    bool _listening;
};

class BleScanner :public BLEAdvertisedDeviceCallbacks{
public:
    BleScanner():_scanning(false),_commandScan(false),_enabled(false){       
    }
    // 
    void begin(void);
    void loop(void);
    void startListen(BleDeviceListener* listener);
    void stopListen(BleDeviceListener* listener);
    BLEScanResults scan(uint32_t scanTime);
    void clearScanData(void);
    // callbacks
    virtual void onResult(NimBLEAdvertisedDevice* advertisedDevice);
protected:
    void _startScan(void);

    void _setupAsyncScan(void);

    bool _scanning;
    bool _commandScan;
    bool _enabled;
    
    std::list<BleDeviceListener*> _bleDeviceListeners;

    BLEScan* _pBLEScan;
    uint32_t _retryCount;
    uint32_t _lastScanTime;
};

extern BleScanner bleScanner;


#endif
#endif