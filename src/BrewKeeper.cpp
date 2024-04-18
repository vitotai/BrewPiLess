#include <FS.h>
#include <ArduinoJson.h>
#include "TimeKeeper.h"

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

#define PointToGravity(p) (1000+(Gravity)((p)+0.5))
#define GravityPoint(p) ((Gravity)((p)-1000))
#define SG2Gravity(sg) (uint16_t)(((sg))*1000 +0.5)
#define Plato2Gravity(p) (uint16_t) ((p) * 10 + 0.5)

void BrewKeeper::updateGravity(float sg){
	if(theSettings.GravityConfig()->usePlato)
	_lastGravity=Plato2Gravity(sg);
	else
	_lastGravity=SG2Gravity(sg);
}

void BrewKeeper::updateOriginalGravity(float sg){ 
	if(theSettings.GravityConfig()->usePlato)
		_profile.setOriginalGravityPoint(Plato2Gravity(sg));
	else
		_profile.setOriginalGravityPoint(GravityPoint(SG2Gravity(sg))); 
}


#if VERIFY_BEER_PROFILE
String BrewKeeper::currentStatus(){
	return _profile.currentStatus();
}
#endif

void BrewKeeper::keep(time_t now)
{
	//DBG_PRINTF("BrewKeeper:%lu last:%lu\n",now,_lastSetTemp);
	if((now - _lastSetTemp) < MINIMUM_TEMPERATURE_SETTING_PERIOD) return;
	_lastSetTemp= now;

	char unit, mode;
	float beerSet;
	unit = brewPi.getUnit();
	mode = brewPi.getMode();
	beerSet = brewPi.getBeerSet();

	// run in loop()
	if (mode != 'p') return;

	// check unit
	_profile.setUnit(unit);

	float temp=_profile.tempByTimeGravity(now,_lastGravity);
	DBG_PRINTF("**beerpfoile:Temp:%d\n",(int)(temp *100));

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
void BrewKeeper::setModeFromRemote(char mode){
	char ori_mode;
	ori_mode = brewPi.getMode();
	if(mode == 'p' && ori_mode != 'p') _profile.setScheduleStartDate(TimeKeeper.getTimeSeconds());
	char buff[36];
	sprintf(buff,"j{mode:%c}",mode);
	DBG_PRINTF("write:%s\n",buff);
	_write(buff);
}

void BrewKeeper::setBeerSet(char* tempStr){
	char buff[36];
	sprintf(buff,"j{beerSet:%s}",tempStr);
	DBG_PRINTF("*write:%s\n",buff);
	_write(buff);
}

void BrewKeeper::setFridgeSet(char* tempStr){
	char buff[36];
	sprintf(buff,"j{fridgeSet:%s}",tempStr);
	DBG_PRINTF("write:%s\n",buff);
	_write(buff);
}

//**********************************************************************************
//class BrewProfile
//**********************************************************************************

void BrewProfile::setScheduleStartDate(time_t time){
	_schedule->startDay=time;
}

void BrewProfile::setUnit(char unit)
{
	_unit = unit;
}


void BrewProfile::setOriginalGravityPoint(uint16_t gravityPoint){
    _status->OGPoints =gravityPoint;
    _saveBrewingStatus();
}

#define MAX_BREWING_STATE_LEN 256

void BrewProfile::_saveBrewingStatus(void){
	theSettings.save();
}


void BrewProfile::_estimateStep(time_t now,Gravity gravity)
{
	_status->startingDate= _schedule->startDay;
	_status->timeEnterCurrentStep = _schedule->startDay;
	_status->currentStep =0;
	while(_status->currentStep<_schedule->numberOfSteps && _status->timeEnterCurrentStep <= now)
	{
		uint32_t csd=currentStepDuration();
		if(checkCondition(now,gravity)){
			_status->timeEnterCurrentStep += csd;
			_status->currentStep++;
		}else{
			break;
		}
	}
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
	_status->startingDate= _schedule->startDay;
	_saveBrewingStatus();
	DBG_PRINTF("\n>>>>>_toNextStep:%d current:%lu, duration:%u<<<<<\n\n",_status->currentStep,time, csd );
}

#if VERIFY_BEER_PROFILE
String BrewProfile::currentStatus(){
	if(_status->currentStep >= _schedule->numberOfSteps){
		return String("Reach end of profiles.");
	}
	ScheduleStep *step = & _schedule->steps[_status->currentStep];

	return String("state:") + String(_status->currentStep)
		+ String(" condition:") + String(step->condition)
		+ String(" time enter step:") + String(_status->timeEnterCurrentStep)
		+ String(" duration:") +  String( ScheduleDayToTime(step->days))
		+ String(" Gravity:") + String(step->gravity.sg)
		+ String(" Stable:") +String(step->stable.stableTime)
		+ String(" StablePoint:") +String(step->stable.stablePoint)
		;
}
#endif


bool BrewProfile::checkCondition(unsigned long time,Gravity gravity){

	ScheduleStep *step = & _schedule->steps[_status->currentStep];

	char condition=step->condition;

	uint32_t csd = ScheduleDayToTime(step->days);

	bool timeCondition =(csd <= (time - _status->timeEnterCurrentStep));
	
	DBG_PRINTF("**checkConditon: %c, time:%c\n",condition,timeCondition? 'Y':'N');

	if(condition == 'r' || condition == 't'){
		if(timeCondition) return true;
	}else{
		// easier to get the comparison result:
		bool sgCondition=false;
		Gravity stepSG=step->gravity.sg;
		if(step->attSpecified) {
			if(theSettings.GravityConfig()->usePlato)
				stepSG =(float) _status->OGPoints * (1.0 - (float)step->gravity.attenuation/100.0);
			else
				stepSG =PointToGravity(((float) _status->OGPoints * (1.0 - (float)step->gravity.attenuation/100.0)));
		}
		bool stableSg = false;
		if(IsGravityValid(gravity) ){
			if(IsGravityValid(stepSG)) sgCondition=(gravity <= stepSG);
			stableSg = gravityTracker.stable(step->stable.stableTime,step->stable.stablePoint);
		}
		DBG_PRINTF("**tempByTimeGravity: sgC:%c,gravity=%d, target=%d stable:%c\n",sgCondition? 'Y':'N',
			gravity,stepSG, stableSg? 'Y':'N');
	
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

float BrewProfile::tempByTimeGravity(time_t time,Gravity gravity)
{
	if(time < _schedule->startDay) return INVALID_CONTROL_TEMP;

	//DBG_PRINTF("*currentStep:%d, timeEnterCurrentSTep:%ld, time:%ld\n",_status->currentStep,_status->timeEnterCurrentStep,time);

	if(	_status->startingDate ==0 
	   || _status->startingDate != _schedule->startDay
		|| (_status->currentStep==0 && _status->timeEnterCurrentStep==0)){
		_estimateStep(time,gravity);
	}
	if(_status->currentStep >= _schedule->numberOfSteps) return INVALID_CONTROL_TEMP;

	DBG_PRINTF("*tempByTimeGravity:now:%ld, step:%d, type=%c, last elapsed:%ld, step duration:%u\n",(long)time,
		_status->currentStep,_schedule->steps[_status->currentStep].condition,(long)(time - _status->timeEnterCurrentStep),
		ScheduleDayToTime(_schedule->steps[_status->currentStep].days));

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

void  BrewProfile::profileUpdated(){
	// the beer profile is updated: update status
	if(_schedule->startDay != _status->startingDate){
		// update current status.
		_status->startingDate=0;
		_status->currentStep=0;
		_status->timeEnterCurrentStep=0;
		DBG_PRINTF("**New Profile\n");
	}else{
		// assume the same schedule. do nothing.
	}
}
