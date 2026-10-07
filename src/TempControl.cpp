/*
 * Copyright 2012-2013 BrewPi/Elco Jacobs.
 *
 * This file is part of BrewPi.
 *
 * BrewPi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BrewPi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BrewPi.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Brewpi.h"

#include "Pins.h"
#include <limits.h>

#include "TemperatureFormats.h"
#include "TempControl.h"
#include "PiLink.h"
#include "TempSensor.h"
#include "Ticks.h"
#include "TempSensorMock.h"
#include "EepromManager.h"
#include "TempSensorDisconnected.h"
#include "RotaryEncoder.h"
#include "Config.h"
#include <string.h>
#include <stdlib.h>

TempControl tempControl;

#if TEMP_CONTROL_STATIC

extern ValueSensor<bool> defaultSensor;
extern ValueActuator defaultActuator;
extern DisconnectedTempSensor defaultTempSensor;

// These sensors are switched out to implement multi-chamber.
TempSensor* TempControl::beerSensor;
TempSensor* TempControl::fridgeSensor;
BasicTempSensor* TempControl::ambientSensor = &defaultTempSensor;


Actuator* TempControl::heater = &defaultActuator;
Actuator* TempControl::cooler = &defaultActuator;
Actuator* TempControl::light = &defaultActuator;
Actuator* TempControl::fan = &defaultActuator;

ValueActuator cameraLightState;
AutoOffActuator TempControl::cameraLight(600, &cameraLightState);	// timeout 10 min
Sensor<bool>* TempControl::door = &defaultSensor;

// Control parameters
ControlConstants TempControl::cc;
ControlSettings TempControl::cs;
ControlVariables TempControl::cv;

	// State variables
uint8_t TempControl::state;
bool TempControl::doPosPeakDetect;
bool TempControl::doNegPeakDetect;
bool TempControl::doorOpen;

	// keep track of beer setting stored in EEPROM
temperature TempControl::storedBeerSetting;

	// Timers
uint16_t TempControl::lastIdleTime;
uint16_t TempControl::lastHeatTime;
uint16_t TempControl::lastCoolTime;
uint16_t TempControl::waitTime;

	// MODE_INDEPENDENT only
bool TempControl::independentHeaterOn;
bool TempControl::independentCoolerOn;
uint16_t TempControl::lastIndependentHeatOnTime;
uint16_t TempControl::lastIndependentCoolOnTime;

uint8_t TempControl::ihPhase;
uint8_t TempControl::ihHeatCycle;
uint8_t TempControl::ihBelowPeakCount;
uint8_t TempControl::ihExtraDone;
uint16_t TempControl::ihSafetyCutCount;
uint16_t TempControl::ihLearnedOnSec;
uint16_t TempControl::ihPlannedOnSec;
uint16_t TempControl::ihCycleOnSec;
uint16_t TempControl::ihExtraRemainSec;
uint16_t TempControl::ihLearnedLagSec;
int16_t TempControl::ihPendingCorrSec;
temperature TempControl::ihLastHeatSlope;
temperature TempControl::ihPeakTemp;
temperature TempControl::ihBeerAtCut;
temperature TempControl::ihSlopeAtCut;
#endif

// Independent predictive learner constants (RAM state machine)
static const uint8_t IH_PEAK_BELOW_N = 3;       // slow-filter samples below peak
static const uint8_t IH_EXTRA_MAX_COUNT = 3;    // max same-cycle top-up pulses
static const temperature IH_CORR_DEADBAND_TEMP = intToTempDiff(1)/40; // ~0.025°C
static const uint16_t IH_CORR_DEADBAND_SEC_FALLBACK = 10; // if slope too small for °C→s
static const uint16_t IH_CORR_ABS_MAX_SEC = 3600; // sanity cap on pending correction
static const uint16_t IH_EXTRA_MAX_SEC = 600;   // max seconds per extra pulse
static const uint16_t IH_LAG_MAX_SEC = 3600;    // sanity cap on learned lag τ
static const temperature IH_MIN_SLOPE = 25;     // ~0.05 C/hour (25/512)
// Same-cycle top-up uses only this fraction of the slope-based seconds (conservative).
static const uint16_t IH_EXTRA_SCALE_NUM = 7;
static const uint16_t IH_EXTRA_SCALE_DEN = 10;

void TempControl::init(void){
	state=IDLE;
	cs.mode = MODE_OFF;

	cameraLight.setActive(false);

	// this is for cases where the device manager hasn't configured beer/fridge sensor.
	if (beerSensor==NULL) {
		beerSensor = new TempSensor(TEMP_SENSOR_TYPE_BEER, &defaultTempSensor);
		beerSensor->init();
	}

	if (fridgeSensor==NULL) {
		fridgeSensor = new TempSensor(TEMP_SENSOR_TYPE_FRIDGE, &defaultTempSensor);
		fridgeSensor->init();
	}

	updateTemperatures();
	reset();

	// Do not allow heating/cooling directly after reset.
	// A failing script + CRON + Arduino uno (which resets on serial connect) could damage the compressor
	// For test purposes, set these to -3600 to eliminate waiting after reset
	lastHeatTime = 0;
	lastCoolTime = 0;
}

void TempControl::reset(void){
	doPosPeakDetect=false;
	doNegPeakDetect=false;
	independentHeaterOn=false;
	independentCoolerOn=false;
	resetIndependentHeatLearn();
}

void TempControl::resetIndependentHeatLearn(void){
	ihPhase = IH_PHASE_IDLE;
	ihHeatCycle = 0;
	ihBelowPeakCount = 0;
	ihExtraDone = 0;
	ihSafetyCutCount = 0;
	ihLearnedOnSec = 0;
	ihPlannedOnSec = 0;
	ihCycleOnSec = 0;
	ihExtraRemainSec = 0;
	ihLearnedLagSec = 0;
	ihPendingCorrSec = 0;
	ihLastHeatSlope = 0;
	ihPeakTemp = INVALID_TEMP;
	ihBeerAtCut = INVALID_TEMP;
	ihSlopeAtCut = 0;
	cv.estimatedPeak = INVALID_TEMP;
}

void updateSensor(TempSensor* sensor) {
	sensor->update();
	sensor->retryInitIfNeeded();
	if(!sensor->isConnected()) {
		sensor->init();
	}
}

void TempControl::updateTemperatures(void){

	updateSensor(beerSensor);
	updateSensor(fridgeSensor);

	// Read ambient sensor to keep the value up to date. If no sensor is connected, this does nothing.
	// This prevents a delay in serial response because the value is not up to date.
	temperature ambientTemp = ambientSensor->read();
	if(ambientTemp == TEMP_SENSOR_DISCONNECTED){
		ambientSensor->init(); // try to reconnect a disconnected, but installed sensor
	}
	DeviceManager::startOneWireConversions();

#if SerialDebug
	char beerTemp[12];
	char fridgeTemp[12];
	char ambientTempString[12];
	tempToString(beerTemp, beerSensor->readRaw(), 2, sizeof(beerTemp));
	tempToString(fridgeTemp, fridgeSensor->readRaw(), 2, sizeof(fridgeTemp));
	tempToString(ambientTempString, ambientTemp, 2, sizeof(ambientTempString));
	DBG_PRINTF("TEMP Beer:%s(%d) Fridge:%s(%d) Ambient:%s(%d)\n",
		beerTemp, beerSensor->isConnected(true), fridgeTemp, fridgeSensor->isConnected(true),
		ambientTempString, ambientSensor->isConnected());
#endif
}

void TempControl::updatePID(void){
	static unsigned char integralUpdateCounter = 0;

	if(cs.mode == MODE_INDEPENDENT){
		// Independent does not use beer→fridge cascade PID (heater predictive
		// time+slope learner lives in updateIndependentState when hysteresisMode is on).
		return;
	}
	if(tempControl.modeIsBeer()){
		if(cs.beerSetting == INVALID_TEMP){
			// beer setting is not updated yet
			cs.fridgeSetting = INVALID_TEMP;
			return;
		}

		// fridge setting is calculated with PID algorithm. Beer temperature error is input to PID
		cv.beerDiff =  cs.beerSetting - beerSensor->readSlowFiltered();
		cv.beerSlope = beerSensor->readSlope();
		temperature fridgeFastFiltered = fridgeSensor->readFastFiltered();

		if(integralUpdateCounter++ == 60){
			integralUpdateCounter = 0;

			temperature integratorUpdate = cv.beerDiff;

			// Only update integrator in IDLE, so it does not increase when fridge is already at maximum/minimum
			if(state != IDLE){
				integratorUpdate = 0;
			}
			else if(abs(integratorUpdate) < cc.iMaxError){
				// difference is smaller than iMaxError
				// check additional conditions to see if integrator should be active to prevent windup
				bool updateSign = (integratorUpdate > 0); // 1 = positive, 0 = negative
				bool integratorSign = (cv.diffIntegral > 0);

				if(updateSign == integratorSign){
					// beerDiff and integrator have same sign. Integrator would be increased.

					// If actuator is already at max increasing actuator will only cause integrator windup.
					integratorUpdate = (cs.fridgeSetting >= cc.tempSettingMax) ? 0 : integratorUpdate;
					integratorUpdate = (cs.fridgeSetting <= cc.tempSettingMin) ? 0 : integratorUpdate;
					integratorUpdate = ((cs.fridgeSetting - cs.beerSetting) >= cc.pidMax) ? 0 : integratorUpdate;
					integratorUpdate = ((cs.beerSetting - cs.fridgeSetting) >= cc.pidMax) ? 0 : integratorUpdate;

					// cooling and fridge temp is more than 2 degrees from setting, actuator is saturated.
					integratorUpdate = (!updateSign && (fridgeFastFiltered > (cs.fridgeSetting +1024))) ? 0 : integratorUpdate;

					// heating and fridge temp is more than 2 degrees from setting, actuator is saturated.
					integratorUpdate = (updateSign && (fridgeFastFiltered < (cs.fridgeSetting -1024))) ? 0 : integratorUpdate;
				}
				else{
					// integrator action is decreased. Decrease faster than increase.
					integratorUpdate = integratorUpdate*2;
				}
			}
			else{
				// decrease integral by 1/8 when far from the end value to reset the integrator
				integratorUpdate = -(cv.diffIntegral >> 3);
			}
			cv.diffIntegral = cv.diffIntegral + integratorUpdate;
		}

		// calculate PID parts. Use long_temperature to prevent overflow
		cv.p = multiplyFactorTemperatureDiff(cc.Kp, cv.beerDiff);
		cv.i = multiplyFactorTemperatureDiffLong(cc.Ki, cv.diffIntegral);
		cv.d = multiplyFactorTemperatureDiff(cc.Kd, cv.beerSlope);
		long_temperature newFridgeSetting = cs.beerSetting;
		newFridgeSetting += cv.p;
		newFridgeSetting += cv.i;
		newFridgeSetting += cv.d;

		// constrain to tempSettingMin or beerSetting - pidMAx, whichever is lower.
		temperature lowerBound = (cs.beerSetting <= cc.tempSettingMin + cc.pidMax) ? cc.tempSettingMin : cs.beerSetting - cc.pidMax;
		// constrain to tempSettingMax or beerSetting + pidMAx, whichever is higher.
		temperature upperBound = (cs.beerSetting >= cc.tempSettingMax - cc.pidMax) ? cc.tempSettingMax : cs.beerSetting + cc.pidMax;

		cs.fridgeSetting = constrain(constrainTemp16(newFridgeSetting), lowerBound, upperBound);
	}
	else if(cs.mode == MODE_FRIDGE_CONSTANT){
		// FridgeTemperature is set manually, use INVALID_TEMP to indicate beer temp is not active
		cs.beerSetting = INVALID_TEMP;
	}
}

void TempControl::updateState(void){
	//update state
	bool stayIdle = false;
	bool newDoorOpen = door->sense();

	if(newDoorOpen!=doorOpen) {
		doorOpen = newDoorOpen;
#ifdef ESP8266  // ESP8266 Doesn't support %S
		String annotation = "";
		annotation += "Fridge door ";
		annotation += doorOpen ? "opened" : "closed";
		piLink.printTemperaturesJSON(0, annotation.c_str());
#else
		piLink.printFridgeAnnotation(PSTR("Fridge door %S"), doorOpen ? PSTR("opened") : PSTR("closed"));
#endif
	}

	if(cs.mode == MODE_INDEPENDENT){
		updateIndependentState();
		return;
	}

	if(cs.mode == MODE_OFF){
		state = STATE_OFF;
		stayIdle = true;
	}
	// stay idle when one of the required sensors is disconnected, or the fridge setting is INVALID_TEMP
	if( cs.fridgeSetting == INVALID_TEMP ||
		!fridgeSensor->isConnected() ||
		(!beerSensor->isConnected() && tempControl.modeIsBeer())){
		state = IDLE;
		stayIdle = true;
	}

	uint16_t sinceIdle = timeSinceIdle();
	uint16_t sinceCooling = timeSinceCooling();
	uint16_t sinceHeating = timeSinceHeating();
	temperature fridgeFast = fridgeSensor->readFastFiltered();
	temperature beerFast = beerSensor->readFastFiltered();
	ticks_seconds_t secs = ticks.seconds();
	switch(state)
	{
		case IDLE:
		case STATE_OFF:
		case WAITING_TO_COOL:
		case WAITING_TO_HEAT:
		case WAITING_FOR_PEAK_DETECT:
		{
			lastIdleTime=secs;
			// set waitTime to zero. It will be set to the maximum required waitTime below when wait is in effect.
			if(stayIdle){
				break;
			}
			resetWaitTime();
			// Classic modes (fridge/beer/profile): original vitotai start hysteresis
			// always uses idleRange*. Independent ignores this path entirely.
			bool coolNeeded = fridgeFast > (cs.fridgeSetting+cc.idleRangeHigh);
			bool heatNeeded = fridgeFast < (cs.fridgeSetting+cc.idleRangeLow);
			if(coolNeeded){  // fridge temperature is too high for the cooler's own band
				#if 1 //SettableMinimumCoolTime
				tempControl.updateWaitTime(cc.mutexDeadTime, sinceHeating);
				#else
				tempControl.updateWaitTime(MIN_SWITCH_TIME, sinceHeating);
				#endif
				if(cs.mode==MODE_FRIDGE_CONSTANT){
					tempControl.updateWaitTime(MIN_COOL_OFF_TIME_FRIDGE_CONSTANT, sinceCooling);
				}
				else{
					if(beerFast < (cs.beerSetting + 16) ){ // If beer is already under target, stay/go to idle. 1/2 sensor bit idle zone
						state = IDLE; // beer is already colder than setting, stay in or go to idle
						break;
					}
					#if 1 // SettableMinimumCoolTime
					tempControl.updateWaitTime(cc.minCoolIdleTime, sinceCooling);
					#else
					tempControl.updateWaitTime(MIN_COOL_OFF_TIME, sinceCooling);
					#endif
				}
				if(tempControl.cooler != &defaultActuator){
					if(getWaitTime() > 0){
						state = WAITING_TO_COOL;
					}
					else{
						state = COOLING;
					}
				}
			}
			if(heatNeeded){  // fridge temperature is too low for the heater's own band
				#if 1 // SettableMinimumCoolTime
				tempControl.updateWaitTime(cc.mutexDeadTime, sinceCooling);
				tempControl.updateWaitTime(cc.minHeatIdleTime, sinceHeating);
				#else
				tempControl.updateWaitTime(MIN_SWITCH_TIME, sinceCooling);
				tempControl.updateWaitTime(MIN_HEAT_OFF_TIME, sinceHeating);
				#endif
				if(cs.mode!=MODE_FRIDGE_CONSTANT){
					if(beerFast > (cs.beerSetting - 16)){ // If beer is already over target, stay/go to idle. 1/2 sensor bit idle zone
						state = IDLE;  // beer is already warmer than setting, stay in or go to idle
						break;
					}
				}
				if(tempControl.heater != &defaultActuator || (cc.lightAsHeater && (tempControl.light != &defaultActuator))){
					if(getWaitTime() > 0){
						state = WAITING_TO_HEAT;
					}
					else{
						state = HEATING;
					}
				}
			}
			if(!coolNeeded && !heatNeeded){
				state = IDLE; // within both actuators' bands, always go to IDLE
				break;
			}
			if(state == HEATING || state == COOLING){
				if(doNegPeakDetect == true || doPosPeakDetect == true){
					// If peak detect is not finished, but the fridge wants to switch to heat/cool
					// Wait for peak detection and display 'Await peak detect' on display
					state = WAITING_FOR_PEAK_DETECT;
					break;
				}
			}
		}
		break;
		case COOLING:
		case COOLING_MIN_TIME:
		{
			doNegPeakDetect=true;
			lastCoolTime = secs;
			updateEstimatedPeak(cc.maxCoolTimeForEstimate, cs.coolEstimator, sinceIdle,
				fridgeSensor->readFastFiltered(), true);
			state = COOLING; // set to cooling here, so the display of COOLING/COOLING_MIN_TIME is correct

			// stop cooling when estimated fridge temp peak reaches the cooler's own lower
			// bound (coolingTargetLower) or if beer is already too cold (1/2 sensor bit idle zone)
			if(cv.estimatedPeak <= (cs.fridgeSetting + cc.coolingTargetLower) || (cs.mode != MODE_FRIDGE_CONSTANT && beerFast < (cs.beerSetting - 16))){
				#if 1 //SettableMinimumCoolTime
				if(sinceIdle > cc.minCoolTime){
				#else
				if(sinceIdle > MIN_COOL_ON_TIME){
				#endif
					cv.negPeakEstimate = cv.estimatedPeak; // remember estimated peak when I switch to IDLE, to adjust estimator later
					state=IDLE;
					break;
				}
				else{
					state = COOLING_MIN_TIME;
					break;
				}
			}
		}
		break;
		case HEATING:
		case HEATING_MIN_TIME:
		{
			doPosPeakDetect=true;
			lastHeatTime=secs;
			updateEstimatedPeak(cc.maxHeatTimeForEstimate, cs.heatEstimator, sinceIdle,
				fridgeSensor->readFastFiltered(), false);
			state = HEATING; // reset to heating here, so the display of HEATING/HEATING_MIN_TIME is correct

			// stop heating when estimated fridge temp peak reaches the heater's own upper
			// bound (heatingTargetUpper), beer already too warm, or maxHeatTime expired (0=off)
			bool maxHeatExpired = (cc.maxHeatTime != 0 && sinceIdle >= cc.maxHeatTime);
			if(cv.estimatedPeak >= (cs.fridgeSetting + cc.heatingTargetUpper)
				|| (cs.mode != MODE_FRIDGE_CONSTANT && beerFast > (cs.beerSetting + 16))
				|| maxHeatExpired){
				#if 1 //SettableMinimumCoolTime
				if(sinceIdle > cc.minHeatTime){
				#else
				if(sinceIdle > MIN_HEAT_ON_TIME){
				#endif
					cv.posPeakEstimate=cv.estimatedPeak; // remember estimated peak when I switch to IDLE, to adjust estimator later
					state=IDLE;
					break;
				}
				else{
					state = HEATING_MIN_TIME;
					break;
				}
			}
		}
		break;
	}
}

// MODE_INDEPENDENT: heater and cooler may be active at the same time.
//   Cooler target = cs.fridgeSetting (Cooling Target on the Independent tab).
//   Heater target = cs.beerSetting (Heating Target; also profile ramp target).
// On/off bands always use heatingTarget*/coolingTarget* (Advanced Settings).
// hysteresisMode On: time+slope learner on beer (cycle1 thermostat measure,
// cycle2 learned on-time, cycle3+ learned ± correction; same-cycle extra if under).
void TempControl::updateIndependentState(void){
	temperature fridgeFast = fridgeSensor->readFastFiltered();
	temperature beerFast = beerSensor->readFastFiltered();
	temperature beerSlow = beerSensor->readSlowFiltered();
	ticks_seconds_t secs = ticks.seconds();
	const bool heaterPredictive = (cc.hysteresisMode == HYSTERESIS_MODE_PID_ON);

	bool coolerAllowed = (cs.fridgeSetting != INVALID_TEMP) && fridgeSensor->isConnected()
		&& (tempControl.cooler != &defaultActuator);
	bool heaterAllowed = (cs.beerSetting != INVALID_TEMP) && beerSensor->isConnected()
		&& (tempControl.heater != &defaultActuator || (cc.lightAsHeater && tempControl.light != &defaultActuator));

	if(!coolerAllowed){
		independentCoolerOn = false;
	}
	else if(independentCoolerOn){
		// already on: only stop once we reach the lower bound AND minCoolTime has elapsed
		if(fridgeFast <= (cs.fridgeSetting + cc.coolingTargetLower)
			&& ticks.timeSince(lastIndependentCoolOnTime) > cc.minCoolTime){
			independentCoolerOn = false;
		}
	}
	else if(fridgeFast > (cs.fridgeSetting + cc.coolingTargetUpper)
		&& ticks.timeSince(lastCoolTime) > cc.minCoolIdleTime){
		// off: only start once we cross the upper bound AND minCoolIdleTime has elapsed
		independentCoolerOn = true;
		lastIndependentCoolOnTime = secs;
	}
	if(independentCoolerOn) lastCoolTime = secs;

	if(!heaterAllowed){
		independentHeaterOn = false;
		if(heaterPredictive){
			resetIndependentHeatLearn();
		}
	}
	else if(heaterPredictive){
		updateIndependentPredictiveHeater(beerFast, beerSlow, secs);
	}
	else{
		// Simple thermostat on beer (predictive Off)
		resetIndependentHeatLearn();
		if(independentHeaterOn){
			uint16_t heatOnFor = ticks.timeSince(lastIndependentHeatOnTime);
			bool atTarget = (beerFast >= (cs.beerSetting + cc.heatingTargetUpper));
			bool maxHeatExpired = (cc.maxHeatTime != 0 && heatOnFor >= cc.maxHeatTime);
			if((atTarget || maxHeatExpired) && heatOnFor > cc.minHeatTime){
				independentHeaterOn = false;
			}
		}
		else if(beerFast < (cs.beerSetting + cc.heatingTargetLower)
			&& ticks.timeSince(lastHeatTime) > cc.minHeatIdleTime){
			independentHeaterOn = true;
			lastIndependentHeatOnTime = secs;
		}
	}
	if(independentHeaterOn) lastHeatTime = secs;

	if(independentCoolerOn){
		// Cooler display priority when both; heater may still be in peak-wait.
		state = COOLING;
	}
	else if(heaterPredictive && ihPhase == IH_PHASE_WAIT_PEAK){
		lastIdleTime = secs;
		state = WAITING_FOR_PEAK_DETECT;
	}
	else if(!independentHeaterOn){
		lastIdleTime = secs;
		state = IDLE;
	}
	else{
		state = HEATING;
	}
}

