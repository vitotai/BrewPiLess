#ifndef BleRaptThermometer_H
#define BleRaptThermometer_H
#include <Arduino.h>
#include <functional>

#include "Config.h"
#include "TempSensorBasic.h"
#include "BleListener.h"

#define RAPT_THERMOMETER_TIMEOUT 60000
// multiple inheritance seems to screw the vtable. 
// better to keep it simple.
class BleRaptThermometerLisener:public BleDeviceListener{
    public:
        BleRaptThermometerLisener(uint8_t mac[6]);
        virtual ~BleRaptThermometerLisener();
        // callbacks for liseners
        bool onDeviceFound(const NimBLEAdvertisedDevice*);

        bool isReporting(void);
        double latestTemperature(void){ return _temp;}
    protected:
        uint8_t _macAddress[6];
        uint32_t _lastUpdate;
        double _temp;
        int   _battery;
        int   _rssi;
};

class BleRaptThermometer:public BasicTempSensor {
    public:
        BleRaptThermometer(uint8_t mac[6],fixed4_4 cal);
        
        // BasicTempSensor
        virtual bool isConnected(void);
        virtual bool init();
        virtual temperature read();
        
        static bool isRaptThermemoter(NimBLEAdvertisedDevice*);    
    protected:
        BleRaptThermometerLisener _bleListener;
        temperature _tempOffset;
};

#endif