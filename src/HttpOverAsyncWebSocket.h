#ifndef HttpOverAsyncWebSocket_H
#define HttpOverAsyncWebSocket_H

#include "Config.h"
#include "ESPAsyncWebServer.h"
#include "AsyncWebSocket.h"
/*
To implement HTTP over WebSocket.
To make easy transition, mimic the interface of ESPAsyncWebServer.
*/

#define MAX_INITIAL_FRAME_SIZE 1400

// Warnning: too much Serial.print() will result in CRASHES!!!!

#if 0
#define HOAWS_PRINTF(...) DebugPort.printf(__VA_ARGS__)
#else
#define HOAWS_PRINTF(...)
#endif

#if 0 // debuging parsing parts
#define HOAWS_PARSE_PRINTF(...) DebugPort.printf(__VA_ARGS__)
#else
#define HOAWS_PARSE_PRINTF(...)
#endif

class HttpOverAsyncWebSocketClient;
class HttpOverAsyncWebSocketHandler;
class HttpOverAsyncWebSocketResponse;
typedef std::function<size_t(uint8_t*, size_t, size_t)> HoawsResponseFiller;

class HttpOverAsyncWebSocketHandler {
  public:
    HttpOverAsyncWebSocketHandler(){}
    virtual ~HttpOverAsyncWebSocketHandler(){}
    virtual bool canHandle(HttpOverAsyncWebSocketClient *request __attribute__((unused))){
      return false;
    }
    virtual void handleRequest(HttpOverAsyncWebSocketClient *request __attribute__((unused))){}
    virtual void handleBody(HttpOverAsyncWebSocketClient *client __attribute__((unused)), uint8_t *data __attribute__((unused)), size_t len __attribute__((unused)), bool final __attribute__((unused))){}
};


class HttpOverAsyncWebSocketServer{
public:
    HttpOverAsyncWebSocketServer();
    ~HttpOverAsyncWebSocketServer();
    void setup(AsyncWebSocket& ws);
    void addHandler(HttpOverAsyncWebSocketHandler *handler);
    
    HttpOverAsyncWebSocketHandler* findHandler(HttpOverAsyncWebSocketClient*);
    
    void boradcast(HttpOverAsyncWebSocketResponse* response);
    AsyncWebSocket* webSocket(void){ return _webSocket;}

protected:
    LinkedList<HttpOverAsyncWebSocketClient *> _clients;
    LinkedList<HttpOverAsyncWebSocketHandler *> _handlers;
    AsyncWebSocket* _webSocket;

    void _wsEventHandler(AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
    void _addClient(AsyncWebSocketClient * client);
    void _removeClient(AsyncWebSocketClient * client);
    void _rcvData(AsyncWebSocketClient * client,uint8_t *data, size_t len,size_t index, size_t total,bool final);
};

typedef enum _HttpOverAsyncWebSocketParseState{
ParseStateNull,
ParseStateRequest,
ParseStateHeader,
ParseStateBody,
ParseStateError
}HttpOverAsyncWebSocketParseState;

class HttpOverAsyncWebSocketClient{
public:
    HttpOverAsyncWebSocketClient(AsyncWebSocketClient *client,HttpOverAsyncWebSocketServer *server);
    ~HttpOverAsyncWebSocketClient();
    bool isEqualClient(AsyncWebSocketClient *client){ return _client->id() == client->id();}
    
    bool onData(uint8_t *data, size_t len,size_t index, size_t total,bool final);

    WebRequestMethod method(){ return _method;}

    // interface to minic ESPAsyncWebRequest for minimum modification
    bool hasParam(const String& name, bool post=false, bool file=false) const;
    bool hasParam(const __FlashStringHelper * data, bool post=false, bool file=false) const;

    AsyncWebParameter* getParam(const String& name, bool post=false, bool file=false) const;
    AsyncWebParameter* getParam(const __FlashStringHelper * data, bool post, bool file) const; 
    bool hasHeader(const String& name) const;
    AsyncWebHeader* getHeader(const String& name) const;

    void send(int code,const String& contextType=String(),const String& data=String());
    void send(HttpOverAsyncWebSocketResponse* response);
    HttpOverAsyncWebSocketResponse* beginResponse(const String& contentType, size_t len, HoawsResponseFiller callback);
/* these are not used
    const String& arg(const String& name) const; // get request argument value by name
    const String& arg(const __FlashStringHelper * data) const; // get request argument value by F(name)    
    bool hasArg(const char* name) const;         // check if argument exists
    bool hasArg(const __FlashStringHelper * data) const;         // check if F(argument) exists
*/

    void sendRawText(String& data);
    const String& url(){ return _path;}
protected:
    AsyncWebSocketClient* _client;
    HttpOverAsyncWebSocketServer *_server;
    HttpOverAsyncWebSocketHandler *_handler;
    HttpOverAsyncWebSocketParseState _state;
    HttpOverAsyncWebSocketResponse *_downloading;
    WebRequestMethod _method;
    String _path;
    String _contentType;

    LinkedList<AsyncWebHeader *> _headers;
    LinkedList<AsyncWebParameter *> _params;

    uint8_t *_packet;
    uint32_t _dataLen;

    bool _parse(uint8_t *data, size_t len);
    bool _parseBody(uint8_t *data, size_t len);
    void _addHeader(const String& name,const String& value);
    void _parseGetQuery(const String& query);
    void _parsePostVars(uint8_t* data, size_t len);
    void _parseQueryString(bool isPost,uint8_t* data, size_t len);
    void _clearParseState(void);
    bool _sendDataChunk(size_t index,size_t size);
};



class HttpOverAsyncWebSocketResponse {
  protected:
    int _code;
    LinkedList<AsyncWebHeader *> _headers;
    String _contentType;
    String _body;
    String _path;
    size_t _contentLength;
    HoawsResponseFiller _filler;
  public:
    HttpOverAsyncWebSocketResponse(const String& path,int code,const String& contentType=String(),const String& data=String());
    HttpOverAsyncWebSocketResponse(const String& path,const String& contentType,size_t size,HoawsResponseFiller datafiller);

    ~HttpOverAsyncWebSocketResponse();
    void addHeader(const String& name, const String& value);
    bool isSimpleText(void);
    const String& path(void) { return _path;}

    String getResponseString(void);

    size_t dataLeft(size_t offset){ return _contentLength - offset;}
    size_t contentLength(void){ return _contentLength;}
    size_t readData(uint8_t *data, size_t index,size_t len);
};


#endif