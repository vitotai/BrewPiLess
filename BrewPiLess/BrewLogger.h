#ifndef BrewLogger_H
#define BrewLogger_H
#include <FS.h>

#include "TimeKeeper.h"

#define INVALID_RECOVERY_TIME 0xFF

#define LOG_PATH "/log"
#define LOG_RECORD_FILE "/loginfo"
#define MAX_FILE_NUMBER 10

#define LogBufferSize 1024

#define EventTag 0xF2

#define StageTag 0xF1
#define SetPointTag 0xF3

#define BeerSetPointTag 0xF7
#define GravityTag 0xF8
#define AuxTempTag 0xF9

#define ModeTag 0xF4


#define StartLogTag 0xFF
#define ResumeBrewTag 0xFE

#define VolatileHeaderSize 16

typedef struct _FileIndexEntry{
	char name[24];
	unsigned long time;
} FileIndexEntry;

typedef struct _FileIndexes
{
	FileIndexEntry files[MAX_FILE_NUMBER];
	char logname[24];
	unsigned long starttime;
} FileIndexes;
extern BrewPiProxy brewPi;
class BrewLogger
{
	
public:
	BrewLogger(void){
		_recording=false;
		_isFileOpen=false;
		_fsspace=0;
		_tempLogPeriod=60000;
	}

	void begin(void)
	{
		loadIdxFile();
		checkspace();
		if(_fileInfo.logname[0]!='\0'){
			resumeSession();
		}else{
			startVolatileLog();
		}
	}

	String fsinfo(void)
	{
		FSInfo fs_info;
		SPIFFS.info(fs_info);
		String ret=String("{\"size\":") + String(fs_info.totalBytes)
			+",\"used\":"  + String(fs_info.usedBytes)
			+",\"block\":" + String(fs_info.blockSize) 
			+",\"page\":"  + String(fs_info.pageSize)
			+"}";
		return ret;
	}
	
	String loggingStatus(void)
	{
		// populate JS
		String ret=String("{\"rec\":");
		if(_fileInfo.logname[0] != NULL){
			ret += "1, \"log\":\"" + String(_fileInfo.logname) 
				+"\",\"start\":" + String(_fileInfo.starttime);
		}else{
			ret += "0";
		}
		ret += ",\"fs\":" + fsinfo();
		ret += ",\"list\":[";
	
		for(int i=0;i<MAX_FILE_NUMBER;i++){
			if(_fileInfo.files[i].name[0] == NULL) break;
			if(i!=0) ret +=",";
			ret +="{\"name\":\"" + String(_fileInfo.files[i].name);
			ret +="\",\"time\":" +String(_fileInfo.files[i].time) +"}";
		}
		ret += "]}";

		return ret;
	}
	
	void rmLog(int index)
	{
		//TODO: race condition
		// multiple access issue
		char buff[36];
		sprintf(buff,"%s/%s",LOG_PATH,_fileInfo.files[index].name);
		SPIFFS.remove(buff);
		DBG_PRINTF("remove %d: %s\n",index,buff);
		int i;
		for(i=index+1;i<MAX_FILE_NUMBER;i++){
			if(_fileInfo.files[i].name[0]=='\0') break;
			DBG_PRINTF("move %d: %s\n",i,_fileInfo.files[i].name);
			strcpy(_fileInfo.files[i-1].name,_fileInfo.files[i].name);
			_fileInfo.files[i-1].time=_fileInfo.files[i].time;
		}
		_fileInfo.files[i-1].name[0]='\0';
		_fileInfo.files[i-1].time =0;

		checkspace();
		saveIdxFile();
	}