// Convert temp error / slope(°C/h) -> seconds. Returns 0 if slope too small.
static uint16_t ihErrorToSeconds(temperature errAbs, temperature slope){
	if(slope < IH_MIN_SLOPE || errAbs <= 0){
		return 0;
	}
	int32_t sec = ((int32_t)errAbs * 3600L) / (int32_t)slope;
	if(sec < 0){
		return 0;
	}
	if(sec > 65535L){
		return 65535;
	}
	return (uint16_t)sec;
}

// Deadband in °C → seconds via slope (consistent across fast/slow heaters).
static uint16_t ihCorrDeadbandSec(temperature slope){
	uint16_t s = ihErrorToSeconds(IH_CORR_DEADBAND_TEMP, slope);
	if(s == 0){
		return IH_CORR_DEADBAND_SEC_FALLBACK;
	}
	return s;
}

// Next-cycle correction: deadband in temperature (via slope→sec), not a fixed clamp.
static int16_t ihCorrFromErrorSec(uint16_t errSec, bool overshoot, temperature slope){
	if(errSec < ihCorrDeadbandSec(slope)){
		return 0;
	}
	if(errSec > IH_CORR_ABS_MAX_SEC){
		errSec = IH_CORR_ABS_MAX_SEC;
	}
	int16_t v = (int16_t)errSec;
	return overshoot ? (int16_t)(-v) : v;
}

