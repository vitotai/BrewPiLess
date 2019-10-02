#include "HttpOverAsyncWebSocket.h"

HttpOverAsyncWebSocketServer::HttpOverAsyncWebSocketServer():
    _clients(LinkedList<HttpOverAsyncWebSocketClient *>([](HttpOverAsyncWebSocketClient *h){ delete h; })),
    _handlers(LinkedList<HttpOverAsyncWebSocketHandler *>([](HttpOverAsyncWebSocketHandler *h){ delete h; }))
{}

HttpOverAsyncWebSocketServer::~HttpOverAsyncWebSocketServer(){
    _clients.free();
    _handlers.free();
}

void HttpOverAsyncWebSocketServer::setup(AsyncWebSocket& ws){
    _webSocket = & ws;
    ws.onEvent([&](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
        this->_wsEventHandler(client,type,arg,data,len);
    });
}

void HttpOverAsyncWebSocketServer::_wsEventHandler(AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
	if(type == WS_EVT_CONNECT){
        // create a client
        _addClient(client);
  	} else if(type == WS_EVT_DISCONNECT){
        // remove the client
        _removeClient(client);
  	} else if(type == WS_EVT_ERROR){
  	} else if(type == WS_EVT_PONG){
  	} else if(type == WS_EVT_DATA){
    	AwsFrameInfo * info = (AwsFrameInfo*)arg;
		
        _rcvData(client,data,len, (info->index + len) == info->len);
    }
}

void HttpOverAsyncWebSocketServer::_addClient(AsyncWebSocketClient * client){
    _clients.add(new HttpOverAsyncWebSocketClient(client,this));
}

void HttpOverAsyncWebSocketServer::_removeClient(AsyncWebSocketClient * client){
    _clients.remove_first([=](HttpOverAsyncWebSocketClient * c){
        return c->isEqualClient(client);
    });
}

void HttpOverAsyncWebSocketServer::_rcvData(AsyncWebSocketClient * client,uint8_t *data, size_t len,bool final){
    for(const auto& c: _clients){
        if(c->isEqualClient(client)){
            c->parse(data,len,final);
            return;
        }
    }
}


void HttpOverAsyncWebSocketServer::addHandler(HttpOverAsyncWebSocketHandler *handler){
    _handlers.add(handler);
}
    
HttpOverAsyncWebSocketHandler* HttpOverAsyncWebSocketServer::findHandler(HttpOverAsyncWebSocketClient* client){

    for(const auto& h: _handlers){
        if (h->canHandle(client)){
            return (HttpOverAsyncWebSocketHandler*)&h;
        }
    }
    return NULL;
}

