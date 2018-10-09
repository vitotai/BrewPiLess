# **Note: ALL BPL settings will be gone after upgrading to v3.0**

# Prebuilt Image

NOTE: If you are upgrading from version prior to v2.3 without glycol option, you will need to run "Device Setup" again. Erase the EEPROM, setup the sensors, and the PINs.


NOTE: The images with **newui** use different flash layout(4m2m) which has 2m file space. They are not "compatible" with those(4m) have 3m file space.  
 * Online update might not work on changing between those two layout. Flashing by USB is recommended.
 * File system will be reformted after changing. All settings will be gone

Those images are prebuilt:
## Simple Configuration
 * The Pins are defined as the main page.
 * Two buttons as input
 * 20x4 LCD at any address

## spainish
 * Default configuration in Spanish

## newui
 * Default configuration
 * Tom's frontend instead of classic one
 * framework 1.8.0
 * flash layout 4m2m(2M program/2M file space)

## newui.spanish
 * same as newui, in Spanish

## ioexpander 
 * Default configuration as BrewShield
 * Buttons are connected to PCF8574.
 * other configurations are the same as default.

## sonoff
 * For SONOFF.
 * Sensor on D5, Coolling on D6 
 * No rotary encoder. No LCD
 * OTA update **NOT** supported.
 * 512K SPIFFS for logging.
 * **Not compatible to NEW SONOFF of ESP8285**

## sonoffota
 * Same as sonsoff, but
 * 64K SPIFFS for configurations
 * OTA update supported
 * **Not compatible with NEW SONOFF of ESP8285**


## oled
 * Standard options
 * SSD1306 OLED I2C 128x64 instead of 20x4 LCD

## thorrax
 * Thorrax's board, Cooling and heating PIN on D5, D0
