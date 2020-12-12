#ifndef BrewLogger_H
#define BrewLogger_H
#include <FS.h>

#if defined(ESP32)
#if UseLittleFS
#include <LittleFS.h>
#else
#include <SPIFFS.h>
#endif
#endif
extern FS& FileSystem;

#include "BPLSettings.h"
#include "TimeKeeper.h"

#define LOG_VERSION 0x6

#define INVALID_RECOVERY_TIME 0xFF
#define INVALID_TEMPERATURE -250
#define INVALID_GRAVITY -1

#define LOG_PATH "/log"

#define LogBufferSize 1024
// Log tags

#define PeriodTag 0xF0
#define StateTag 0xF1
#define EventTag 0xF2
#define CorrectionTempTag 0xF3
#define ModeTag 0xF4

#define TargetPsiTag 0xF5
#define TimeSyncTag 0xF6

#define FillTag 0xF7
#define OriginGravityTag 0xF8
#define CalibrationPointTag 0xF9
#define IgnoredCalPointMaskTag 0xFA

#define SpecificGravityTag 0xFB

#define HumidityTag 0xFC
#define HumiditySetTag 0xFD

#define ResumeBrewTag 0xFE
#define StartLogTag 0xFF


#define INVALID_TEMP_INT 0x7FFF
#define INVALID_GRAVITY_INT 0x7FFF

#define VolatileDataHeaderSize 7
#if EnableDHTSensorSupport
#define VolatileHeaderSize ( VolatileDataHeaderSize*2 + 18)
#else
#define VolatileHeaderSize ( VolatileDataHeaderSize*2 + 16)
#endif

#define OrderBeerSet 0
#define OrderBeerTemp 1
#define OrderFridgeTemp 2
#define OrderFridgeSet 3
#define OrderRoomTemp 4
#define OrderExtTemp 5
//#define OrderGravity 6
//#define OrderTiltAngle 7
// use one for Graviyt or Tilt only.
// when in calibrating, record only Tilt
// else record gravity only
#define OrderGravityInfo  6
#define OrderPressure 7

#define NumberDataBitMask 8

#define TiltEncode(g) (uint16_t)(100.0 * (g) + 0.5)
#define INVALID_TILT_ANGLE 0x7FFF

#define GravityEncode(g) (uint16_t)(10000.0 * (g) + 0.5)
#define GravityDecode(a) (float)(a)/10000.0
#define PlatoEncode(g) (uint16_t)(100.0 * (g) + 0.5)
#define PlatoDecode(a) (float)(a)/100.0

#define HighOctect(a) (uint8_t)((a)>>8) 
#define LowOctect(a) (uint8_t)((a)&0xFF)


#define INVALID_PRESSURE_INT 0x7FFF
#define PressureEncode(p) (((p)>125 || (p)<-50)? INVALID_PRESSURE_INT:(int16_t)(10.0 * ((p) + 100 )+ 0.5))
#define PressureDecode(p) (float)(p)/10.0


class BrewLogger
{

public:
	BrewLogger(void);
	
	bool begin(void);

	String fsinfo(void);
	const char* currentLog(void);

	String loggingStatus(void);
	void rmLog(int index);
	bool isLogging(void){ return _recording; }

	bool startSession(const char *filename,bool calibrating);
	void endSession(void);
	bool resumeSession();

	void loop(void);
	void logData(void);

	size_t beginCopyAfter(size_t last);
	size_t read(uint8_t *buffer, size_t maxLen, size_t index);
	void getFilePath(char* buf,int index);
	// read data
	size_t volatileStartIndex(void);
	// read data
	size_t volatileDataAvailable(void);
	size_t volatileDataAvailable(size_t logicalAfter);
	size_t readVolatileData(uint8_t *buffer, size_t maxLen, size_t index, size_t logicalAfter);
	// add data
	void addGravity(float gravity,bool isOg=false);
	void addAuxTemp(float temp);
	void addTiltAngle(float tilt);
	void addCorrectionTemperature(float temp);
	void addTiltInWater(float tilt,float reading);
	bool isCalibrating(void){ return _calibrating;}
	void addIgnoredCalPointMask(uint32_t mask);
	//format file system
	void onFormatFS(void);
private:
	size_t _fsspace;
	uint32_t  _chartTime;
	uint32_t _lastTempLog;
    uint32_t _resumeLastLogTime;
	uint32_t _trackedTime;

	bool _recording;
	bool _calibrating;

	size_t _logIndex;
	size_t _savedLength;
	size_t _lastRead;
	char _logBuffer[LogBufferSize];

	File    _logFile;

	// brewpi specific info
	uint8_t _mode;
	uint8_t _state;
	bool _usePlato;
	
	uint16_t  _iTempData[5];
	uint16_t  _extTemp;
	uint16_t  _extGravity;
	uint16_t  _extOriginGravity;
	uint16_t  _extTileAngle;

	int16_t  _lastPressureReading;

	// for circular buffer
	int _logHead;
	uint32_t _headTime;
	uint32_t _startOffset;
	//bool _sendHeader;
	//uint32_t _sendOffset;
	FileIndexes *_pFileInfo;
	uint8_t _targetPsi;

#if EnableDHTSensorSupport	
	uint8_t _lastHumidity;
	uint8_t _savedHumidityValue;
	uint8_t _lastHumidityTarget;
	uint8_t _savedHumidityTarget;
#endif

	uint16_t  _headData[VolatileDataHeaderSize];

	void _resetTempData(void);
	void _checkspace(void);

	void _volatileHeader(char *buf);

	void _startLog(bool fahrenheit,bool calibrating);
	void _startVolatileLog(void);
	int _availableBufferSpace(void);
	void _dropData(void);
	int _volatileLoggingAlloc(int size);
	int _allocByte(byte size);
	void _writeBuffer(int idx,uint8_t data);
	void _commitData(int idx,int len);
	void _addOgRecord(uint16_t og);
	void _addSgRecord(uint16_t sg);
	void _addGravityRecord(bool isOg, uint16_t gravity);
#if EnableDHTSensorSupport	
	void _addHumidityRecord(uint8_t humidity);
	void _addHumidityTargetRecord(uint8_t target);
#endif

	void _addModeRecord(char mode);
	uint32_t _addResumeTag(void);
	void _addTimeSyncTag(void);
	void _addStateRecord(char state);
	uint16_t _convertTemperature(float temp);
	void _addTargetPsiRecord(void);

	void _loadIdxFile(void);
	void _saveIdxFile(void);
};

extern BrewLogger brewLogger;
#endif
