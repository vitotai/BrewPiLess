# Prebuilt Image

Those images are prebuilt:
## Default Configuration
 * The Pins are defined as the main page.
 * No rotary encoder supported
 * 20x4 LCD at any address
 * LCD backlight won't turn off automatically, because there is no rotary encoder available.
  
## Thorrax board
 * The Pins are defined as Thorrax's 'brewpi-esp8266'
 * Other features are the same as default configuration.
 
## Rotary Encoder by PCF8574A
 * The rotary encoder is supported by using a PCF8574A IO expander. The PCF8574A addresses is set to 0x38. Hence, you cannot use a LCD with address 0x38.(It's rare, but it could be.)
 * LCD backlight will be turned off automatically after 3 minutes.
 
## Rotary Encoder by PCF8574A, and SSD1306 128x64 OLED LCD
 * Using SSD1306 128x64 OLED LCD. The address could be 0x3C-3D.
 * Other features are the same as Rotary Encoder supported version. 
