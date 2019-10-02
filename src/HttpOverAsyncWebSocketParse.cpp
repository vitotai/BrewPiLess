#include "Config.h"
#include "HttpOverAsyncWebSocket.h"


HttpOverAsyncWebSocketClient::HttpOverAsyncWebSocketClient(AsyncWebSocketClient *client,HttpOverAsyncWebSocketServer *server)
:_client(client),
_server(server),
_state(ParseStateNull),
_headers(LinkedList<AsyncWebHeader *>([](AsyncWebHeader *h){ delete h; })),
_params(LinkedList<AsyncWebParameter *>([](AsyncWebParameter *h){ delete h; }))
{}

HttpOverAsyncWebSocketClient::~HttpOverAsyncWebSocketClient(){
    _headers.free();
    _params.free();

}

uint8_t* getStringUntil(String &str,const char delimiter,uint8_t *data, size_t len){
    uint8_t *ptr=data;
    while( *ptr != delimiter && (ptr - data) < len){
        str += *ptr;
        ptr ++;
    }
    return ptr;
}

uint8_t* skipUntil(char delimiter,uint8_t *data, size_t len){
    uint8_t *ptr=data;
    while( *ptr != delimiter && (ptr - data) < len){
        ptr ++;
    }
    return ptr;
}

#define METHOD_GET_STR "GET"
#define METHOD_POST_STR "POST"
#define METHOD_PUT_STR "PUT"
#define METHOD_DELETE_STR "DELETE"

// header must be in one frame only.
bool HttpOverAsyncWebSocketClient::_parseBody(uint8_t *data, size_t len,bool final){

    if(len > 0){
        if(_contentType.startsWith("application/x-www-form-urlencoded")){
            // post
            _parsePostVars(data,len);
        }else{
            _handler->handleBody(this,data,len,final);
        }
    }else{
        // empty body
        DBG_PRINTF("empty BODY\n");
    }
    if(final){
        _state =ParseStateNull;
        _handler->handleRequest(this);
    }else{
        _state = ParseStateBody;
    }
    return true;
}

void HttpOverAsyncWebSocketClient::_clearParseState(void){
    _params.free();
    _headers.free();
}

bool HttpOverAsyncWebSocketClient::parse(uint8_t *data, size_t len,bool final){
 
 
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
    uint8_t *ptr;
    String method="";
    ptr=getStringUntil(method,' ',data,len);
    ptr ++;

    if( method == METHOD_GET_STR) _method = HTTP_GET;
    else if( method == METHOD_POST_STR) _method = HTTP_POST;
    else if( method == METHOD_PUT_STR) _method = HTTP_PUT;
    else if( method == METHOD_DELETE_STR) _method = HTTP_DELETE;
    else {
        DBG_PRINTF("Error: unsupport method:%s\n",method.c_str());
        _state =final? ParseStateNull:ParseStateError;
        return false;
    }

    _path="";
    
    ptr = getStringUntil(_path,' ',ptr,data+len-ptr);

    int query=_path.indexOf('?');
    String queryStr;
    if( query> 0){
        queryStr = _path.substring(query + 1);
        _path = _path.substring(0, query);
    }
    // find handler

    _handler=_server->findHandler(this);
    if(!_handler){
        DBG_PRINTF("Error: unsupport method:%s\n",method.c_str());
        _state =final? ParseStateNull:ParseStateError;
        return false;
    }
    // late proceessing arguments

    if(queryStr.length() > 0) _parseGetQuery(queryStr);

    // skip the HTTP part since we don't care
    ptr= skipUntil('\n',ptr,data-ptr);
    ptr ++;

    // parse headers
    String header;
    while(ptr < (data + len) ){
        header="";
        ptr=getStringUntil(header,'\r',ptr,data+len-ptr);
        if(header == "") break;
        ptr++; // '\n'
        // process header
        int index = header.indexOf(':');
        if(index){
            String name = header.substring(0, index);
            String value = header.substring(index + 2);
            _addHeader(name,value);
        }
    } // while
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
    
    uint8_t *ptr=data;
    uint8_t *nptr;
    String arg;
    for(;;){
        arg="";
        nptr=getStringUntil(arg,'&',ptr,len);
        if(arg.length()>0){
            int index = arg.indexOf('=');
            if(index){
                String name = arg.substring(0, index);
                String value = arg.substring(index + 2);
                _params.add(new AsyncWebParameter(urlDecode(name), urlDecode(value),isPost));
            }            
        }
        nptr++;
        if(nptr >= (ptr +len)){
            // end of string, or
            break;
        }
        ptr=nptr;
    }
}
