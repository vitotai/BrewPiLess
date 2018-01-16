# BrewPiLess
 **Note: re-SETUP is necessary after upgrading to v2.4**

## Features
 * I2C LCD support
 * Rotary Encoder support (* not supported by default)
 * Remote LCD display on browser
 * Remote Temperature control
 * Temperature schedule
 * Device(temperature sensor and actuator) setup
 * Temperature logging to specified **remote** destination. 
 * Web-based OTA firmware update.
 * Web-based network setting
 * SoftAP mode
 * Local Temperature log and temperature chart
 * Gravity logging. The gravity data can be manually input or from iSpindel.
 * iSpindel support. 
 * Gravity-based temperature schedule.
 * Save and resuse of beer profiles.
 * Static IP setting.
 * Export saved datat to csv format by offline log viewer.
 * Brew and Calibrate iSpindel. **new!**
# Contents
---
* [Introduction](#introduction)
    * [Version History](#version-history)
* [Software configuration](#software-configuration)
  * [Libraries](#libraries)
* [Usage](#usage)
  * [WiFi setting](#wifi-setting)
  * [Soft AP mode](#soft-ap-mode)
  * [Service Pages](#service-pages)
  * [Gravity logging](#gravity-logging)
  * [iSpindel Support](#ispindel-support) 
  * [Local logging](#local-logging)
    * [Viewing Chart under AP mode](#viewing-chart-under-ap-mode)
  * [Remote logging](#remote-logging) 
  * [Saved Beer Profiles](#saved-beer-profiles)
  * [Beer Profile](#beer-profile) 
  * [Glycol](#glycol) 
  * [Brew and Calibrate iSpindel](#brew-and-calibrate-ispindel) **New!**
  * [Using iSpindel as Beer Temperature Sensor](#using-ispindel-as-beer-temperature-sensor)
* [Hardware Setup](#hardware-setup)
  * [Support of Rotary Encoder](#support-of-rotary-encoder)
  * [Wake-up button](#wake-up-button)
* [Extra](#extra)
  * [Offline Log Viewer](#offline-log-viewer) 
  * [Logging Data to Google Sheets](#logging-data-to-google-sheets) 
  * [Upload HTML/Javascript to ESP8266](#upload-htmljavascript-to-esp8266)
  * [Development tools](#development-tools)
  * [JSON commands](#json-commands)
  * [Sensor Calibrartion](#sensor-calibration)
  * [Reset WiFi](#reset-wifi)
  * [iSpindel Calibration](#ispindel-calibration) 
* [FAQ](#faq)
   
---
# Introduction
This project uses a single ESP8266 to replace RPI and Arduino.
![Main Screen](img/bplmain.jpg)
BrewPi is the greatest, if not the ultimate, fermentation temperature controller. The original design uses a RPI to log temperatures and maintain a temperature schedule. The RPI also hosts a web server as the front-end of internet web access. 
Using a RPI or a PC allows the maximum power of BrewPi to be used but with the additional of a RPI or PC. 

ESP8266 is cheap and powerful WiFi-enabling IOT solution. 
Although it can't be as powerful as a RPI, it's a good solution to maximize the functionality and minimize the cost. Using a single ESP8266 as the temperature controller(replacing Arduino), web server and schedule maintainer(replacing RPI) also reduces the work in building a brewpi system.

## !!Special Note
You will need to run the hardware setup procedure after upgrading to v2.4 from prior version other than Glycol option enabled. Note the configuration or save the options before you update the firmware so that you can recover the settings quickly.
 
## Known issues
* ESP8266 won't restart after saving system configuratoin.
 Sometimes ESP8266 can't restart by Software watchdog timer reset, which is the only way to reset the system by software. It did happened on my NodeMcu and D1 mini boards that didn't connect to anything but USB. I have no solution for it.
* ESP8266 won't start after selecting WiFi network.
 The web server used is ESPAsyncWebServer which uses ESPAsyncTCP. I found that if ESP8266 ever enters SoftAP mode before connecting to the network, the Web server will fail on tcp_bind() and the web service will be unavailable. Not tracing the source code of the LWIP, I just worked around by reseting the system. However, ESP8266 sometimes doesn't reset.
* The page can't be loaded correctly.
 It rarely happens after HTTP caching is used, but it does happen especially when Javascript Console is opened. During developing and testing, I found corrupted html/javascript pages. Without the abliity and time to debug or develop the web server and or TCP/IP stack, I decide to live with it.
* Incorrect tempeerature chart.
 The log format before v2.0 is vulnerable. There seems to be some unconsidered conditions that break the log. 

## Version History
 * v2.4.2x (2018/1/15)
    * bug fixed for 0 TILT value 
    * force to use framework 1.5

 * v2.4.2 (2017/12/27)
    * bug fixed for resume display
    
 * v2.4.1 (2017/11/28)
    * URL to Format File System 
    * missing "Calculated by BPL" in v2.4
 
 * v2.4 (2017/11/09)
    * Brew and calibrate iSpindel.
    * Use iSpindel temperature reading as Beer Sensor.
    * Display tilt value of iSpindel.
    * Enhance SSE re-establishment
    * Default configurable minimum cooling/heating time & back-up sensor. (That is, Glycol supported.)
    * HTTP Port settings.

 * v2.3.3 (2017/10/08)
    * All HTML files can be replaced by files on SPIFFS. Gzip support.
    * updated HTML/JS
    * Add "Title" to be displayed at banner in config page.
    * Workaround for accepting HTTP Post body length not equal to Content-Length.( for iSpindel v5.2+)

 * v2.3.2
    * Beer Profile scheculde bug fix
    * Show hostname at banner
 * v2.3.1
    * WiFi signal
    * /getstatus web service
 * v2.3
    * Fix error in time of reset. (New log format! Use new log viewer)
    * State coloring in chart
    * Remove "view" action in log list.
 * v2.2
    * 4 decimals of gravity
    * Switch to PlatformIO instead of Arduino IDE.
 * v2.1
    * more gravity-based condition 
 * v2.0
    * gravity-based beer profil schedule
 * v1.2.7
    * iSpindel support 


[WiKi](https://github.com/vitotai/BrewPiLess/wiki) 

