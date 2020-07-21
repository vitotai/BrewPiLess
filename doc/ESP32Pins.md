# ESP32

ESP32 is future-proof, more powerful, and has more memory than ESP8266. However, it is "new" and hasn't been well tested. 
Please take the risk into consideration.

HTTPS is more complicated. BPL relies on AsyncWebServer library, which doesn's support SSL right now. 

HTTPS client is possible for ESP32, but the finger print isn't checked for simplicity. 

# Default PIN allocation

| GPIO   | IO | Functions       | Notes |  Preference | BPL |
| ------ |:-----:| :--------- | ---------- | ---- | ---- |
| 0 | IO | Touch, RTC, **boot** | Output PWM at boot | X | |
| 1 |  O  | UART TX | debug output at boot HIGH at boot | X | |
| 2 | IO | Touch, RTC, boot | connected to on-board LED | * | |
| 3 | I  | UART RX | HIGH at boot | X | |
| 4 | IO | Touch, RTC, boot |  | +  | |
| 5 | IO | VSPI, boot | Hight at boot | + | |
| 12 | IO | Touch, RTC, HSPI, JTAG, Boot | | * | |
| 13 | IO | Touch, RTC, HSPI, JTAG | | * | |
| 14 | IO | Touch, RTC, HSPI, JTAG | output PWM at boot | * | | 
| 15 | IO | Touch, RTC, HSPI, JTAG, boot |  output PWM at boot | * | | 
| 16 | IO | | | | actuator |
| 17 | IO | | | | actuator |
| 18 | IO | VSPI | | | (buzzer) |
| 19 | IO | VSPI | | | actuator |
| 21 | IO | I2C | | | I2C(SDA) |
| 22 | IO | I2C | | | I2C(SCL) |
| 23 | IO | VSPI | | | OneWire |
| 25 | IO | DAC2, RTC| | | Rotary Push |
| 26 | IO | DAC2, RTC| | | actuator |
| 27 | IO | Touch, RTC| | |  actuator |
| 32 | IO | Touch, ADC1, RTC | | | Rotary A |
| 33 | IO | Touch, ADC1, RTC | | | Rotary B |
| 34 | I | ADC1, RTC | no pull-up/down | | |
| 35 | I | ADC1, RTC | no pull-up/down | | |
| 36 | I | ADC1, RTC | no pull-up/down | | ADC |
| 39 | I | ADC1, RTC | no pull-up/down | | |

Preferences:

| Symbol   | Preferences |
| ------ | :----- |
| X | Avoid at any cost |
| * | Avoid |
| + | avoid if possible |



### References
* [ESP32 Pinout Reference: Which GPIO pins should you use?](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
* [ESP32 Boot Mode Selection](https://github.com/espressif/esptool/wiki/ESP32-Boot-Mode-Selection)
* [How to build image for ESP32](ESP32BuildInstructions.md)