// Predicted peak if heater stops now: beer + slope×learned lag τ (not onFor).
static temperature ihPeakIfStopNow(temperature beerSlow, temperature slope, uint16_t lagSec){
	if(slope < IH_MIN_SLOPE || lagSec == 0){
		return beerSlow;
	}
	temperature rise = (temperature)(((int32_t)slope * (int32_t)lagSec) / 3600L);
	return beerSlow + rise;
}

// EMA-update τ from post-cut rise: tau = (peak - beerAtCut) / slopeAtCut.
static void ihUpdateLearnedLag(uint16_t* learnedLagSec, temperature beerAtCut,
	temperature slopeAtCut, temperature peak)
{
	if(beerAtCut == INVALID_TEMP || peak <= beerAtCut || slopeAtCut < IH_MIN_SLOPE){
		return;
	}
	uint16_t tau = ihErrorToSeconds(peak - beerAtCut, slopeAtCut);
	if(tau == 0){
		return;
	}
	if(tau > IH_LAG_MAX_SEC){
		tau = IH_LAG_MAX_SEC;
	}
	if(*learnedLagSec == 0){
		*learnedLagSec = tau;
	}
	else{
		// EMA: 3/4 old + 1/4 new
		*learnedLagSec = (uint16_t)((((uint32_t)(*learnedLagSec) * 3UL) + (uint32_t)tau) / 4UL);
	}
}

