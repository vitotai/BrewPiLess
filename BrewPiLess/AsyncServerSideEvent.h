#ifndef AsyncServerSideEvent_H
#define AsyncServerSideEvent_H
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "espconfig.h"

#if SerialDebug == true
#define DEBUGF(...) DebugPort.printf(__VA_ARGS__)
#else
#define DEBUGF(...) 
#endif

#define ServerSideEventClientBufferSize 2048

class AsyncServerSideEventResponse;
class AsyncServerSideEventResponse;
class AsyncServerSideEventServer;

class AsyncServerSideEventClient
{
protected:
    AsyncClient *_client;
    AsyncServerSideEventServer *_server;
    uint32_t _clientId;
    
    size_t _wptr;
    size_t _rptr;
    uint8_t _wbuffer[ServerSideEventClientBufferSize];

	size_t _bufferSpace(void);    
	void _copyDataToBuffer(const char* data,size_t len);
	void _sendBufferedData(void);
	
public:
	AsyncServerSideEventClient *next;
	AsyncServerSideEventClient(AsyncWebServerRequest *request, AsyncServerSideEventServer *server);
	void send(const char * message, size_t len);
	void sendData(String& message);
	void sendData(const char * message, size_t len);
	void sendData(const char * message){sendData(message,strlen(message));}

	uint32_t id(){ return _clientId;}

    //system callbacks (do not call)
    void _onAck(size_t len, uint32_t time);
    void _onError(int8_t);
    void _onPoll();
    void _onTimeout(uint32_t time);
    void _onDisconnect();
    void _onData(void *buf, size_t plen);	
};

typedef enum _SseEventType{
SseEventConnected,
SseEventDisconnected
}SseEventType;

typedef std::function<void(AsyncServerSideEventServer * server, AsyncServerSideEventClient * client, SseEventType type)> SseEventHandler;

class AsyncServerSideEventServer: public AsyncWebHandler 
{
protected:
	String _url;
	uint32_t _cNextId;
	AsyncServerSideEventClient *_clients;
	
	SseEventHandler _eventHandler;
public:
	AsyncServerSideEventServer(String url);
	void broadcast(const char * message, size_t len);
	void broadcastData(String& message);
	void broadcastData(const char* message);

    //event listener
    void onEvent(SseEventHandler handler){
      _eventHandler = handler;
    }

	// AsyncWebHandler functions
    bool canHandle(AsyncWebServerRequest *request);
	void handleRequest(AsyncWebServerRequest *request);

	// interface for AsyncServerSideEventClient
	void _addClient(AsyncServerSideEventClient* client);
	void _handleDisconnect(AsyncServerSideEventClient* client);
	uint32_t _getNextId(){ return _cNextId++; }

};

//*********
// ServerResponse is used to response the request and 'detach' TCP connection 
// it can be considered as TCP connection delegate which handles TCP events
//*********

class AsyncServerSideEventResponse: public AsyncWebServerResponse {
  private:
    String _content;
    AsyncServerSideEventServer *_server;
  public:
    AsyncServerSideEventResponse(AsyncServerSideEventServer *server);
    
    void _respond(AsyncWebServerRequest *request);
    size_t _ack(AsyncWebServerRequest *request, size_t len, uint32_t time);
    
    bool _sourceValid(){ return true; }
};

#endif






















































































































































































































































































































































