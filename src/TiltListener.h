#ifndef TiltListener_H
#define TiltListener_H
#include <Arduino.h>
#include <functional>

#include "Config.h"
#if SupportTiltHydrometer
#if 0
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#endif

#include <NimBLEDevice.h>
#include <NimBLEAdvertisedDevice.h>
#include "NimBLEEddystoneURL.h"
#include "NimBLEEddystoneTLM.h"
#include "NimBLEBeacon.h"


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

typedef struct _TiltHydrometerInfo {
    uint16_t gravity;
    uint16_t temperature;
    int rssi;
    TiltColor color;
}TiltHydrometerInfo;


typedef std::function<void(TiltHydrometerInfo&)> TiltDataHandler;


class TiltListener :public BLEAdvertisedDeviceCallbacks{
public:
    TiltListener():_scanning(false),_commandScan(false),_scanCompleteHandler(NULL),_dataAvailableHandler(NULL),_targetColor(TiltColorInvalid){
        _clearData();
    }
    void begin(void);
    void loop(void);
    void scan(void (*scanCompleteHandler)(int,TiltHydrometerInfo*)); // callback result.

    void listen(TiltColor color,TiltDataHandler onData);
    void stopListen(void);
    
    TiltHydrometerInfo *getTiltBy(TiltColor color);
    // callbacks
    void scanComplete(NimBLEScanResults& result);
    virtual void onResult(NimBLEAdvertisedDevice* advertisedDevice);
protected:
    void _scan(void);
    void _clearData(void);
    bool _getTiltInfo(NimBLEAdvertisedDevice* advertisedDevice,TiltHydrometerInfo& tiltInfo);
    
    bool _scanning;
    bool _commandScan;
    void (*_scanCompleteHandler)(int,TiltHydrometerInfo*);
    TiltDataHandler _dataAvailableHandler;

    BLEScan* _pBLEScan;

    TiltColor _targetColor;
    uint32_t _lastScanTime;
};

extern TiltListener tiltListener;

#endif

#endif