void TempControl::updateIndependentPredictiveHeater(temperature beerFast, temperature beerSlow,
	uint16_t secs)
{
	const temperature U = cs.beerSetting + cc.heatingTargetUpper;
	const bool beerTooWarm = (beerFast > (cs.beerSetting + 16));
	doPosPeakDetect = false; // classic fridge/beer estimator path not used here

	// --- WAIT_PEAK: heater off, track peak on slow beer, detect decline ---
	if(ihPhase == IH_PHASE_WAIT_PEAK){
		independentHeaterOn = false;
		cv.estimatedPeak = ihPeakTemp;

		if(ihPeakTemp == INVALID_TEMP || beerSlow > ihPeakTemp){
			ihPeakTemp = beerSlow;
			ihBelowPeakCount = 0;
		}
		else if(beerSlow < ihPeakTemp){
			if(ihBelowPeakCount < 255){
				ihBelowPeakCount++;
			}
		}
		else{
			ihBelowPeakCount = 0; // equal to peak: still at plateau
		}

		if(ihBelowPeakCount >= IH_PEAK_BELOW_N
			|| ticks.timeSince(lastHeatTime) > HEAT_PEAK_DETECT_TIME){
			// Peak broken (3s declining on slow filter), or peak-wait timeout
			temperature peak = (ihPeakTemp != INVALID_TEMP) ? ihPeakTemp : beerSlow;
			cv.posPeak = peak;
			cv.posPeakEstimate = peak;

			// Learn thermal lag τ from post-cut coast: (peak - beerAtCut) / slopeAtCut.
			ihUpdateLearnedLag(&ihLearnedLagSec, ihBeerAtCut, ihSlopeAtCut, peak);

			if(peak < U && ihExtraDone < IH_EXTRA_MAX_COUNT){
				uint16_t extra = ihErrorToSeconds(U - peak, ihLastHeatSlope);
				if(extra > IH_EXTRA_MAX_SEC){
					extra = IH_EXTRA_MAX_SEC;
				}
				// Conservative top-up: only ~70% of slope-based seconds.
				extra = (uint16_t)(((uint32_t)extra * IH_EXTRA_SCALE_NUM) / IH_EXTRA_SCALE_DEN);
				// Skip tiny top-ups (temp deadband → seconds via slope).
				if(extra >= ihCorrDeadbandSec(ihLastHeatSlope)){
					ihExtraRemainSec = extra;
					ihExtraDone++;
					ihPhase = IH_PHASE_HEAT_EXTRA;
					independentHeaterOn = true;
					lastIndependentHeatOnTime = secs;
					ihBelowPeakCount = 0;
					ihPeakTemp = INVALID_TEMP;
					return;
				}
			}

			// Finish cycle: fold total on-time into learned. If peak overshot U,
			// shorten learned immediately (time-to-U ≠ time-to-peak-at-U).
			ihLearnedOnSec = ihCycleOnSec;
			if(peak > U){
				uint16_t overSec = ihErrorToSeconds(peak - U, ihLastHeatSlope);
				if(overSec >= ihCorrDeadbandSec(ihLastHeatSlope)){
					if(ihLearnedOnSec > overSec){
						ihLearnedOnSec = (uint16_t)(ihLearnedOnSec - overSec);
					}else{
						ihLearnedOnSec = 1;
					}
				}
				// Overshoot already folded into learned; do not also stack pendingCorr.
				ihPendingCorrSec = 0;
			}
			else if(peak < U){
				uint16_t underSec = ihErrorToSeconds(U - peak, ihLastHeatSlope);
				ihPendingCorrSec = ihCorrFromErrorSec(underSec, false, ihLastHeatSlope);
			}
			else{
				ihPendingCorrSec = 0;
			}
			ihPhase = IH_PHASE_IDLE;
			ihExtraDone = 0;
			ihBelowPeakCount = 0;
			ihPeakTemp = INVALID_TEMP;
			ihExtraRemainSec = 0;
			cv.estimatedPeak = INVALID_TEMP;
		}
		return;
	}

	// --- HEAT_EXTRA: same-cycle top-up ---
	if(ihPhase == IH_PHASE_HEAT_EXTRA){
		independentHeaterOn = true;
		temperature slope = beerSensor->readSlope();
		if(slope > ihLastHeatSlope){
			ihLastHeatSlope = slope;
		}
		uint16_t onFor = ticks.timeSince(lastIndependentHeatOnTime);
		uint16_t totalOn = ihCycleOnSec + onFor;

		uint16_t remain = (ihExtraRemainSec > onFor) ? (ihExtraRemainSec - onFor) : 0;
		if(ihLastHeatSlope >= IH_MIN_SLOPE){
			temperature overshootRemain = (temperature)(((int32_t)ihLastHeatSlope * (int32_t)remain) / 3600L);
			cv.estimatedPeak = beerSlow + overshootRemain;
		}
		else{
			cv.estimatedPeak = beerSlow;
		}
		temperature peakIfStopNow = ihPeakIfStopNow(beerSlow, ihLastHeatSlope, ihLearnedLagSec);

		bool maxHeatExpired = (cc.maxHeatTime != 0 && totalOn >= cc.maxHeatTime);
		bool safetyCut = (beerFast >= U);
		// Early cut: beer + slope×τ >= U (τ=0 → no early cut until first peak).
		bool earlyCut = (peakIfStopNow >= U && ihLearnedLagSec > 0
			&& ihLastHeatSlope >= IH_MIN_SLOPE);
		if(safetyCut || earlyCut || onFor >= ihExtraRemainSec || maxHeatExpired || beerTooWarm){
			if(safetyCut && ihSafetyCutCount < 65535){
				ihSafetyCutCount++;
			}
			ihCycleOnSec = totalOn;
			independentHeaterOn = false;
			ihBeerAtCut = beerSlow;
			ihSlopeAtCut = ihLastHeatSlope;
			ihPhase = IH_PHASE_WAIT_PEAK;
			ihPeakTemp = beerSlow;
			ihBelowPeakCount = 0;
			ihExtraRemainSec = 0;
			cv.posPeakEstimate = cv.estimatedPeak;
		}
		return;
	}

	// --- HEAT_MAIN ---
	if(ihPhase == IH_PHASE_HEAT_MAIN){
		independentHeaterOn = true;
		temperature slope = beerSensor->readSlope();
		if(slope > IH_MIN_SLOPE){
			ihLastHeatSlope = slope;
		}
		uint16_t onFor = ticks.timeSince(lastIndependentHeatOnTime);

		uint16_t remain = 0;
		if(ihHeatCycle == 1){
			// Thermostat: estimate remain loosely for chart
			if(beerSlow < U && ihLastHeatSlope >= IH_MIN_SLOPE){
				remain = ihErrorToSeconds(U - beerSlow, ihLastHeatSlope);
			}
		}
		else if(ihPlannedOnSec > onFor){
			remain = ihPlannedOnSec - onFor;
		}
		if(ihLastHeatSlope >= IH_MIN_SLOPE){
			temperature overshootRemain = (temperature)(((int32_t)ihLastHeatSlope * (int32_t)remain) / 3600L);
			cv.estimatedPeak = beerSlow + overshootRemain;
		}
		else{
			cv.estimatedPeak = beerSlow;
		}
		temperature peakIfStopNow = ihPeakIfStopNow(beerSlow, ihLastHeatSlope, ihLearnedLagSec);

		bool maxHeatExpired = (cc.maxHeatTime != 0 && onFor >= cc.maxHeatTime);
		bool stop = false;
		bool safetyCut = (beerFast >= U);
		// Early cut once τ known: stop when beerSlow + slope×τ >= U.
		bool earlyCut = (ihLearnedLagSec > 0 && ihLastHeatSlope >= IH_MIN_SLOPE
			&& peakIfStopNow >= U);
		if(safetyCut || beerTooWarm || earlyCut){
			stop = true;
		}
		else if(ihHeatCycle == 1){
			// Cycle 1: thermostat to U (minHeatTime still applies if not yet at U)
			if(maxHeatExpired && onFor > cc.minHeatTime){
				stop = true;
			}
		}
		else{
			// Cycle 2+: timed pulse
			if((onFor >= ihPlannedOnSec) || maxHeatExpired){
				stop = true;
			}
		}
		if(stop){
			if(safetyCut && ihSafetyCutCount < 65535){
				ihSafetyCutCount++;
			}
			ihCycleOnSec = onFor;
			independentHeaterOn = false;
			ihBeerAtCut = beerSlow;
			ihSlopeAtCut = ihLastHeatSlope;
			ihPhase = IH_PHASE_WAIT_PEAK;
			ihPeakTemp = beerSlow;
			ihBelowPeakCount = 0;
			cv.posPeakEstimate = cv.estimatedPeak;
		}
		return;
	}

	// --- IDLE: start new main heat when below lower band ---
	independentHeaterOn = false;
	cv.estimatedPeak = INVALID_TEMP;
	if(beerFast < (cs.beerSetting + cc.heatingTargetLower)
		&& ticks.timeSince(lastHeatTime) > cc.minHeatIdleTime){
		ihHeatCycle++;
		ihCycleOnSec = 0;
		ihExtraDone = 0;
		ihExtraRemainSec = 0;
		ihBelowPeakCount = 0;
		ihPeakTemp = INVALID_TEMP;
		ihLastHeatSlope = 0;

		if(ihHeatCycle == 1){
			ihPlannedOnSec = 0; // thermostat
			ihPendingCorrSec = 0;
		}
		else{
			// Cycle 2+: learned ± pending correction (do not discard corr on cycle 2).
			int32_t planned = (int32_t)ihLearnedOnSec + (int32_t)ihPendingCorrSec;
			ihPendingCorrSec = 0;
			if(planned < 1){
				planned = 1;
			}
			if(planned > 65535L){
				planned = 65535L;
			}
			ihPlannedOnSec = (uint16_t)planned;
			if(ihPlannedOnSec < cc.minHeatTime && ihLearnedOnSec == 0){
				ihPlannedOnSec = cc.minHeatTime;
			}
		}

		ihPhase = IH_PHASE_HEAT_MAIN;
		independentHeaterOn = true;
		lastIndependentHeatOnTime = secs;
	}
}

