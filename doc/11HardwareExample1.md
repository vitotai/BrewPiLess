* NodeMcu 1.0 Board or D1 mini
* Relay Board, two way recommended.
* Two DS18B20 sensors
* 4.7K resistor.
* [_Optional_] 20×4 I2C LCD
* [_Optional_] TWO pushdown buttons
* Power supply to ESP8266. (I use a USB adapter) 1.5+A is recommended. 
* Power cable, sockets, project box, and etc.

![](image/BPL_simple.jpg?raw=true)

This the simplest setup which mixes 5v and 3.3v circuits. It might _**not**_ be a good practice, but it works. I have been using this for more than one year. Some additional notes:
* LCD and buttons are good to have. You will be able to operate it without a computer or phone.
* SSD1306 based 128x64 I2C OLED LCD can be used in place of the 20x4 LCD. They are small and cheap.
* One addition sensor can be used as room temperature sensors. (more than three is useless.)
* Some relay modules might not work under 3.3v. All of mine work, though.
* Some DS18B20 waterproof sensors, especiallly from Amazon, support only parasitic power mode. The sensors run in Parasitic mode are not stable, so BrewPi, hence BrewPiLess, **doesn’t support** parasitic power mode. The sensors can be detected by other sketches, but BrewPi(less) won't recognize them

