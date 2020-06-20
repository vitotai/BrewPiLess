BrewPiLess can run on [Electrodragon SDPT board](https://www.electrodragon.com/product/wifi-iot-relay-board-spdt-based-esp8266/). It uses ESP-12F, so you can deploy full nodemcuv2. Schematic is [available here](https://www.electrodragon.com/w/ESP_Relay_Board_SPDT).

For flashing, hold down btn2 before plugging in. 


| ESP8266 GPIO   | NodeMcu Label | Connect to       |
| -------------- |:-------------:| :--------------------|
| GPIO5          | D1            | I2C SCL             |
| GPIO4          | D2            | I2C SDA             |
| GPIO0          | D3            | UP Button (BTN2) or flash  |
| GPIO2          | D4            | DOWN Button (BTN1)  |
| GPIO14         | D5            | Temperature Sensors *   |
| GPIO12         | D6            | Cooling Actuator |
| GPIO13         | D7            | Heating Actuator   |
| GPIO15         | D8            | Available   			   |

*IO14 is marked as DHT on the board. It already has 10k pull-up resistor soldered correctly. 

When configuring the devices, D6 and D7 should be configured as "Not inverted".

***
[Index](index.md)