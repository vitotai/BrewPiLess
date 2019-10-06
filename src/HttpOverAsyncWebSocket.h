#ifndef HttpOverAsyncWebSocket_H
#define HttpOverAsyncWebSocket_H

#include "ESPAsyncWebServer.h"
#include "AsyncWebSocket.h"

class HttpOverAsyncWebSocketClient;
class HttpOverAsyncWebSocketHandler;
class HttpOverAsyncWebSocketResponse;

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

protected:
    LinkedList<HttpOverAsyncWebSocketClient *> _clients;
    LinkedList<HttpOverAsyncWebSocketHandler *> _handlers;
    AsyncWebSocket* _webSocket;

    void _wsEventHandler(AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
    void _addClient(AsyncWebSocketClient * client);
    void _removeClient(AsyncWebSocketClient * client);
    void _rcvData(AsyncWebSocketClient * client,uint8_t *data, size_t len,bool final);
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
    
    bool parse(uint8_t *data, size_t len,bool final);

    String path(){ return _path;}
    WebRequestMethod method(){ return _method;}

    // interface to minic ESPAsyncWebRequest for minimum modification
    bool hasParam(const String& name, bool post=false, bool file=false) const;
    bool hasParam(const __FlashStringHelper * data, bool post=false, bool file=false) const;

    AsyncWebParameter* getParam(const String& name, bool post=false, bool file=false) const;
    AsyncWebParameter* getParam(const __FlashStringHelper * data, bool post, bool file) const; 

    void send(int code,const String& contextType=String(),const String& data=String());
    void send(HttpOverAsyncWebSocketResponse* response);
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
    
    WebRequestMethod _method;
    String _path;
    String _contentType;

    LinkedList<AsyncWebHeader *> _headers;
    LinkedList<AsyncWebParameter *> _params;

    bool _parseBody(uint8_t *data, size_t len,bool final);
    void _addHeader(const String& name,const String& value);
    void _parseGetQuery(const String& query);
    void _parsePostVars(uint8_t* data, size_t len);
    void _parseQueryString(bool isPost,uint8_t* data, size_t len);
    void _clearParseState(void);
};


class HttpOverAsyncWebSocketResponse {
  protected:
    int _code;
    LinkedList<AsyncWebHeader *> _headers;
    String _contentType;
    String _body;
    String _path;
    size_t _contentLength;
  public:
    HttpOverAsyncWebSocketResponse(const String& path,int code,const String& contentType=String(),const String& data=String()):
        _headers(LinkedList<AsyncWebHeader *>([](AsyncWebHeader *h){ delete h; })){
        _path =path;
        _code =code;
        _contentType=contentType;
        _body = data;
        if(contentType.length()){
            addHeader("Content-Type",contentType);
        }
    }
    ~HttpOverAsyncWebSocketResponse(){
        _headers.free();
    }
    void addHeader(const String& name, const String& value){
        _headers.add(new AsyncWebHeader(name, value));
    }
    void getResponseString(String& content){
        content = String(_code) + " " + _path +"\r\n";
        for(const auto& h: _headers){
            content += h->name() +": " + h->value() + "\r\n";
        }
        content += "\r\n" + _body;
    }

};

#endif