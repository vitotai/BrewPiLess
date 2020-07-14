You can upload HTML files to overwrite the pages by file manager. The priority of file served is
1. *.gz file @ file system
2. file @ file system
3. embedded file in the image

Take the homepage(http://brewpiless.local) for example. The default homepage is "index.htm"(not html), so when you access "http://brewpiless.local", BPL will check in sequence:
1. index.htm.gz file @ file system
2. index.htm @ file system
3. embedded index.htm in the image

You can customize the web pages. However, keep in mind that
* ESP8266, using LWIP, allows maximum 5 connections. 
* SPIFFS of ESP8266 supports maximum 5 opened files.
* ESP8266 is a small MCU that is nothing near, like RPI.

Therefore,
* avoid multiple files to load in one page.
* use gzip to reduce the file size.

Tom has created a set of elegant frontend of BPL:
https://github.com/tommueller/BrewPiLess/tree/master/htmljs/dist
Tom's work has been merged into this project:
https://github.com/vitotai/BrewPiLess/tree/master/htmljs

