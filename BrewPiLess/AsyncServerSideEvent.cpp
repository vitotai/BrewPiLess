
#include "AsyncServerSideEvent.h"

AsyncServerSideEventServer::AsyncServerSideEventServer(String url)
  :_url(url)
  ,_clients(NULL)
  ,_cNextId(0)
{
	_eventHandler = NULL;
}


void AsyncServerSideEventServer::_addClient(AsyncServerSideEventClient * client){
	if(_clients == NULL){
    	_clients = client;
	}else {
  		AsyncServerSideEventClient * c = _clients;
  		while(c->next != NULL) c = c->next;
  		c->next = client;
  	}
  	if(_eventHandler != NULL){
	    _eventHandler(this, client, SseEventConnected);
	}
}

void AsyncServerSideEventServer::_handleDisconnect(AsyncServerSideEventClient * client){
	if(_clients == NULL){
    	os_printf("we have no clients to disconnect!");
    	return;
  	}
  	if(_clients->id() == client->id()){
    	_clients = client->next;
  	}else{
  		AsyncServerSideEventClient * c = _clients;
  		while(c->next != NULL && c->next->id() != client->id()) c = c->next;
  		if(c->next == NULL){
    		os_printf("we could not find client [%u] to disconnect!", client->id());
    		return;
  		}
  		c->next = client->next;
  	}
  	
  	delete client;
	
	if(_eventHandler != NULL){
	    _eventHandler(this, client, SseEventDisconnected);
	}
}

void AsyncServerSideEventServer::broadcastData(const char* message)
{
	size_t len=strlen(message);

	AsyncServerSideEventClient* c = _clients;
  	while(c != NULL){
      	c->sendData(message,len);
    	c = c->next;
  	}	
}

void AsyncServerSideEventServer::broadcastData(String& message)
{
	AsyncServerSideEventClient* c = _clients;
  	while(c != NULL){
      	c->sendData(message);
    	c = c->next;
  	}	
}


void AsyncServerSideEventServer::broadcast(const char * message, size_t len){
	AsyncServerSideEventClient* c = _clients;
  	while(c != NULL){
      	c->send(message, len);
    	c = c->next;
  	}
}

bool AsyncServerSideEventServer::canHandle(AsyncWebServerRequest *request){
	if(request->method() != HTTP_GET || !request->url().equals(_url))
    	return false;
	return true;
}

void AsyncServerSideEventServer::handleRequest(AsyncWebServerRequest *request){
	AsyncServerSideEventResponse *response = new AsyncServerSideEventResponse(this);
	request->send(response);
}

/*
*  Server Side Event connected client
*/

AsyncServerSideEventClient::AsyncServerSideEventClient(AsyncWebServerRequest *request, AsyncServerSideEventServer *server)
{
	next = NULL;
  	// get the tcp client
  	_client = request->client();
  	_server = server;
  	_clientId = _server->_getNextId();
	delete request;
  	_wptr = _rptr=0;
  
  	_client->onError([](void *r, AsyncClient* c, int8_t error){ ((AsyncServerSideEventClient*)(r))->_onError(error); }, this);
  	_client->onAck([](void *r, AsyncClient* c, size_t len, uint32_t time){ ((AsyncServerSideEventClient*)(r))->_onAck(len, time); }, this);
  	_client->onDisconnect([](void *r, AsyncClient* c){ ((AsyncServerSideEventClient*)(r))->_onDisconnect(); }, this);
  	_client->onTimeout([](void *r, AsyncClient* c, uint32_t time){ ((AsyncServerSideEventClient*)(r))->_onTimeout(time); }, this);
  	_client->onData([](void *r, AsyncClient* c, void *buf, size_t len){ ((AsyncServerSideEventClient*)(r))->_onData(buf, len); }, this);
  	_client->onPoll([](void *r, AsyncClient* c){ ((AsyncServerSideEventClient*)(r))->_onPoll(); }, this);
  	
  	_server->_addClient(this);
}

// TCP callbacks

void AsyncServerSideEventClient::_onAck(size_t len, uint32_t time){
	_rptr += len;
	if(_rptr >= ServerSideEventClientBufferSize) _rptr -= ServerSideEventClientBufferSize;
	
	_sendBufferedData();
}

void AsyncServerSideEventClient::_onError(int8_t){
	// _onDisconnect should be called. later
}

void AsyncServerSideEventClient::_onTimeout(uint32_t time){
	os_printf("_onTimeout: %u, state: %s\n", time, _client->stateToString());
	_client->close();
	// _onDisconnect should be called. later
}