	void resumeSession()
	{
		char buff[36];
		sprintf(buff,"%s/%s",LOG_PATH,_fileInfo.logname);

		_logFile=SPIFFS.open(buff,"a+");
		
		size_t fsize= _logFile.size();

		_logIndex = fsize % LogBufferSize;
		_savedLength=fsize - _logIndex;
		_logFile.seek(_savedLength,SeekSet);
		_logFile.readBytes(_logBuffer,_logIndex);
		// log a "new start" log
		DBG_PRINTF("resume, total _savedLength:%d, _logIndex:%d\n",_savedLength,_logIndex);
		
		_lastTempLog=0;
		_recording = true;
		_isFileOpen=false;

		char unit;
		brewPi.getLogInfo(&unit,&_mode,&_state);

		// add resume tag
		addResumeTag();
		//DBG_PRINTF("resume done _savedLength:%d, _logIndex:%d\n",_savedLength,_logIndex);
	}
	
	bool isLogging(void){ return _recording; }
	
	bool startSession(const char *filename){
		if(_recording) return false; // alread start
		
		if(_fsspace < 100){
			DBG_PRINTF("Not enough space:%d\n",_fsspace);
			return false;
		}
		strcpy(_fileInfo.logname,filename);
		char buff[36];
		sprintf(buff,"%s/%s",LOG_PATH,filename);

		_logFile=SPIFFS.open(buff,"w");
		
		if(!_logFile){
			DBG_PRINTF("Error open temp file\n");
			return false;
		}
		
		_fileInfo.starttime= TimeKeeper.getTimeSeconds();
		_logIndex = 0;

		_lastTempLog=0;
		_recording = true;
		_savedLength=0;
		
		char unit;
		brewPi.getLogInfo(&unit,&_mode,&_state);
		startLog(unit == 'F');
		addMode(_mode);
		addState(_state);
		addBeerSetPoint(_beerSet);
		
		loop(); // get once
		
		saveIdxFile();
		return true;
	}

	void endSession(void){
		if(!_recording) return;
		_recording=false;
		_logFile.close();
		// copy the file name into last entry
		int index=0;
		for(;index<MAX_FILE_NUMBER;index++)
		{
			if(_fileInfo.files[index].name[0] == NULL) break;
		}
		// exceptional case. 
		if(index == MAX_FILE_NUMBER){
			rmLog(0);
			index = MAX_FILE_NUMBER-1;
		}
		strcpy(_fileInfo.files[index].name,_fileInfo.logname);
		_fileInfo.files[index].time = _fileInfo.starttime;
		_fileInfo.logname[0]='\0';
		_fileInfo.starttime=0;
		saveIdxFile();
		
		startVolatileLog();
	}
	
	void loop(void){
		//if(!_recording) return;

		unsigned long miliseconds = millis();

		if((miliseconds -_lastTempLog) < _tempLogPeriod) return;
		
		uint8_t state, mode;
		float beerSet,fridgeSet;
		float beerTemp,fridgeTemp,roomTemp;

		brewPi.getAllStatus(&state,&mode,& beerTemp,& beerSet,& fridgeTemp,& fridgeSet,& roomTemp);
		
		if(beerSet != _beerSet){
			_beerSet = beerSet;
			addBeerSetPoint(_beerSet);
		}
		if(mode != _mode){
			_mode = mode;
			addMode(mode);
		}

		if(state != _state){
			_state = state;
			addState(state);
		}
		
		addTemperature(beerTemp);
		addTemperature(fridgeTemp);
		addTemperature(fridgeSet);
		addTemperature(roomTemp);

		_lastTempLog= miliseconds;
//		DBG_PRINTF("room sensor connected: %d\n", brewPi.ambientSensorConnected());
		
	}


	size_t beginCopyAfter(size_t last)
	{
		if(!_recording) return 0;
		_readStart = last;
		DBG_PRINTF("beginCopyAfter:%d, _logIndex=%ld, saved=%ld last >= (_logIndex +_savedLength)=%c\n",last,_logIndex,_savedLength, (last >= (_logIndex +_savedLength))? 'Y':'N' );
		if(last >= (_logIndex +_savedLength)) return 0;
		return ( _logIndex+_savedLength - last);
	}
	
