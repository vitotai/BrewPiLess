#ifndef TiltReceiver_H
#define TiltReceiver_H
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>


#define UUID_TILT_RED     "A495BB10C5B14B44B5121370F02D74DE"
#define UUID_TILT_GREEN   "A495BB20C5B14B44B5121370F02D74DE"
#define UUID_TILT_BLACK   "A495BB30C5B14B44B5121370F02D74DE"
#define UUID_TILT_PURPLE  "A495BB40C5B14B44B5121370F02D74DE"
#define UUID_TILT_ORANGE  "A495BB50C5B14B44B5121370F02D74DE"
#define UUID_TILT_BLUE    "A495BB60C5B14B44B5121370F02D74DE"
#define UUID_TILT_YELLOW  "A495BB70C5B14B44B5121370F02D74DE"
#define UUID_TILT_PINK    "A495BB80C5B14B44B5121370F02D74DE"


typedef enum _TiltColor{
    TiltColorRed=0,
    TiltColorGreen,
    TiltColorBlack,
    TiltColorPurple,
    TiltColorOrange,
    TiltColorBlue,
    TiltColorYellow,
    TiltColorPink,
    MaxTiltNumber
} TiltColor;

class TiltHydrometer {
public:
    TiltHydrometer(){}

    float gravity;
    float temperature;
    int rssi;
    TiltColor color;
    bool valid;
};

class TiltReceiver :public BLEAdvertisedDeviceCallbacks{
public:
    TiltReceiver():_scanning(false),_listening(false),_scanCompleteHandler(NULL){
        _clearData();
    }
    void begin(void);
    void loop(void);
    void scan(void (*scanCompleteHandler)(TiltHydrometer*)); // callback result.
    void listen(void);
    void stop(void);
    
    TiltHydrometer *getTiltBy(TiltColor color);
    // callbacks
    void scanComplete(void);
    void onResult(BLEAdvertisedDevice advertisedDevice);
protected:
    void _scan(void);
    void _clearData(void);
    
    bool _scanning;
    bool _listening;
    void (*_scanCompleteHandler)(TiltHydrometer*);
    BLEScan* _pBLEScan;
    TiltHydrometer _tilts[MaxTiltNumber];
};

extern TiltReceiver tiltReceiver;

#endif