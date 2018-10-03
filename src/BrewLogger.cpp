#include "BrewPiProxy.h"
#include "BrewLogger.h"

BrewLogger brewLogger;

BrewLogger::BrewLogger(void){
	_recording=false;
	_fsspace=0;
	_tempLogPeriod=60000;
	resetTempData();
	_calibrating=false;
}

	
	bool BrewLogger::begin(void)
	{
    	bool resumeSuccess=false;
		loadIdxFile();
		checkspace();
		if(_pFileInfo->logname[0]!='\0'){
			resumeSuccess=resumeSession();
		}

		if(!resumeSuccess){
			DBG_PRINTF("start volatiel log\n");
			logData();
			startVolatileLog();
		}
        return resumeSuccess;
	}

	String BrewLogger::fsinfo(void)
	{
#if defined(ESP32)
		String ret=String("{\"size\":") + String(SPIFFS.totalBytes())
			+",\"used\":"  + String(SPIFFS.usedBytes())
//			+",\"block\":" + String(fs_info.blockSize)
//			+",\"page\":"  + String(fs_info.pageSize)
			+"}";
#else
		FSInfo fs_info;
		SPIFFS.info(fs_info);
		String ret=String("{\"size\":") + String(fs_info.totalBytes)
			+",\"used\":"  + String(fs_info.usedBytes)
			+",\"block\":" + String(fs_info.blockSize)
			+",\"page\":"  + String(fs_info.pageSize)
			+"}";
		return ret;
#endif
	}
	
	const char* BrewLogger::currentLog(void)
	{
		if(!_recording) return NULL;
		if(_pFileInfo->logname[0] != 0)
			return _pFileInfo->logname;
		else return NULL;
	}

	String BrewLogger::loggingStatus(void)
	{
		// populate JS
		String ret=String("{\"rec\":");
		if(_recording){
			ret += "1, \"log\":\"" + String(_pFileInfo->logname)
				+"\",\"start\":" + String(_pFileInfo->starttime);
		}else{
			ret += "0";
		}
		ret += ",\"fs\":" + fsinfo();

		ret += ",\"plato\":" + String(theSettings.GravityConfig()->usePlato? "1":"0");

		ret += ",\"list\":[";

		for(int i=0;i<MAX_LOG_FILE_NUMBER;i++){
			if(_pFileInfo->files[i].name[0] == 0) break;
			if(i!=0) ret +=",";
			ret +="{\"name\":\"" + String(_pFileInfo->files[i].name);
			ret +="\",\"time\":" +String(_pFileInfo->files[i].time) +"}";
		}
		ret += "]}";

		return ret;
	}

	void BrewLogger::rmLog(int index)
	{
		//TODO: race condition
		// multiple access issue
		char buff[36];
		sprintf(buff,"%s/%s",LOG_PATH,_pFileInfo->files[index].name);
		SPIFFS.remove(buff);
		DBG_PRINTF("remove %d: %s\n",index,buff);
		int i;
		for(i=index+1;i<MAX_LOG_FILE_NUMBER;i++){
			if(_pFileInfo->files[i].name[0]=='\0') break;
			DBG_PRINTF("move %d: %s\n",i,_pFileInfo->files[i].name);
			strcpy(_pFileInfo->files[i-1].name,_pFileInfo->files[i].name);
			_pFileInfo->files[i-1].time=_pFileInfo->files[i].time;
		}
		_pFileInfo->files[i-1].name[0]='\0';
		_pFileInfo->files[i-1].time =0;

		checkspace();
		saveIdxFile();
	}

	bool BrewLogger::resumeSession()
	{
    	_resumeLastLogTime = _pFileInfo->starttime;

		char buff[36];
		sprintf(buff,"%s/%s",LOG_PATH,_pFileInfo->logname);

		_logFile=SPIFFS.open(buff,"a+");
		if(! _logFile){
            DBG_PRINTF("resume failed\n");
            return false;
		}
		size_t fsize= _logFile.size(); 	
		DBG_PRINTF("resume file:%s size:%d\n",buff,fsize);

/*		if(fsize < 8){
            DBG_PRINTF("resume failed\n");
			_logFile.close();
			return false;
		}
*/
		int dataRead;
		size_t offset=0;
		int    processIndex=0;
		uint8_t tag, mask;


		dataRead=_logFile.read((uint8_t*)_logBuffer,LogBufferSize);
		DBG_PRINTF("read:%ld\n",dataRead);
		if(dataRead < 8){
            DBG_PRINTF("resume failed\n");
			_logFile.close();
			return false;
		}

		tag= _logBuffer[processIndex++];
		mask = _logBuffer[processIndex++];

		if(tag == StartLogTag){
			_calibrating = (mask & 0x20) != 0x20;
			_usePlato = (mask & 0x40) != 0x40;
			DBG_PRINTF("resume cal:%d\n",_calibrating);
			processIndex += 6;
		}else{
			DBG_PRINTF("resume failed, no start tag\n");
			_logFile.close();
			return false;
		}
		do{
			while((dataRead-processIndex)>=2){
				//DBG_PRINTF("dataavail:%ld\n",dataRead-processIndex);
				tag= _logBuffer[processIndex++];
				mask = _logBuffer[processIndex++];

				if(tag == PeriodTag){
					// advance one tick
		    	    _resumeLastLogTime += _tempLogPeriod/1000;

					//TODO: check available data?
		       		// int numberInRecord=0;
					size_t recordSize;
					recordSize =0; 
		        	uint8_t bitmask;
					bitmask=1;
					for(int i=0;i<NumberDataBitMask;i++, bitmask=bitmask<<1) 
						if(mask & bitmask)
							recordSize +=2;

					if (dataRead-processIndex < (int) recordSize ){
						processIndex -= 2;
						break;
					}
					bitmask=1;
        			for(int i=0;i<NumberDataBitMask;i++, bitmask=bitmask<<1){
	        			if(mask & bitmask){
								#if SerialDebug
							int d0=_logBuffer[processIndex++];
    			    	   	int d1=_logBuffer[processIndex++];
							   #endif
							// get gravity data that we need							
		        			if( i == OrderGravity){        
								#if SerialDebug
								int gravityInt = (d0 << 8) | d1;
                            	DBG_PRINTF("resume@%ld, SG:%d\n",_resumeLastLogTime,gravityInt);
								#endif
                                    // dont trust the data
//                            	if(gravityInt > 8000 && gravityInt < 12500)
//                                    gravityTracker.add(GravityDecode(gravityInt),_resumeLastLogTime);
				        	} // if this is gravity data
			        	} // if the field exists
			    	} // for each bit
		    	}else if(tag == ResumeBrewTag){
						if ((dataRead-processIndex)<2 ){
							processIndex -=2;
							break;
						}
						size_t d1 =(size_t)_logBuffer[processIndex++];
		   	 			size_t d0 =(size_t)_logBuffer[processIndex++];
						size_t tdiff= (mask <<16) + (d1 << 8) + d0;
			    		_resumeLastLogTime = _pFileInfo->starttime + tdiff;
				}else if(tag == CalibrationPointTag  || tag == OriginGravityTag || tag == IgnoredCalPointMaskTag) {
					if (dataRead-processIndex<2 ){
						processIndex -=2;
						break;
					}
					processIndex +=2;
				}else if(tag == StateTag  || tag == ModeTag  || tag ==CorrectionTempTag ){
					// DO nothing.
				}else{
					DBG_PRINTF("Unknown tag %d,%d @%ld\n",tag,mask,offset+processIndex);
				}
			}//while() processing data in buffer
			int dataLeft=0;
			offset += processIndex;
			for(;processIndex < dataRead;){
				_logBuffer[dataLeft] = _logBuffer[processIndex];
				dataLeft++,processIndex++;
			}
			size_t len=_logFile.read((uint8_t*)_logBuffer+dataLeft,LogBufferSize - dataLeft);
			if(len==0) break; // nothing to do
			dataRead = len +  dataLeft;
			DBG_PRINTF("read:%ld, all:%ld\n",len,dataRead);
			processIndex=0;

		}while(true);
		// log a "new start" log
		if(processIndex !=dataRead) {
			DBG_PRINTF("Incomplete record:%d\n",dataRead-processIndex);
		}
		// seek for SeekEnd might has a bug. use 
		//_logFile.seek(dataAvail,SeekEnd);
		//_logFile.seek(fsize,SeekSet);
		_logIndex =0;
		_savedLength = fsize;
		DBG_PRINTF("resume, total _savedLength:%d, _logIndex:%d\n",_savedLength,_logIndex);

		_lastTempLog=0;
		_recording = true;

		char unit;
		brewPi.getLogInfo(&unit,&_mode,&_state);

		// add resume tag
		addResumeTag();
		//DBG_PRINTF("resume done _savedLength:%d, _logIndex:%d\n",_savedLength,_logIndex);
		return true;
	}
	bool BrewLogger::startSession(const char *filename,bool calibrating){		
		if(_recording) return false; // alread start

		if(_fsspace < 100){
			DBG_PRINTF("Not enough space:%d\n",_fsspace);
			return false;
		}
		strcpy(_pFileInfo->logname,filename);
		char buff[36];
		sprintf(buff,"%s/%s",LOG_PATH,filename);

		_logFile=SPIFFS.open(buff,"a+");

		if(!_logFile){
			DBG_PRINTF("Error open temp file\n");
			return false;
		}

		_pFileInfo->starttime= TimeKeeper.getTimeSeconds();
		_logIndex = 0;

		_lastTempLog=0;
		_recording = true;
		_savedLength=0;

		char unit;
		brewPi.getLogInfo(&unit,&_mode,&_state);

		startLog(unit == 'F',calibrating);
		_calibrating = calibrating;
		
		resetTempData();
		loop(); // get once
		addMode(_mode);
		addState(_state);

		saveIdxFile();
		// flush to force write to file system.
		_logFile.flush();
		return true;
	}

	void BrewLogger::endSession(void){
		if(!_recording) return;
		_recording=false;
		_logFile.close();
		// copy the file name into last entry
		int index=0;
		for(;index<MAX_LOG_FILE_NUMBER;index++)
		{
			if(_pFileInfo->files[index].name[0] == 0) break;
		}
		// exceptional case.
		if(index == MAX_LOG_FILE_NUMBER){
			rmLog(0);
			index = -1;
		}
		strcpy(_pFileInfo->files[index].name,_pFileInfo->logname);
		_pFileInfo->files[index].time = _pFileInfo->starttime;
		_pFileInfo->logname[0]='\0';
		_pFileInfo->starttime=0;
		saveIdxFile();

		startVolatileLog();
	}

	void BrewLogger::loop(void){
		//if(!_recording) return;

		unsigned long miliseconds = millis();

		if((miliseconds -_lastTempLog) < _tempLogPeriod) return;
		_lastTempLog = miliseconds;
		logData();
	}

	void BrewLogger::logData(void){
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
		
		if( _extTileAngle != INVALID_TILT_ANGLE){
			changeMask |= (1 << OrderTiltAngle);
			changeNum ++;
		}

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

		if( _extTileAngle != INVALID_TILT_ANGLE){
			writeBuffer(idx++,(_extTileAngle >>8) & 0x7F);
			writeBuffer(idx++,_extTileAngle & 0xFF);
			_extTileAngle = INVALID_TILT_ANGLE;
		}

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
	
	size_t BrewLogger::beginCopyAfter(size_t last)
	{
		//for recording log only.
		if(!_recording) return 0;
		_lastRead = last;
		// _logIndex: data in buffer
		// _savedLength: data in file. However, all data are "writen" to file at the first place.
		//               Though, some data might remain in write buffer.
		// under normal condition. _savedLength = total size - _logIndex;
		//                        that is,  total size = _savedLength + _logIndex
		// in abnormal cases, the file size is total size since all data are "written".

		DBG_PRINTF("beginCopyAfter:%d, _logIndex=%ld, saved=%ld, return:%ld, last >= (_logIndex +_savedLength)=%c\n",last,_logIndex,_savedLength,( _logIndex+_savedLength - last), (last >= (_logIndex +_savedLength))? 'Y':'N' );
		if(last >= (_logIndex +_savedLength)){
            DBG_PRINTF(" return:0\n");
            return 0;
        }
        DBG_PRINTF(" return:%ld\n",_logIndex+_savedLength - last);
		return ( _logIndex+_savedLength - last);
	}

	size_t BrewLogger::read(uint8_t *buffer, size_t maxLen, size_t index)
	
	{
		size_t sizeRead;
		// index is start of "this" read. _lastRead is the starting of request
		// rindex is the real index of the whole log
		size_t rindex= index + _lastRead;

		DBG_PRINTF("read index:%ld, max:%ld, _lastRead =%ld, rindex=%ld\n",index,maxLen,_lastRead,rindex);

		// the reqeust data index is more than what we have.
		if(rindex > (_savedLength +_logIndex)) return maxLen; // return whatever it wants.
    
		// the staring data is not in buffer
		if( rindex < _savedLength){

			sizeRead = _savedLength +_logIndex - rindex;
			if(sizeRead > maxLen) sizeRead=maxLen;

			_logFile.seek(rindex,SeekSet);

			sizeRead=_logFile.read(buffer,sizeRead);

			if(sizeRead < maxLen){
				DBG_PRINTF("!Error: file read:%ld of %ld, file size:%ld\n",sizeRead,maxLen,_logFile.size());
				size_t insufficient = maxLen - sizeRead;
                if(insufficient > _logIndex){
                    size_t fillsize=insufficient - _logIndex;
                    memset(buffer + sizeRead,FillTag,fillsize);
                    sizeRead += fillsize;
                    insufficient = _logIndex;
    				DBG_PRINTF("!Error: fill blank:%ld\n",fillsize);
                }
                memcpy(buffer+ sizeRead,_logBuffer,insufficient);
				sizeRead += insufficient;
			}
			DBG_PRINTF("read file:%ld\n",sizeRead);
		}else{
			//DBG_PRINTF("read from buffer\n");
			// read from buffer
			rindex -=  _savedLength;
			// rindex should be smaller than _logIndex
			sizeRead = _logIndex - rindex;
			if(sizeRead > maxLen) sizeRead=maxLen;
			memcpy(buffer,_logBuffer+rindex,sizeRead);
			DBG_PRINTF("read buffer:%ld\n",sizeRead);
		}
		
		return sizeRead;
	}

	void BrewLogger::getFilePath(char* buf,int index)
	{
		sprintf(buf,"%s/%s",LOG_PATH,_pFileInfo->files[index].name);
	}

	size_t BrewLogger::volatileDataOffset(void)
	{
		return _startOffset;
	}

	size_t BrewLogger::volatileDataAvailable(size_t start,size_t offset)
	{
		// get size;
		size_t dataAvail=(_logHead <= (int)_logIndex)? (_logIndex-_logHead):(LogBufferSize + _logIndex - _logHead);
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

	size_t BrewLogger::readVolatileData(uint8_t *buffer, size_t maxLen, size_t index)
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

	void BrewLogger::addGravity(float gravity,bool isOg)
	{
		if(isOg){
			if(_usePlato) _extOriginGravity = PlatoEncode(gravity);
			else _extOriginGravity = GravityEncode(gravity);
		}else{
			if(_usePlato) _extGravity=PlatoEncode(gravity);
			else _extGravity=GravityEncode(gravity);
		}
	}

	void BrewLogger::addAuxTemp(float temp)
	{
		_extTemp = convertTemperature(temp);
		DBG_PRINTF("AuxTemp:%d\n",_extTemp);
	}
	void BrewLogger::addTiltAngle(float tilt)
	{
		_extTileAngle = TiltEncode(tilt);
	}
	void BrewLogger::addCorrectionTemperature(float temp)
	{
		if(!_recording) return;
		int idx = allocByte(2);
		if(idx < 0) return;
		writeBuffer(idx,CorrectionTempTag);
		writeBuffer(idx+1,(uint8_t)temp);
		commitData(idx,2);
	}
	#define ICPM_B1(m)  (((m)>>14) & 0x7F)
	#define ICPM_B2(m)  (((m)>>7) & 0x7F)
	#define ICPM_B3(m)  ((m) & 0x7F)
	void BrewLogger::addIgnoredCalPointMask(uint32_t mask)
	{
		if(!_recording) return;
		int idx = allocByte(4);
		if(idx < 0) return;
		writeBuffer(idx,IgnoredCalPointMaskTag);
		writeBuffer(idx+1,ICPM_B1(mask));
		writeBuffer(idx+2,ICPM_B2(mask));
		writeBuffer(idx+3,ICPM_B3(mask));
		commitData(idx,4);
	}

	void BrewLogger::addTiltInWater(float tilt,float reading)
	{
		if(!_recording) return;
		int idx = allocByte(4);
		if(idx < 0) return;
		// the readings of water is suppose to be in a very small ranges
		// around 1.000
		// for example, 0.959 @ 100C -> corrected to 15
		// for example, 1.002 @ 0C -> corrected to 20
		// to save space, pack the data into ONE byte by simply 
		// subtract 0.9 to make it ranges from 0.059 - 0.102
		uint8_t compressed_reading =(uint8_t)((int)(reading * 1000.0) - 900);
		uint16_t angle=TiltEncode(tilt);		
		writeBuffer(idx,CalibrationPointTag);
		writeBuffer(idx+1,compressed_reading);
		writeBuffer(idx+2,HighOctect(angle));
		writeBuffer(idx+3,LowOctect(angle));
		commitData(idx,4);		
	}
	void BrewLogger::resetTempData(void)
	{
		for(int i=0;i<5;i++) _iTempData[i]=INVALID_TEMP_INT;
		_extTemp=INVALID_TEMP_INT;
		_extGravity=INVALID_GRAVITY_INT;
		_extOriginGravity=INVALID_GRAVITY_INT;
		_extTileAngle = INVALID_TILT_ANGLE;
	}

#define RESERVED_SIZE 8196*2

	void BrewLogger::checkspace(void)
	{
#if defined(ESP32)
		_fsspace = SPIFFS.totalBytes() - SPIFFS.usedBytes();

		if(_fsspace > RESERVED_SIZE){
			_fsspace -= RESERVED_SIZE;
		}else{
			_fsspace=0;
		}

#else
		FSInfo fs_info;
		SPIFFS.info(fs_info);

		_fsspace = fs_info.totalBytes - fs_info.usedBytes;
		if(_fsspace > fs_info.blockSize * 2){
			_fsspace -= fs_info.blockSize * 2;
		}else{
			_fsspace=0;
		}
#endif
		DBG_PRINTF("SPIFFS space:%d\n",_fsspace);
	}

	void BrewLogger::volatileHeader(char *buf)
	{
		char unit;
		uint8_t mode,state;

		brewPi.getLogInfo(&unit,&mode,&state);
		bool fahrenheit=(unit == 'F');

		char* ptr=buf;
		uint8_t headerTag=5;
		//8
		*ptr++ = StartLogTag;
		headerTag = headerTag | (fahrenheit? 0xF0:0xE0) ;
		_usePlato =theSettings.GravityConfig()->usePlato;
		if(_usePlato) headerTag = headerTag ^ 0x40;

		*ptr++ = headerTag;
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
	void BrewLogger::startLog(bool fahrenheit,bool calibrating)
	{
		char *ptr=_logBuffer;
		_usePlato=theSettings.GravityConfig()->usePlato;

		// F0FF  peroid   4 bytes
		// Start system time 4bytes
		uint8_t headerTag=5;
		*ptr++ = StartLogTag;

		headerTag = headerTag | (fahrenheit? 0xF0:0xE0) ;		
		if(calibrating) headerTag = headerTag ^ 0x20 ;
		if(_usePlato) headerTag = headerTag ^ 0x40;

		*ptr++ = headerTag;
		
		int period = _tempLogPeriod/1000;
		*ptr++ = (char) (period >> 8);
		*ptr++ = (char) (period & 0xFF);
		*ptr++ = (char) (_pFileInfo->starttime >> 24);
		*ptr++ = (char) (_pFileInfo->starttime >> 16);
		*ptr++ = (char) (_pFileInfo->starttime >> 8);
		*ptr++ = (char) (_pFileInfo->starttime & 0xFF);
		_logIndex=0;
		_savedLength=0;
		commitData(_logIndex,ptr - _logBuffer );

		//DBG_PRINTF("*startLog*\n");
	}

	void BrewLogger::startVolatileLog(void)
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



	int BrewLogger::freeBufferSpace(void)
	{
		//DBG_PRINTF("_logHead:%d, _logIndex: %d\n",_logHead,_logIndex);
		if(_logIndex >= (size_t)_logHead){
			return LogBufferSize - _logIndex -1 + _logHead;
		}else {
			// _logIndex < _logHead
			return _logHead - _logIndex - 1;
		}
	}

	void BrewLogger::dropData(void)
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

	int BrewLogger::volatileLoggingAlloc(int size)
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

	int BrewLogger::allocByte(byte size)
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

	void BrewLogger::writeBuffer(int idx,uint8_t data)
	{
		if(idx < LogBufferSize){
			_logBuffer[idx] = data;
		}else{
			_logBuffer[idx - LogBufferSize]=data;
		}
	}

	void BrewLogger::commitData(int idx,int len)
	{
		//WARNNING: we are relying on the write cache of SPIFFS
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
			sprintf(buff,"%s/%s",LOG_PATH,_pFileInfo->logname);

			_logFile=SPIFFS.open(buff,"a+");
			_logFile.write((const uint8_t*)buf+wlen,len-wlen);
		}*/

		//frequent writing is not good for flash.
		//_logFile.flush();
		
	}

	void BrewLogger::addOG(uint16_t og){
		int idx = allocByte(4);
		if(idx < 0) return;
		writeBuffer(idx,OriginGravityTag);
		writeBuffer(idx+1,0);
		writeBuffer(idx+2,(og >> 8) | 0x80); //*ptr = ModeTag;
		writeBuffer(idx+3,og & 0xFF); //*(ptr+1) = mode;
		commitData(idx,4);
	}


	void BrewLogger::addMode(char mode){
		int idx = allocByte(2);
		if(idx < 0) return;
		writeBuffer(idx,ModeTag); //*ptr = ModeTag;
		writeBuffer(idx+1,mode); //*(ptr+1) = mode;
		commitData(idx,2);
	}

	void BrewLogger::addState(char state){
		int idx = allocByte(2);
		if(idx <0) return;
		writeBuffer(idx,StateTag); //*ptr = StateTag;
		writeBuffer(idx+1,state); //*(ptr+1) = state;
		commitData(idx,2);
	}

	uint16_t BrewLogger::convertTemperature(float temp){
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


	void BrewLogger::addResumeTag(void)
	{
		int idx = allocByte(4);
		if(idx < 0) return;
		writeBuffer(idx,ResumeBrewTag); //*ptr = ResumeBrewTag;
		size_t rtime= TimeKeeper.getTimeSeconds();
		size_t gap=rtime - _pFileInfo->starttime;
		DBG_PRINTF("resume, start:%d, current:%d gap:%d\n",_pFileInfo->starttime,rtime,gap);
		//if (gap > 255) gap = 255;
		writeBuffer(idx+1,(uint8_t) (gap>>16)&0xFF );
		writeBuffer(idx+2,(uint8_t) (gap>>8)&0xFF );
		writeBuffer(idx+3,(uint8_t) (gap)&0xFF);
		commitData(idx,4);
	}

	void BrewLogger::loadIdxFile(void)
	{
        _pFileInfo = theSettings.logFileIndexes();
	}

	void BrewLogger::saveIdxFile(void)
	{
        theSettings.save();
	}

