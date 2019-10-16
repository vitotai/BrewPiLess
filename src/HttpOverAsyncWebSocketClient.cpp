#include "HttpOverAsyncWebSocket.h"

/* For "real" webserver, requests are concurrent, and so implemented as "request"-based.
*  WebSocket connection is maintained for ONE client and keep alive, so it is implemented as "client"-based.
*/

HttpOverAsyncWebSocketClient::HttpOverAsyncWebSocketClient(AsyncWebSocketClient *client,HttpOverAsyncWebSocketServer *server)
:_client(client),
_server(server),
_state(ParseStateNull),
_downloading(NULL),
_path(),
_contentType(),
_headers(LinkedList<AsyncWebHeader *>([](AsyncWebHeader *h){ delete h; })),
_params(LinkedList<AsyncWebParameter *>([](AsyncWebParameter *h){ delete h; }))
{}

HttpOverAsyncWebSocketClient::~HttpOverAsyncWebSocketClient(){
    _headers.free();
    _params.free();
    if(_downloading) delete _downloading;
}

String getStringUntil(const char delimiter,uint8_t **pptr, size_t len){
    String str;
    uint8_t *ptr=*pptr;
    uint8_t *limit = ptr + len;
    while( *ptr != delimiter && ptr <  limit ){
        str += String((char)*ptr);
        ptr ++;
    }
    *pptr = ptr;
    return str;
}

uint8_t* skipUntil(char delimiter,uint8_t *data, size_t len){
    uint8_t *ptr=data;
    while( *ptr != delimiter && ((size_t) (ptr - data) < len)){
        ptr ++;
    }
    return ptr;
}

// header must be in one frame only.
bool HttpOverAsyncWebSocketClient::_parseBody(uint8_t *data, size_t len,bool final){
    HOAWS_PARSE_PRINTF("_parseBody len:%u, final:%d, content-type:%s\n",len,final,_contentType.c_str());
    // data request can't have body
    if(len > 0){
        if(_contentType.startsWith("application/x-www-form-urlencoded")){
            // post
            //HOAWS_PARSE_PRINTF("parsePost data\n");
            _parsePostVars(data,len);
        }else{
            if(_handler) _handler->handleBody(this,data,len,final);
        }
    }else{
        // empty body
//        HOAWS_PARSE_PRINTF("empty BODY\n");
    }
    if(final){
        HOAWS_PARSE_PRINTF("handleRequest:\n");
        if(_handler) _handler->handleRequest(this);
        _state =ParseStateNull;
    }else{
        _state = ParseStateBody;
    }
    return true;
}

bool HttpOverAsyncWebSocketClient::_sendDataChunk(size_t index,size_t size){

    size_t left = _downloading->dataLeft(index);
    size_t toSend = (left < size)? left:size;

    AsyncWebSocketMessageBuffer * buffer = _server->webSocket()->makeBuffer(toSend);
    if (buffer) {
        size_t dataRead = _downloading->readData(buffer->get(),index,toSend);
        _client->binary(buffer);
        if ((dataRead + index) >= _downloading->contentLength() ){
            // end of transfer
            delete _downloading;
             _downloading = NULL;
            HOAWS_PARSE_PRINTF("Finish downloading.\n");

        }
    }else{
        return false;
    }
    return true;
}

void HttpOverAsyncWebSocketClient::_clearParseState(void){
    _params.free();
    _headers.free();
}

