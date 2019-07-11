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

## spainish, Portuguese-br, slovak
 * Default configuration in languages other than English

## classic
 * Default configuration
 * Classic web interface
 * flash layout 4m1m(1M program/3M file space)


## sonoff
 * For SONOFF.
 * Sensor on D5, Coolling on D6 
 * No rotary encoder. No LCD
 * OTA update **NOT** supported.
 * 512K SPIFFS for logging.
 * classic ui

## sonoffota
 * Same as sonsoff, but
 * 64K SPIFFS for configurations
 * OTA update supported
 * classic ui
 * NO longger supported after v3.6

## Newer SONOFF/esp8285
 * flash mode is set to DOUT
 * classic ui

## oled
 * Standard options
 * SSD1306 OLED I2C 128x64 instead of 20x4 LCD
 * new ui

## thorrax
 * Thorrax's board, Cooling and heating PIN on D5, D0
 * new ui
