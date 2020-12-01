#ifndef DHTSensor_H
#define DHTSensor_H

#include "DHT.h"


class DHTSensor{
public:
    DHTSensor(uint8_t pin,uint8_t type,int8_t cal):dht(pin,type){
        _pin = pin;
        _cal = cal;
        dht.begin();
    }
    uint8_t humidity(){
        float h=dht.readHumidity(true);
        DBG_PRINTF("DHxx H=%d, cal=%d/10\n",(int)(h *10),_cal);
        return (uint8_t) (h + _cal);
    }
    float temperature(bool isFarenheit){
        float temp=dht.readTemperature(isFarenheit);
        DBG_PRINTF("DHxx TEMP=%d/10\n",(int)(temp *10));
        return temp;
    }
    inline uint8_t pin(){ return _pin;}

private:
    DHT dht;
    int8_t _cal;
    uint8_t _pin;
};

#endif //DHTSensor_H