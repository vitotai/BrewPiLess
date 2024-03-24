#ifndef Bme280Sensor_H
#define Bme280Sensor_H
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "EnvironmentSensor.h"


class Bme280Sensor:public EnvironmentSensor{

public:
    Bme280Sensor(uint8_t address){
       _connected=_bme.begin(address,&Wire);
       DBG_PRINTF("BME280 begin(0x%x):%d sensor ID:0x%x\n",address,_connected,_bme.sensorID());
       _addr = address;
    }
    
    uint8_t humidity(){
        uint8_t ret=(uint8_t) _bme.readHumidity();
        //DBG_PRINTF("BME280 Hum:%d \n",ret);
        return ret;
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