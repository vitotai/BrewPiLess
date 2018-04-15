#ifndef BrewKeeper_H
#define BrewKeeper_H
#include <Arduino.h>
#include "Config.h"
#include "GravityTracker.h"
#include "BPLSettings.h"
// always enabled #if EnableGravitySchedule


class BrewProfile
{
	BeerTempSchedule *_schedule;
	BrewStatus  *_status;

	char _unit;
	uint8_t _stableThreshold;

	void _tempConvert(void);

	void _estimateStep(time_t now,Gravity gravity);

	void _toNextStep(unsigned long time);
	bool checkCondition(unsigned long time,Gravity gravity);
	bool _loadProfile(String filename);
	uint32_t currentStepDuration(void);
	void _saveBrewingStatus(void);
public:
	BrewProfile(void):_unit('U'){
    	_stableThreshold = 1;
		_schedule =  theSettings.beerTempSchedule();
		_status =  theSettings.brewStatus();
	}

	void setUnit(char unit);

	void setOriginalGravity(float gravity);
	float tempByTimeGravity(unsigned long time,Gravity gravity);
	void setStableThreshold(uint8_t threshold){ _stableThreshold=threshold; }
};


class BrewKeeper
{
protected:
	time_t _lastSetTemp;

	BrewProfile _profile;
	Gravity _lastGravity;

	void (*_write)(const char*);
	void _loadProfile(void);
public:

	BrewKeeper(void(*puts)(const char*)):_write(puts),_lastGravity(INVALID_GRAVITY){}
	void updateGravity(float sg){ _lastGravity=FloatToGravity(sg);}
	void updateOriginalGravity(float sg){ _profile.setOriginalGravity(sg); }

	void keep(time_t now);

	void setStableThreshold(uint8_t threshold){_profile.setStableThreshold(threshold);}
};

#endif
