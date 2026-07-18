#ifndef ESP32HTTPUpdateServer_H
#define ESP32HTTPUpdateServer_H
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>


/*
 * setup function
 */
class ESP32HTTPUpdateServer{
public:
    ESP32HTTPUpdateServer(){}
    static void setup(WebServer& server,String servicePath,const char* user=NULL, const char* pass=NULL);
protected:
    static const char* _username;
    static const char* _password;
};

#endif