#ifndef BleListener_H
#define BleListener_H
#include <Arduino.h>
#include <functional>

#include "Config.h"
#define DefaultScanPeriod 10000
#include <NimBLEDevice.h>
#include <NimBLEAdvertisedDevice.h>
#include "NimBLEEddystoneURL.h"
#include "NimBLEEddystoneTLM.h"
#include "NimBLEBeacon.h"

class BleHydrometerDevice{
public:
    BleHydrometerDevice(){}
    virtual BleHydrometerDevice* duplicate(void)=0;
};

class BleDeviceScanner{
public:
    BleDeviceScanner(void);
    // request to scan now
    void requestScan(void);
    // start to scan periodic
    void startListen(void);
    void stopListen(void);
    // 
    virtual BleHydrometerDevice* identifyDevice(NimBLEAdvertisedDevice*)=0;
    virtual void scanDone(std::vector<BleHydrometerDevice*> foundDevices)=0;

    // call back for BLE scanner
    void scanComplete(NimBLEScanResults&);

protected:
    std::vector<BleHydrometerDevice*> _scannedDevices;
    bool _scanAll;

    void clearResult(void);
};

class BleListener :public BLEAdvertisedDeviceCallbacks{
public:
    BleListener():_scanning(false),_commandScan(false),_enabled(false),_bleDeviceScanner(NULL),_scanPeriod(DefaultScanPeriod){
        _clearData();
    }
    // 
    void begin(void);
    void loop(void);
    void scanNow(void); 
    void startListen(void);
    void stopListen(void);
    void setBleDeviceScanner(BleDeviceScanner* deviceScanner){
        _bleDeviceScanner = deviceScanner;
    }
    // callbacks
    void scanComplete(NimBLEScanResults& result);
    virtual void onResult(NimBLEAdvertisedDevice* advertisedDevice);
protected:
    void _startScan(void);
    void _clearData(void);
    
    bool _scanning;
    bool _commandScan;
    bool _enabled;
    
    BleDeviceScanner *_bleDeviceScanner;
    
    BLEScan* _pBLEScan;
    uint32_t _scanPeriod;
    uint32_t _lastScanTime;    
};

extern BleListener bleListener;


#endif