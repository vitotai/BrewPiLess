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

#define INVALID_VOLTAGE -1
#define INVALID_GRAVITY -1
#ifdef INVALID_TEMP
#undef INVALID_TEMP
#endif
#define INVALID_TEMP  -250

#define IsVoltageValid(v) ((v) > 0)
//#define IsGravityValid(g) ((g) > 0)

#define IsGravityInValidRange(g) ((g) > 0.8 && (g) < 1.25)
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
	float _gravity;
	float _auxTemp;
	time_t _lastUpdate;
	float  _deviceVoltage;
//	float _og;
	SimpleFilter filter;
    char *_ispindelName;
	float _ispindelTilt;
	bool  _calibrating;
	float _filteredGravity;


	GravityDeviceConfiguration *_cfg;

	float temperatureCorrection(float sg, float t, float c);

	void setTilt(float tilt,float temp,time_t now);
	void setGravity(float sg, time_t now,bool log=true);
	void setAuxTemperatureCelsius(float temp);
	void setOriginalGravity(float og);	
public:
	ExternalData(void):_gravity(INVALID_GRAVITY),_auxTemp(INVALID_TEMP),
	_deviceVoltage(INVALID_VOLTAGE),_lastUpdate(0)
	,_ispindelName(NULL),_calibrating(false)
	{ _filteredGravity = INVALID_GRAVITY;}

	float gravity(bool filtered=false){ return filtered? _filteredGravity:_gravity;}

	// to prevent from calculate gravity when no valid formula available.
	void waitFormula();
	void setCalibrating(bool cal){ _calibrating=cal;}
	//configuration reading
    bool iSpindelEnabled(void);
	float hydrometerCalibration(void);

    void sseNotify(char *buf);
	//configuration processs
    bool processconfig(char* configdata);
	void loadConfig(void);
	//update formula
	void formula(float coeff[4],uint32_t npt);


	// for remote data logger
	float auxTemp(void){return _auxTemp; }
//	void setUpdateTime(time_t update){ _lastUpdate=update;}
	time_t lastUpdate(void){return _lastUpdate;}
	void setDeviceVoltage(float vol){ _deviceVoltage = vol; }
	float deviceVoltage(void){return _deviceVoltage;}
	float tiltValue(void){return _ispindelTilt;}
	void invalidateDeviceVoltage(void) { _deviceVoltage= INVALID_VOLTAGE; }

	bool processGravityReport(char data[],size_t length, bool authenticated, uint8_t& error);
};

extern ExternalData externalData;

#endif
