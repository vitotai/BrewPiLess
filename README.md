# BrewPiLess
 *Remember to clear browser cache to get new interface!*
 *Note: New log format after V1.2.7&v2.0!*
 *Note: default username/password/hostname changes to `brewpiless` after v1.2.7*

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
 * **[New 1.2.7]** Gravity logging. The gravity data can be manually input or from iSpindel.
* **[New 1.2.7]** iSpindel support. 
* **[New 2.0]** Gravity-based temperature schedule.
* **[New 2.0]** Save and resuse of beer profiles.
* **[New 2.0]** Static IP.

# Contents
---
* [Introduction](#introduction)
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
---
# Introduction
This project uses a single ESP8266 to replace RPI and Arduino.
![Main Screen](img/brewpiless126p1.jpg)
BrewPi is the greatest, if not the ultimate, fermentation temperature controller. The original design uses a RPI to log temperatures and maintain a temperature schedule. The RPI also hosts a web server as the front-end of internet web access. 
Using a RPI or a PC allows the maximum power of BrewPi to be used but with the additional of a RPI or PC. 

ESP8266 is cheap and powerful WiFi-enabling IOT solution. 
Although it can't be as powerful as a RPI, it's a good solution to maximize the functionality and minimize the cost. Using a single ESP8266 as the temperature controller(replacing Arduino), web server and schedule maintainer(replacing RPI) also reduces the work in building a brewpi system.

## !!Special Note
Uploading files to ESP8266 is no longer needed because the "files" are now embedded in the code. However, you can still upload files to the File System by using the Data Upload tool or web based file manager. **The file in File System takes higher priority.** That is, if you have an "index.htm" in the file system, you will get this file instead of the server page embedded within the code when you visit "http://brewpiless.local". **If you have ever uploaded the data folder using the upload tool, please delete this when you update to new version, or you will not get updated files.** Please also note that you might need to hit "refresh" button on your browser to force it to get new files.

The difference bewteen v1.2.7 and v.20 is
 * v2.0 uses gravity-based beer profile while v1.2.7 uses BrewPi style beer profile. 
 * v2.0 supports profile re-use.
 
 To build v1.2.7 from source. Open `espconfig.h`, and change line 29
 `
 #define EnableGravitySchedule false
 `
 
## Known issues
* ESP8266 won't restart after saving system configuratoin.
 Sometimes ESP8266 can't restart by Software watchdog timer reset, which is the only way to reset the system by software. It did happened on my NodeMcu and D1 mini boards that didn't connect to anything but USB. I have no solution for it.
* ESP8266 won't start after selecting WiFi network.
 The web server used is ESPAsyncWebServer which uses ESPAsyncTCP. I found that if ESP8266 ever enters SoftAP mode before connecting to the network, the Web server will fail on tcp_bind() and the web service will be unavailable. Not tracing the source code of the LWIP, I just worked around by reseting the system. However, ESP8266 sometimes doesn't reset.
* The page can't be loaded correctly.
 It rarely happens after HTTP caching is used, but it does happen especially when Javascript Console is opened. During developing and testing, I found corrupted html/javascript pages. Without the abliity and time to debug or develop the web server and or TCP/IP stack, I decide to live with it.
* Incorrect tempeerature chart.
 The log format before v2.0 is vulnerable. There seems to be some unconsidered conditions that break the log. 

# Software configuration
BrewPi related configuration is defined in `config.h` while networking related configuration is defined in `espconfig.h`. They are both self-explanatory and commented. Please check the files directly.

## Libraries
You will need the ESP8266/Arduino environment, as well as the following libraries.
 * ArduinoJson https://github.com/bblanchon/ArduinoJson
 * WiFiManager (my branch) https://github.com/vitotai/WiFiManager
 * ESPAsyncTCP https://github.com/me-no-dev/ESPAsyncTCP 
 * ESPAsyncWebServer https://github.com/me-no-dev/ESPAsyncWebServer 
 * ESP8266HTTPUpdateServer (newer version is needed. you might need to manually download the files.) https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266HTTPUpdateServer
 * OneWire https://github.com/PaulStoffregen/OneWire

**Some of the libraries are modified and provided. Please use the libraries provided to ensure compatiblity.** 

---
# Usage
## WiFi setting

WiFi Manager is used to setup the the network. On first use or if the connected network changes or disappears, WiFi Manager will
setup a AP named `brewpiless`. The network setting can be done through the web page after connecting the ESP8266 as AP.
Please check URL for more detail.

https://github.com/tzapu/WiFiManager

After **three minutes**, ESP8266 will proceed to enter soft AP mode. That means you only have three minutes to setup the network. 
## Soft AP mode
BrewPiLess now can run in AP mode, which enables it to run as stand alone device. The newly modified WiFiManager has a new option, "Soft AP Mode". Soft AP mode will also be entered if the network setting is not configured in three minutes (and previous connected network doesn't exist, or there is no previously connected network.)
**This design is to enable recovery from a power shortage or system reset.** Without this feature, BrewPiLess will hang at the network setting state and won't perform temperature management funcitons.

For scheduled temperature management, aka Beer Profile mode, the "time" information is needed to manage the temperature, but an NTP server will not be accessible without an internet connection. Therefore, **manual setup of "time" is necessary in this mode**. In page of "Temperature Management", aka /control.htm, a SET TIME button will be shown when the time of ESP8266 is far away from the computer/phone. Pressing that button will set the time of ESP8266 to the time of the computer/phone, or the browser to be exact.

**To enable automatic recovery from power shortage or system reset**, the time informatoin is saved periodically and restored at boot-up if NTP is not accessible, which means the duration of power shortage is assumed to be zero. If the power shortage lasts too long, the shedule will not be on track. For example, if the power shortage lasts 8 hours, the schedule will be off for 8 hours since that 8 hours is missing for ESP8266. Without a RTC, this might be the best I can do.

mDNS doesn't work under AP mode. Therefore, "brewpiless.local" can not be used under AP mode, but "brewpiless.org" will do the job. In fact, all domain names except those in Apple's Captive Portal checklists will do.

After v2.0, static IP is supported at the page of network selection. To use DHCP, leave the IP blank.

## Service Pages
 
BrewPiLess implements mDNS, so you can use "brewpiless.local" instead of the IP address if you are using platforms from Apple. You can change the name in system configuration page. 
 Default username and password are both `brewpiless`. (It was `brewpi` before 1.2.6.)
 
* Main page - `http://brewpiless.local/`
    The main page. The menu includes:
  * Device setup - `http://brewpiless.local/setup.htm`
    The device setup procedure as original BrewPi is necessary.
  * Logging setting - `http://brewpiless.local/log`
  * System configuration - `http://brewpiless.local/config`
    Updating the settings will result in restart of the system.
  * Gravity Sensor
    Gravity sensor(iSpindel) settting.
* LCD page - `http://brewpiless.local/lcd` 
    The LCD display of BrewPi. Clicking the LCD brings out the pop menu to other functions. This page is good for mobile device or when temperature chart is not necessary.
 
* OTA update - `http://brewpiless.local:8008/systemupdate`
    
    The menu from the main page doesn't include this page.

## Gravity logging
To add gravity reading record, click the **SG reading**. The number entered with the time when it is entered will be saved as a SG reading record.
To enter OG, click the **OG value**. Once OG is availble, the Attenuation and ABV will be calcuated when SG reading is updated. The OG is for calculation of attenuation and ABV only.

## iSpindel Support
### Modification to iSpindel firmware
**Important!! Without this modification, BrewPiLess will crash after 5 times of report.**
There is an issue of ESP8266. (To be more specific, it is an issue of LWIP or TCP/IP.)
**TL;DR** The issue is described here: https://github.com/esp8266/Arduino/issues/2750 

The work-around for it is add a `delay(50);` aftet finishing data report and before ESP8266 goes into deep sleep, so that ESP8266 has a chance to close the connection.
Around `Line 870@iSpindel.ino` of release 01.05.2017 5.x: 

```C
  if (WiFi.status() == WL_CONNECTED)
  {
    SerialOut(WiFi.localIP());
    uploadData(my_api);
    delay(50); // <<< add this line
  }
```  

### Connection setup for iSpindel
BrewPiLess supports iSpindel by accepting data from iSpindel and acting an **AP** for iSpindel to connect to, BrewPiLess and iSpindel can connect to the same router. 
To support **softAP**, set the correct settings in `System configuration`. Please note that the password(passphrase) should be at least **8** characters. The same password(pass phrase) is used for setting and for connection certification. Default value is `brewpiless`.

![Main Screen](img/gdsetting.jpg)

| Setting   | Description       |
| -------------- |:---------------------------------|
| iSpindel         | To enable iSpindel support.   	   |
| Calculated by BPL | Do the conversion from tilt angle to gravity by BrewPiLess. If this option is OFF, all the following options are not used. |
| SG Calibration   |  Offset of gravity reading. This value will be applied(add) to the calcuated SG if SG is calculated by BrewPiLess. |
| Temp. Correction | Apply temperature correction to the calculated gravity reading. Celsius only. Usually it is 20&deg;C(68&deg;F) or 15&deg;C(59&deg;F). | 
| Coefficients | The coefficients of the formula to calculate gravity. Note: this set of coefficients is for calcuation of **specific gravity**, **not** plato. Use 0 for x^3 term if quadratic polynomial is used.|

Note: enable iSpindel setting only enable the initial display of iSpindel status. The gravity report will be process even when the option is OFF.

### iSpindel Settting
 * the **iSpindel Name** must start with `iSpindel`, like `iSpindel000` 
 * Select `Generic HTTP`
 * Server address set to `192.168.4.1` if iSpindel connect to the AP created by BrewPiless. or use the ip of BrewPiless if iSpindel connects the AP that BrewPiLess connects to.
 * set url to `/gravity`

For other iSpindel setting, like network settting, please refer to iSpindel project.

## Local logging

 * The log won’t start automatically, you have to start it at log setting page.
 * **(new in v1.2.5) When logging is not started, BrewPiLess still logs the data and keep that latest 2 to 3 hours of data. The data will not be saved to file system, though.**
 * The temperatures are logged every minute.
 * A 30 day log will take around 350k bytes. That might imply that 3M space can record around 6 month data. However, there is no guarantee of the robustness of SPIFFS.
 * Changing temperature when logging will result in wrong data interpretation.
 * A maximum of 10 logs is allowed. The logs will not be deleted automatically. Manual deleting is necessary.
 * Off-line viewer is available. You can download the log and view it from your computer. Download the file "BPLLogViewer.htm" in the "extra" subfolder. Save it anywhere in your computer. Open it using a web browser.
 * **Internet access is required to view the chart**. To save some more space and to alleviate the loading of ESP8266, the library is not put in the ESP8266. (It is possible after v1.2.5.)
 * The loggging format changed after v1.2.7/v2.0. Use BPLLogViewer**V2**.htm to view the new logs and BPLLogViewer.htm for old logs. The old logs cannot be viewed 'on-line' by the "view" button. Please download them and view by the off line viewer.

### Viewing Char under AP mode
There are two wasy that make it possible to have the temperature chart under AP mode. 
 * Let the browser cache the file. The simple way is having BrewPiLess connect to a router that has internet access, then connnection to BrewPiLess. Usually, the browser will cache the library laod from disk after that. However, the browser might clear the cache for some reasons, so this might not always work.
 * Put the library in ESP8266. 
  Go to http://dygraphs.com/download.html  and download the v1.1.1 `dygraph-combined.js`.
  open http://brewpiless.local:8008/filemanager, and upload the downlowed libarry to ESP8266. The file shoule be named exact `dygraph-combined.js`. 

## Remote logging
Remote logging can be used to post data to a HTTP server that BrewPiLess can connect to.The `format` field in log setup page is like the format in `printf` but uses the following specifiers:

| Specifier   | output  |
| -------------- |:-------------|
| %b         | Beer temperature   |
| %B         | Beer setting   |
| %f         | fridge temperature   |
| %F         | fridge setting   |
| %r         | room temperature   |
| %g         | gravity   |
| %a         | auxiliary temperature   |
| %v         | external device voltage   |
| %u         | UNIX timestamp of last gravity update   |

For example, let beer setting be `20.0` and beer temperature be `18.3`, if the `format` is `api_key=TheRealApiKeyHere&field1=%B&field2=%b`,
the data will be `api_key=TheRealApiKeyHere&field1=20.0&field2=18.3`.
If the method is `GET`, the data will append to the url with additional `?`, so the result will be
`http://api.thingspeak.com/update?api_key=TheRealApiKeyHere&field1=20.0&field2=18.3`.
(GET is usually not recommended.)
The `Data Type` field is used as "Content-Type" in HTTP protocol. When it is left blank, the default value of `application/x-www-form-urlencoded` will be used. The default type is good for content like `A=V1&B=V2`.

Example setting for Thingspeak.com
 ![Log Setting](img/log_thingSpeak.jpg)

Example setting for ubidots.com
 ![Log Setting](img/log_ubidots.jpg)
---
# Hardware Setup
Fortunately, 3.3V is regarded as HIGH in 5V logic, so the **output** of ESP8266 can be connected directly to the devices. Luckily, DS18B20 works under 3.3V. I just replace the Arduino Nano with the ESP8266, and it works. You should still be careful not to burn the ESP8266.

A lot of PINs are required, so ESP-12 or ESP-07 should be a better choice. NodeMcu development board is a simple solution for those who arn't good at soldering.

Search the BrewPi DIY on homebrewtalk.com forum to find out the hardware setup, but use these ESP8266 pins in place of Arduino pins.
This is default configuration, you can change it in `config.h`.

| ESP8266 GPIO   | NodeMcu Label | Connect to       |
| -------------- |:-------------:| :--------------------|
| GPIO16         | D0            | Buzzer			   |
| GPIO5          | D1            | I2C SCL             |
| GPIO4          | D2            | I2C SDA             |
| GPIO0          | D3            | INT from PCF8574 * Or Wakeup Button  |
| GPIO2          | D4            | Door (not used)     |
| GPIO14         | D5            | Cooling Actuator*   |
| GPIO12         | D6            | Temperature Sensors |
| GPIO13         | D7            | Heating Actuator*   |
| GPIO15         | D8            |      			   |

*cooling/heating actuator PINs are configurable.

Note: The GPIOs of ESP8266 are not all **General Purpose**. Some of them has special functions, and might not be usable. For example, some PINs on my NodeMcu board don't work normally.
**!!Important !!** It is hightly recommended to pull up GPIO0 and GPIO2 while pull down GPIO15 so that the system will start up normally instead of staying in bootrom mode in case the system crashes. Updating the system configuration and firmware also results in restart of system, and sometimes this issue happens if the circuit isn't implementated. Check this url for detail information:
https://github.com/esp8266/Arduino/blob/master/doc/boards.md#minimal-hardware-setup-for-bootloading-and-usage

**!! The rotary encoder is not supported by directly connecting it to ESP8266. That will prevents ESP8266 to boot up when the rotary encoder is at certain positions.!!** 

## Support of Rotary Encoder
Due to the special usage at bootup of GPIO0,2,15(D3,D4,D8), they can't be used as inputs for rotary encoder. One of the solution is buy an IO Expander. 
Currently, PCF8574 is supported if you really need the rotary encoder. You will have to change the compile options in Config.h to enable this feature.
## Wake-up button
Without a rotary encoder input, the backlight of the LCD will never be turned-off because there is no way to "wake" it up. Since BrewPiLess can be controlled by network easily, the rotary encoder seems unnecessary. A wake-up button is a solution for this. The button connects to D3 by default and grounds D3 when pushed.

---

# Extra
## Offline Log Viewer
Although the flash size of ESP8266 module might be as big as 16 Mega bytes, it is still limited. The logs can be downloaed and viewed offline. Goto `Extra` folder and download the BPLLogVewer.htm/BPLLogVewerV2.htm. Place them anywhere on your computer, and open it by your browser.
BPLLogVewerV2.htm is for logs created by v2.0/v1.2.7 or after, while BPLLogVewer.htm is for logs created by older version.

## Logging data to Google Sheets
Due to the resource limit of ESP8266, establishment of **HTTPS** connection while serving other functions will crash the system. 
Therefore, a generic interface is provided to enable pushing/sending data to a specified URL(HTTP, NO HTTPs). The file to support Google Sheet logging is in `extra` folder.

A simple script as the proxy to push data to Google Sheet is needed. Here is how to do it.

1. Create a script to access Google Sheet. 

 a. you must have a google account. obviously.

 b. got to “script.google.com”, create a new project, and copy the content of `code.gs` in `extra` folder.

 c. release it as a Web Application, and set it to “Run as ME” and "anyone can access.” Note the **script ID** in the URL

2. Create a Spread Sheet for logging.

 a. Create a google spreadsheet in any name you like, but note the **spreadsheet ID**.
    input the lables at cell 'A1', and 3rd rows. leave 2nd row empty. like this.
    _You might need to manually change the format of Time column to display Date & Time._ The default format seems to show Date only.

 |     |    A        |   B      |     C    |   D     |    E     |
 |:---:|:-----------:|:--------:|:--------:|:--------:|:--------:|
 |  1  |Last Update: |			|          |	      |          |
 |	2  |             |          |          |          |          |
 |	3  | Time	     | BeerTemp	| BeerSet  |FridgeTemp|	FridgeSet|
 |	4  |             |          |          |          |          |
  
 b. rename or keep the **sheet label**. note it
 
 

3. Setup proxy script. you must have access to a server that you can run your CGI script and the CGI script can connect to other hosts.

 Do it yourself or ask someone for help. The URL that we will use is this script. 

4. Settings
 In the log setting pages, default to `http://brewpiless.local/log` 

 * URL: http:// `{your server} `/ `{your path} `/logdata.php 
 <= your script in **step 3**
 * Method: POST
 * format: `script=[script_ID]&ss=[spreadsheet_ID]&st=[sheet_label]&pc=thisistest&bt=%b&bs=%B&ft=%f&fs=%F`
    * **[script_ID]** is the id from **step 1c**
    * **[spreadsheet_ID]**  is from **step 2a**
     * **[sheet_label]** is the value in **step 2b**
    * `thisistest` is the passcode for google script app.

 * Log time period. <= set the value you like, in seconds.

Then, BrewPiLess will post temperature data  to the URL, and the script(logdata.php) at that URL will get the data and do HTTPS request to google script which write the data to the specified spreadsheet.

## Logging data to other destination
If you write your own proxy script or push data to other IOT website. you can change the settings to your needs.

The periodical request can be also used as an I-AM-ALIVE message. For example, if the period is set to 10 minutes, and the temperature hasn't been updated for 11 minutes,
there must be something wrong. `/extra/brewpimon.php` is an example which is executed by cronjob every few minutes to check the updating of temperature data. The PHP will notice by email if the temperature data isn't updated in specified time.
## Upload HTML/Javascript to ESP8266
** In newer version, those files are embedded in the code, and it is not necessary to upload them. **
To upload files onto ESP8266, check the following link:
https://github.com/esp8266/Arduino/blob/master/doc/filesystem.md

In newer version, the basic files are embedded. However, the file in SPIFFS takes higher prioity. That will be useful if you want to change them.

## Development tools
Two additional tools are available. One is web-based file manager to manipulate the files directly from the web. You can download and upload files the the web.
To enable this feature, set `DEVELOPMENT_FILEMANAGER` to true in `espconfig.h`.
You can access the files by the url (you should know you can change it):
`http://brewpiless.local:8008/filemanager` 

The second tool is used to access the BrewPi directly. The original BrewPi on Arduino uses serial to communicate with RPI in the JSON-like format.
By using the JSON-like commands, you have full access to BrewPi.   
For example, you can erase the EEPROM by sending `E` letter, get the LCD display by issuing `l` command, and get device list by `h` command.
The page is at
`http://brewpiless.local/testcmd.htm` 

## JSON commands
By using `http://brewpiless.local/testcmd.htm`, you can control BrewPi core directly. For example, to set temperature to Fahrenheit. Open the testcm.htm page, and enter the following string, and send.

`j{"tempFormat":"F"}` 


You can set multiple parameters in one command. The command after `j` is in formal JSON format. Please include the double quote(") for key and string value.


| Key            | Meaning       | Note       |
| -------------- |:-------------:| :--------------------|
| tempFormat         | Temperature format          |  F for Fahrenheit, C for Celius	|
| tempSetMin        | Minimum setting temperature    |              |
| tempSetMax        | Maximum setting Temperature    |        |
| pidMax            | PID Max                        |   | 
| Kp                | Kp parameters of PID           |   | 
| Ki                | Ki parameters of PID           |   | 
| Kd                | Kd parameters of PID           |   | 
| iMaxErr           | iMaxError                      |   |
| idleRangeH        | idleRangeHigh                  |   |
| idleRangeL        | idleRangeLow                   |   |
| heatTargetH       | heatingTargetUpper             |   |
| heatTargetL       | heatingTargetLower             |   |
| coolTargetH       | coolingTargetUpper             |   |
| coolTargetL       | coolingTargetLower             |   |
| maxHeatTimeForEst | maxHeatTimeForEstimate         |   |
| maxCoolTimeForEst | maxCoolTimeForEstimate         |   |
| fridgeFastFilt    | fridgeFastFilter               |   |
| fridgeSlowFilt    | fridgeSlowFilter               |   |
| fridgeSlopeFilt   | fridgeSlopeFilter              |   |
| beerFastFilt      | beerFastFilter                 |   |
| beerSlowFilt      | beerSlowFilter                 |   |
| beerSlopeFilt     | beerSlopeFilter                |   |
| lah               | lightAsHeater                  |   |
| hs                | rotaryHalfSteps                |   |
| heatEst           | heatEstimator                  |   |
| coolEst           | coolEstimator                  |   |

## Sensor Calibration
 The command to set sensor calibration is
 
 `U{i:0,j:0.36}`
 
 where **0** is the device ID that is assigned to the sensor during device setup, and **0.36** is the adjustment to the sensor.
 
## Reset WiFi
To reset WiFi setting, use the following URL (assuming you can access it by brewpiless.local.)
    `http://brewpiless.local/erasewifisetting`
BrewPiLess will erase the wifi setting and then reset.