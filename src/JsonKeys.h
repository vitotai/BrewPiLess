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

#pragma once

#include "Brewpi.h"

#ifdef ARDUINO
#if defined(ESP8266) || defined(ESP32)
// There is no concept of PROGMEM with the ESP8266
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif
#endif

static const char JSONKEY_mode[] PROGMEM = "mode";
static const char JSONKEY_beerSetting[] PROGMEM = "beerSet";
static const char JSONKEY_fridgeSetting[] PROGMEM = "fridgeSet";
static const char JSONKEY_heatEstimator[] PROGMEM = "heatEst";
static const char JSONKEY_coolEstimator[] PROGMEM = "coolEst";

// constant;
static const char JSONKEY_tempFormat[] PROGMEM = "tempFormat";
static const char JSONKEY_tempSettingMin[] PROGMEM = "tempSetMin";
static const char JSONKEY_tempSettingMax[] PROGMEM = "tempSetMax";
static const char JSONKEY_pidMax[] PROGMEM = "pidMax";
static const char JSONKEY_Kp[] PROGMEM = "Kp";
static const char JSONKEY_Ki[] PROGMEM = "Ki";
static const char JSONKEY_Kd[] PROGMEM = "Kd";
static const char JSONKEY_iMaxError[] PROGMEM = "iMaxErr";
static const char JSONKEY_idleRangeHigh[] PROGMEM = "idleRangeH";
static const char JSONKEY_idleRangeLow[] PROGMEM = "idleRangeL";
static const char JSONKEY_heatingTargetUpper[] PROGMEM = "heatTargetH";
static const char JSONKEY_heatingTargetLower[] PROGMEM = "heatTargetL";
static const char JSONKEY_coolingTargetUpper[] PROGMEM = "coolTargetH";
static const char JSONKEY_coolingTargetLower[] PROGMEM = "coolTargetL";
static const char JSONKEY_maxHeatTimeForEstimate[] PROGMEM = "maxHeatTimeForEst";
static const char JSONKEY_maxCoolTimeForEstimate[] PROGMEM = "maxCoolTimeForEst";
static const char JSONKEY_fridgeFastFilter[] PROGMEM = "fridgeFastFilt";
static const char JSONKEY_fridgeSlowFilter[] PROGMEM = "fridgeSlowFilt";
static const char JSONKEY_fridgeSlopeFilter[] PROGMEM = "fridgeSlopeFilt";
static const char JSONKEY_beerFastFilter[] PROGMEM = "beerFastFilt";
static const char JSONKEY_beerSlowFilter[] PROGMEM = "beerSlowFilt";
static const char JSONKEY_beerSlopeFilter[] PROGMEM = "beerSlopeFilt";
static const char JSONKEY_lightAsHeater[] PROGMEM = "lah";
static const char JSONKEY_rotaryHalfSteps[] PROGMEM = "hs";
#if SettableMinimumCoolTime 
static const char JSONKEY_minCoolTime[] PROGMEM = "minCoolTime";
static const char JSONKEY_minCoolIdleTime[] PROGMEM = "minCoolIdleTime";
static const char JSONKEY_minHeatTime[] PROGMEM = "minHeatTime";
static const char JSONKEY_minHeatIdleTime[] PROGMEM = "minHeatIdleTime";
static const char JSONKEY_mutexDeadTime[] PROGMEM = "deadTime";
#endif
// variable;
static const char JSONKEY_beerDiff[] PROGMEM = "beerDiff";
static const char JSONKEY_diffIntegral[] PROGMEM = "diffIntegral";
static const char JSONKEY_beerSlope[] PROGMEM = "beerSlope";
static const char JSONKEY_p[] PROGMEM = "p";
static const char JSONKEY_i[] PROGMEM = "i";
static const char JSONKEY_d[] PROGMEM = "d";
static const char JSONKEY_estimatedPeak[] PROGMEM = "estPeak"; // current peak estimate
static const char JSONKEY_negPeakEstimate[] PROGMEM = "negPeakEst"; // last neg peak estimate before switching to idle
static const char JSONKEY_posPeakEstimate[] PROGMEM = "posPeakEst";
static const char JSONKEY_negPeak[] PROGMEM = "negPeak"; // last true neg peak
static const char JSONKEY_posPeak[] PROGMEM = "posPeak";

static const char JSONKEY_logType[] PROGMEM = "logType";
static const char JSONKEY_logID[] PROGMEM = "logID";
