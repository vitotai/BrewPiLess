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
        HOAWS_PRINTF("Add client:%u\n",client->id());
  	} else if(type == WS_EVT_DISCONNECT){
        // remove the client
        _removeClient(client);
        HOAWS_PRINTF("Remove client:%u\n",client->id());
  	} else if(type == WS_EVT_ERROR){
  	} else if(type == WS_EVT_PONG){
  	} else if(type == WS_EVT_DATA){
    	AwsFrameInfo * info = (AwsFrameInfo*)arg;
		HOAWS_PRINTF("RCV: len:%u  info->len:%u, final:%d\n",len,info->len,info->final!=0);
        _rcvData(client,data,len, info->final);
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
            return h;
        }
    }
    return NULL;
}

void HttpOverAsyncWebSocketServer::boradcast(HttpOverAsyncWebSocketResponse* response){
    String msg;
    response->getResponseString(msg);

    for(const auto& h: _clients){
        h->sendRawText(msg);
    }

    delete response;
}