	size_t read(uint8_t *buffer, size_t maxLen, size_t index)
	{
		size_t sizeRead;
		size_t rindex= index + _readStart;
		DBG_PRINTF("read index:%d, rindex=%ld\n",index,rindex);
		
		if(rindex > (_savedLength +_logIndex)) return 0;
		
		if( rindex < _savedLength){
			//DBG_PRINTF("read from file\n");
			// read from file
			if(!_isFileOpen){
				char buff[36];
				sprintf(buff,"%s/%s",LOG_PATH,_fileInfo.logname);
				_file=SPIFFS.open(buff,"r");
				
				DBG_PRINTF("Open file:%s\n",buff);

				if(!_file){
					DBG_PRINTF("error open file\n");
					return 0;
				}
				_isFileOpen=true;
				DBG_PRINTF("file opened.\n");
//				if(rindex !=0) _file.seek(rindex,SeekSet);
			}
			sizeRead = _savedLength - rindex;
			if(sizeRead > maxLen) sizeRead=maxLen;

			_file.seek(rindex,SeekSet);
			sizeRead=_file.read(buffer,sizeRead);
			if(sizeRead ==0){
				DBG_PRINTF("!!!!file read error!!!!!!\n");
				_file.close();
				_isFileOpen=false;
				// read will be called again if return length is zero.
			}
			
		}else{
			DBG_PRINTF("read from buffer\n");
			// read from buffer
			rindex -=  _savedLength;
			// rindex should be smaller than _logIndex
			sizeRead = _logIndex - rindex;
			if(sizeRead > maxLen) sizeRead=maxLen;
			memcpy(buffer,_logBuffer+rindex,sizeRead);
		}
/*
			if((sizeRead+ rindex) >= _savedLength ){
				_file.close();
				_isFileOpen=false;
//				DBG_PRINTF("file closed.\n");
			}*/

		DBG_PRINTF("sizeRead:%ld\n",sizeRead);
		return sizeRead;
	}
	
	void getFilePath(char* buf,int index)
	{
		sprintf(buf,"%s/%s",LOG_PATH,_fileInfo.files[index].name);
	}
	
	size_t volatileDataOffset(void)
	{
		return _startOffset;
	}
	
	size_t volatileDataAvailable(size_t start,size_t offset)
	{
		// get size;
		size_t dataAvail=(_logHead <= _logIndex)? (_logIndex-_logHead):(LogBufferSize + _logIndex - _logHead);
		dataAvail += VolatileHeaderSize; // for make-up header
		//DBG_PRINTF("volatileDataAvailable,start:%d, offset:%d, _logHead %d _logIndex %d, _startOffset:%d, dataAvail:%d\n",start, offset,_logHead,_logIndex,_startOffset, dataAvail);
		if( ((start + offset) == 0) 
			|| ((start + offset) < _startOffset)   // error case
		    || ((start + offset) > (_startOffset + dataAvail))) {  //error case
			// force reload, start=offset=0, the same case
			// send header?
			_sendHeader=true;
			_sendOffset=0;
		}else{
			
			size_t d= _startOffset + dataAvail - (start + offset);
			dataAvail= d;
			// assume the header should already be sent.
			_sendHeader=false;
			_sendOffset=start + offset - _startOffset - VolatileHeaderSize + _logHead;
			if(_sendOffset > LogBufferSize) _sendOffset -= LogBufferSize;
			
			//DBG_PRINTF("prepare send from %d of %d\n",_sendOffset,d);
		}
		
		return dataAvail;
	}
	
	size_t readVolatileData(uint8_t *buffer, size_t maxLen, size_t index)
	{
		size_t bufIdx=0;
		size_t readIdx;
		if(_sendHeader){
			if(index < VolatileHeaderSize){
				// maxLen < VolatileHeaderSize?
				char header[VolatileHeaderSize];
				volatileHeader(header);
				for(int i=index;bufIdx<maxLen && i<VolatileHeaderSize;i++)
					buffer[bufIdx++]=header[i];
				readIdx = _logHead;
			}else{
				readIdx = index -VolatileHeaderSize;
			}
		}else{
			readIdx = _sendOffset + index;
		}
		
		//DBG_PRINTF("readVolatileData maxLen=%d, index=%d, readIdx=%d\n",maxLen,index,readIdx);
		
		while(bufIdx < maxLen){
			if(readIdx >= LogBufferSize) readIdx -= LogBufferSize;
			buffer[bufIdx++] = _logBuffer[readIdx++];
		}
		
		return bufIdx;
	}

