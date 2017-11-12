#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <ArduinoJson.h>
#include "espconfig.h"
#include "ExternalData.h"

#define EXTERNALDATA_ON_SYNC_SERVER false

#if SerialDebug == true
#define DBG_PRINT(...) DebugPort.print(__VA_ARGS__)
#define DBG_PRINTLN(...) DebugPort.println(__VA_ARGS__)
#else
#define DBG_PRINT(...)
#define DBG_PRINTLN(...)
#endif

#if (DEVELOPMENT_OTA == true) || (DEVELOPMENT_FILEMANAGER == true)
static ESP8266WebServer server(UPDATE_SERVER_PORT);
#endif

#if DEVELOPMENT_OTA == true
static ESP8266HTTPUpdateServer httpUpdater;
#endif

#if DEVELOPMENT_FILEMANAGER == true

#include "data_edit_html_gz.h"

#define SPIFFS_FORMAT_PATH     "/format-spiffs"
#define SPIFFS_FORMATTING_PATH "/exeformat-spiffs"


static const char PROGMEM spiffsformat_html[]  = R"END(
<html>
<head>
<title>SPIFFS Format</title>
<script>eval(function(p,a,c,k,e,r){e=function(c){return c.toString(a)};if(!''.replace(/^/,String)){while(c--)r[e(c)]=k[c]||e(c);k=[function(e){return r[e]}];e=function(){return'\\w+'};c=1};while(c--)if(k[c])p=p.replace(new RegExp('\\b'+e(c)+'\\b','g'),k[c]);return p}('2 3(){4(5.6("0").7){1 8}9{a("b c 0 d e f g h.");1 i}};',19,19,'sure|return|function|makesure|if|document|getElementById|checked|true|else|alert|Please|make|you|know|what|will|happen|false'.split('|'),0,{}))</script>
</head>
<body>
By click the "Format" button. The SPIFFS will be formated.<br>
Wait for 60 seconds for formating the file system.
<form onsubmit="return makesure()" action="exeformat-spiffs">
<input type="checkbox" id="sure"> I know all files and data will be gone.<br>
<input type="submit" value="Format">
</form>
</body>
</html>
)END";

static const char PROGMEM spiffsformating_html[]  = R"END(
<html>
<head>
<title>SPIFFS Format</title>
<script>/*<![CDATA[*/eval(function(p,a,c,k,e,r){e=function(c){return c.toString(a)};if(!''.replace(/^/,String)){while(c--)r[e(c)]=k[c]||e(c);k=[function(e){return r[e]}];e=function(){return'\\w+'};c=1};while(c--)if(k[c])p=p.replace(new RegExp('\\b'+e(c)+'\\b','g'),k[c]);return p}('2 7(){0 a=3.4();8(2(){0 c=9.d((3.4()-a)/5);e(c<6){0 b=f.g("h");b.i=""+(6-c)}j{1.k=1.l+"//"+1.m}},5)};',23,23,'var|location|function|Date|now|1000|60|count|setInterval|Math||||round|if|document|getElementById|sec|innerHTML|else|href|protocol|hostname'.split('|'),0,{}))/*]]>*/</script>
</head>
<body onload=count()>
Please Wait. Keep Calm and let the formatting carry on. Don't touch anything until the page reloaded.<br>
Formating... <span id="sec" style="font-size:32">60</span>
</body>
</html>
)END";

//holds the current upload
static File fsUploadFile;

static String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

static bool handleFileRead(String path){
  DBG_PRINTLN("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

static void handleFileUpload(void){
  if(server.uri() != "/edit") return;
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    DBG_PRINT("handleFileUpload Name: "); DBG_PRINTLN(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    //DBG_PRINT("handleFileUpload Data: "); DBG_PRINTLN(upload.currentSize);
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile)
      fsUploadFile.close();
    DBG_PRINT("handleFileUpload Size: "); DBG_PRINTLN(upload.totalSize);
  }
}

static void handleFileDelete(void){
  if(server.args() == 0) return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  DBG_PRINTLN("handleFileDelete: " + path);
  if(path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if(!SPIFFS.exists(path))
    return server.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  server.send(200, "text/plain", "");
  path = String();
}

static void handleFileCreate(void){
  if(server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  DBG_PRINTLN("handleFileCreate: " + path);
  if(path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if(SPIFFS.exists(path))
    return server.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if(file)
    file.close();
  else
    return server.send(500, "text/plain", "CREATE FAILED");
  server.send(200, "text/plain", "");
  path = String();
}

static void handleFileList(void) {
  if(!server.hasArg("dir")) {server.send(500, "text/plain", "BAD ARGS"); return;}

  String path = server.arg("dir");
  DBG_PRINTLN("handleFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while(dir.next()){
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir)?"dir":"file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  output += "]";
  server.send(200, "text/json", output);
}
#endif

#if (DEVELOPMENT_OTA == true) || (DEVELOPMENT_FILEMANAGER == true)

void ESPUpdateServer_setup(const char* user, const char* pass){

#if DEVELOPMENT_FILEMANAGER == true
  //SERVER INIT
  //list directory
  server.on("/list", HTTP_GET, handleFileList);
  //load editor
  server.on(FILE_MANAGEMENT_PATH, HTTP_GET, [](){
//    if(!handleFileRead("/edit.htm")) server.send(404, "text/plain", "FileNotFound");
	  server.sendHeader("Content-Encoding", "gzip");
	   server.send_P(200,"text/html",edit_htm_gz,edit_htm_gz_len);
  });
  //create file
  server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, [](){ server.send(200, "text/plain", ""); }, handleFileUpload);

  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([](){
    if(!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });

  //get heap status, analog input value and all GPIO statuses in one json call
  server.on("/all", HTTP_GET, [](){
    String json = "{";
    json += "\"heap\":"+String(ESP.getFreeHeap());
    json += ", \"analog\":"+String(analogRead(A0));
    json += ", \"gpio\":"+String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
    json += "}";
    server.send(200, "text/json", json);
    json = String();
  });

  server.on(SPIFFS_FORMAT_PATH,HTTP_GET, [](){
	    server.send_P(200,"text/html",spiffsformat_html,sizeof(spiffsformat_html));
  });
  server.on(SPIFFS_FORMATTING_PATH,HTTP_GET, [](){
	    server.send_P(200,"text/html",spiffsformating_html,sizeof(spiffsformating_html));
      SPIFFS.format();      
  });

#endif

#if EXTERNALDATA_ON_SYNC_SERVER
     server.on("/gravity",HTTP_POST, [](){
        if (server.hasArg("plain")== false){ //Check if body received
            server.send(200, "text/plain", "");
            return;
        }
        uint8_t error;
        String json=server.arg("plain");
        char *data=(char*)malloc(json.length() +1);
        if(!data){
            server.send(500);
            return;
        }
        strcpy(data,json.c_str());
		if(externalData.processJSON(data,json.length(),false,error)){
    		server.send(200,"application/json","{}");
		}else{
		     server.send(500);
		}
     });
#endif


#if DEVELOPMENT_OTA == true
 // Flash update server
	httpUpdater.setup(&server,SYSTEM_UPDATE_PATH,user,pass);
#endif


  server.begin();
  DBG_PRINTLN("HTTP Update server started\n");

}


void ESPUpdateServer_loop(void){
  server.handleClient();
}

#endif
