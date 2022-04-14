# **Note: ALL BPL settings will be gone after upgrading to v3.0**

# Prebuilt Image

NOTE: If you are upgrading from version prior to v2.3 without glycol option, you will need to run "Device Setup" again. Erase the EEPROM, setup the sensors, and the PINs.


NOTE: The images with **newui** use different flash layout(4m2m) which has 2m file space. They are not "compatible" with those(4m) have 3m file space.  
 * Online update might not work on changing between those two layout. Flashing by USB is recommended.
 * File system will be reformted after changing. All settings will be gone

NOTE: Due to the size increase of latest framework, all images will use 2m file space after version 3.6. The last version for 4m1m and SONOFF is v3.5.1. 

Those images are prebuilt:
## Default Configuration
 * The Pins are defined as the main page.
 * Two buttons as input
 * 20x4 LCD at any address
 * Tom's frontend instead of classic one
 * framework 2.2.0
 * flash layout 4m2m(2M program/2M file space)
 * LITTLEFS for ESP8266, SPIFFS for ESP32

## spainish, Portuguese-br, slovak, italian, norwegian
 * Default configuration in languages other than English

## classic
 * Deprecated.


## sonoff
 * For SONOFF.
 * Sensor on D5, Coolling on D6 
 * OTA update **NOT** supported.
 * 64K SPIFFS for logging.


## Newer SONOFF/esp8285
 * flash mode is set to DOUT

## sonoff-am2301
 * same as esp8285, but D5 is released to be used with AM2301. Manual setup is required, though. 


## oled
 * Standard options
 * SSD1306 OLED I2C 128x64 instead of 20x4 LCD

## thorrax
 * Thorrax's board, Cooling and heating PIN on D5, D0

## esp32 & esp32.oled
 * ESP32 w/ 20x4 LCD & OLED LCD

## littlefs for esp32
 * LittleFS instead of SPIFFS

## ispindel
 * Dedicated display of iSpindel infomration. For OLED only.