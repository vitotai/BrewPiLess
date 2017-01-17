# Prebuilt Image

Those images are prebuilt:
## Simple Configuration
 * The Pins are defined as the main page.
 * No rotary encoder supported
 * 20x4 LCD at any address
 * LCD backlight won't turn off automatically, because there is no rotary encoder available.
  
## Thorrax board
 * The Pins are defined as Thorrax's 'brewpi-esp8266'
 * Other features are the same as default configuration.

## Wakeup-button
 * LCD backlight will be turned off automatically after 3 minutes.
 * Use a button to "wake-up" the backlight of LCD.
 
## Rotary Encoder by PCF8574A (Default)
 * The rotary encoder is supported by using a PCF8574 IO expander. The PCF85 addresses is set to 0x20. Hence, you cannot use a LCD with address 0x320(It's rare, but it could be.)
 * LCD backlight will be turned off automatically after 3 minutes.
 
## Rotary Encoder by PCF8574A, and SSD1306 128x64 OLED LCD (Current not supported)
 * Using SSD1306 128x64 OLED LCD. The address could be 0x3C-3D.
 * Other features are the same as Rotary Encoder supported version. 