void TempControl::updateEstimatedPeak(uint16_t timeLimit, temperature estimator, uint16_t sinceIdle,
	temperature currentTemp, bool cooling)
{
	uint16_t activeTime = min(timeLimit, sinceIdle); // heat or cool time in seconds
	temperature estimatedOvershoot = ((long_temperature) estimator * activeTime)/3600; // overshoot estimator is in overshoot per hour
	if(cooling){
		estimatedOvershoot = -estimatedOvershoot; // when cooling subtract overshoot from temperature
	}
	cv.estimatedPeak = currentTemp + estimatedOvershoot;
}

void TempControl::updateOutputs(void) {
	if (cs.mode==MODE_TEST)
		return;

	cameraLight.update();
	// In independent mode, `state` can only show one of heat/cool (display priority),
	// so use the real independent per-actuator flags instead of stateIsHeating/Cooling.
	bool heating = (cs.mode == MODE_INDEPENDENT) ? independentHeaterOn : stateIsHeating();
	bool cooling = (cs.mode == MODE_INDEPENDENT) ? independentCoolerOn : stateIsCooling();
	cooler->setActive(cooling);
	heater->setActive(!cc.lightAsHeater && heating);
	light->setActive(isDoorOpen() || (cc.lightAsHeater && heating) || cameraLightState.isActive());
	fan->setActive(heating || cooling);
}