	void addGravity(float gravity,bool isOg=false){
		int idx = allocByte(4);
		if(idx < 0) return;
		writeBuffer(idx,GravityTag);
		writeBuffer(idx+1,(isOg)? 1:0);
		
		uint16_t val= 1000 +(int16_t)(( gravity - 1.0) * 1000.0); 
		val = val | 0x8000;
		writeBuffer(idx+2,val >> 8);
		writeBuffer(idx+3,val & 0xFF);
		commitData(idx,4);
	}
	void addAuxTemp(float temp){
		int idx = allocByte(4);
		if(idx < 0) return;
		writeBuffer(idx,AuxTempTag);
		writeBuffer(idx+1,0);
		int spi;
		if(temp > 250 || temp < -100.0 )
			spi = 0x7FFF;
		else
			spi=(int) (temp * 100.0);
		spi = spi | 0x8000;
		writeBuffer(idx+2,spi >> 8);//*(ptr+2) =(char) (spi >> 8);
		writeBuffer(idx+3,spi & 0xFF);//*(ptr+3) =(char)(spi & 0xFF);
		commitData(idx,4);
	}

private:
	size_t _fsspace;
	uint32_t  _tempLogPeriod;
	uint32_t _lastTempLog;

	bool _recording;

	int _logIndex;
	int _savedLength;
	int _readStart;	
	char _logBuffer[LogBufferSize];
	File _file;
	bool _isFileOpen;

	File    _logFile;
	
	// brewpi specific info
	uint8_t _mode;
	uint8_t _state;
	float _beerSet;
	
	// for circular buffer
	int _logHead;
	uint32_t _headTime;
	uint32_t _startOffset;
	bool _sendHeader;
	uint32_t _sendOffset;
	
	void checkspace(void)
	{
		FSInfo fs_info;
		SPIFFS.info(fs_info);
		
		_fsspace = fs_info.totalBytes - fs_info.usedBytes;
		if(_fsspace > fs_info.blockSize * 2){
			_fsspace -= fs_info.blockSize * 2;
		}else{
			_fsspace=0;
		}
		DBG_PRINTF("SPIFFS space:%d\n",_fsspace);
	}

	void volatileHeader(char *buf)
	{
		char unit;
		uint8_t mode,state;

		brewPi.getLogInfo(&unit,&mode,&state);
		bool fahrenheit=(unit == 'F');
		
		char* ptr=buf;

		//8
		*ptr++ = StartLogTag;
		*ptr++ = 4 | (fahrenheit? 0xF0:0xE0) ;
		int period = _tempLogPeriod/1000;
		*ptr++ = (char) (period >> 8);
		*ptr++ = (char) (period & 0xFF);
		*ptr++ = (char) (_headTime >> 24);
		*ptr++ = (char) (_headTime >> 16);
		*ptr++ = (char) (_headTime >> 8);
		*ptr++ = (char) (_headTime & 0xFF);
		// mode : 2
		*ptr++ = ModeTag;
		*ptr++ = mode;
		// state: 2
		*ptr++ = StageTag;
		*ptr++ = state;
		// beer set: 4
		*ptr++ =BeerSetPointTag;
		*ptr++ =0;
		
		int spi;
		if(_beerSet > 250 || _beerSet < -100.0 )
			spi = 0x7FFF;
		else
			spi=(int) (_beerSet * 100.0);
			
		spi = spi | 0x8000;
		*ptr++ =(char) (spi >> 8);
		*ptr ++ =(char)(spi & 0xFF);

	}
	
