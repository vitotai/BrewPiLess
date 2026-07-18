#ifndef BleListener_H
#define BleListener_H
#include <Arduino.h>
#include <functional>
#include <list>
#include "Config.h"
#if ESP32
#include <NimBLEDevice.h>
#include <NimBLEAdvertisedDevice.h>
#include "NimBLEBeacon.h"


typedef std::function<void(NimBLEAdvertisedDevice*)> ScannedDevicdFoundFunc;

class BleHydrometerDevice{
public:
    BleHydrometerDevice(){}
    virtual BleHydrometerDevice* duplicate(void)=0;

    int rssi;   
    NimBLEAddress macAddress;
};

typedef std::function<void(BleHydrometerDevice*)> BleHydrometerDataHandler;
typedef std::function<void(std::vector<BleHydrometerDevice*>)> BleHydrometerScanResultHandler;

// Device specific interace.
// when a device found, the Device is passed to the BleDeviceListener
//  - Note: MAC address could have been used to identify the device instead of passing all data
//          However, for Tilt, ONLY UUID is used to identify the color.
class BleDeviceListener{
public:
    BleDeviceListener(void):_listening(false){}
    // start to listen to information periodically
    void startListen(void);
    void stopListen(void);
    // called when a device found
    virtual bool onDeviceFound(const NimBLEAdvertisedDevice*)=0;
protected:
    bool _listening;
};

// BleScanner handles BLEScan, including periodical scanning and scanning for devices.
class BleScanner :public BLEAdvertisedDeviceCallbacks{
public:
    BleScanner():_scanning(false),_commandScan(false),_enabled(false){       
    }
    // begin() and loop() are called in setup() and loop(), to handle periodical scanning
    void begin(void);
    void loop(void);

    // periodical scanning
    void addListener(BleDeviceListener* listener);
    void removeListener(BleDeviceListener* listener);

    // Active scan, for finding devices.
    // BLEScanResults scan(uint32_t scanTime);
    void scanForDevices(uint32_t scanTime,ScannedDevicdFoundFunc);

    void clearScanData(void);

    // callbacks of BLEAdvertisedDeviceCallbacks
    void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override;
    void onDiscovered(const NimBLEAdvertisedDevice* advertisedDevice) override;
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