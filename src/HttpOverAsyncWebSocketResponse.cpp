#include "HttpOverASyncWebSocket.h"

HttpOverAsyncWebSocketResponse::HttpOverAsyncWebSocketResponse(const String& path,int code,const String& contentType,const String& data):
        _headers(LinkedList<AsyncWebHeader *>([](AsyncWebHeader *h){ delete h; })),
        _filler(NULL){
        _path =path;
        _code =code;
        _contentType=contentType;
        _body = data;
        if(contentType.length()){
            addHeader("Content-Type",contentType);
        }
    }

HttpOverAsyncWebSocketResponse::HttpOverAsyncWebSocketResponse(const String& path,const String& contentType,size_t size,HoawsResponseFiller dataFiller):
        HttpOverAsyncWebSocketResponse(path,206,contentType){
        _filler = dataFiller;
        _contentLength=size;
        addHeader("Content-Length",String(_contentLength));
}

bool HttpOverAsyncWebSocketResponse::isSimpleText(void){
    return _code !=206 || _filler == NULL;
}

size_t HttpOverAsyncWebSocketResponse::readData(uint8_t *data, size_t index,size_t len){
    size_t end = index + len;
    size_t expectedLength = (end > _contentLength)? (_contentLength - index):len;

    //HOAWS_PRINTF("readData index:%u len:%d, red:%d\n",index,len, expectedLength);

    _filler(data,expectedLength,index);
    // printout 4 butes
    //HOAWS_PRINTF("readData:%d,%d,%d,%d\n",data[0],data[1],data[2],data[3]);

    return expectedLength;
}


HttpOverAsyncWebSocketResponse::~HttpOverAsyncWebSocketResponse(){
        _headers.free();
}
void HttpOverAsyncWebSocketResponse::addHeader(const String& name, const String& value){
        _headers.add(new AsyncWebHeader(name, value));
}

void HttpOverAsyncWebSocketResponse::getResponseString(String& content){
        content = String(_code) + " " + _path +"\r\n";
        for(const auto& h: _headers){
            content += h->name() +": " + h->value() + "\r\n";
        }
        content += "\r\n" + _body;
        #if SerialDebug
        if(_path != "/pi")
        HOAWS_PRINTF("rsp:%s\n",content.c_str());
        #endif
}