void TempControl::detectPeaks(void){
	// Independent predictive heater: peak wait/learn lives in updateIndependentPredictiveHeater.
	if(cs.mode == MODE_INDEPENDENT){
		return;
	}

	//detect peaks in fridge temperature to tune overshoot estimators
	LOG_ID_TYPE detected = 0;
	temperature peak, estimate, error, oldEstimator, newEstimator;

	if(doPosPeakDetect && !stateIsHeating()){
		peak = fridgeSensor->detectPosPeak();
		estimate = cv.posPeakEstimate;
		error = peak-estimate;
		oldEstimator = cs.heatEstimator;
		if(peak != INVALID_TEMP){
			// positive peak detected
			if(error > cc.heatingTargetUpper){
				// Peak temperature was higher than the estimate.
				// Overshoot was higher than expected
				// Increase estimator to increase the estimated overshoot
				increaseEstimator(&(cs.heatEstimator), error);
			}
			if(error < cc.heatingTargetLower){
				// Peak temperature was lower than the estimate.
				// Overshoot was lower than expected
				// Decrease estimator to decrease the estimated overshoot
				decreaseEstimator(&(cs.heatEstimator), error);
			}
			detected = INFO_POSITIVE_PEAK;
		}
		else if(timeSinceHeating() > HEAT_PEAK_DETECT_TIME){
			if(fridgeSensor->readFastFiltered() < (cv.posPeakEstimate+cc.heatingTargetLower)){
				// Idle period almost reaches maximum allowed time for peak detection
				// This is the heat, then drift up too slow (but in the right direction).
				// estimator is too high
				peak=fridgeSensor->readFastFiltered();
				decreaseEstimator(&(cs.heatEstimator), error);
				detected = INFO_POSITIVE_DRIFT;
			}
			else{
				// maximum time for peak estimation reached
				doPosPeakDetect = false;
			}
		}
		if(detected){
			newEstimator = cs.heatEstimator;
			cv.posPeak = peak;
			doPosPeakDetect = false;
		}
	}
	else if(doNegPeakDetect && !stateIsCooling()){
		peak = fridgeSensor->detectNegPeak();
		estimate = cv.negPeakEstimate;
		error = peak-estimate;
		oldEstimator = cs.coolEstimator;
		if(peak != INVALID_TEMP){
			// negative peak detected
			if(error < cc.coolingTargetLower){
				// Peak temperature was lower than the estimate.
				// Overshoot was higher than expected
				// Increase estimator to increase the estimated overshoot
				increaseEstimator(&(cs.coolEstimator), error);
			}
			if(error > cc.coolingTargetUpper){
				// Peak temperature was higher than the estimate.
				// Overshoot was lower than expected
				// Decrease estimator to decrease the estimated overshoot
				decreaseEstimator(&(cs.coolEstimator), error);

			}
			detected = INFO_NEGATIVE_PEAK;
		}
		else if(timeSinceCooling() > COOL_PEAK_DETECT_TIME){
			if(fridgeSensor->readFastFiltered() > (cv.negPeakEstimate+cc.coolingTargetUpper)){
				// Idle period almost reaches maximum allowed time for peak detection
				// This is the cooling, then drift down too slow (but in the right direction).
				// estimator is too high
				peak = fridgeSensor->readFastFiltered();
				decreaseEstimator(&(cs.coolEstimator), error);
				detected = INFO_NEGATIVE_DRIFT;
			}
			else{
				// maximum time for peak estimation reached
				doNegPeakDetect=false;
			}
		}
		if(detected){
			newEstimator = cs.coolEstimator;
			cv.negPeak = peak;
			doNegPeakDetect=false;
		}
	}
	if(detected){
		// send out log message for type of peak detected
		logInfoTempTempFixedFixed(detected, peak, estimate, oldEstimator, newEstimator);
	}
}

// Increase estimator at least 20%, max 50%s
void TempControl::increaseEstimator(temperature * estimator, temperature error){
	temperature factor = 614 + constrainTemp(abs((int)error)>>5, 0, 154); // 1.2 + 3.1% of error, limit between 1.2 and 1.5
	*estimator = multiplyFactorTemperatureDiff(factor, *estimator);
	if(*estimator < 25){
		*estimator = intToTempDiff(5)/100; // make estimator at least 0.05
	}
	eepromManager.storeTempSettings();
}

// Decrease estimator at least 16.7% (1/1.2), max 33.3% (1/1.5)
void TempControl::decreaseEstimator(temperature * estimator, temperature error){
	temperature factor = 426 - constrainTemp(abs((int)error)>>5, 0, 85); // 0.833 - 3.1% of error, limit between 0.667 and 0.833
	*estimator = multiplyFactorTemperatureDiff(factor, *estimator);
	eepromManager.storeTempSettings();
}

uint16_t TempControl::timeSinceCooling(void){
	return ticks.timeSince(lastCoolTime);
}

uint16_t TempControl::timeSinceHeating(void){
	return ticks.timeSince(lastHeatTime);
}

uint16_t TempControl::timeSinceIdle(void){
	return ticks.timeSince(lastIdleTime);
}

void TempControl::loadDefaultSettings(){
#if BREWPI_EMULATE
	setMode(MODE_BEER_CONSTANT);
#else
	setMode(MODE_OFF);
#endif
	cs.beerSetting = intToTemp(20);
	cs.fridgeSetting = intToTemp(20);
	cs.heatEstimator = intToTempDiff(2)/10; // 0.2
	cs.coolEstimator=intToTempDiff(5);
}