	void startLog(bool fahrenheit)
	{
		char *ptr=_logBuffer;
		// F0FF  peroid   4 bytes
		// Start system time 4bytes
		*ptr++ = StartLogTag;
		*ptr++ = 4 | (fahrenheit? 0xF0:0xE0) ;
		int period = _tempLogPeriod/1000;
		*ptr++ = (char) (period >> 8);
		*ptr++ = (char) (period & 0xFF);
		*ptr++ = (char) (_fileInfo.starttime >> 24);
		*ptr++ = (char) (_fileInfo.starttime >> 16);
		*ptr++ = (char) (_fileInfo.starttime >> 8);
		*ptr++ = (char) (_fileInfo.starttime & 0xFF);
		_logIndex=8;
		_isFileOpen=false;
		_savedLength=0;
		
		commitData(0,_logIndex);
		
		//DBG_PRINTF("*startLog*\n");
	}
	
	void startVolatileLog(void)
	{
		_headTime=TimeKeeper.getTimeSeconds();
		_logHead = 0;
		_logIndex = 0;
		_startOffset=0;
		_lastTempLog=0;
		loop();
	}
	
	
	
	int freeBufferSpace(void)
	{
		//DBG_PRINTF("_logHead:%d, _logIndex: %d\n",_logHead,_logIndex);
		if(_logIndex >= _logHead){
			return LogBufferSize - _logIndex -1 + _logHead;
		}else {
			// _logIndex < _logHead
			return _logHead - _logIndex - 1;
		}
	}
	
	void dropData(void)
	{
		noInterrupts();
		// move headto nex time stamp.
		// four temperatures in one period
		int idx = _logHead;
		int dataDrop=0;
		int tempRecord=0;
		while(tempRecord < 4){
			if(idx >= LogBufferSize) idx -= LogBufferSize;

			byte data=_logBuffer[idx];

			if(data < 128){
				//DBG_PRINTF("T:%X,%X ",_logBuffer[idx],_logBuffer[idx+1]);
				tempRecord ++;
				idx +=2; // 2 bytes for temp 
				dataDrop +=2;
			}else{
				if(data == BeerSetPointTag || data ==GravityTag  || data == AuxTempTag){
					//DBG_PRINTF("B:%X,%X,%X,%X",_logBuffer[idx],_logBuffer[idx+1],_logBuffer[idx+2],_logBuffer[idx+3]);
					idx +=4;
					dataDrop +=4;
				}else{
					//DBG_PRINTF("S:%X,%X ",_logBuffer[idx],_logBuffer[idx+1]);
					idx +=2;
					dataDrop +=2;
				}
			}
		}
		if(idx >= LogBufferSize) idx -= LogBufferSize;
		_startOffset += dataDrop;
		_logHead = idx;
		_headTime += _tempLogPeriod/1000;
		interrupts();
		//DBG_PRINTF(",drop %d\n",dataDrop);
	}
	
	int volatileLoggingAlloc(int size)
	{
		int space=freeBufferSpace();
;
		while(space < size){
			//DBG_PRINTF("Free %d req: %d\n",space,size);
			dropData();
			space=freeBufferSpace();
		}
		
		return _logIndex;
	}
	
	int allocByte(byte size)
	{
		if(!_recording){
			return volatileLoggingAlloc(size);
		}
		if((_logIndex+size) > LogBufferSize){
			DBG_PRINTF("buffer full, %d + %d >= %d! saved=%d\n",_logIndex,size,LogBufferSize,_savedLength);
				_savedLength += _logIndex;
				_logIndex =0;
		}
		if(size >= _fsspace){
			// run out of space.
			DBG_PRINTF("file system full, space: %d  req %d!\n",_fsspace,size);
			endSession(); // forced stop
			return -1;
		}
		_fsspace -= size;
		
		//race condition: read before data update. _logIndex += size;
		return _logIndex;
	}
	
	void writeBuffer(int idx,uint8_t data)
	{
		if(idx < LogBufferSize){
			_logBuffer[idx] = data;
		}else{
			_logBuffer[idx - LogBufferSize]=data;
		}
	}
	
