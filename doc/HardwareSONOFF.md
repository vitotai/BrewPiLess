BrewPiLess can run on [SONOFF Basic](https://www.itead.cc/wiki/Sonoff) and [SONOFF TH10/16](https://www.itead.cc/wiki/Sonoff_TH_10/16). You will need to solder the header pins for the first flashing and install temperature sensors. However, because of the small 1M byte flash memory, you will either sacrifice 
* OTA web update for log storage, or
* 500k log storage for OTA update
* LCD not available.

SONOFF dual is not supported, because the Relays are controlled by Serial instead of GPIO.


***
[Index](index.md)