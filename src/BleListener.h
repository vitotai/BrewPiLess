#ifndef BleListener_H
#define BleListener_H
#include <Arduino.h>
#include <functional>

#include "Config.h"
#define DefaultScanPeriod 5000
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
    BleDeviceListener(void){}
    // start to scan periodic
    void startListen(void);
    void stopListen(void);
    // 
    virtual bool identifyDevice(NimBLEAdvertisedDevice*)=0;
};


class BleDeviceScanner{
public:
    BleDeviceScanner(void);
    // request to scan now
    void scan(BleHydrometerScanResultHandler resultHandler); // callback result.

    virtual BleHydrometerDevice* checkDevice(NimBLEAdvertisedDevice*)=0;
    // call back for BLE scanner
    void scanComplete(NimBLEScanResults&);
protected:
    std::vector<BleHydrometerDevice*> _scannedDevices;
    BleHydrometerScanResultHandler _scanResultHandler;
    void _clearResult(void);
};

class BleListener :public BLEAdvertisedDeviceCallbacks{
public:
    BleListener():_scanning(false),_commandScan(false),_enabled(false),_scanPeriod(DefaultScanPeriod){
        _clearData();
    }
    // 
    void begin(void);
    void loop(void);
    void scanForDevices(BleDeviceScanner* scanner); 
    void startListen(BleDeviceListener* listener);
    void stopListen(void);
    // callbacks
    void scanComplete(NimBLEScanResults& result);
    virtual void onResult(NimBLEAdvertisedDevice* advertisedDevice);
protected:
    void _startScan(void);
    void _clearData(void);
    
    bool _scanning;
    bool _commandScan;
    bool _enabled;
    
    BleDeviceScanner*  _bleDeviceScanner;
    BleDeviceListener* _bleDeviceListener;

    BLEScan* _pBLEScan;
    uint32_t _scanPeriod;
    uint32_t _lastScanTime;
};

extern BleListener bleListener;


#endif