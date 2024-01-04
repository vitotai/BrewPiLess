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

typedef std::function<void(NimBLEAdvertisedDevice*)> ScannedDeviceHandler;
typedef std::function<void(NimBLEScanResults&)> ScannedResultHandler;


class BleListener :public BLEAdvertisedDeviceCallbacks{
public:
    BleListener():_scanning(false),_commandScan(false),_enabled(false),_deviceFoundCb(NULL),_scanResultCb(NULL),_scanPeriod(DefaultScanPeriod){
        _clearData();
    }
    // 
    void begin(void);
    void loop(void);
    void scanNow(ScannedResultHandler scanResultCb); 

    void listen(ScannedDeviceHandler scannedDeviceCb);
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
    
    ScannedDeviceHandler _deviceFoundCb;
    ScannedResultHandler _scanResultCb;
    
    BLEScan* _pBLEScan;
    uint32_t _scanPeriod;
    uint32_t _lastScanTime;    
};

extern BleListener bleListener;


#endif