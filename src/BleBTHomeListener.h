#ifndef BleBTHomeListener_H
#define BleBTHomeListener_H
#include <Arduino.h>
#include <functional>

#include "Config.h"
#include "BleListener.h"
#include "EnvironmentSensor.h"

#define MaximumReportPeriod 60000

class BTHomeSensorDefinition{
public:
    BTHomeSensorDefinition(){};
    std::string name;
    uint8_t address[6];
    float   temperature;
    uint8_t humidity;
};
typedef std::function<void(const uint8_t*)> BTHomeDevicdFoundFunc;

class BTHomeEnvironmentSensor :public BleDeviceListener, EnvironmentSensor {
public:
    BTHomeEnvironmentSensor(uint8_t mac[6]){memcpy(_macAddress,mac,6);}
    // callbacks
    
    void begin(void);
    void stop(void);
    bool onDeviceFound(NimBLEAdvertisedDevice*);

    virtual bool isConnected();
    uint8_t humidity();
    float  readTemperature();

    EnvironmentSensorType sensorType(){ return SensorType_BTHome;}
    void setHumidityCalibration(int8_t cal){
        _hCal =cal;
    }

    static int scanForDevice(BTHomeDevicdFoundFunc foundCb);
protected:
    uint32_t _lastUpdate;
    uint8_t _macAddress[6];
    float _temperature;
    uint8_t _humidity;
    int8_t _hCal;

    bool _getData(NimBLEAdvertisedDevice*);
};

#endif
