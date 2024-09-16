#ifndef BleSensorListener_H
#define BleSensorListener_H
#include <Arduino.h>
#include <functional>

#include "Config.h"
#include "BleListener.h"
#include "EnvironmentSensor.h"
#include "TempSensorBasic.h"

#define MaximumReportPeriod 60000
#define ScanDeviceTime 6

typedef enum _BleSensorType{
BleSensorTypeBTHome=0,
BleSensorTypeAtc=1,
BleSensorTypePvvx=2
} BleSensorType;

typedef std::function<void(uint8_t type,const uint8_t*,float,uint8_t)> BTHomeDevicdFoundFunc;

class BleSensorListener: public BleDeviceListener {
public:
    BleSensorListener(const uint8_t mac[6],uint8_t format){memcpy(_macAddress,mac,6); _broadcastFormat=format;}
    bool sameDevice(const uint8_t mac[6]);
    uint8_t *macAddress(void){ return _macAddress; }
    void begin(void);
    void stop(void);
    bool onDeviceFound(NimBLEAdvertisedDevice*);
    bool isConnected();

    float temperature(void){ return _temperature;}
    uint8_t humidity(void){ return _humidity;}
    uint8_t broadcastFormat(){ return _broadcastFormat;}
    
    static int scanForDevice(BTHomeDevicdFoundFunc foundCb);
    static BleSensorListener* findBleSensor(const uint8_t mac[6]);
    static BleSensorListener* getBleSensor(const uint8_t mac[6],uint8_t format);
    static void releaseSensor(BleSensorListener* sensor);
    static std::list<BleSensorListener*> getAll(){ return allSensors;}
protected:
    uint32_t _lastUpdate;
    uint8_t _macAddress[6];
    uint8_t _count;
    float _temperature;
    uint8_t _humidity;
    uint8_t _broadcastFormat;

    bool _getData(NimBLEAdvertisedDevice*);
    static std::list<BleSensorListener*> allSensors;
};

class BleHumiditySensor :public EnvironmentSensor{
public:
    BleHumiditySensor(const uint8_t mac[6],uint8_t format);
    ~BleHumiditySensor(void);

    EnvironmentSensorType sensorType(){ return SensorType_BleSensor;}
    bool isConnected(){return _listener->isConnected();}
    unsigned char humidity();
    float  readTemperature (){ return _listener->temperature();}
protected:
    BleSensorListener* _listener;
};

class BleThermometer : public BasicTempSensor{
public:
    BleThermometer(const uint8_t mac[6],uint8_t format);
    ~BleThermometer(void);
    bool isConnected(){return _listener->isConnected();}
    bool init();
    temperature read();
    void setTemperatureCalibration(fixed4_4 cal);
protected:
    BleSensorListener* _listener;
    temperature _tempOffset;
};

#endif
