#ifndef BrewLogger_H
#define BrewLogger_H
#include <FS.h>

#include "TimeKeeper.h"

#define INVALID_RECOVERY_TIME 0xFF
#define INVALID_TEMPERATURE -250
#define INVALID_GRAVITY -1

#define LOG_PATH "/log"
#define LOG_RECORD_FILE "/loginfo"
#define MAX_FILE_NUMBER 10

#define LogBufferSize 1024

#define StartLogTag 0xFF
#define ResumeBrewTag 0xFE
#define PeriodTag 0xF0
#define StateTag 0xF1
#define EventTag 0xF2
//#define SetPointTag 0xF3
#define ModeTag 0xF4
//#define BeerSetPointTag 0xF7
#define OriginGravityTag 0xF8

#if BREW_AND_CALIBRATION
#define CalibrationPointTag 0xF9
#endif

#define INVALID_TEMP_INT 0x7FFF
#define INVALID_GRAVITY_INT 0x7FFF

#define VolatileHeaderSize 28

#define OrderBeerSet 0
#define OrderBeerTemp 1
#define OrderFridgeTemp 2
#define OrderFridgeSet 3
#define OrderRoomTemp 4
#define OrderExtTemp 5
#define OrderGravity 6

#define NumberDataBitMask 7

#if BREW_AND_CALIBRATION
#undef NumberDataBitMask
#define NumberDataBitMask 8

#define OrderTiltAngle 7
#define TiltEncode(g) (uint16_t)(100.0 * (g) + 0.5)
#define INVALID_TILT_ANGLE 0x7FFF
#endif

