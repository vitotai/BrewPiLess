#include "BrewPiProxy.h"
#include "Config.h"
#include "BrewLogger.h"
#if SupportPressureTransducer
#include "PressureMonitor.h"
#endif

#if EnableDHTSensorSupport
#include "HumidityControl.h"
#endif

#define LoggingPeriod 60000  //in ms
#define MinimumGapToSync  600  // in seconds

#if VERIFY_BEER_PROFILE
#ifndef VERIFY_BEER_PROFILE_PERIOD
#define VERIFY_BEER_PROFILE_PERIOD 600
#endif
#endif

BrewLogger brewLogger;

BrewLogger::BrewLogger(void){
	_recording=false;
	_fsspace=0;
	
	_resetTempData();
	_calibrating=false;
	_lastPressureReading = INVALID_PRESSURE_INT;
	_targetPsi =0;

#if EnableDHTSensorSupport
	_lastHumidity = INVALID_HUMIDITY_VALUE;
	_lastHumidityTarget = INVALID_HUMIDITY_VALUE;
#endif
}

	
	bool BrewLogger::begin(void)
	{
    	bool resumeSuccess=false;
		_loadIdxFile();
		_checkspace();
		if(_pFileInfo->logname[0]!='\0'){
			resumeSuccess=resumeSession();
		}

		if(!resumeSuccess){
			DBG_PRINTF("**Resume failed!.start volatiel log\n");
			_startVolatileLog();
			loop();
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
		FileSystem.info(fs_info);
		String ret=String("{\"size\":") + String(fs_info.totalBytes)
			+",\"used\":"  + String(fs_info.usedBytes)
			+",\"block\":" + String(fs_info.blockSize)
			+",\"page\":"  + String(fs_info.pageSize)
			+"}";
#endif
		return ret;
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
		FileSystem.remove(buff);
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

		_checkspace();
		_saveIdxFile();
	}

	bool BrewLogger::resumeSession()
	{
    	_resumeLastLogTime = _pFileInfo->starttime;

		char filename[36];
		sprintf(filename,"%s/%s",LOG_PATH,_pFileInfo->logname);
#if ESP32
		// weird behavior of ESP32
		_logFile=FileSystem.open(filename,"r");
#else
		_logFile=FileSystem.open(filename,"a+");
#endif
		if(! _logFile){
            DBG_PRINTF("resume failed\n");
            return false;
		}
		size_t fsize= _logFile.size(); 	
		DBG_PRINTF("resume file:%s size:%d\n",filename,fsize);

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
		DBG_PRINTF("read:%d\n",dataRead);
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
					#ifdef VERIFY_BEER_PROFILE
					_resumeLastLogTime += VERIFY_BEER_PROFILE_PERIOD;
					#else
		    	    _resumeLastLogTime += LoggingPeriod/1000;
					#endif
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
		    				if(!_calibrating){
			
			    				if( i == OrderGravityInfo){        
									#if SerialDebug
										int gravityInt = (d0 << 8) | d1;
    	                        		DBG_PRINTF("resume@%u, SG:%d\n",_resumeLastLogTime,gravityInt);
									#endif
                                    // dont trust the data
//                            	if(gravityInt > 8000 && gravityInt < 12500)
//                                    gravityTracker.add(GravityDecode(gravityInt),_resumeLastLogTime);
				        		} // if this is gravity data
							}
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
				}else if(tag == CalibrationPointTag  || tag == OriginGravityTag  || tag == SpecificGravityTag || tag == IgnoredCalPointMaskTag) {
					if (dataRead-processIndex<2 ){
						processIndex -=2;
						break;
					}
					processIndex +=2;
				}else if(tag == StateTag  || tag == ModeTag  || tag ==CorrectionTempTag ||tag == TargetPsiTag || tag==HumidityTag){
					// DO nothing.
				}else if(tag == TimeSyncTag){
					if (dataRead-processIndex<4 ){
						processIndex -=4;
						break;
					}
					processIndex +=4;
				}else{
					DBG_PRINTF("Unknown tag %d,%d @%u\n",tag,mask,offset+processIndex);
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
			DBG_PRINTF("read:%u, all:%u\n",len,dataRead);
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
		_chartTime = _addResumeTag();
		//DBG_PRINTF("resume done _savedLength:%d, _logIndex:%d\n",_savedLength,_logIndex);
#if ESP32
		_logFile.close();
		_logFile=FileSystem.open(filename,"a+");
		if(! _logFile){
            DBG_PRINTF("resume failed\n");
            return false;
		}
#endif
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

		_logFile=FileSystem.open(buff,"a+");

		if(!_logFile){
			DBG_PRINTF("Error open temp file\n");
			return false;
		}

		_pFileInfo->starttime= TimeKeeper.getTimeSeconds();
		_chartTime = _pFileInfo->starttime;
		_logIndex = 0;

		_lastTempLog=0;
		_recording = true;
		_savedLength=0;

		char unit;
		brewPi.getLogInfo(&unit,&_mode,&_state);

		_startLog(unit == 'F',calibrating);
		_calibrating = calibrating;
		#if SupportPressureTransducer
		_targetPsi =0; // force to record
		#endif
		_resetTempData();
		loop(); // get once
		_addModeRecord(_mode);
		_addStateRecord(_state);

		#if EnableDHTSensorSupport
		if(humidityControl.sensorInstalled()){
			uint8_t humidity = humidityControl.humidity();
			_lastHumidity=humidity;
			_addHumidityRecord(humidity);
		}
		#endif


		_saveIdxFile();
		// flush to force write to file system.
		_logFile.flush();
		return true;
	}

	void BrewLogger::endSession(void){
		if(!_recording) return;
		_recording=false;
		_calibrating=false;
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
		_saveIdxFile();

		_startVolatileLog();
	}

	void BrewLogger::loop(void){
		//if(!_recording) return;
		#ifndef VERIFY_BEER_PROFILE
		unsigned long miliseconds = millis();

		if((miliseconds -_lastTempLog) < LoggingPeriod) return;
		_lastTempLog = miliseconds;
		_chartTime += LoggingPeriod/1000;
		
		uint32_t now = TimeKeeper.getTimeSeconds();
//		if( ((_chartTime >  now) && (_chartTime -  now >  MinimumGapToSync)) 
//			|| (( _chartTime < now) && (now - _chartTime > MinimumGapToSync)) ){
		if(  ( _chartTime < now) && ((now - _chartTime) > MinimumGapToSync) ){

			if(_recording){
				_addTimeSyncTag();
				DBG_PRINTF("**Sync time from:%d  to:%d",_chartTime,now);
				_chartTime=now;
			}else{
				DBG_PRINTF("**Time out of sync :%d :%d",_chartTime,now);
				_startVolatileLog();
				DBG_PRINTF("**Sync time from:%d  to:%d",_chartTime,_headTime);
			}
		}
		#else
		uint32_t now = TimeKeeper.getTimeSeconds();
		if((now -_lastTempLog) <= VERIFY_BEER_PROFILE_PERIOD) return;
		_lastTempLog = now;
		#endif
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
			iTemp=_convertTemperature(fTemps[i]);
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

		if(! _calibrating){
			if( _extGravity != INVALID_GRAVITY_INT){
					changeMask |= (1 << OrderGravityInfo);
					changeNum ++;
			}
		}else{
			if( _extTileAngle != INVALID_TILT_ANGLE){
				changeMask |= (1 << OrderGravityInfo);
				changeNum ++;
			}
		}
		#if SupportPressureTransducer
		// pressure, if any
		//DBG_PRINTF("Pressure mode:%d _lastPressureReading:%d, current:%d\n",PressureMonitor.mode(),_lastPressureReading,PressureEncode(PressureMonitor.currentPsi()));
		if(PressureMonitor.mode() != PMModeOff){
			int16_t pressure = PressureEncode(PressureMonitor.currentPsi());
			if(pressure != _lastPressureReading){
				changeMask |= (1 << OrderPressure);
				changeNum ++;
				_lastPressureReading = pressure;
			}
		}
		#endif

		int startIdx = _allocByte(2+ changeNum * 2);
		if(startIdx < 0) return;
		int idx=startIdx;
		_writeBuffer(idx++,PeriodTag);
		_writeBuffer(idx++,changeMask);

		for(int i=0;i<5;i++){
			if(changeMask & (1<<i)){

				_writeBuffer(idx ++,(_iTempData[i]>> 8) & 0x7F);
				_writeBuffer(idx ++,_iTempData[i] & 0xFF);
			}
		}


		//if( _extTemp != INVALID_TEMP_INT){
		if(changeMask & (1 << OrderExtTemp)){
			_writeBuffer(idx++,(_extTemp >>8) & 0x7F);
			_writeBuffer(idx++,_extTemp & 0xFF);
			_extTemp = INVALID_TEMP_INT;
		}

		if(changeMask & (1 << OrderGravityInfo)){
			if(! _calibrating){
				_writeBuffer(idx++,(_extGravity >>8) & 0x7F);
				_writeBuffer(idx++,_extGravity & 0xFF);
				//DBG_PRINTF("gravity %d: %d %d\n",_extGravity,(_extGravity >>8) & 0x7F,_extGravity & 0xFF);
				_extGravity = INVALID_GRAVITY_INT;
			}else{
				_writeBuffer(idx++,(_extTileAngle >>8) & 0x7F);
				_writeBuffer(idx++,_extTileAngle & 0xFF);
				_extTileAngle = INVALID_TILT_ANGLE;
			}
		}
		// pressure data
		#if SupportPressureTransducer
		if(changeMask & (1 << OrderPressure) ){
			_writeBuffer(idx ++,(_lastPressureReading>> 8) & 0x7F);
			_writeBuffer(idx ++,_lastPressureReading & 0xFF);
		}
		#endif
		
		_commitData(startIdx,2+ changeNum * 2);

		if(_extOriginGravity != INVALID_GRAVITY_INT){
			_addOgRecord(_extOriginGravity);
			_extOriginGravity = INVALID_GRAVITY_INT;
		}

		if(_calibrating){
			if( _extGravity != INVALID_GRAVITY_INT){
				_addSgRecord(_extGravity);
				_extGravity = INVALID_GRAVITY_INT;
			}
		}

		if(mode != _mode){
			DBG_PRINTF("mode %c => %c\n",_mode,mode);
			_mode = mode;
			_addModeRecord(mode);
		}

		if(state != _state){
			DBG_PRINTF("state %d => %d\n",_state,state);
			_state = state;
			_addStateRecord(state);
		}
		#if SupportPressureTransducer
		uint8_t psi= PressureMonitor.getTargetPsi();
		if(psi != _targetPsi){
			_targetPsi = psi;
			_addTargetPsiRecord();
		}
		#endif

		#if EnableDHTSensorSupport
		if(humidityControl.sensorInstalled()){
			// To save memory, only log when data changes.
			uint8_t humidity = humidityControl.humidity();
			if(_lastHumidity !=humidity){
				_lastHumidity=humidity;
				_addHumidityRecord(humidity);
			}
			uint8_t target = humidityControl.targetRH();
			if(_lastHumidityTarget !=target){
				_lastHumidityTarget =target;
				_addHumidityTargetRecord(target);
			}
		}
		#endif
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

		DBG_PRINTF("beginCopyAfter:%d, _logIndex=%u, saved=%u, return:%u, last >= (_logIndex +_savedLength)=%c\n",last,_logIndex,_savedLength,( _logIndex+_savedLength - last), (last >= (_logIndex +_savedLength))? 'Y':'N' );
		if(last >= (_logIndex +_savedLength)){
            DBG_PRINTF(" return:0\n");
            return 0;
        }
        DBG_PRINTF(" return:%u\n",_logIndex+_savedLength - last);
		return ( _logIndex+_savedLength - last);
	}

	size_t BrewLogger::read(uint8_t *buffer, size_t maxLen, size_t index)
	
	{
		size_t sizeRead;
		// index is start of "this" read. _lastRead is the starting of request
		// rindex is the real index of the whole log
		size_t rindex= index + _lastRead;

		DBG_PRINTF("read index:%u, max:%u, _lastRead =%u, rindex=%u\n",index,maxLen,_lastRead,rindex);

		// the reqeust data index is more than what we have.
		if(rindex > (_savedLength +_logIndex)) return maxLen; // return whatever it wants.
    
		// the staring data is not in buffer
		if( rindex < _savedLength){

			sizeRead = _savedLength +_logIndex - rindex;
			if(sizeRead > maxLen) sizeRead=maxLen;

			_logFile.seek(rindex,SeekSet);

			sizeRead=_logFile.read(buffer,sizeRead);

			if(sizeRead < maxLen){
				DBG_PRINTF("!Error: file read:%u of %u, file size:%u\n",sizeRead,maxLen,_logFile.size());
				size_t insufficient = maxLen - sizeRead;
                if(insufficient > _logIndex){
                    size_t fillsize=insufficient - _logIndex;
                    memset(buffer + sizeRead,FillTag,fillsize);
                    sizeRead += fillsize;
                    insufficient = _logIndex;
    				DBG_PRINTF("!Error: fill blank:%u\n",fillsize);
                }
                memcpy(buffer+ sizeRead,_logBuffer,insufficient);
				sizeRead += insufficient;
			}
			DBG_PRINTF("read file:%u\n",sizeRead);
		}else{
			//DBG_PRINTF("read from buffer\n");
			// read from buffer
			rindex -=  _savedLength;
			// rindex should be smaller than _logIndex
			sizeRead = _logIndex - rindex;
			if(sizeRead > maxLen) sizeRead=maxLen;
			memcpy(buffer,_logBuffer+rindex,sizeRead);
			//DBG_PRINTF("read buffer:%u\n",sizeRead);
		}
		
		return sizeRead;
	}

	void BrewLogger::getFilePath(char* buf,int index)
	{
		sprintf(buf,"%s/%s",LOG_PATH,_pFileInfo->files[index].name);
	}

	size_t BrewLogger::volatileStartIndex(void)
	{
		return _startOffset;
	}

	size_t BrewLogger::volatileDataAvailable(void){
		size_t dataAvail=(_logHead <= (int)_logIndex)? (_logIndex-_logHead):(LogBufferSize + _logIndex - _logHead);
		return dataAvail + VolatileHeaderSize; // for make-up header
	}

	size_t BrewLogger::volatileDataAvailable(size_t logicalAfter)
	{
		// get size;
		size_t dataAvail= volatileDataAvailable();
		//DBG_PRINTF("volatileDataAvailable,start:%d, offset:%d, _logHead %d _logIndex %d, _startOffset:%d, dataAvail:%d\n",start, offset,_logHead,_logIndex,_startOffset, dataAvail);
		if( (logicalAfter == 0)
			|| (logicalAfter < _startOffset)   // error case
		    || (logicalAfter > (_startOffset + dataAvail))) {  //error case
			// force reload, start=offset=0, the same case
			// send header?
			return dataAvail;
		}

		return _startOffset + dataAvail - logicalAfter;
	}

	size_t BrewLogger::readVolatileData(uint8_t *buffer, size_t maxLen, size_t index, size_t logicalAfter)
	{
		size_t bufIdx=0;
		size_t readIdx;

		// get size;
		size_t dataAvail= volatileDataAvailable();
		//DBG_PRINTF("volatileDataAvailable,start:%d, offset:%d, _logHead %d _logIndex %d, _startOffset:%d, dataAvail:%d\n",start, offset,_logHead,_logIndex,_startOffset, dataAvail);
		if( (logicalAfter == 0)
			|| (logicalAfter < _startOffset)   // error case
		    || (logicalAfter > (_startOffset + dataAvail))) {  //error case
			// force reload, start=offset=0, the same case
			// send header
			if(index < VolatileHeaderSize){
				// maxLen < VolatileHeaderSize?
				char header[VolatileHeaderSize];
				_volatileHeader(header);
				for(int i=index;bufIdx<maxLen && i<VolatileHeaderSize;i++)
					buffer[bufIdx++]=header[i];
				readIdx = _logHead;
			}else{
				readIdx = index -VolatileHeaderSize;
			}

		}else{

			// assume the header should already be sent.
			size_t sendOffset= logicalAfter - _startOffset - VolatileHeaderSize + _logHead;
			if(sendOffset > LogBufferSize) sendOffset -= LogBufferSize;
			
			readIdx = sendOffset + index;

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


	void BrewLogger::_addOgRecord(uint16_t og){
		_addGravityRecord(true,og);
	}
	void BrewLogger::_addSgRecord(uint16_t sg){
		_addGravityRecord(false,sg);
	}

	void BrewLogger::addAuxTemp(float temp)
	{
		_extTemp = _convertTemperature(temp);
		DBG_PRINTF("AuxTemp:%d\n",_extTemp);
	}
	void BrewLogger::addTiltAngle(float tilt)
	{
		_extTileAngle = TiltEncode(tilt);
	}
	void BrewLogger::addCorrectionTemperature(float temp)
	{
		if(!_recording) return;
		int idx = _allocByte(2);
		if(idx < 0) return;
		_writeBuffer(idx,CorrectionTempTag);
		_writeBuffer(idx+1,(uint8_t)temp);
		_commitData(idx,2);
	}
	#define ICPM_B1(m)  (((m)>>14) & 0x7F)
	#define ICPM_B2(m)  (((m)>>7) & 0x7F)
	#define ICPM_B3(m)  ((m) & 0x7F)
	void BrewLogger::addIgnoredCalPointMask(uint32_t mask)
	{
		if(!_recording) return;
		int idx = _allocByte(4);
		if(idx < 0) return;
		_writeBuffer(idx,IgnoredCalPointMaskTag);
		_writeBuffer(idx+1,ICPM_B1(mask));
		_writeBuffer(idx+2,ICPM_B2(mask));
		_writeBuffer(idx+3,ICPM_B3(mask));
		_commitData(idx,4);
	}

	void BrewLogger::addTiltInWater(float tilt,float reading)
	{
		// potential race condition
		if(!_recording) return;
		int idx = _allocByte(4);
		if(idx < 0) return;
		// the readings of water is suppose to be in a very small ranges
		// around 1.000
		// for example, 0.959 @ 100C -> corrected to 15
		// for example, 1.002 @ 0C -> corrected to 20
		// to save space, pack the data into ONE byte by simply 
		// subtract 0.9 to make it ranges from 0.059 - 0.102
		uint8_t compressed_reading =(uint8_t)((int)(reading * 1000.0) - 900);
		uint16_t angle=TiltEncode(tilt);		
		_writeBuffer(idx,CalibrationPointTag);
		_writeBuffer(idx+1,compressed_reading);
		_writeBuffer(idx+2,HighOctect(angle));
		_writeBuffer(idx+3,LowOctect(angle));
		_commitData(idx,4);		
	}
	void BrewLogger::_resetTempData(void)
	{
		for(int i=0;i<5;i++) _iTempData[i]=INVALID_TEMP_INT;
		_extTemp=INVALID_TEMP_INT;
		_extGravity=INVALID_GRAVITY_INT;
		_extOriginGravity=INVALID_GRAVITY_INT;
		_extTileAngle = INVALID_TILT_ANGLE;
		#if EnableDHTSensorSupport
		_savedHumidityValue = 0xFF;
		#endif
	}

#define RESERVED_SIZE 8196*2

	void BrewLogger::_checkspace(void)
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
		FileSystem.info(fs_info);

		_fsspace = fs_info.totalBytes - fs_info.usedBytes;
		if(_fsspace > fs_info.blockSize * 2){
			_fsspace -= fs_info.blockSize * 2;
		}else{
			_fsspace=0;
		}
#endif
		DBG_PRINTF("FileSystem space:%d\n",_fsspace);
	}

	void BrewLogger::_volatileHeader(char *buf)
	{
		char unit;
		uint8_t mode,state;

		brewPi.getLogInfo(&unit,&mode,&state);
		bool fahrenheit=(unit == 'F');

		char* ptr=buf;
		uint8_t headerTag=LOG_VERSION;
		//8
		*ptr++ = StartLogTag; // 1
		headerTag = headerTag | (fahrenheit? 0xF0:0xE0) ;
		_usePlato =theSettings.GravityConfig()->usePlato;
		if(_usePlato) headerTag = headerTag ^ 0x40;

		*ptr++ = headerTag; //2
		#if VERIFY_BEER_PROFILE
		int period = VERIFY_BEER_PROFILE_PERIOD;
		#else
		int period = LoggingPeriod/1000;
		#endif
		*ptr++ = (char) (period >> 8);
		*ptr++ = (char) (period & 0xFF);
		*ptr++ = (char) (_headTime >> 24);
		*ptr++ = (char) (_headTime >> 16);
		*ptr++ = (char) (_headTime >> 8);
		*ptr++ = (char) (_headTime & 0xFF); //8
		// a record full of all data = 2 + 7 * 2= 16
		*ptr++ = (char) PeriodTag; //9
		*ptr++ = (char) 0x7F; //10
		for(int i=0;i<VolatileDataHeaderSize;i++){ // 10 + VolatileDataHeaderSetNumber *2
			*ptr++ = _headData[i] >> 8;
			*ptr++ = _headData[i] & 0xFF;
		}
		// mode : 2
		*ptr++ = ModeTag; // 11 + VolatileDataHeaderSetNumber*2
		*ptr++ = mode; // // 12 + VolatileDataHeaderSetNumber*2
		// state: 2
		*ptr++ = StateTag; // 13 + VolatileDataHeaderSetNumber*2
		*ptr++ = state;  // 14 + VolatileDataHeaderSetNumber*2
		*ptr++ = TargetPsiTag; //15		
		*ptr++ = _targetPsi;  // 16

		#if EnableDHTSensorSupport

		// to simplify, just let deocoder ignore invalid dataset
		// 
		*ptr++ = HumidityTag; //17		
		*ptr++ = _savedHumidityValue;  // 18
		#endif
 	}
	void BrewLogger::_startLog(bool fahrenheit,bool calibrating)
	{
		char *ptr=_logBuffer;
		_usePlato=theSettings.GravityConfig()->usePlato;

		// F0FF  peroid   4 bytes
		// Start system time 4bytes
		// 0xFF startLogtag
		//  x |plato |calibrating  |Temp unit| 4-bit = 5 |

		uint8_t headerTag=LOG_VERSION;
		*ptr++ = StartLogTag;

		headerTag = headerTag | (fahrenheit? 0xF0:0xE0) ;		
		if(calibrating) headerTag = headerTag ^ 0x20 ;
		if(_usePlato) headerTag = headerTag ^ 0x40;

		*ptr++ = headerTag;
		#if VERIFY_BEER_PROFILE
		int period = VERIFY_BEER_PROFILE_PERIOD;
		#else
		int period = LoggingPeriod/1000;
		#endif
		*ptr++ = (char) (period >> 8);
		*ptr++ = (char) (period & 0xFF);
		*ptr++ = (char) (_pFileInfo->starttime >> 24);
		*ptr++ = (char) (_pFileInfo->starttime >> 16);
		*ptr++ = (char) (_pFileInfo->starttime >> 8);
		*ptr++ = (char) (_pFileInfo->starttime & 0xFF);
		_logIndex=0;
		_savedLength=0;
		_commitData(_logIndex,ptr - _logBuffer );

		//DBG_PRINTF("*_startLog*\n");
	}

	void BrewLogger::_startVolatileLog(void)
	{
		DBG_PRINTF("_startVolatileLog, mode=%c, beerteemp=%d\n",_mode,_iTempData[OrderBeerTemp]);
		_headTime=TimeKeeper.getTimeSeconds();
		_logHead = 0;
		_logIndex = 0;
		_startOffset=0;
		_lastTempLog=0;
		for(int i=0;i<5;i++) _headData[i]=_iTempData[i];
		_headData[5]= _extTemp;
		_headData[6]= (_calibrating)? _extTileAngle:_extGravity;

		_chartTime = _headTime;
	}



	int BrewLogger::_availableBufferSpace(void)
	{
		//DBG_PRINTF("_logHead:%d, _logIndex: %d\n",_logHead,_logIndex);
		if(_logIndex >= (size_t)_logHead){
			return LogBufferSize - _logIndex -1 + _logHead;
		}else {
			// _logIndex < _logHead
			return _logHead - _logIndex - 1;
		}
	}

	void BrewLogger::_dropData(void)
	{
		noInterrupts();
		// move headto nex time stamp.
		// four temperatures in one period
		int idx = _logHead;
		int dataDrop=0;
		byte tag;
		byte mask;
		bool timeCorrected=false;
		uint32_t time=0;

		while(1){
			if(idx >= LogBufferSize) idx -= LogBufferSize;
			tag =_logBuffer[idx++];
		    mask=_logBuffer[idx++];
			dataDrop +=2;

			if(tag == PeriodTag) break;

			if(tag == OriginGravityTag || tag == SpecificGravityTag || tag == CalibrationPointTag){
    			idx += 2;
	    		dataDrop +=2;
			}else if(tag == TimeSyncTag){
				// 4 additional bytes
    			idx += 4;
	    		dataDrop +=4;
				time = (_logBuffer[idx] << 24)
						| (_logBuffer[idx + 1] << 16)
						| (_logBuffer[idx + 2] << 8)
						| _logBuffer[idx + 3];				
				timeCorrected = true;
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
			if(_logBuffer[idx] == TimeSyncTag){
				idx +=6;
				dataDrop +=6;
				time = (_logBuffer[idx + 2] << 24)
						| (_logBuffer[idx + 3] << 16)
						| (_logBuffer[idx + 4] << 8)
						| _logBuffer[idx + 5];				
				timeCorrected = true;			
			}else if(OriginGravityTag == _logBuffer[idx] || SpecificGravityTag == _logBuffer[idx]){
				idx +=4;
				dataDrop +=4;
			}else{

				#if EnableDHTSensorSupport
				if(HumidityTag ==  _logBuffer[idx]){
					_savedHumidityValue = _logBuffer[idx+1];
				}
				#endif

				idx +=2;
				dataDrop +=2;
			}
		}

		if(idx >= LogBufferSize) idx -= LogBufferSize;
		_startOffset += dataDrop;
		_logHead = idx;
		if(timeCorrected) _headTime = time;
		#ifdef VERIFY_BEER_PROFILE
		else _headTime += VERIFY_BEER_PROFILE_PERIOD;
		#else
		else _headTime += LoggingPeriod/1000;
		#endif
		interrupts();
		DBG_PRINTF("Drop %d\n",dataDrop);
	}

	int BrewLogger::_volatileLoggingAlloc(int size){
		int space=_availableBufferSpace();
		while(space < size){
			//DBG_PRINTF("Free %d req: %d\n",space,size);
			_dropData();
			space=_availableBufferSpace();
		}

		return _logIndex;
	}

	int BrewLogger::_allocByte(byte size)
	{
		if(!_recording){
			return _volatileLoggingAlloc(size);
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

	void BrewLogger::_writeBuffer(int idx,uint8_t data)
	{
		if(idx < LogBufferSize){
			_logBuffer[idx] = data;
		}else{
			_logBuffer[idx - LogBufferSize]=data;
		}
	}

	void BrewLogger::_commitData(int idx,int len)
	{
		//WARNNING: we are relying on the write cache of FileSystem
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

			_logFile=FileSystem.open(buff,"a+");
			_logFile.write((const uint8_t*)buf+wlen,len-wlen);
		}*/

		//frequent writing is not good for flash.
		//_logFile.flush();
		
	}

	void BrewLogger::_addGravityRecord(bool isOg, uint16_t gravity){
		int idx = _allocByte(4);
		if(idx < 0) return;
		_writeBuffer(idx, isOg? OriginGravityTag:SpecificGravityTag);
		_writeBuffer(idx+1,0);
		_writeBuffer(idx+2,(gravity >> 8) | 0x80); //*ptr = ModeTag;
		_writeBuffer(idx+3,gravity & 0xFF); //*(ptr+1) = mode;
		_commitData(idx,4);
	}

	void BrewLogger::_addTimeSyncTag(void){
		int idx = _allocByte(6);
		if(idx < 0) return;
		_writeBuffer(idx,TimeSyncTag);
		_writeBuffer(idx+1,0);
		uint32_t now=TimeKeeper.getTimeSeconds();
		_writeBuffer(idx+2,(uint8_t)(now >> 24));
		_writeBuffer(idx+3,(uint8_t)(now >> 16));
		_writeBuffer(idx+4,(uint8_t)(now >> 8));
		_writeBuffer(idx+5,(uint8_t)(now & 0xFF));

		_commitData(idx,6);
	}

	void BrewLogger::_addModeRecord(char mode){
		int idx = _allocByte(2);
		if(idx < 0) return;
		_writeBuffer(idx,ModeTag); //*ptr = ModeTag;
		_writeBuffer(idx+1,mode); //*(ptr+1) = mode;
		_commitData(idx,2);
	}

	void BrewLogger::_addTargetPsiRecord(void){
		int idx = _allocByte(2);
		if(idx < 0) return;
		_writeBuffer(idx,TargetPsiTag); //*ptr = TargetPsiTag;
		_writeBuffer(idx+1,_targetPsi); //*(ptr+1) = mode;
		_commitData(idx,2);
	}

#if EnableDHTSensorSupport	
	void BrewLogger::_addHumidityRecord(uint8_t humidity){
		int idx = _allocByte(2);
		if(idx < 0) return;
		_writeBuffer(idx,HumidityTag); //*ptr = TargetPsiTag;
		_writeBuffer(idx+1,humidity); //*(ptr+1) = mode;
		_commitData(idx,2);
	}

	void BrewLogger::_addHumidityTargetRecord(uint8_t target){
		int idx = _allocByte(2);
		if(idx < 0) return;
		_writeBuffer(idx,HumiditySetTag); //*ptr = TargetPsiTag;
		_writeBuffer(idx+1,target); //*(ptr+1) = mode;
		_commitData(idx,2);
	}

#endif

	void BrewLogger::_addStateRecord(char state){
		int idx = _allocByte(2);
		if(idx <0) return;
		_writeBuffer(idx,StateTag); //*ptr = StateTag;
		_writeBuffer(idx+1,state); //*(ptr+1) = state;
		_commitData(idx,2);
	}

	uint16_t BrewLogger::_convertTemperature(float temp){
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


	uint32_t BrewLogger::_addResumeTag(void)
	{
		int idx = _allocByte(4);
		if(idx < 0) return 0;
		_writeBuffer(idx,ResumeBrewTag); //*ptr = ResumeBrewTag;
		size_t rtime= TimeKeeper.getTimeSeconds();
		size_t gap=rtime - _pFileInfo->starttime;
		if(rtime < 1545211593L || gap > 60*60*24*30){
			// something wrong. just give it a minute, relying on following TimeSync Tag
			DBG_PRINTF("abnormal resume, start:%lu, current:%u gap:%u\n",_pFileInfo->starttime,rtime,gap);
			gap =60;
			rtime =  _pFileInfo->starttime + gap;
		}
		DBG_PRINTF("resume, start:%lu, current:%u gap:%u\n",_pFileInfo->starttime,rtime,gap);
		//if (gap > 255) gap = 255;
		_writeBuffer(idx+1,(uint8_t) (gap>>16)&0xFF );
		_writeBuffer(idx+2,(uint8_t) (gap>>8)&0xFF );
		_writeBuffer(idx+3,(uint8_t) (gap)&0xFF);
		_commitData(idx,4);
		return rtime;
	}

	void BrewLogger::_loadIdxFile(void)
	{
        _pFileInfo = theSettings.logFileIndexes();
	}

	void BrewLogger::_saveIdxFile(void)
	{
        theSettings.save();
	}

	void BrewLogger::onFormatFS(void){
		// force to end session
		if(_recording){
			_recording=false;
			_logFile.close();
			_pFileInfo->logname[0]='\0';
			_pFileInfo->starttime=0;
		}
		// clear all logs
		int index=0;
		for(;index<MAX_LOG_FILE_NUMBER;index++){
			_pFileInfo->files[index].name[0] = 0;
		}
	}