void TempControl::storeConstants(eptr_t offset){
	eepromAccess.writeControlConstants(offset,  cc, sizeof(ControlConstants));
}

void TempControl::loadConstants(eptr_t offset){
	eepromAccess.readControlConstants(cc, offset, sizeof(ControlConstants));
	bool valid = (cc.tempFormat == 'C' || cc.tempFormat == 'F')
		&& cc.tempSettingMin < cc.tempSettingMax
		&& cc.fridgeFastFilter <= 7 && cc.fridgeSlowFilter <= 7 && cc.fridgeSlopeFilter <= 7
		&& cc.beerFastFilter <= 7 && cc.beerSlowFilter <= 7 && cc.beerSlopeFilter <= 7;
	if(!valid){
		loadDefaultConstants();
		storeConstants(offset);
		return;
	}
	// hysteresisMode = Independent predictive heater only (0/1)
	if(cc.hysteresisMode > HYSTERESIS_MODE_PID_ON){
		cc.hysteresisMode = HYSTERESIS_MODE_PID_OFF;
	}
	initFilters();
}

// write new settings to EEPROM to be able to reload them after a reset
// The update functions only write to EEPROM if the value has changed
void TempControl::storeSettings(eptr_t offset){
	eepromAccess.writeControlSettings(offset, cs, sizeof(ControlSettings));
	storedBeerSetting = cs.beerSetting;
}

void TempControl::loadSettings(eptr_t offset){
	eepromAccess.readControlSettings(cs, offset, sizeof(ControlSettings));
	logDebug("loaded settings");
	storedBeerSetting = cs.beerSetting;
	setMode(cs.mode, true);		// force the mode update
}

void TempControl::loadDefaultConstants(void){
	memcpy_P((void*) &tempControl.cc, (void*) &tempControl.ccDefaults, sizeof(ControlConstants));
	initFilters();
}

void TempControl::initFilters()
{
	fridgeSensor->setFastFilterCoefficients(cc.fridgeFastFilter);
	fridgeSensor->setSlowFilterCoefficients(cc.fridgeSlowFilter);
	fridgeSensor->setSlopeFilterCoefficients(cc.fridgeSlopeFilter);
	beerSensor->setFastFilterCoefficients(cc.beerFastFilter);
	beerSensor->setSlowFilterCoefficients(cc.beerSlowFilter);
	beerSensor->setSlopeFilterCoefficients(cc.beerSlopeFilter);
}

void TempControl::setMode(char newMode, bool force){
	logDebug("TempControl::setMode from %c to %c", cs.mode, newMode);

	if(newMode != cs.mode || state == WAITING_TO_HEAT || state == WAITING_TO_COOL || state == WAITING_FOR_PEAK_DETECT){
		state = IDLE;
		force = true;
	}
	if (force) {
		cs.mode = newMode;
		if(newMode == MODE_OFF){
			cs.beerSetting = INVALID_TEMP;
			cs.fridgeSetting = INVALID_TEMP;
		}
		eepromManager.storeTempSettings();
	}
}

temperature TempControl::getBeerTemp(void){
	if(beerSensor->isConnected()){
		return beerSensor->readFastFiltered();
	}
	else{
		return INVALID_TEMP;
	}
}

temperature TempControl::getBeerSetting(void){
	return cs.beerSetting;
}

temperature TempControl::getFridgeTemp(void){
	if(fridgeSensor->isConnected()){
		return fridgeSensor->readFastFiltered();
	}
	else{
		return INVALID_TEMP;
	}
}

temperature TempControl::getFridgeSetting(void){
	return cs.fridgeSetting;
}

bool TempControl::isIndependentPidOn(void){
	return (cs.mode == MODE_INDEPENDENT && cc.hysteresisMode == HYSTERESIS_MODE_PID_ON);
}

static const char* ihPhaseName(uint8_t phase){
	switch(phase){
		case 0: return "IDLE";
		case 1: return "HEAT_MAIN";
		case 2: return "WAIT_PEAK";
		case 3: return "HEAT_EXTRA";
		default: return "?";
	}
}

void TempControl::appendIndependentHeatDebug(String& out){
	char tbuf[16];
	out += "mode:";
	out += cs.mode;
	out += " hystMode:";
	out += String((unsigned)cc.hysteresisMode);
	out += " predictive:";
	out += isIndependentPidOn() ? "1" : "0";
	out += "\n";
	out += "phase:";
	out += ihPhaseName(ihPhase);
	out += " (";
	out += String((unsigned)ihPhase);
	out += ")\n";
	out += "heatCycle:";
	out += String((unsigned)ihHeatCycle);
	out += "\n";
	out += "heaterOn:";
	out += independentHeaterOn ? "1" : "0";
	out += " coolerOn:";
	out += independentCoolerOn ? "1" : "0";
	out += "\n";
	out += "learnedOnSec:";
	out += String(ihLearnedOnSec);
	out += "\n";
	out += "plannedOnSec:";
	out += String(ihPlannedOnSec);
	out += "\n";
	out += "cycleOnSec:";
	out += String(ihCycleOnSec);
	out += "\n";
	out += "extraRemainSec:";
	out += String(ihExtraRemainSec);
	out += "\n";
	out += "pendingCorrSec:";
	out += String(ihPendingCorrSec);
	out += "\n";
	out += "learnedLagSec:";
	out += String(ihLearnedLagSec);
	out += "\n";
	out += "extraDone:";
	out += String((unsigned)ihExtraDone);
	out += "\n";
	out += "safetyCutCount:";
	out += String(ihSafetyCutCount);
	out += "\n";
	out += "belowPeakCount:";
	out += String((unsigned)ihBelowPeakCount);
	out += "\n";
	out += "lastHeatSlope_C_per_h:";
	if(ihLastHeatSlope == 0){
		out += "0";
	}else{
		out += tempDiffToString(tbuf, ihLastHeatSlope, 3, 12);
	}
	out += "\n";
	out += "peakTemp:";
	if(ihPeakTemp == INVALID_TEMP){
		out += "na";
	}else{
		out += tempToString(tbuf, ihPeakTemp, 3, 12);
	}
	out += "\n";
	out += "estPeak:";
	if(cv.estimatedPeak == INVALID_TEMP){
		out += "na";
	}else{
		out += tempToString(tbuf, cv.estimatedPeak, 3, 12);
	}
	out += "\n";
	out += "beerFast:";
	out += tempToString(tbuf, beerSensor->readFastFiltered(), 3, 12);
	out += " beerSlow:";
	out += tempToString(tbuf, beerSensor->readSlowFiltered(), 3, 12);
	out += "\n";
	out += "beerSet:";
	out += tempToString(tbuf, cs.beerSetting, 3, 12);
	out += " U(beerSet+heatTargetH):";
	out += tempToString(tbuf, cs.beerSetting + cc.heatingTargetUpper, 3, 12);
	out += "\n";
	out += "heatTargetH:";
	out += tempDiffToString(tbuf, cc.heatingTargetUpper, 3, 12);
	out += " heatTargetL:";
	out += tempDiffToString(tbuf, cc.heatingTargetLower, 3, 12);
	out += "\n";
	out += "minHeatTime:";
	out += String(cc.minHeatTime);
	out += " maxHeatTime:";
	out += String(cc.maxHeatTime);
	out += " minHeatIdleTime:";
	out += String(cc.minHeatIdleTime);
	out += "\n";
}

