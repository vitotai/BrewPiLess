# **Note: ALL BPL settings will be gone after upgrading to v3.0**

# Prebuilt Image

NOTE: If you are upgrading from version prior to v2.3 without glycol option, you will need to run "Device Setup" again. Erase the EEPROM, setup the sensors, and the PINs.

Those images are prebuilt:
## Simple Configuration
 * The Pins are defined as the main page.
 * Two buttons as input
 * 20x4 LCD at any address

## ioexpander
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