void AsyncServerSideEventClient::_onDisconnect(){
	_client->free();
	delete _client;
	_server->_handleDisconnect(this);
}

void AsyncServerSideEventClient::_onData(void *buf, size_t plen){
	// it is not supposed to happen for server side event
}

void AsyncServerSideEventClient::_onPoll(){
  if(_client->canSend()){
    _sendBufferedData();
  }
}

// data handling
// 
void AsyncServerSideEventClient::_sendBufferedData(void){
	if(_wptr == _rptr) return; // empty

	// moveing the reading pointer (_rptr) when ack'ed
	size_t readPtr = _rptr;
	size_t spaceAvail = _client->space();
	size_t allDataToSent;
	size_t dataToSent;
	
	do{	
		if(_wptr > readPtr){
			dataToSent=allDataToSent = _wptr - readPtr;
		}else{
			// _wptr is wrapped and is "before" _rptr
			allDataToSent =  _wptr + (ServerSideEventClientBufferSize - readPtr);
			dataToSent = ServerSideEventClientBufferSize - readPtr;
			
		}
		
		if (dataToSent > spaceAvail) dataToSent=spaceAvail;
		
		_client->write((const char*)(_wbuffer + readPtr), dataToSent);
		
		readPtr += dataToSent;
		if(readPtr == ServerSideEventClientBufferSize) readPtr = 0;
		
		allDataToSent -= dataToSent;
		spaceAvail  -= dataToSent;

	}while(spaceAvail > 0 && allDataToSent >0); 
}

size_t  AsyncServerSideEventClient::_bufferSpace(void){
	return ServerSideEventClientBufferSize - 1 -(_wptr - _rptr);
}

void AsyncServerSideEventClient::sendData(const char * message, size_t len)
{
	_copyDataToBuffer("data:",5);
	_copyDataToBuffer(message,len);
	_copyDataToBuffer("\n\n",2);
	_sendBufferedData();
}

void AsyncServerSideEventClient::sendData(String& message)
{
	sendData(message.c_str(),message.length());
}

void AsyncServerSideEventClient::send(const char * message, size_t len)
{
	_copyDataToBuffer(message,len);
  	_sendBufferedData();
}

void AsyncServerSideEventClient::_copyDataToBuffer(const char * message, size_t len){
	const char *ptr = message;
	size_t space= _bufferSpace();
	size_t lenToWrite=(len > space)? space:len;
  	
  	do{
  		if (_wptr < _rptr)
  		{
			// 111000011
  		
  			memcpy((void*)(_wbuffer + _wptr),ptr,lenToWrite);
  			_wptr += lenToWrite;
  			lenToWrite=0;
  		}
  		else
  		{
			// 00011110, 0000x0000  x00000000
			size_t cspace = ServerSideEventClientBufferSize - _wptr;
			// in case of both wptr & rptr equals to zero. lentoWrite should be
			// total space -1, if it is greater than total space -1
			size_t cwrite = (lenToWrite > cspace)? cspace:lenToWrite;
		
			memcpy(_wbuffer + _wptr,ptr,cwrite);
			ptr += cwrite;

			_wptr += cwrite;
			if(_wptr == ServerSideEventClientBufferSize) _wptr=0;
			lenToWrite -= cwrite;
  		}
  	}while(lenToWrite > 0);
}



/*
*  AsyncSseResponse is used to response request, and create a Client
*  to hold the TCP object
*/
AsyncServerSideEventResponse::AsyncServerSideEventResponse(AsyncServerSideEventServer *server)
{
	_server = server;
   	_code = 200;
    _sendContentLength = false;
  	_contentType = "text/event-stream";
  	addHeader("Cache-Control","no-cache");
}


void AsyncServerSideEventResponse::_respond(AsyncWebServerRequest *request)
{
  	if(_state == RESPONSE_FAILED){
    	request->client()->close(true);
    	return;
  	}
 	String out = _assembleHead(request->version());
  	request->client()->write(out.c_str(), _headLength);
  	_state = RESPONSE_WAIT_ACK;
}

size_t AsyncServerSideEventResponse::_ack(AsyncWebServerRequest *request, size_t len, uint32_t time)
{
	if(len){
  		// create a client object to take over the TCP connection. 
    	new AsyncServerSideEventClient(request, _server);
  	}
  	return 0;
}






















































































































































































































































































































































