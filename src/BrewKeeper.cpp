#include <FS.h>
#include <ArduinoJson.h>

#include "BrewPiProxy.h"
#include "BrewKeeper.h"
#include "mystrlib.h"
#define F2C(d) (((d)-32)/1.8)
#define C2F(d) (((d)*1.8)+32)


#define BrewStatusFile "/brewing.s"
#define CurrentProfileVersion 2

#define OUT_OF_RANGE(a,b,c) ((((a) - (b)) > (c)) || (((a) - (b)) < -(c)))
#define IS_INVALID_CONTROL_TEMP(t) ((t)< -99.0)
#define INVALID_CONTROL_TEMP -100.0

void BrewKeeper::keep(time_t now)
{
	if((now - _lastSetTemp) < MINIMUM_TEMPERATURE_SETTING_PERIOD) return;
	_lastSetTemp= now;

	char unit, mode;
	float beerSet,fridgeSet;
	brewPi.getControlParameter(&unit,&mode,&beerSet,&fridgeSet);

	// run in loop()
	if (mode != 'p') return;

	// check unit
	_profile.setUnit(unit);

	float temp=_profile.tempByTimeGravity(now,_lastGravity);

	if(IS_INVALID_CONTROL_TEMP(temp)) return;
	if(OUT_OF_RANGE(temp,beerSet,MINIMUM_TEMPERATURE_STEP)){
		// set temp
		//_write("j{beerSet:" + temp + "}");
		char buff[36];
		strcpy(buff,"j{beerSet:");
		int len=strlen(buff);
		len+=sprintFloat(buff+len,temp,2);
		strcpy(buff+len,"}");

		//BPSerial.print(buff);
		_write(buff);

	}
}

//**********************************************************************************
//class BrewProfile
//**********************************************************************************


void BrewProfile::setUnit(char unit)
{
	_unit = unit;
}


void BrewProfile::setOriginalGravity(float gravity){
    _status->OGPoints =(uint8_t)( (gravity - 1.0) * 1000.0);
    _saveBrewingStatus();
}

#define MAX_BREWING_STATE_LEN 256

void BrewProfile::_saveBrewingStatus(void){
	theSettings.save();
}


void BrewProfile::_estimateStep(time_t now,Gravity gravity)
{
	uint32_t timeEnterCurrentStep = _schedule->startDay;
	uint8_t currentStep =0;
	while(currentStep<_schedule->numberOfSteps)
	{
		uint32_t csd=currentStepDuration();
		if(checkCondition(now,gravity)){
			timeEnterCurrentStep += csd;
			currentStep++;
		}else{
			break;
		}
	}
	_status->currentStep = currentStep;
	_status->timeEnterCurrentStep = timeEnterCurrentStep;
}

void BrewProfile::_toNextStep(unsigned long time)
{
	uint32_t csd;
	do{
		_status->currentStep++;
		if(_status->currentStep < _schedule->numberOfSteps)
			csd = currentStepDuration();
	}while(csd == 0 && _status->currentStep < _schedule->numberOfSteps );
	_status->timeEnterCurrentStep=time;	
	_saveBrewingStatus();
	DBG_PRINTF("_toNextStep:%d current:%ld, duration:%ld\n",_status->currentStep,time, csd );
}

bool BrewProfile::checkCondition(unsigned long time,Gravity gravity){

	ScheduleStep *step = & _schedule->steps[_status->currentStep];

	char condition=step->condition;
	uint32_t csd = currentStepDuration();
	bool timeCondition =(csd <= (time - _status->timeEnterCurrentStep));
	
	if(condition == 'r' || condition == 't'){
		if(timeCondition) return true;
	}else{
		// easier to get the comparison result:
		bool sgCondition=false;
		Gravity stepSG=step->gravity.sg;
		if(step->attSpecified) {
			stepSG =PointToGravity(((float) _status->OGPoints * (1.0 - (float)step->gravity.attenuation/100.0)));
		}
		if(IsGravityValid(stepSG)) sgCondition=(stepSG <= stepSG);
		bool stableSg = gravityTracker.stable(step->gravity.stable.stableTime,step->gravity.stable.stablePoint);

		DBG_PRINTF("tempByTimeGravity: sgC:%c,gravity=%d, target=%d\n",sgCondition? 'Y':'N',
			gravity,_schedule->steps[_status->currentStep].gravity.sg);
	
		if(condition == 'g'){
			if(sgCondition) return true;
		}else if(condition == 'a'){
			if(timeCondition && sgCondition) return true;
		}else if(condition == 'o'){
			if(timeCondition || sgCondition) return true;
		}else if(condition == 's'){ // stable
			if(stableSg ) return true;
		}else if(condition == 'u'){ // time || stable
			if(timeCondition  || stableSg ) return true;
		}else if(condition == 'v'){ // time && stable
			if(timeCondition  && stableSg ) return true;
		}else if(condition == 'b'){ // sg || stable
			if(sgCondition  || stableSg ) return true;
		}else if(condition == 'x'){ // sg && stable
			if(sgCondition  && stableSg ) return true;
		}else if(condition == 'w'){ // time && sg && stable
			if(sgCondition  && timeCondition && stableSg ) return true;
		}else if(condition == 'e'){ // time || sg || stable
			if(sgCondition  || timeCondition || stableSg ) return true;
		}
	}
	return false;
}

float BrewProfile::tempByTimeGravity(unsigned long time,Gravity gravity)
{
	if(time < _schedule->startDay) return INVALID_CONTROL_TEMP;

	if(	_status->currentStep==0 && _status->timeEnterCurrentStep==0){
		_estimateStep(time,gravity);
	}
	if(_status->currentStep >= _schedule->numberOfSteps) return INVALID_CONTROL_TEMP;

	DBG_PRINTF("tempByTimeGravity:now:%ld, step:%d, type=%c, last elapsed:%ld\n",time,
		_status->currentStep,_schedule->steps[_status->currentStep].condition,time - _status->timeEnterCurrentStep);

    if(checkCondition(time,gravity)){
    		// advance to next stage
    		_toNextStep(time);
    }

	if(_status->currentStep >= _schedule->numberOfSteps) return INVALID_CONTROL_TEMP;

	float target;

	if(_schedule->steps[_status->currentStep].condition == 'r'){
		// ramping
		if(_status->currentStep ==0 || _status->currentStep >= (_schedule->numberOfSteps-1))
			return INVALID_CONTROL_TEMP;

		float prevTemp=ScheduleTemp( _schedule->steps[_status->currentStep-1].temp);
		float nextTemp=ScheduleTemp( _schedule->steps[_status->currentStep+1].temp);
		
		uint32_t csd =currentStepDuration();

		float interpolatedTemp = ((float)(time - _status->timeEnterCurrentStep)/
				(float)(csd) * (nextTemp - prevTemp) + prevTemp);
    	
		target=interpolatedTemp = roundf(interpolatedTemp*10.0)/10.0;
    }else{
	    target= ScheduleTemp(_schedule->steps[_status->currentStep].temp);
	}
	if(_unit == _schedule->unit) return target;
	else if(_unit == 'C' ) return F2C(target);
	else return C2F(target);
}

uint32_t BrewProfile::currentStepDuration(void){
	return ScheduleDayToTime(_schedule->steps[_status->currentStep].days);
}
