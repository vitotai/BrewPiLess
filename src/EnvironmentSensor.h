#ifndef EnvironmentSensor_H
#define EnvironmentSensor_H

typedef enum _EnvironmentSensorType{
SensorType_None,
SensorType_BME280,
SensorType_DHT
} EnvironmentSensorType;

class EnvironmentSensor
{
public:
    virtual ~EnvironmentSensor(){}
    virtual bool isConnected(){return false;}
    virtual unsigned char humidity(){return 0xFF;}
    virtual float  readTemperature(){return -100.0;}
    virtual EnvironmentSensorType sensorType(){ return SensorType_None;}
    void setHumidityCalibration(int8_t cal){
        _hCal =cal;
    }
protected:
    int8_t _hCal;
};


extern EnvironmentSensor nullEnvironmentSensor;
#endif