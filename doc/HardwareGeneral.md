Fortunately, 3.3V is regarded as HIGH in 5V logic, so the **output** of ESP8266 can be connected directly to the devices. Luckily, DS18B20 works under 3.3V. I just replace the Arduino Nano with the ESP8266, and it works. You should still be careful not to burn the ESP8266.

A lot of PINs are required, so ESP-12 or ESP-07 should be a better choice. NodeMcu development board is a simple solution for those who arn't good at soldering.

Search the BrewPi DIY on homebrewtalk.com forum to find out the hardware setup, but use these ESP8266 pins in place of Arduino pins.
This is default configuration, you can change it in `config.h`.

| ESP8266 GPIO   | NodeMcu Label | Connect to       |
| -------------- |:-------------:| :--------------------|
| GPIO16         | D0            | Buzzer			   |
| GPIO5          | D1            | I2C SCL             |
| GPIO4          | D2            | I2C SDA             |
| GPIO0          | D3            | INT from PCF8574 * Or UP Button  |
| GPIO2          | D4            | DOWN Button     |
| GPIO14         | D5            | Cooling Actuator*   |
| GPIO12         | D6            | Temperature Sensors |
| GPIO13         | D7            | Heating Actuator*   |
| GPIO15         | D8            |      			   |

*cooling/heating actuator PINs are configurable.

Note: The GPIOs of ESP8266 are not all **General Purpose**. Some of them has special functions, and might not be usable. For example, some PINs on my NodeMcu board don't work normally.
**!!Important !!** It is hightly recommended to pull up GPIO0 and GPIO2 while pull down GPIO15 so that the system will start up normally instead of staying in bootrom mode in case the system crashes. Updating the system configuration and firmware also results in restart of system, and sometimes this issue happens if the circuit isn't implementated. Check this url for detail information:
https://github.com/esp8266/Arduino/blob/master/doc/boards.rst#minimal-hardware-setup-for-bootloading-and-usage

**!! The rotary encoder is not supported by directly connecting it to ESP8266. That will prevents ESP8266 to boot up when the rotary encoder is at certain positions.!!** 

## I2C LCD Support
The default I2C LCD supported is 20x4 HD44780 LCD. BPL will scan the address of LCD, so don't worry about it.
I2C OLED LCD of 128x64 with SH1106 and SSD1306 controllers are also supported. 

To use SSD1306, add `-DOLED_LCD=true` to `build_flags`. To use SH1106, add `-DOLED_LCD=true -DBREWPI_OLED_SH1106=true`

There are one additional status bar or line when using OLED LCDs. The status bar will show IP address at the first 10 minutes and time after that.

## Input options
### 2 Buttons - _Default configuration_
Two simple pushdown buttons can be used to replace a rotary encoder. Connect UP button to D3 and DOWN button to D4, the default setting.

The pin allocation is defined in `Config.h`:

`#define UpButtonPin NODEMCU_PIN_D3`

`#define DownButtonPin NODEMCU_PIN_D4`

You can change the pin allocation to your configuration. **Just don't use D8**, which is pulled low to enable normal booting. You might need to use pull-up register if you choose D0, which doesn't support internal pull-up.

### Rotary Encoder _Not Recommended_
Due to the special usage at bootup of GPIO0,2,15(D3,D4,D8), they can't be used as inputs for rotary encoder. One of the solution is buy an IO Expander. 
Currently, PCF8574 is supported if you really need the rotary encoder. You will have to change the compile options in Config.h to enable this feature.
The Rotary Encoder doesn't work with OLED LCD.

### Wake-up button _deprecated_
No longer available after v2.7. Two-button configuration will work on the hardware setup for Wake-up button and pressing the button will "wake up" BPL also.


### 2 Buttons via PCF8574 _Not Recommended_
Use this option to share the same hardware with BrewManiacEx. UP is connect to P1 while DOWN is connect to P0. (Enter is at P2, and Start is at P3. BrewPiLess uses only two buttons.)
