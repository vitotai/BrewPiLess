#ifndef BleBTHomeListener_H
#define BleBTHomeListener_H
#include <Arduino.h>
#include <functional>

#include "Config.h"
#include "BleListener.h"
#include "EnvironmentSensor.h"
#include "TempSensorBasic.h"

#define MaximumReportPeriod 60000
#define ScanDeviceTime 6

typedef std::function<void(const uint8_t*,float,uint8_t)> BTHomeDevicdFoundFunc;

class BTHomeEnvironmentSensor :public EnvironmentSensor, public BasicTempSensor, public BleDeviceListener {
public:
    BTHomeEnvironmentSensor(const uint8_t mac[6]):_hCal(0){memcpy(_macAddress,mac,6);}
    bool sameDevice(const uint8_t mac[6]);
    uint8_t *macAddress(void){ return _macAddress; }
    void begin(void);
    void stop(void);
    bool onDeviceFound(NimBLEAdvertisedDevice*);

    bool isConnected() override;
    unsigned char humidity() override;
    float  readTemperature () override;

    EnvironmentSensorType sensorType(){ return SensorType_BTHome;}
    void setHumidityCalibration(int8_t cal){
        _hCal =cal;
    }

    //BasicTempSensor
    bool init();
    temperature read();
    void setTemperatureCalibration(fixed4_4 cal);

    static int scanForDevice(BTHomeDevicdFoundFunc foundCb);
    static BTHomeEnvironmentSensor* findBTHomeSensor(const uint8_t mac[6]);
    static BTHomeEnvironmentSensor* getBTHomeSensor(const uint8_t mac[6]);
    static void releaseSensor(BTHomeEnvironmentSensor* sensor);
    static std::list<BTHomeEnvironmentSensor*> getAll(){ return allSensors;}
protected:
    uint32_t _lastUpdate;
    uint8_t _macAddress[6];
    uint8_t _count;
    float _temperature;
    uint8_t _humidity;
    int8_t _hCal;
    temperature _tempOffset;

    bool _getData(NimBLEAdvertisedDevice*);

    static std::list<BTHomeEnvironmentSensor*> allSensors;
};

#endif
