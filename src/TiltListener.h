#ifndef TiltListener_H
#define TiltListener_H
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

typedef struct _TiltHydrometerInfo {
    uint16_t gravity;
    uint16_t temperature;
    int rssi;
    TiltColor color;
}TiltHydrometerInfo;


typedef std::function<void(TiltHydrometerInfo&)> TiltDataHandler;


class TiltListener {
public:
    TiltListener():_scanCompleteHandler(NULL),_dataAvailableHandler(NULL),_targetColor(TiltColorInvalid),_availTilts(NULL){}

    void scan(void (*scanCompleteHandler)(int,TiltHydrometerInfo*)); // callback result.

    void listen(TiltColor color,TiltDataHandler onData);
    void stopListen(void);
    // callbacks
    void scanComplete(NimBLEScanResults& result);
    void deviceFound(NimBLEAdvertisedDevice* device);
protected:
    bool _parseTiltInfoFromAdvertise(NimBLEAdvertisedDevice* advertisedDevice,TiltHydrometerInfo& tiltInfo);
    
    void (*_scanCompleteHandler)(int,TiltHydrometerInfo*);
    TiltDataHandler _dataAvailableHandler;

    TiltColor _targetColor;
    TiltHydrometerInfo* _availTilts;
    uint8_t  _numTiltFound;
};

extern TiltListener tiltListener;

#endif

#endif