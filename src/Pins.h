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

#if BREWPI_STATIC_CONFIG==BREWPI_SHIELD_REV_A
#ifndef beerSensorPin
#define beerSensorPin    A5 // OneWire 1
#endif

#ifndef fridgeSensorPin
#define fridgeSensorPin  A4 // OneWire 2
#endif

#ifndef coolingPin
#define coolingPin	6
#endif

#ifndef heatingPin
#define heatingPin	5
#endif

#endif

#if BREWPI_STATIC_CONFIG==BREWPI_SHIELD_REV_C

#ifndef oneWirePin
#define oneWirePin A4
#endif

#ifndef actuatorPin1
#define actuatorPin1 2
#endif

#ifndef actuatorPin2
#define actuatorPin2 5
#endif

#ifndef actuatorPin3
#define actuatorPin3 6
#endif

#ifndef actuatorPin4
#define actuatorPin4 A5
#endif

#endif

#if BREWPI_STATIC_CONFIG==BREWPI_SHIELD_REV_A || BREWPI_STATIC_CONFIG==BREWPI_SHIELD_REV_C

#ifndef doorPin
#define doorPin		4
#endif

#ifndef alarmPin
#define alarmPin	3
#endif

#ifndef lcdLatchPin
#define lcdLatchPin 10
#endif

// If you change the interrupt pins, you will also have to review the interrupt vectors of the rotary encoder
#define rotarySwitchPin 7 // INT6 on leo or PCINT23 on uno
#define rotaryAPin 8 // PCINT4 on leo or PCINT0 on uno
#define rotaryBPin 9 // PCINT5 on leo or PCINT1 on uno

#define BREWPI_INVERT_ACTUATORS 1

#elif BREWPI_STATIC_CONFIG==BREWPI_SHIELD_DIY

// pins
#ifndef oneWirePin // So we can specify a single bus if we prefer
#ifndef beerSensorPin
#define beerSensorPin    10
#endif

#ifndef fridgeSensorPin
#define fridgeSensorPin  11
#endif
#endif
// Pay attention when changing the pins for the rotary encoder.
// They should be connected to external interrupt INT0, INT1 and INT3

#ifndef rotaryAPin
#define rotaryAPin 2 // INT1
#endif

#ifndef rotaryBPin
#define rotaryBPin 1 // INT3
#endif

#ifndef rotarySwitchPin
#define rotarySwitchPin 0 // INT2
#endif

#ifndef coolingPin
#define coolingPin 12
#endif

#ifndef heatingPin
#define heatingPin 13
#endif

#ifndef doorPin
#define doorPin    A5
#endif

// TODO - Fix the code for the LCD module
#define DISP_RS 9
#define DISP_RW 8
#define DISP_EN 7
#define DISP_D4 6
#define DISP_D5 5
#define DISP_D6 4
#define DISP_D7 3

#ifndef BREWPI_INVERT_ACTUATORS
#define BREWPI_INVERT_ACTUATORS 0
#endif


#endif

// You can use the internal pull-up resistors instead of external ones for the doorPin and the rotary encoder pins
#ifndef USE_INTERNAL_PULL_UP_RESISTORS
#define USE_INTERNAL_PULL_UP_RESISTORS 1
#endif
