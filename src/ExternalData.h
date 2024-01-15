#ifndef ExternalData_H
#define ExternalData_H
#include "BrewPiProxy.h"
#include "BrewKeeper.h"
#include "BrewLogger.h"
#include "mystrlib.h"
#include "BPLSettings.h"

#if BREWPI_EXTERNAL_SENSOR
#include "TempSensorWireless.h"
#endif

#if SupportTiltHydrometer 
#include "BleTiltListener.h"
#endif

#if SupportPillHydrometer
#include "BlePillListener.h"
#endif

#define INVALID_VOLTAGE -1
#define INVALID_GRAVITY -1
#ifdef INVALID_TEMP
#undef INVALID_TEMP
#endif
#define INVALID_TEMP  -250

#define IsVoltageValid(v) ((v) > 0)
//#define IsGravityValid(g) ((g) > 0)

//#define IsGravityInValidRange(g) ((g) > 0.8 && (g) < 1.25)
#define GavityDeviceConfigFilename "/gdconfig"
#define MAX_CONFIGDATA_SIZE 256

extern BrewKeeper brewKeeper;
extern BrewLogger brewLogger;
extern BrewPiProxy brewPi;

#define MimimumDifference 0.0000001

#define ErrorNone 0
#define ErrorAuthenticateNeeded 1
#define ErrorJSONFormat 2
#define ErrorMissingField 3
#define ErrorUnknownSource 4

#define C2F(t) ((t)*1.8+32)

class SimpleFilter
{
	float _y;
	float _b;
public:
	SimpleFilter(){ _b = 0.1;}
	void setInitial(float v){ _y=v;}
	void setBeta(float b) { _b = b; }
	float beta(void){ return _b; }

	float addData(float x){
		_y = _y + _b * (x - _y);
		return _y;
	}
};

class ExternalData
{
protected:

	// shared values for all devices: gravity(calculated or not), temperature, RSSI, last update time
	float _gravity;
	float _auxTemp;
	time_t _lastUpdate;
	int16_t _rssi;
	bool _rssiValid;

	// iSpindel and Pill: battery, tilt Angle

	float  _deviceVoltage;
	float _tiltAngle;
	
	// iSpindel specific: name
    char *_ispindelName;


	bool  _calibrating;
	float _filteredGravity;
	SimpleFilter filter;

	#if SupportTiltHydrometer || SupportPillHydrometer
	uint16_t _tiltRawGravity;
	BleDeviceListener  *_bleHydrometer; // tilt or pill
	uint8_t   _bleHydrometerType;
	#endif

	GravityDeviceConfiguration *_cfg;

	float _calculateGravitybyAngle(float tilt,float temp);
	void _setGravity(float sg, time_t now,bool log=true);
	void _setAuxTemperatureCelsius(float temp);
	void _setOriginalGravity(float og);	
	void _setDeviceRssi(int16_t rssi){_rssi = rssi;  _rssiValid=true;}

	void _reconfig(void);
	#if SupportTiltHydrometer
	void _gotTiltInfo(TiltHydrometerInfo* info);
	#endif
	#if SupportPillHydrometer
	void _gotPillInfo(PillHydrometerInfo* info);
	#endif

public:
	ExternalData(void):_gravity(INVALID_GRAVITY),_auxTemp(INVALID_TEMP),
	_lastUpdate(0),_deviceVoltage(INVALID_VOLTAGE)
	,_ispindelName(NULL),_calibrating(false),_rssiValid(false){
		_filteredGravity = INVALID_GRAVITY;
		#if SupportTiltHydrometer || SupportPillHydrometer		
		_bleHydrometerType = GravityDeviceNone;
		#endif
	}


	// to prevent from calculate gravity when no valid formula available.
	void waitFormula();
	void setCalibrating(bool cal){ _calibrating=cal;}
	//configuration reading
//    bool iSpindelEnabled(void);

//	float hydrometerCalibrationTemp(void);

    void gravityDeviceSetting(char *buf);
	//configuration processs
    bool processconfig(char* configdata);
	void loadConfig(void);
	//update formula
	void setFormula(float coeff[4],uint32_t npt);

	void setUpdateTime(time_t update){ _lastUpdate=update;}

	// for remote data logger
	float gravity(bool filtered=false);
	float plato(bool filtered=false);
    bool gravityDeviceEnabled(void);
	float auxTemp(void){return _auxTemp; }
	time_t lastUpdate(void){return _lastUpdate;}
	int16_t rssi(void){return _rssiValid? _rssi:-999;}
	float deviceVoltage(void){return _deviceVoltage;}
	float tiltValue(void){return _tiltAngle;}
	
	// to process web request
	bool processGravityReport(char data[],size_t length, bool authenticated, uint8_t& error);
};

extern ExternalData externalData;

#endif
