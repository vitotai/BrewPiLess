#include "BrewPiProxy.h"
#include "Config.h"
#include "BrewLogger.h"
#if SupportPressureTransducer
#include "PressureMonitor.h"
#endif

#if EnableHumidityControlSupport
#include "HumidityControl.h"
#endif
#include "ExternalData.h"
#include "ErrorCode.h"

#define LoggingPeriod 60000  //in ms
#define MinimumGapToSync  600  // in seconds

		// the readings of water is suppose to be in a very small ranges
		// around 1.000
		// for example, 0.959 @ 100C -> corrected to 15
		// for example, 1.002 @ 0C -> corrected to 20
		// to save space, pack the data into ONE byte by simply 
		// subtract 0.9 to make it ranges from 0.059 - 0.102

#define CompressedGravity(r) (uint8_t)((int)((r) * 1000.0) - 900)
#define DecompressGravity(g) (float)(((g) +900) /1000.0)

BrewLogger brewLogger;

BrewLogger::BrewLogger(void){
	_recording=false;
	_fsspace=0;
	
	_resetTempData();
	_calibrating=false;
	_lastPressureReading = INVALID_PRESSURE_INT;
	_targetPsi =0;
	_newcalibratingdata  = false;
#if EnableHumidityControlSupport
	_lastHumidity = INVALID_HUMIDITY_VALUE;
	_lastRoomHumidity = INVALID_HUMIDITY_VALUE;
	_lastHumidityTarget = INVALID_HUMIDITY_VALUE;
#endif
	_errorCode = ErrorNone;
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
			loop();
			_startVolatileLog();
		}
        return resumeSuccess;
	}

	String BrewLogger::fsinfo(void)
	{
#if defined(ESP32)
		String ret=String("{\"size\":") + String(FileSystem.totalBytes())
			+",\"used\":"  + String(FileSystem.usedBytes())
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
		ret += ",\"wobf\":" + String(_writeOnBufferFull? "1":"0");

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

		char filename[128];
		sprintf(filename,"%s/%s",LOG_PATH,_pFileInfo->logname);
		// debug
		if(FileSystem.exists(filename)){
			DBG_PRINTF("%s exists\n",filename);
		}else{
			DBG_PRINTF("%s doesnot exists\n",filename);
		}

#if ESP32
	#if UseLittleFS
		_logFile=FileSystem.open(filename,"r");
	#else
		// weird behavior of ESP32
		_logFile=FileSystem.open(filename,"r");
	#endif
#else
		_logFile=FileSystem.open(filename,"a+");
#endif
		if(! _logFile){
            DBG_PRINTF("!!!! fail to open, resume failed\n");
			_errorCode = ErrorLogFailedResumeOpenFile;
            return false;
		}
		DBG_PRINTF("resume file:%s size:%d\n",filename, _logFile.size());

		int dataRead;
		size_t offset=0;
		int    processIndex=0;
		uint8_t tag, mask;


		dataRead=_logFile.read((uint8_t*)_logBuffer,LogBufferSize);
		DBG_PRINTF("read:%d\n",dataRead);
		if(dataRead < 8){
            DBG_PRINTF("resume failed\n");
			_logFile.close();
			_errorCode = ErrorLogFailedResumeReadFile;
			return false;
		}

		tag= _logBuffer[processIndex++];
		mask = _logBuffer[processIndex++];

		if(tag == StartLogTag){
			_calibrating = (mask & MaskCalibration) != MaskCalibration;
			_usePlato = (mask & MaskPlato) != MaskPlato;
			DBG_PRINTF("resume cal:%d\n",_calibrating);
			processIndex += 6;
		}else{
			DBG_PRINTF("resume failed, no start tag\n");
			_logFile.close();
			_errorCode = ErrorLogFailedResumeInvalidFormat;
			return false;
		}
		do{
			while((dataRead-processIndex)>=2){
				//DBG_PRINTF("dataavail:%ld\n",dataRead-processIndex);
				tag= _logBuffer[processIndex++];
				mask = _logBuffer[processIndex++];

				if(tag == PeriodTag){
					// advance one tick
		    	    _resumeLastLogTime += LoggingPeriod/1000;

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
							// get gravity data that we need							
		    				if(i == OrderGravityInfo){
								_lastGravityDeviceUpdate  = _resumeLastLogTime;
							}

							processIndex +=2;			        		
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
				}else if(tag == OriginGravityTag  || tag == SpecificGravityTag || tag == IgnoredCalPointMaskTag) {
					if (dataRead-processIndex<2 ){
						processIndex -=2;
						break;
					}
					processIndex +=2;
				}else if(tag == StateTag  || tag == ModeTag  ||tag == TargetPsiTag || tag==HumidityTag){
					// DO nothing.
				}else if(tag == CalibrationDataTag){
					processIndex += mask * 4;
				}else if(tag == TimeSyncTag){
					if (dataRead-processIndex<4 ){
						processIndex -=4;
						break;
					}
					processIndex +=4;
				}else{
					DBG_PRINTF("Unknown tag %X,%X @%X\n",tag,mask,offset+processIndex);
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
		_logIndex =0;
		_savedLength =  _logFile.size();
		DBG_PRINTF("resume, total _savedLength:%d, _logIndex:%d\n",_savedLength,_logIndex);

		_lastTempLog=0;
		_recording = true;


		// add resume tag
		_chartTime = _addResumeTag();
		//DBG_PRINTF("resume done _savedLength:%d, _logIndex:%d\n",_savedLength,_logIndex);
#if ESP32
		_logFile.close();
		_logFile=FileSystem.open(filename,"a+");
		if(! _logFile){
            DBG_PRINTF("resume failed\n");
			_errorCode = ErrorLogFailedResumeOpenForWrite;
            return false;
		}
#endif
		return true;
	}
	bool BrewLogger::startSession(const char *filename,bool calibrating,bool wobf){
		if(_recording) return false; // alread start

		_pFileInfo->writeOnBufferFull = _writeOnBufferFull = wobf;

		if(_fsspace < 100){
			DBG_PRINTF("Not enough space:%d\n",_fsspace);
			_errorCode  = ErrorLogNotEnoughSpace;
			return false;
		}
		strcpy(_pFileInfo->logname,filename);
		char buff[128];
		sprintf(buff,"%s/%s",LOG_PATH,filename);
		#if ESP32
		#if UseLittleFS
		if(!FileSystem.exists(LOG_PATH)){
			if(FileSystem.mkdir(LOG_PATH)){
				DBG_PRINTF("*%s Created",LOG_PATH);
			}else{
				DBG_PRINTF("***%s failed to creat",LOG_PATH);
				_errorCode = ErrorLogFailedToCreateDirectory;
				return false;
			}
		}
		_logFile=FileSystem.open(buff,"a+");
		#else
		// weird behaviour of ESP32 SPIFFS
		_logFile=FileSystem.open(buff,"a+");
		#endif
		#else
		_logFile=FileSystem.open(buff,"a+");
		#endif

		if(!_logFile){
			DBG_PRINTF("***Error open temp file\n");
			_errorCode = ErrorLogFailedOpenFile;
			return false;
		}

		_pFileInfo->starttime= TimeKeeper.getTimeSeconds();
		_chartTime = _pFileInfo->starttime;
		_logIndex = 0;

		_lastTempLog=0;
		_recording = true;
		_savedLength=0;

		char unit= brewPi.getUnit();
		_mode = brewPi.getMode();
		_state = brewPi.getState();

		_startLog(unit == 'F',calibrating);
		_calibrating = calibrating;
		if(calibrating){
			_addCalibrationRecords();
		}
		#if SupportPressureTransducer
		_targetPsi =0; // force to record
		#endif
		_resetTempData();
		loop(); // get once
		_addModeRecord(_mode);
		_addStateRecord(_state);

		#if EnableHumidityControlSupport
		if(humidityControl.isChamberSensorInstalled()){
			uint8_t humidity = humidityControl.humidity();
			_lastHumidity=humidity;
			_addHumidityRecord(humidity);
		}
		if(humidityControl.isRoomSensorInstalled()){
			uint8_t humidity = humidityControl.roomHumidity();
			_lastRoomHumidity=humidity;
			_addRoomHumidityRecord(humidity);
		}
		#endif


		_saveIdxFile();
		// flush to force write to file system.
		_logFile.flush();
		return true;
	}

	void BrewLogger::endSession(void){
		if(!_recording) return;
		
		if(_writeOnBufferFull){
			if(_logIndex >0){
				size_t wlen=_logFile.write((const uint8_t*)_logBuffer,_logIndex);
				DBG_PRINTF("Finished Log writen  %d\n",wlen);

				if(wlen != _logIndex){
					DBG_PRINTF("!!!write failed @ %d\n",_logIndex);
				}
				#if UseLittleFS
				_logFile.flush();
				#endif
			}
		}

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
		logData();
	}

	void BrewLogger::logData(void){
		uint8_t state, mode;
		float fTemps[5];

        state = brewPi.getState();
        mode = brewPi.getMode();
        fTemps[OrderBeerTemp] = brewPi.getBeerTemp();
        fTemps[OrderBeerSet] = brewPi.getBeerSet();
        fTemps[OrderFridgeTemp] = brewPi.getFridgeTemp();
        fTemps[OrderFridgeSet] = brewPi.getFridgeSet();
        fTemps[OrderRoomTemp] = brewPi.getRoomTemp();


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
			if( _extTiltAngle != INVALID_TILT_ANGLE){
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
				_writeBuffer(idx++,(_extTiltAngle >>8) & 0x7F);
				_writeBuffer(idx++,_extTiltAngle & 0xFF);
				_extTiltAngle = INVALID_TILT_ANGLE;
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

		if(_calibrating && _newcalibratingdata){
			_newcalibratingdata = false;
			_addCalibrationRecords();
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

		#if EnableHumidityControlSupport
		if(humidityControl.isChamberSensorInstalled() || 
			humidityControl.isRoomSensorInstalled() ){
			// To save memory, only log when data changes.
			if(humidityControl.isChamberSensorInstalled()){
					uint8_t humidity = humidityControl.humidity();
				if(_lastHumidity !=humidity){
					_lastHumidity=humidity;
					_addHumidityRecord(humidity);
				}
			}
			if(humidityControl.isRoomSensorInstalled()){
					uint8_t humidity = humidityControl.roomHumidity();
				if(_lastRoomHumidity !=humidity){
					_lastRoomHumidity=humidity;
					_addRoomHumidityRecord(humidity);
				}
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
		// _logIndex: data in buffer
		// _savedLength: data in file. However, all data are "writen" to file at the first place.
		//               Though, some data might remain in write buffer.
		// under normal condition. _savedLength = total size - _logIndex;
		//                        that is,  total size = _savedLength + _logIndex
		// in abnormal cases, the file size is total size since all data are "written".

		//DBG_PRINTF("beginCopyAfter:%d, _logIndex=%u, saved=%u, return:%u, last >= (_logIndex +_savedLength)=%c\n",last,_logIndex,_savedLength,( _logIndex+_savedLength - last), (last >= (_logIndex +_savedLength))? 'Y':'N' );
		if(last >= (_logIndex +_savedLength)){
            //DBG_PRINTF(" return:0\n");
            return 0;
        }
        //DBG_PRINTF(" return:%u\n",_logIndex+_savedLength - last);
		return ( _logIndex+_savedLength - last);
	}

	size_t BrewLogger::read(uint8_t *buffer, size_t maxLen, size_t index)	
	{
		size_t sizeRead ;

		//DBG_PRINTF("read index:%u, max:%u\n",index,maxLen);

		// the reqeust data index is more than what we have.
		if(index > (_savedLength +_logIndex)) return 0; // return whatever it wants.
    
		// the staring data is not in buffer
		if( index < _savedLength){
			size_t totalAvail = _savedLength +_logIndex - index;

			size_t size2Read  = (totalAvail > maxLen)? maxLen:totalAvail;

			size_t sizeAavailFromFile =_savedLength - index;

			size_t sizeReadFromFile = (size2Read > sizeAavailFromFile)? sizeAavailFromFile:size2Read;

			_logFile.seek(index,SeekSet);

			#if ReadFileByPortion
			sizeRead=0;
			size_t left = sizeReadFromFile;
			size_t toRead;
			do{
				toRead = (left > MaximumFileRead)? MaximumFileRead:left;
				sizeRead += _logFile.read(buffer+sizeRead,toRead);
				left -= toRead;
			}while(left>0);

			#else
			sizeRead=_logFile.read(buffer,sizeReadFromFile);
			#endif

			if(sizeRead != sizeReadFromFile){
				DBG_PRINTF("!!!Error: file read:%u of %u, file size:%u\n",sizeRead,maxLen,_logFile.size());
			}

			if(sizeRead < size2Read){
				size_t insufficient = size2Read - sizeRead;
                if(insufficient > _logIndex){
                    size_t fillsize=insufficient - _logIndex;
                    memset(buffer + sizeRead,FillTag,fillsize);
                    sizeRead += fillsize;
                    insufficient = _logIndex;
    				DBG_PRINTF("!!!Error: fill blank:%u\n",fillsize);
                }
                memcpy(buffer+ sizeRead,_logBuffer,insufficient);
				sizeRead += insufficient;
			}
			DBG_PRINTF("read file:%u, total:%u\n",sizeReadFromFile,sizeRead);

		}else{
			//DBG_PRINTF("read from buffer\n");
			// read from buffer
			size_t mIndex = index - _savedLength;
			// index should be smaller than _logIndex
			sizeRead = _logIndex - mIndex;
			if(sizeRead > maxLen) sizeRead=maxLen;
			memcpy(buffer,_logBuffer+mIndex,sizeRead);
			//DBG_PRINTF("read buffer:%u\n",sizeRead);
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
				_volatileHeader(header);
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


	void BrewLogger::_addOgRecord(uint16_t og){
		_addGravityRecord(true,og);
	}
	void BrewLogger::_addSgRecord(uint16_t sg){
		_addGravityRecord(false,sg);
	}

	void BrewLogger::addAuxTemp(float temp)
	{
		_extTemp = _convertTemperature(temp);
		//DBG_PRINTF("AuxTemp:%d\n",_extTemp);
	}
	void BrewLogger::addTiltAngle(float tilt)
	{
		_extTiltAngle = TiltEncode(tilt);
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

	void BrewLogger::_resetTempData(void)
	{
		for(int i=0;i<5;i++) _iTempData[i]=INVALID_TEMP_INT;
		_extTemp=INVALID_TEMP_INT;
		_extGravity=INVALID_GRAVITY_INT;
		_extOriginGravity=INVALID_GRAVITY_INT;
		_extTiltAngle = INVALID_TILT_ANGLE;
		#if EnableHumidityControlSupport
		_savedHumidityValue = 0xFF;
		#endif
	}

#define RESERVED_SIZE 8196*2

	void BrewLogger::_checkspace(void)
	{
#if defined(ESP32)
		_fsspace = FileSystem.totalBytes() - FileSystem.usedBytes();

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
		char unit=brewPi.getUnit();
		uint8_t mode,state;
		mode = brewPi.getMode();
		state = brewPi.getState();
		
		bool fahrenheit=(unit == 'F');

		char* ptr=buf;
		uint8_t headerTag=LOG_VERSION;
		//8
		*ptr++ = StartLogTag; // 1
		headerTag = headerTag | (fahrenheit? 0xF0:0xE0) ;
		_usePlato =theSettings.GravityConfig()->usePlato;
		if(_usePlato) headerTag = headerTag ^ MaskPlato;

		*ptr++ = headerTag; //2
		int period = LoggingPeriod/1000;
		*ptr++ = (char) (period >> 8);
		*ptr++ = (char) (period & 0xFF);
		*ptr++ = (char) (_headTime >> 24);
		*ptr++ = (char) (_headTime >> 16);
		*ptr++ = (char) (_headTime >> 8);
		*ptr++ = (char) (_headTime & 0xFF); //8
		// insert two bytes
		*ptr++ = 1; // length
		*ptr++ = theSettings.GravityConfig()->gravityDeviceType;

		// a record full of all data = 2 + 7 * 2= 16
		*ptr++ = (char) PeriodTag; //9 +2 ..
		*ptr++ = (char) 0x7F; //10
		for(int i=0;i<VolatileDataHeaderSize;i++){ // 10 + VolatileDataHeaderSize *2
			*ptr++ = _headData[i] >> 8;
			*ptr++ = _headData[i] & 0xFF;
		}
		// mode : 2
		*ptr++ = ModeTag; // 11 + VolatileDataHeaderSize*2
		*ptr++ = mode; // // 12 + VolatileDataHeaderSize*2
		// state: 2
		*ptr++ = StateTag; // 13 + VolatileDataHeaderSize*2
		*ptr++ = state;  // 14 + VolatileDataHeaderSize*2
		*ptr++ = TargetPsiTag; //15		
		*ptr++ = _targetPsi;  // 16

		#if EnableHumidityControlSupport

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
		if(calibrating) headerTag = headerTag ^ MaskCalibration ;
		if(_usePlato) headerTag = headerTag ^ MaskPlato;

		*ptr++ = headerTag;
		
		int period = LoggingPeriod/1000;
		*ptr++ = (char) (period >> 8);
		*ptr++ = (char) (period & 0xFF);
		*ptr++ = (char) (_pFileInfo->starttime >> 24);
		*ptr++ = (char) (_pFileInfo->starttime >> 16);
		*ptr++ = (char) (_pFileInfo->starttime >> 8);
		*ptr++ = (char) (_pFileInfo->starttime & 0xFF);

		// from V7, reserve a block for gravity device data
		// starting from length of this block
		// followed by the device type
		// more information like names to be come
		// b0: length of whole block
		// b1 gravity device type
		// TLV: type, length, value
		GravityDeviceConfiguration *gdCfg=theSettings.GravityConfig();
		if(gdCfg->gravityDeviceType == GravityDeviceIspindel){
			// length is ??
			const char *name = externalData.getDeviceName();
			if(name){
				uint8_t length =(uint8_t) strlen(name);
				*ptr++ = 3 + length;
				*ptr++ = GravityDeviceIspindel;

				*ptr++  = GDIIdentity;
				*ptr++  = length;
				for(uint8_t n=0;n<length;n++){
					*ptr++ = name[n];
				}
			}else{
				*ptr++ = 1;
				*ptr++ = GravityDeviceIspindel;
			}

		#if SupportBleHydrometer
		}else if(gdCfg->gravityDeviceType == GravityDeviceTilt){
			// total length: 
			*ptr++ = 4;
			*ptr++ = GravityDeviceTilt; //1
			*ptr++  = GDIIdentity;
			*ptr++  = 1;
			*ptr++  = theSettings.tiltConfiguration()->tiltColor;
		}else if(gdCfg->gravityDeviceType == GravityDevicePill){
			*ptr++ = 9;
			*ptr++ = GravityDevicePill; //1
			*ptr++  = GDIAddress;
			*ptr++  = 6;
			uint8_t *mac =theSettings.pillConfiguration()->macAddress;
			for(int m=0;m<6;m++){
				*ptr++ =mac[m];
			}
		#endif
		}else{
			// no device
			*ptr++ = 1;  // length
			*ptr++ = GravityDeviceNone;  // no device
		}
		_logIndex=0;
		_savedLength=0;

		_commitData(_logIndex,ptr - _logBuffer );

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
		_headData[6]= (_calibrating)? _extTiltAngle:_extGravity;

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

			if(tag == OriginGravityTag || tag == SpecificGravityTag){
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


		//DBG_PRINTF("before tag %d, mask=%x\n",dataDrop,mask);


		for(int i=0;i<NumberDataBitMask;i++){
			if(mask & (1<<i)){
				if(idx >= LogBufferSize) idx -= LogBufferSize;
				byte d0=_logBuffer[idx++];
				byte d1=_logBuffer[idx++];
				dataDrop +=2;
				_headData[i] = (d0<<8) | d1;
				//DBG_PRINTF("update idx:%d to %d\n",i,_headData[i]);
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

				#if EnableHumidityControlSupport
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
		else _headTime += LoggingPeriod/1000;
		interrupts();
		//DBG_PRINTF("Drop %d\n",dataDrop);
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

			if(_writeOnBufferFull){
				size_t wlen=_logFile.write((const uint8_t*)_logBuffer,_logIndex);
				DBG_PRINTF("Log writen  %d\n",wlen);

				if(wlen != _logIndex){
					DBG_PRINTF("!!!write failed @ %d\n",_logIndex);
				}
				#if UseLittleFS
				_logFile.flush();
				#endif
			}

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
		if(!_writeOnBufferFull){
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
			#if UseLittleFS
			_logFile.flush();
			#endif
		}
	}
	void BrewLogger::_addCalibrationRecords(void){

		GravityDeviceConfiguration *gdc=theSettings.GravityConfig();

		int start = _allocByte(2 + 4 * gdc->numCalPoints);
		if(start < 0) return;

			// 
		_writeBuffer(start, CalibrationDataTag);
		_writeBuffer(start + 1,gdc->numCalPoints);
		int idx=2;

		for(int i=0;i<gdc->numCalPoints;i++){
				_writeBuffer(start + idx,(uint8_t) (gdc->calPoints[i].raw >>8));
				idx++;
				_writeBuffer(start + idx,(uint8_t) (gdc->calPoints[i].raw & 0xFF));
				idx++;
				_writeBuffer(start + idx,(uint8_t) (gdc->calPoints[i].calsg >>8));
				idx++;
				_writeBuffer(start + idx,(uint8_t) (gdc->calPoints[i].calsg &0xFF));
				idx++;
		}

		_commitData(start,idx);
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

#if EnableHumidityControlSupport	
	void BrewLogger::_addHumidityRecord(uint8_t humidity){
		int idx = _allocByte(2);
		if(idx < 0) return;
		_writeBuffer(idx,HumidityTag); //*ptr = TargetPsiTag;
		_writeBuffer(idx+1,humidity); //*(ptr+1) = mode;
		_commitData(idx,2);
	}

	void BrewLogger::_addRoomHumidityRecord(uint8_t humidity){
		int idx = _allocByte(2);
		if(idx < 0) return;
		_writeBuffer(idx,HumidityTag); //*ptr = TargetPsiTag;
		_writeBuffer(idx+1,humidity | 0x80); //*(ptr+1) = mode;
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
			DBG_PRINTF("abnormal resume, start:%lu, current:%u gap:%u\n",_pFileInfo->starttime,rtime,gap);
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
		_writeOnBufferFull = _pFileInfo->writeOnBufferFull;
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
