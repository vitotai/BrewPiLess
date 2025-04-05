#ifndef DHTSensor_H
#define DHTSensor_H

#include "DHT.h"
#include "EnvironmentSensor.h"

#define INVALID_HUMIDITY_VALUE 0xFF


class DHTSensor: public EnvironmentSensor{
public:
    DHTSensor(uint8_t pin,uint8_t type):dht(pin,type){
        _pin = pin;
        dht.begin();
    }
    uint8_t humidity(){
        float h=dht.readHumidity(true);
        if(h == NAN){
            DBG_PRINTF("Invalid value from sensor!\n");
            DBG_PRINTF("DHxx H=%d/10, cal=%d\n",(int)(h *10),_cal);
            return INVALID_HUMIDITY_VALUE;
        }
        return (uint8_t) (h + _hCal);
    }
    float readTemperature(){
        float temp=dht.readTemperature(false);
//        DBG_PRINTF("DHxx TEMP=%d/10\n",(int)(temp *10));
        return temp;
    }
    inline uint8_t pin(){ return _pin;}
    EnvironmentSensorType sensorType(){ return SensorType_DHT;};
    bool isConnected() { return true;}

private:
    DHT dht;
    int8_t _cal;
    uint8_t _pin;
};

#endif //DHTSensor_H