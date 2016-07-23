# BrewPiLess
## Introduction
## Features
## Hardware Setup
## Logging temperature data to Google Sheets
Due to the resource limit of ESP8266, establishment of **HTTPS** connection while serving other functions will crash the system. 
Therefore, a generic interface is provided to enable pushing/sending data to a specified URL(HTTP, NO HTTPs). The file to support Google Sheet logging is in `extra` folder.

A simple script as the proxy to push data to Google Sheet is needed. Here is how to do it.

1. Create a script to access Google Sheet. 

 a. you must have a google account. obviously.

 b. got to “script.google.com”, create a new project, and copy the content of `code.gs` in `extra` folder.

 c. release it as a Web Application, and set it to “Run as ME” and "anyone can access.” Note the **script ID** in the URL

2. Create a Spread Sheet for logging.

 a. Create a google spreadsheet in any name you like, but note the **spreadsheet ID**.

 b. rename or keep the **sheet label**. note it

3. Setup proxy script. you must have access to a server that you can run your CGI script and the CGI script can connect to other hosts.

 Do it yourself or ask someone for help. The URL that we will use is this script. 

4. Settings
 In the log setting pages, default to `http://brewpi.local/log` 

 * URL: http:// `{your server} `/ `{your path} `/logdata.php 
 <= your script in **step 3**
 * Method: POST
 * Beer and fridge temperature names. BeerTemp:`bt`, BeerSet:`bs`, FridgeTemp:`ft`, FridgeSet:`fs`
<=these are for the proxy script. 

 * Extra parameter: `script=[script_ID]&ss=[spreadsheet_ID]&st=[sheet_label]&pc=thisistest`

  Extra parameter provides a way for ESP8266 to send extra information to the proxy script.
    * **[script_ID]** is the id from **step 1c**
    * **[spreadsheet_ID]**  is from **step 2a**
    * **[sheet_label]** is the value in **step 2b**
    * `thisistest` is the passcode for google script app.
 * Log time period.

Then, ESP8266 will post temperature data together with the extra information to the URL, 
and the script(logdata.php) at that URL will get the data and do HTTPS request to google script which write the data to the specified spreadsheet.

## Logging temperature data to other destination
If you write your own proxy script or push data to other IOT website. you can change the settings to your needs.
For example, if the method is set to "GET", the the url will be
 http:// `{your server} `/ `{your path} `/logdata.php?bt=20.50&bs=20.00&ft=21.00&fs=19.00&script=**[script_ID]**&ss=**[spreadsheet_ID]**&st=**[sheet_label]**&pc=thisistest
 