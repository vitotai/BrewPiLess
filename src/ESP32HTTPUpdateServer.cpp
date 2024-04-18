#if ESP32
#include "Config.h"
#include "ESP32HTTPUpdateServer.h"

const char* ESP32HTTPUpdateServer::_username=NULL;
const char* ESP32HTTPUpdateServer::_password=NULL;

static const char* IndexPage = R"END(
<html><body><form method='POST' action='/flash' enctype='multipart/form-data'>
<input type='file' name='update'>
<input type='submit' value='Update'>
</form></body></html>
)END";

void ESP32HTTPUpdateServer::setup(WebServer& server,String servicePath,const char* user,const char* pass) {
    _username = user;
    _password = pass;
    
    server.on(servicePath, HTTP_GET, [&]() {
     	if(_username != NULL && _password != NULL && !server.authenticate(_username, _password))
 	    	return server.requestAuthentication();
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", IndexPage);
    });
    
    server.on("/flash", HTTP_POST, [&]() {
        server.sendHeader("Connection", "close");
        if(Update.hasError())
            server.send(501, "text/plain", String("Update Failed:") + String(Update.getError()));
        else{
            server.send(200, "text/plain", "Update Successfull!");
            delay(1000);
            ESP.restart();
        }
    }, [&]() {
        HTTPUpload& upload = server.upload();
        if (upload.status == UPLOAD_FILE_START) {
            Serial.printf("Update: %s\n", upload.filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
                //Update.printError(Serial);
                DBG_PRINTF("Update Error:%d\n",Update.getError());
            }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            /* flashing firmware to ESP*/
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                //Update.printError(Serial);
                DBG_PRINTF("Update Error:%d\n",Update.getError());
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) { //true to set the size to the current progress
                //Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
            } else {
                //Update.printError(Serial);
                DBG_PRINTF("Update Error:%d\n",Update.getError());
            }
        }
    });
}
#endif