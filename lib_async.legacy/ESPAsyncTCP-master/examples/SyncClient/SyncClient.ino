#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#else
#include <ESP31BWiFi.h>
#endif
#include "ESPAsyncTCP.h"
#include "SyncClient.h"

const char* ssid = "**********";
const char* password = "************";

void setup(){
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("WiFi Failed!\n");
    return;
  }
#ifdef ESP8266
  ArduinoOTA.begin();
#endif
  
  SyncClient client;
  if(!client.connect("www.google.com", 80)){
    Serial.println("Connect Failed");
    return;
  }
  client.setTimeout(2);
  if(client.printf("GET / HTTP/1.1\r\nHost: www.google.com\r\n\r\n") > 0){
    while(!client.available())
      delay(1);
    while(client.connected()){
      while(client.connected() && client.available() == 0) delay(1);
      while(client.connected() && client.available()){
        Serial.write(client.read());
      }
    }
  } else {
    client.stop();
    Serial.println("Send Failed");
    while(client.connected()) delay(0);
  }
}

void loop(){
#ifdef ESP8266
  ArduinoOTA.handle();
#endif
}
