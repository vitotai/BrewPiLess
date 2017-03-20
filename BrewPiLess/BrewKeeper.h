#ifndef BrewKeeper_H
#define BrewKeeper_H
#include <Arduino.h>
#include "espconfig.h"


typedef int16_t Gravity;

#define INVALID_GRAVITY -1
#define IsGravityValid(g) ((g)>0)
#define FloatToGravity(f) ((Gravity)((f) * 1000.0))
#define GravityToFloat(g) (((float)(g) / 1000.0))

typedef struct _ProfileStep{
 float    temp;
 float    days;
 Gravity    sg;
 char     condition;
} ProfileStep;

class BrewProfile
{
	time_t _startDay;
	
	int  _numberOfSteps;
	
	int  _currentStep;
	time_t _timeEnterCurrentStep;
	time_t _currentStepDuration;

	ProfileStep *_steps;

	bool _profileLoaded;
	char _unit;
	
	void _tempConvert(void);
	void _loadBrewingStatus(void);
	void _saveBrewingStatus(void);
	void _estimateStep(time_t now);
	void _toNextStep(unsigned long time);
public:
	BrewProfile(void):_profileLoaded(false),_numberOfSteps(0),_unit('U'),_steps(NULL){}
	int numberOfSteps(void){ return _numberOfSteps;}
	bool loaded(void){return _profileLoaded;}

	void setUnit(char unit);
	bool load(String filename);
	float tempByTimeGravity(unsigned long time,Gravity gravity);

	void reload(void){_profileLoaded=false;}
};


class BrewKeeper
{
protected:
	time_t _lastSetTemp;
	
	BrewProfile _profile;
	String _filename;
	Gravity _lastGravity;
	
	void (*_write)(const char*);
	void _loadProfile(void);
public:
	BrewKeeper(void(*puts)(const char*)):_filename(NULL),_write(puts),_lastGravity(INVALID_GRAVITY){}
	void updateGravity(float sg){ _lastGravity=FloatToGravity(sg);}
	void setFile(String filename){_filename=filename;}
//	void keep(time_t now,char unit,char mode,float beerSet);
	void keep(time_t now);
	void reloadProfile(){ _profile.reload(); _lastSetTemp=0;}
};

#endif








































































































































































