bool TempControl::applyIndependentHeatDebug(const char* key, const char* value){
	if(!key || !value){
		return false;
	}
	if(strcmp(key, "reset") == 0){
		resetIndependentHeatLearn();
		independentHeaterOn = false;
		return true;
	}
	if(strcmp(key, "learned") == 0 || strcmp(key, "learnedOnSec") == 0){
		ihLearnedOnSec = (uint16_t)atoi(value);
		return true;
	}
	if(strcmp(key, "planned") == 0 || strcmp(key, "plannedOnSec") == 0){
		ihPlannedOnSec = (uint16_t)atoi(value);
		return true;
	}
	if(strcmp(key, "pending") == 0 || strcmp(key, "pendingCorrSec") == 0){
		ihPendingCorrSec = (int16_t)atoi(value);
		return true;
	}
	if(strcmp(key, "cycle") == 0 || strcmp(key, "heatCycle") == 0){
		ihHeatCycle = (uint8_t)atoi(value);
		return true;
	}
	if(strcmp(key, "hystMode") == 0 || strcmp(key, "predictive") == 0){
		uint8_t v = (uint8_t)atoi(value);
		cc.hysteresisMode = (v != 0) ? HYSTERESIS_MODE_PID_ON : HYSTERESIS_MODE_PID_OFF;
		if(cc.hysteresisMode == HYSTERESIS_MODE_PID_OFF){
			resetIndependentHeatLearn();
		}
		eepromManager.storeTempConstantsAndSettings();
		return true;
	}
	if(strcmp(key, "maxHeatTime") == 0){
		cc.maxHeatTime = (uint16_t)atoi(value);
		eepromManager.storeTempConstantsAndSettings();
		return true;
	}
	return false;
}

void TempControl::appendIndependentHeatDebugForm(String& html){
	html += F("<form method=GET action=/pid>");
	html += F("<label>learnedOnSec <input name=learned type=number value=");
	html += String(ihLearnedOnSec);
	html += F("> s</label>");
	html += F("<label>plannedOnSec <input name=planned type=number value=");
	html += String(ihPlannedOnSec);
	html += F("> s</label>");
	html += F("<label>pendingCorrSec <input name=pending type=number value=");
	html += String(ihPendingCorrSec);
	html += F("> s</label>");
	html += F("<label>heatCycle <input name=cycle type=number min=0 max=255 value=");
	html += String((unsigned)ihHeatCycle);
	html += F("> #</label>");
	html += F("<label>hystMode <input name=hystMode type=number min=0 max=1 value=");
	html += String((unsigned)cc.hysteresisMode);
	html += F("> 0/1</label>");
	html += F("<label>maxHeatTime <input name=maxHeatTime type=number min=0 value=");
	html += String(cc.maxHeatTime);
	html += F("> s (0=unlimited)</label>");
	html += F("<button type=submit>Apply</button>");
	html += F("<button type=submit name=reset value=1>Reset learn</button>");
	html += F("</form>");
}

void TempControl::setBeerTemp(temperature newTemp){
	temperature oldBeerSetting = cs.beerSetting;
	cs.beerSetting= newTemp;
	if(abs(oldBeerSetting - newTemp) > intToTempDiff(1)/2){ // more than half degree C difference with old setting
		reset(); // reset controller
	}
	updatePID();
	updateState();
	if((cs.mode != MODE_BEER_PROFILE && cs.mode != MODE_INDEPENDENT) || abs(storedBeerSetting - newTemp) > intToTempDiff(1)/4){
		// more than 1/4 degree C difference with EEPROM
		// Do not store settings every time in profile mode, because EEPROM has limited number of write cycles.
		// A temperature ramp would cause a lot of writes. The same applies to independent mode,
		// since the ramp there drives cs.beerSetting (the heater target) exactly the same way.
		// If Raspberry Pi is connected, it will update the settings anyway. This is just a safety feature.
		eepromManager.storeTempSettings();
	}
}

void TempControl::setFridgeTemp(temperature newTemp){
	cs.fridgeSetting = newTemp;
	reset(); // reset peak detection and PID
	updatePID();
	updateState();
	eepromManager.storeTempSettings();
}

bool TempControl::stateIsCooling(void){
	return (state==COOLING || state==COOLING_MIN_TIME);
}
bool TempControl::stateIsHeating(void){
	return (state==HEATING || state==HEATING_MIN_TIME);
}

const ControlConstants TempControl::ccDefaults PROGMEM =
{
	// Do Not change the order of these initializations!
	/* tempFormat */ 'C',
	/* tempSettingMin */ intToTemp(1),	// +1 deg Celsius
	/* tempSettingMax */ intToTemp(30),	// +30 deg Celsius

	// control defines, also in fixed point format (7 int bits, 9 frac bits), so multiplied by 2^9=512
	/* Kp	*/ intToTempDiff(5),	// +5
	/* Ki	*/ intToTempDiff(1)/4, // +0.25
	/* Kd	*/ intToTempDiff(-3)/2,	// -1.5
	/* iMaxError */ intToTempDiff(5)/10,  // 0.5 deg

	// Classic idle-range start thresholds (fridge/beer/profile)
	/* idleRangeHigh */ intToTempDiff(1),	// +1.0 deg Celsius
	/* idleRangeLow */ intToTempDiff(-1),	// -1.0 deg Celsius

	// Peak estimator (classic) / Independent on/off bands
	/* heatingTargetUpper */ intToTempDiff(3)/10,	// +0.3 deg Celsius
	/* heatingTargetLower */ intToTempDiff(-2)/10,	// -0.2 deg Celsius
	/* coolingTargetUpper */ intToTempDiff(2)/10,	// +0.2 deg Celsius
	/* coolingTargetLower */ intToTempDiff(-3)/10,	// -0.3 deg Celsius

	// maximum history to take into account, in seconds
	/* maxHeatTimeForEstimate */ 600,
	/* maxCoolTimeForEstimate */ 1200,

	// Set filter coefficients. This is the b value. See FilterFixed.h for delay times.
	// The delay time is 3.33 * 2^b * number of cascades
	/* fridgeFastFilter */ 1u,
	/* fridgeSlowFilter */ 4u,
	/* fridgeSlopeFilter */ 3u,
	/* beerFastFilter */ 3u,
	/* beerSlowFilter */ 4u,
	/* beerSlopeFilter */ 4u,

	/* lightAsHeater */ 0,
	/* rotaryHalfSteps */ 0,

	/* pidMax */ intToTempDiff(10),	// +/- 10 deg Celsius
#if 1 //SettableMinimumCoolTime
    /* minCoolTime */ 180,
    /* minCoolIdleTime */ 300,
    /* minHeatTime */  180,
    /* minHeatIdleTime */ 300,
    /* mutexDeadTime */ 600,
#endif
	/* hysteresisMode */ HYSTERESIS_MODE_PID_OFF,
	/* maxHeatTime */ 0,
	/* staleSensorSec */ 20,
	/* oneWireRetryCount */ 2
};