bool HttpOverAsyncWebSocketClient::parse(uint8_t *data, size_t len,bool final){

    #if SerialDebug
    char *pdata = (char*)data;
    String datastr;
    for(size_t i=0;i<len;i++) datastr += String(*pdata++);
    HOAWS_PARSE_PRINTF("WS parse:%s\n",datastr.c_str());
    #endif
 
    if(_state == ParseStateBody){
        return _parseBody(data,len,final);
    }else if(_state == ParseStateError){
        if(final) _state =ParseStateNull;        
        return false;
    }

    //GET /path HTTP/1.1\r\n
    // { headers}
    // Read the first line of HTTP request
    _clearParseState();

    uint8_t* ptr=data;
    String method="";
    method=getStringUntil(' ',&ptr,len);
    ptr ++;

    if( method == "GET"){
        _method = HTTP_GET;
    }else if( method == "POST"){
         _method = HTTP_POST;
    }else if( method == "PUT"){
        _method = HTTP_PUT;
    }else if( method == "DELETE"){
        _method = HTTP_DELETE;
    }else {
        _state =final? ParseStateNull:ParseStateError;
        HOAWS_PARSE_PRINTF("Error: unsupported method:\"%s\"\n",method.c_str());
        send(400);
        return false;
    }
    
    _path = getStringUntil(' ',&ptr,data+len-ptr);

    int query=_path.indexOf('?');
    String queryStr;
    if( query> 0){
        queryStr = _path.substring(query + 1);
        _path = _path.substring(0, query);
    }
    // find handler
    // check path
    bool download=false;
    if(_downloading && _downloading->path() == _path){
        HOAWS_PARSE_PRINTF("Download continue.\n");
        download=true;
    }else{

        _handler=_server->findHandler(this);
        if(!_handler){
            HOAWS_PARSE_PRINTF("Error: unknow handler for:%s\n",_path.c_str());
            _state =final? ParseStateNull:ParseStateError;
            send(404);
            return false;
        }
        // late proceessing arguments
    }
    
    if(queryStr.length() > 0){
        HOAWS_PARSE_PRINTF("QueryString:\"%s\"\n",queryStr.c_str());
        _parseGetQuery(queryStr);
    }

    // skip the HTTP part since we don't care
    ptr= skipUntil('\n',ptr,data-ptr);
    ptr ++;

    // parse headers
    String header;
    while(ptr < (data + len) ){
        header=getStringUntil('\r',&ptr,data+len-ptr);
        
        //HOAWS_PARSE_PRINTF("header:\"%s\"\n",header.c_str());
        ptr+=2; // '\r\n'
        if(header == ""){
            break;
        }
        // process header
        int index = header.indexOf(':');
        if(index){
            String name = header.substring(0, index);
            String value = header.substring(index + 2);
            HOAWS_PARSE_PRINTF("Add header:\"%s\" : \"%s\"\n",name.c_str(),value.c_str());
            _addHeader(name,value);
        }
    } // while
    if(download){
        if(hasHeader("offset") && hasHeader("size")){
            int offset= getHeader("offset")->value().toInt();
            int size = getHeader("size")->value().toInt();
                // allocate buffer
            if(! _sendDataChunk(offset,size)){
                send(500);
            }
        }else{
            // error
            send(400);
        }
        return true;
    }else
        return _parseBody(ptr,data+len-ptr,final);
}

void HttpOverAsyncWebSocketClient::_addHeader(const String& name,const String& value){
    _headers.add(new AsyncWebHeader(name, value));
    if(name.equalsIgnoreCase("Content-Type")){
        _contentType=value;
    }
}

void HttpOverAsyncWebSocketClient::_parseGetQuery(const String& query){
    _parseQueryString(false,(uint8_t*)query.c_str(),query.length());
}
void HttpOverAsyncWebSocketClient::_parsePostVars(uint8_t* data, size_t len){
    _parseQueryString(true,data,len);
}

String urlDecode(const String& text){
  char temp[] = "0x00";
  unsigned int len = text.length();
  unsigned int i = 0;
  String decoded = String();
  decoded.reserve(len); // Allocate the string internal buffer - never longer from source text
  while (i < len){
    char decodedChar;
    char encodedChar = text.charAt(i++);
    if ((encodedChar == '%') && (i + 1 < len)){
      temp[2] = text.charAt(i++);
      temp[3] = text.charAt(i++);
      decodedChar = strtol(temp, NULL, 16);
    } else if (encodedChar == '+') {
      decodedChar = ' ';
    } else {
      decodedChar = encodedChar;  // normal ascii char
    }
    decoded.concat(decodedChar);
  }
  return decoded;
}

