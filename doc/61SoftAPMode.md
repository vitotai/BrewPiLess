BrewPiLess can run in AP mode, which enables it to run as stand alone device. The modified WiFiManager has a new option, "Soft AP Mode". Soft AP mode will also be entered if the network setting is not configured in three minutes (and previous connected network doesn't exist, or there is no previously connected network.) This design is to enable recovery from a power shortage or system reset. Without this feature, BrewPiLess will hang at the network setting state and won't perform temperature management funcitons.

For scheduled temperature management, aka Beer Profile mode, the "time" information is needed to manage the temperature, but an NTP server will not be accessible without an internet connection. Therefore, manual setup of "time" is necessary in this mode. In page of "Temperature Management", aka /control.htm, a SET TIME button will be shown when the time of ESP8266 is far away from the computer/phone. Pressing that button will set the time of ESP8266 to the time of the computer/phone, or the browser to be exact.

To enable automatic recovery from power shortage or system reset, the time informatoin is saved periodically and restored at boot-up if NTP is not accessible, which means the duration of power shortage is assumed to be zero. If the power shortage lasts too long, the shedule will not be on track. For example, if the power shortage lasts 8 hours, the schedule will be off for 8 hours since that 8 hours is missing for ESP8266. Without a RTC, this might be the best I can do.

mDNS might not work under AP mode. Therefore, "brewpiless.local" can not be used under AP mode, but "brewpiless.org" will do the job. In fact, all domain names except those in Apple's Captive Portal checklists will do.

To save space of ESP8266 and accerate page rendering, the charting library, dygraphs.js, is hosted at
http://cdnjs.cloudflare.com/ajax/libs/dygraph/1.1.1/dygraph-combined.js

In SoftAP mode, the computer is connect to BPL without internet access. Therefore, the browser can't get the charting library, and the front page will not be rendered. Two options are available
* "cache" the file. Simply copy the URL above and paste it in the address line of your browser and go. In most case, the browser will download and cache the js library. However, the browser might clear the cache for some reasons, so this might not always work.
*  Go to http://dygraphs.com/download.html and download the v1.1.1 dygraph-combined.js. open http://brewpiless.local:8008/filemanager, and upload the downlowed libarry to ESP8266. The file shoule be named exact "/dygraph-combined.js".

