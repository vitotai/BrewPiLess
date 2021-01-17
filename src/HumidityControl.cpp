#include "HumidityControl.h"

#if EnableHumidityControlSupport
EnvironmentSensor nullEnvironmentSensor;


EnvironmentSensor* HumidityControl::chamberSensor= &nullEnvironmentSensor;
EnvironmentSensor* HumidityControl::roomSensor= &nullEnvironmentSensor;

extern ValueActuator defaultActuator;
Actuator* HumidityControl::humidifier= &defaultActuator;;
Actuator* HumidityControl::dehumidifier= &defaultActuator;;

HumidityControl humidityControl;
#endif