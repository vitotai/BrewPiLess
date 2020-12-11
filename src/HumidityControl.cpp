#include "HumidityControl.h"

#if EnableDHTSensorSupport
DHTSensor* HumidityControl::dhtSensor=NULL;
extern ValueActuator defaultActuator;
Actuator* HumidityControl::humidifier= &defaultActuator;;
Actuator* HumidityControl::dehumidifier= &defaultActuator;;

HumidityControl humidityControl;
#endif