#define GravityEncode(g) (uint16_t)(10000.0 * (g) + 0.5)
#define GravityDecode(a) (float)(a)/10000.0
#define HighOctect(a) (uint8_t)((a)>>8) 
#define LowOctect(a) (uint8_t)((a)&0xFF)

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
		resetTempData();
		#if BREW_AND_CALIBRATION
		_calibrating=false;
		#endif
	}

	
	void begin(void)
	{
    	bool resumeSuccess=false;
		loadIdxFile();
		checkspace();
		if(_fileInfo.logname[0]!='\0'){
			resumeSuccess=resumeSession();
		}

		if(!resumeSuccess){
			DBG_PRINTF("start volatiel log\n");
			logData();
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
		if(_fileInfo.logname[0] != 0){
			ret += "1, \"log\":\"" + String(_fileInfo.logname)
				+"\",\"start\":" + String(_fileInfo.starttime);
		}else{
			ret += "0";
		}
		ret += ",\"fs\":" + fsinfo();
		ret += ",\"list\":[";

		for(int i=0;i<MAX_FILE_NUMBER;i++){
			if(_fileInfo.files[i].name[0] == 0) break;
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

    void initProcessSavedData(void){
        _resumeLastLogTime = _fileInfo.starttime;
    }
    size_t processSavedData(char *buffer,size_t size)
    {
        size_t idx=0;
        uint8_t tag,mask;

        while(idx < size)
        {
            // read tag
			tag =_logBuffer[idx++];
		    mask=_logBuffer[idx++];
			if(tag == PeriodTag){
		        _resumeLastLogTime += _tempLogPeriod/1000;
		        int numberInRecord=0;

		        for(int i=0;i<NumberDataBitMask;i++) if(mask & (1<<i)) numberInRecord +=2;

		        if((numberInRecord + idx) > size){
		            // not enough data for this record!
		            // rewind and return
		            return idx - 2;
		        }else{
		            if(mask & (1<<OrderGravity)){
		                size_t ridx = idx;
        		        for(int i=0;i<NumberDataBitMask;i++){
	        		        if(mask & (1<<i)){
		        		        if( i == OrderGravity){
			        	    	    byte d0=_logBuffer[ridx];
    			        	        byte d1=_logBuffer[ridx+1];
    				                int gravityInt = (d0 << 8) | d1;
                                    DBG_PRINTF("resume@%ld, SG:%d\n",_resumeLastLogTime,gravityInt);
                                    // dont trust the data
                                    if(gravityInt > 8000 && gravityInt < 12500)
                                        gravityTracker.add(GravityDecode(gravityInt),_resumeLastLogTime);
				                } // if this is Gravity data
				                ridx+=2;
			                } // if the field exists
			            } // for each bit
		            } // if gravity data exists

		            idx += numberInRecord;
			    } // else, of data not enough
		    }else if(tag == ResumeBrewTag){
				if((2 + idx) > size){
		            // not enough data for this record!
		            // rewind and return
		            return idx - 2;
				}else{
					size_t d1 =_logBuffer[idx++];
		    		size_t d0 =_logBuffer[idx++];
					size_t tdiff= (mask <<16) + (d1 << 8) + d0;
			    	_resumeLastLogTime = _fileInfo.starttime + tdiff;
				}
			}else if(tag == CalibrationPointTag){
				if((2 + idx) > size){
		            // not enough data for this record!
		            // rewind and return
		            return idx - 2;
				}else{
					idx += 2;
				}
			}else if(tag == StartLogTag){
				_calibrating = (mask & 0x20) ^ 0x20;
			}
        } // while data available
        return idx;
    }
	bool resumeSession()
	{
    	initProcessSavedData();
		char buff[36];
		sprintf(buff,"%s/%s",LOG_PATH,_fileInfo.logname);

		_logFile=SPIFFS.open(buff,"a+");
		if(! _logFile){
            DBG_PRINTF("resume failed\n");
            return false;
		}
		size_t fsize= _logFile.size();
 		
		 DBG_PRINTF("resume file:%s size:%d\n",buff,fsize);

		size_t totalRead=0;
		size_t byteRead;
		size_t processed;
		size_t byteToRead=LogBufferSize;
		size_t left=0;
		char *readPtr=_logBuffer;

		while(totalRead < fsize){
		    byteRead =_logFile.readBytes(readPtr,byteToRead);
		    //process the data
            processed=processSavedData(_logBuffer,byteRead+left);

            if(processed != (byteRead+left)){
                // some data left , copy them to head,
                // don't use memcpy, because it's the same buffer
                // memcpy(_logBuffer, _logBuffer + processed, byteRead- processed);
                char *src=_logBuffer + processed;
                char *dst=_logBuffer;
                left =  (byteRead+left)- processed;
                for(int i=0;i<left;i++) *dst++ = *src++;

	        	byteToRead=LogBufferSize - left;
    		    readPtr=dst;
            }else{
	        	byteToRead=LogBufferSize;
    		    readPtr=_logBuffer;
    		    left=0;
            }
             DBG_PRINTF("Move %d bytes, read:%d, process:%d \n",left,byteRead,processed);
		    //
		    totalRead += byteRead;
		}
		// log a "new start" log
		_logFile.seek(0,SeekEnd);
		_logIndex =0;
		_savedLength = fsize;
		DBG_PRINTF("resume, total _savedLength:%d, _logIndex:%d\n",_savedLength,_logIndex);

		_lastTempLog=0;
		_recording = true;
		_isFileOpen=false;

		char unit;
		brewPi.getLogInfo(&unit,&_mode,&_state);

		// add resume tag
		addResumeTag();
		//DBG_PRINTF("resume done _savedLength:%d, _logIndex:%d\n",_savedLength,_logIndex);
		return true;
	}

	bool isLogging(void){ return _recording; }
	#if BREW_AND_CALIBRATION
	bool startSession(const char *filename,bool calibrating){		
	#else
	bool startSession(const char *filename){
	#endif
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

		#if BREW_AND_CALIBRATION
		startLog(unit == 'F',calibrating);
		_calibrating = calibrating;
		
		#else
		startLog(unit == 'F');		
		#endif
		
		resetTempData();
		loop(); // get once
		addMode(_mode);
		addState(_state);

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
			if(_fileInfo.files[index].name[0] == 0) break;
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
		_lastTempLog = miliseconds;
		logData();
	}

	void logData(void){
		uint8_t state, mode;
		float fTemps[5];

		//brewPi.getAllStatus(&state,&mode,& beerTemp,& beerSet,& fridgeTemp,& fridgeSet,& roomTemp);
		brewPi.getAllStatus(&state,&mode,&fTemps[OrderBeerTemp],& fTemps[OrderBeerSet],
				& fTemps[OrderFridgeTemp],& fTemps[OrderFridgeSet],& fTemps[OrderRoomTemp]);


		uint16_t iTemp;
		uint8_t changeMask=0;
		int   changeNum=0;

		for(int i=0;i<5;i++){
			iTemp=convertTemperature(fTemps[i]);
			if(_iTempData[i] != iTemp){
				changeMask |= (1 << i);
				_iTempData[i] = iTemp;
				changeNum ++;
				//DBG_PRINTF("tempData %i changed:%d\n",i,iTemp);
			}
		}
		if( _extTemp != INVALID_TEMP_INT){
				changeMask |= (1 << OrderExtTemp);
				changeNum ++;
		}

		if( _extGravity != INVALID_GRAVITY_INT){
				changeMask |= (1 << OrderGravity);
				changeNum ++;
		}
		
		#if BREW_AND_CALIBRATION
		if( _extTileAngle != INVALID_TILT_ANGLE){
			changeMask |= (1 << OrderTiltAngle);
			changeNum ++;
		}
		#endif

		int startIdx = allocByte(2+ changeNum * 2);
		if(startIdx < 0) return;
		int idx=startIdx;
		writeBuffer(idx++,PeriodTag);
		writeBuffer(idx++,changeMask);

		for(int i=0;i<5;i++){
			if(changeMask & (1<<i)){

				writeBuffer(idx ++,(_iTempData[i]>> 8) & 0x7F);
				writeBuffer(idx ++,_iTempData[i] & 0xFF);
			}
		}


		if( _extTemp != INVALID_TEMP_INT){
			writeBuffer(idx++,(_extTemp >>8) & 0x7F);
			writeBuffer(idx++,_extTemp & 0xFF);
			_extTemp = INVALID_TEMP_INT;
		}

		if( _extGravity != INVALID_GRAVITY_INT){
			writeBuffer(idx++,(_extGravity >>8) & 0x7F);
			writeBuffer(idx++,_extGravity & 0xFF);
			//DBG_PRINTF("gravity %d: %d %d\n",_extGravity,(_extGravity >>8) & 0x7F,_extGravity & 0xFF);
			_extGravity = INVALID_GRAVITY_INT;
		}

		#if BREW_AND_CALIBRATION
		if( _extTileAngle != INVALID_TILT_ANGLE){
			writeBuffer(idx++,(_extTileAngle >>8) & 0x7F);
			writeBuffer(idx++,_extTileAngle & 0xFF);
			_extTileAngle = INVALID_TILT_ANGLE;
		}
		#endif

		commitData(startIdx,2+ changeNum * 2);

		if(_extOriginGravity != INVALID_GRAVITY_INT){
			addOG(_extOriginGravity);
			_extOriginGravity = INVALID_GRAVITY_INT;
		}

		if(mode != _mode){
			DBG_PRINTF("mode %c => %c\n",_mode,mode);
			_mode = mode;
			addMode(mode);
		}

		if(state != _state){
			DBG_PRINTF("state %d => %d\n",_state,state);
			_state = state;
			addState(state);
		}
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
		DBG_PRINTF("read _readStart =%ld, index:%ld, rindex=%ld\n",_readStart,index,rindex);

		if(rindex > (_savedLength +_logIndex)) return maxLen; // return whatever it wants.

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
			}else{
				if(sizeRead < maxLen && (rindex + sizeRead == _savedLength)){
					// end of file
					size_t request = maxLen - sizeRead;
					memcpy(buffer + sizeRead,_logBuffer,request);
					DBG_PRINTF("from file:%ld from buffer:%ld\n",sizeRead,request);
					sizeRead += request;
				}
			}
		}else{
			//DBG_PRINTF("read from buffer\n");
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

	void addGravity(float gravity,bool isOg=false)
	{
		if(isOg){
			_extOriginGravity = GravityEncode(gravity);
		}else{
			_extGravity=GravityEncode(gravity);
		}
	}

	void addAuxTemp(float temp)
	{
		_extTemp = convertTemperature(temp);
		DBG_PRINTF("AuxTemp:%d\n",_extTemp);
	}
	#if BREW_AND_CALIBRATION
	void addTiltAngle(float tilt)
	{
		_extTileAngle = TiltEncode(tilt);
	}
	void addTiltInWater(float tilt)
	{
		if(!_recording) return;
		int idx = allocByte(4);
		if(idx < 0) return;
		uint16_t angle=TiltEncode(tilt);		
		writeBuffer(idx,CalibrationPointTag);
		writeBuffer(idx+1,0);
		writeBuffer(idx+2,HighOctect(angle));
		writeBuffer(idx+3,LowOctect(angle));
		commitData(idx,4);		
	}
	bool calibrating(void){
		return _calibrating;
	}
	#endif
private:
	size_t _fsspace;
	uint32_t  _tempLogPeriod;
	uint32_t _lastTempLog;
    uint32_t _resumeLastLogTime;

	bool _recording;
	bool _calibrating;

	size_t _logIndex;
	size_t _savedLength;
	size_t _readStart;
	char _logBuffer[LogBufferSize];
	File _file;
	bool _isFileOpen;

	File    _logFile;

	// brewpi specific info
	uint8_t _mode;
	uint8_t _state;

	uint16_t  _iTempData[5];
	uint16_t  _extTemp;
	uint16_t  _extGravity;
	uint16_t  _extOriginGravity;
#if BREW_AND_CALIBRATION
	uint16_t  _extTileAngle;
#endif

	// for circular buffer
	int _logHead;
	uint32_t _headTime;
	uint32_t _startOffset;
	bool _sendHeader;
	uint32_t _sendOffset;
	
	#define VolatileDataHeaderSize 7
	uint16_t  _headData[VolatileDataHeaderSize];

	void resetTempData(void)
	{
		for(int i=0;i<5;i++) _iTempData[i]=INVALID_TEMP_INT;
		_extTemp=INVALID_TEMP_INT;
		_extGravity=INVALID_GRAVITY_INT;
		_extOriginGravity=INVALID_GRAVITY_INT;
		#if BREW_AND_CALIBRATION
		_extTileAngle = INVALID_TILT_ANGLE;
		#endif
	}

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
		uint8_t headerTag=5;
		//8
		*ptr++ = StartLogTag;
		*ptr++ = headerTag | (fahrenheit? 0xF0:0xE0) ;
		int period = _tempLogPeriod/1000;
		*ptr++ = (char) (period >> 8);
		*ptr++ = (char) (period & 0xFF);
		*ptr++ = (char) (_headTime >> 24);
		*ptr++ = (char) (_headTime >> 16);
		*ptr++ = (char) (_headTime >> 8);
		*ptr++ = (char) (_headTime & 0xFF);
		// a record full of all data = 2 + 7 * 2= 16
		*ptr++ = (char) PeriodTag;
		*ptr++ = (char) 0x7F;
		for(int i=0;i<VolatileDataHeaderSize;i++){
			*ptr++ = _headData[i] >> 8;
			*ptr++ = _headData[i] & 0xFF;
		}
		// mode : 2
		*ptr++ = ModeTag;
		*ptr++ = mode;
		// state: 2
		*ptr++ = StateTag;
		*ptr++ = state;
	}
	#if BREW_AND_CALIBRATION
	void startLog(bool fahrenheit,bool calibrating)
	#else
	void startLog(bool fahrenheit)	
	#endif
	{
		char *ptr=_logBuffer;
		// F0FF  peroid   4 bytes
		// Start system time 4bytes
		uint8_t headerTag=5;
		*ptr++ = StartLogTag;

		headerTag = headerTag | (fahrenheit? 0xF0:0xE0) ;

		#if BREW_AND_CALIBRATION
		if(calibrating) headerTag = headerTag ^ 0x20 ;
		#endif

		*ptr++ = headerTag;
		
		int period = _tempLogPeriod/1000;
		*ptr++ = (char) (period >> 8);
		*ptr++ = (char) (period & 0xFF);
		*ptr++ = (char) (_fileInfo.starttime >> 24);
		*ptr++ = (char) (_fileInfo.starttime >> 16);
		*ptr++ = (char) (_fileInfo.starttime >> 8);
		*ptr++ = (char) (_fileInfo.starttime & 0xFF);
		_logIndex=0;
		_isFileOpen=false;
		_savedLength=0;
		commitData(_logIndex,ptr - _logBuffer );

		//DBG_PRINTF("*startLog*\n");
	}

	void startVolatileLog(void)
	{
		DBG_PRINTF("startVolatileLog, mode=%c, beerteemp=%d\n",_mode,_iTempData[OrderBeerTemp]);
		_headTime=TimeKeeper.getTimeSeconds();
		_logHead = 0;
		_logIndex = 0;
		_startOffset=0;
		_lastTempLog=0;
		for(int i=0;i<5;i++) _headData[i]=_iTempData[i];
		_headData[5]= _extTemp;
		_headData[6]= _extGravity;
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
		byte tag;
		byte mask;

		while(1){
			if(idx >= LogBufferSize) idx -= LogBufferSize;
			tag =_logBuffer[idx++];
		    mask=_logBuffer[idx++];
			dataDrop +=2;

			if(tag == PeriodTag) break;

			if(tag == OriginGravityTag || tag == CalibrationPointTag){
    			idx += 2;
	    		dataDrop +=2;
			}
		}


		DBG_PRINTF("before tag %d, mask=%x\n",dataDrop,mask);


		for(int i=0;i<NumberDataBitMask;i++){
			if(mask & (1<<i)){
				if(idx >= LogBufferSize) idx -= LogBufferSize;
				byte d0=_logBuffer[idx++];
				byte d1=_logBuffer[idx++];
				dataDrop +=2;
				_headData[i] = (d0<<8) | d1;
				DBG_PRINTF("update idx:%d to %d\n",i,_headData[i]);
			}
		}
		// drop any F tag
		while(_logBuffer[idx] != PeriodTag ){
			if(idx >= LogBufferSize) idx -= LogBufferSize;
			if(OriginGravityTag == _logBuffer[idx]){
				idx +=4;
				dataDrop +=4;
			}else{
				idx +=2;
				dataDrop +=2;
			}
		}

		if(idx >= LogBufferSize) idx -= LogBufferSize;
		_startOffset += dataDrop;
		_logHead = idx;
		_headTime += _tempLogPeriod/1000;
		interrupts();
		DBG_PRINTF("Drop %d\n",dataDrop);
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
		if(idx + len <= LogBufferSize){
			// continues block
			wlen=_logFile.write((const uint8_t*)buf,len);
		}else{
			wlen=_logFile.write((const uint8_t*)buf,LogBufferSize - idx);
			buf = _logBuffer;
			int nlen = len  + idx -LogBufferSize;
			wlen += _logFile.write((const uint8_t*)buf,nlen);
		}
		/*
		if(len !=(wlen=_logFile.write((const uint8_t*)buf,len))){

			DBG_PRINTF("!!!write failed @ %d\n",_logIndex);
			_logFile.close();
			char buff[36];
			sprintf(buff,"%s/%s",LOG_PATH,_fileInfo.logname);

			_logFile=SPIFFS.open(buff,"a+");
			_logFile.write((const uint8_t*)buf+wlen,len-wlen);
		}*/
		_logFile.flush();
	}

	void addOG(uint16_t og){
		int idx = allocByte(4);
		if(idx < 0) return;
		writeBuffer(idx,OriginGravityTag);
		writeBuffer(idx+1,0);
		writeBuffer(idx+2,(og >> 8) | 0x80); //*ptr = ModeTag;
		writeBuffer(idx+3,og & 0xFF); //*(ptr+1) = mode;
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
		writeBuffer(idx,StateTag); //*ptr = StateTag;
		writeBuffer(idx+1,state); //*(ptr+1) = state;
		commitData(idx,2);
	}

	uint16_t convertTemperature(float temp){
		if(temp >= 225 || temp < -100) 
			return INVALID_TEMP_INT;
		
		int temp_int=(int)(temp * 100.0);
		//DBG_PRINTF("add temperature:%d\n",temp_int);
		// valid temp range, 225 ~ -100 
		// 0 ~ 225: 
		// -100 ~ 0 :  226  - t  , maximum 32500 ( max uint16 32767)  

		uint16_t ret=(temp_int < 0)? (uint16_t)(22500 - temp_int):(uint16_t)temp_int;
		return ret & 0x7FFF;
	}


	void addResumeTag(void)
	{
		int idx = allocByte(4);
		if(idx < 0) return;
		writeBuffer(idx,ResumeBrewTag); //*ptr = ResumeBrewTag;
		size_t rtime= TimeKeeper.getTimeSeconds();
		size_t gap=rtime - _fileInfo.starttime;
		DBG_PRINTF("resume, start:%d, current:%d gap:%d\n",_fileInfo.starttime,rtime,gap);
		//if (gap > 255) gap = 255;
		writeBuffer(idx+1,(uint8_t) (gap>>16)&0xFF );
		writeBuffer(idx+2,(uint8_t) (gap>>8)&0xFF );
		writeBuffer(idx+3,(uint8_t) (gap)&0xFF);
		commitData(idx,4);
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