	void commitData(int idx,int len)
	{
		 _logIndex += len;

		if(!_recording){
			if(_logIndex >= LogBufferSize) 
				_logIndex -= LogBufferSize;
			return;
		}
		char *buf = _logBuffer + idx;
		int wlen;
		if(len !=(wlen=_logFile.write((const uint8_t*)buf,len))){
			DBG_PRINTF("!!!write failed @ %d\n",_logIndex);
			_logFile.close();
			char buff[36];
			sprintf(buff,"%s/%s",LOG_PATH,_fileInfo.logname);

			_logFile=SPIFFS.open(buff,"a+");
			_logFile.write((const uint8_t*)buf+wlen,len-wlen);
		}
		_logFile.flush();
	}
	
	void addBeerSetPoint(float beerset){
		int idx = allocByte(4);
		if(idx < 0) return;
		writeBuffer(idx,BeerSetPointTag);
		writeBuffer(idx+1,0);
		
		int spi;
		if(beerset > 250 || beerset < -100.0 )
			spi = 0x7FFF;
		else
			spi=(int) (beerset * 100.0);
			
		spi = spi | 0x8000;
		writeBuffer(idx+2,spi >> 8);//*(ptr+2) =(char) (spi >> 8);
		writeBuffer(idx+3,spi & 0xFF);//*(ptr+3) =(char)(spi & 0xFF);
		commitData(idx,4);
	}

	void addMode(char mode){
		int idx = allocByte(2);
		if(idx < 0) return;
		writeBuffer(idx,ModeTag); //*ptr = ModeTag;
		writeBuffer(idx+1,mode); //*(ptr+1) = mode;
		commitData(idx,2);
	}

	void addState(char state){
		int idx = allocByte(2);
		if(idx <0) return;
		writeBuffer(idx,StageTag); //*ptr = StageTag;
		writeBuffer(idx+1,state); //*(ptr+1) = state;
		commitData(idx,2);
	}
		
	void addTemperature(float temp){
		int idx = allocByte(2);
		if(idx < 0) return;
		int temp_int=(int)(temp * 100.0);
		// assume temp is smaller than 300, -> maximum temp *100= 30000 < 32767
		//DBG_PRINTF("add temperature:%d\n",temp_int);

		if(temp_int > 25000 || temp < -100.0 ){
			writeBuffer(idx,0x7F);    //*ptr = 0x7F;
			writeBuffer(idx+1,0xFF); //*(ptr+1) = 0xFF;
		}else{
			writeBuffer(idx,(temp_int >> 8) & 0x7F);//*ptr = (char)((temp_int >> 8) & 0x7F);
			writeBuffer(idx+1,temp_int & 0xFF);//*(ptr+1) =(char)(temp_int & 0xFF);		
		}
		commitData(idx,2);
	}


	void addResumeTag(void)
	{
		int idx = allocByte(2);
		if(idx < 0) return;
		writeBuffer(idx,ResumeBrewTag); //*ptr = ResumeBrewTag;
		size_t rtime= TimeKeeper.getTimeSeconds();
		writeBuffer(idx+1,(rtime - _fileInfo.starttime)/60);  //*(ptr+1)=(uint8_t)((rtime - _fileInfo.starttime)/60);
		commitData(idx,2);
	}
		
	FileIndexes _fileInfo;
		
	void loadIdxFile(void)
	{
		// load index
		File idxFile= SPIFFS.open(LOG_RECORD_FILE,"r+");
		if(idxFile){
			idxFile.readBytes((char*)&_fileInfo,sizeof(_fileInfo));
			idxFile.close();
			//DBG_PRINTF("Load index from file\n");
		}else{
			for(uint8_t i=0;i<MAX_FILE_NUMBER;i++){
				_fileInfo.files[i].name[0] = '\0';
				_fileInfo.logname[0]='\0';
				_fileInfo.starttime=0;
			}
		}		
	}
	
	void saveIdxFile(void)
	{
		File idxFile= SPIFFS.open(LOG_RECORD_FILE,"w+");
		if(idxFile){
			idxFile.write((uint8_t*)&_fileInfo,sizeof(_fileInfo));
			idxFile.close();
			//DBG_PRINTF("save file index\n");
		}
	}

};

extern BrewLogger brewLogger;
#endif














































































































































