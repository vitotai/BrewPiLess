#ifndef EnvironmentSensor_H
#define EnvironmentSensor_H

typedef enum _EnvironmentSensorType{
SensorType_None,
SensorType_BME280,
SensorType_DHT,
SensorType_BTHome
} EnvironmentSensorType;

class EnvironmentSensor
{
public:
    virtual ~EnvironmentSensor(){}
    virtual bool isConnected()=0;
    virtual unsigned char humidity()=0;
    virtual float  readTemperature()=0;
    virtual EnvironmentSensorType sensorType()=0;
    void setHumidityCalibration(int8_t cal){
        _hCal =cal;
    }
protected:
    int8_t _hCal;
};

class NullEnvironmentSensor:public EnvironmentSensor
{
public:
    ~NullEnvironmentSensor(){}
    bool isConnected(){return false;}
    unsigned char humidity(){return 0xFF;}
    float  readTemperature(){return -100.0;}
    EnvironmentSensorType sensorType(){ return SensorType_None;}
};


extern NullEnvironmentSensor nullEnvironmentSensor;
#endif