#ifndef Bme280Sensor_H
#define Bme280Sensor_H
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "EnvironmentSensor.h"


class Bme280Sensor:public EnvironmentSensor{

public:
    Bme280Sensor(uint8_t address){
       _connected=_bme.begin(address);
       _addr = address;
    }
    
    uint8_t humidity(){
        return (uint8_t) _bme.readHumidity();
    }
    float readTemperature(){
        return _bme.readTemperature();
    }
    EnvironmentSensorType sensorType(){ return SensorType_BME280;};
    bool isConnected(){
        return _connected;
    }
private:
    Adafruit_BME280 _bme;
    bool _connected;
    uint8_t _addr;
};


#endif