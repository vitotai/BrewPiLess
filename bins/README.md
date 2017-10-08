# Prebuilt Image

Those images are prebuilt:
## Simple Configuration
 * The Pins are defined as the main page.
 * No rotary encoder supported
 * 20x4 LCD at any address
 * LCD backlight won't turn off automatically, because there is no rotary encoder available.

## sonoff
* For SONOFF.
    * Sensor on D5, Coolling on D6 
 * No rotary encoder. No LCD
 * OTA update **NOT** supported.
 * 512K SPIFFS for logging.

## sonoffota
* Same as sonsoff, but
 * 64K SPIFFS for configurations
 * OTA update supported

## glycol
* Standard options with GlycolSupport on.

## oled
* Standard options
* SSD1306 OLED I2C 128x64 instead of 20x4 LCD

## thorrax
* Thorrax's board, Cooling and heating PIN on D5, D0