void HttpOverAsyncWebSocketClient::_parseQueryString(bool isPost,uint8_t* data, size_t len){
    HOAWS_PARSE_PRINTF("_parseQueryString:%u\n",len);
    uint8_t *ptr=data;
    size_t length=len;
    String arg;
    for(;;){
        arg=getStringUntil('&',&ptr,length);
        if(arg.length()>0){
            int index = arg.indexOf('=');
            if(index){
                String name = arg.substring(0, index);
                String value = arg.substring(index + 1);
                _params.add(new AsyncWebParameter(urlDecode(name), urlDecode(value),isPost));
            }            
        }
        ptr++;
        if(ptr >= (data +len)){
            // end of string, or
            break;
        }
        length = (data +len) - ptr;
        HOAWS_PARSE_PRINTF("_parseQueryString next:%u\n",length);
    }
}


void HttpOverAsyncWebSocketClient::sendRawText(String& data){
    this->_client->text(data);
}

void HttpOverAsyncWebSocketClient::send(int code,const String& contextType,const String& data){
    send(new HttpOverAsyncWebSocketResponse(_path,code,contextType,data));
}

void HttpOverAsyncWebSocketClient::send(HttpOverAsyncWebSocketResponse* response){

    String msg=response->getResponseString();
    _client->text(msg);

    if(response->isSimpleText()){
        delete response;
    }else{
        if(_downloading != NULL){
            HOAWS_PARSE_PRINTF("!!Error non-finished downloading!\n");
            delete _downloading;
        }
        _downloading = response;
        _sendDataChunk(0,MAX_INITIAL_FRAME_SIZE);
        // enter sending binary data mode. 
        // 
    }
}



bool HttpOverAsyncWebSocketClient::hasParam(const String& name, bool post, bool file) const {
  for(const auto& p: _params){
    if(p->name() == name && p->isPost() == post && p->isFile() == file){
      return true;
    }
  }
  return false;
}

bool HttpOverAsyncWebSocketClient::hasParam(const __FlashStringHelper * data, bool post, bool file) const {
  PGM_P p = reinterpret_cast<PGM_P>(data);
  size_t n = strlen_P(p);

  char * name = (char*) malloc(n+1);
  name[n] = 0; 
  if (name) {
    strcpy_P(name,p);    
    bool result = hasParam( name, post, file); 
    free(name); 
    return result; 
  } else {
    return false; 
  }
}

AsyncWebParameter* HttpOverAsyncWebSocketClient::getParam(const String& name, bool post, bool file) const {
  for(const auto& p: _params){
    if(p->name() == name && p->isPost() == post && p->isFile() == file){
      return p;
    }
  }
  return nullptr;
}

AsyncWebParameter* HttpOverAsyncWebSocketClient::getParam(const __FlashStringHelper * data, bool post, bool file) const {
  PGM_P p = reinterpret_cast<PGM_P>(data);
  size_t n = strlen_P(p);
  char * name = (char*) malloc(n+1);
  if (name) {
    strcpy_P(name, p);   
    AsyncWebParameter* result = getParam(name, post, file); 
    free(name); 
    return result; 
  } else {
    return nullptr; 
  }
}


bool HttpOverAsyncWebSocketClient::hasHeader(const String& name) const {
  for(const auto& h: _headers){
    if(h->name().equalsIgnoreCase(name)){
      return true;
    }
  }
  return false;
}

AsyncWebHeader* HttpOverAsyncWebSocketClient::getHeader(const String& name) const {
  for(const auto& h: _headers){
    if(h->name().equalsIgnoreCase(name)){
      return h;
    }
  }
  return nullptr;
}

HttpOverAsyncWebSocketResponse* HttpOverAsyncWebSocketClient::beginResponse(const String& contentType, size_t len, HoawsResponseFiller callback){
    return new HttpOverAsyncWebSocketResponse(_path,contentType,len,callback);
}
