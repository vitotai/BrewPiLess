#ifndef BrewKeeper_H
#define BrewKeeper_H
#include <Arduino.h>
#include "espconfig.h"

class BrewProfile
{
	time_t _startDay;
	int  _numberOfSteps;
	time_t *_times;
	float  *_setTemps;
	bool _profileLoaded;
	char _unit;
	
	void _tempConvert(void);
public:
	BrewProfile(void):_profileLoaded(false),_numberOfSteps(0),_unit('U'),_setTemps(NULL),_times(NULL){}
	int numberOfSteps(void){ return _numberOfSteps;}
	bool loaded(void){return _profileLoaded;}

	void setUnit(char unit);
	bool load(String filename);
	float tempByTime(unsigned long time);
	void reload(void){_profileLoaded=false;}
};

class BrewKeeper
{
protected:
	time_t _lastSetTemp;
	
	BrewProfile _profile;
	String _filename;
	void (*_write)(const char*);
	void _loadProfile(void);
public:
	BrewKeeper(void(*puts)(const char*)):_filename(NULL),_write(puts){}
	
	void setFile(String filename){_filename=filename;}
	void keep(time_t now,char unit,char mode,float beerSet);
	void reloadProfile(){ _profile.reload(); _lastSetTemp=0;}
};

#endif













