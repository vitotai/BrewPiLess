
/*
 * Copyright 2012-2013 BrewPi/Elco Jacobs.
 *
 * This file is part of BrewPi.
 *
 * BrewPi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later v7ersion.
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
#include "BrewpiStrings.h"
#include <limits.h>
#include <stdint.h>

#ifdef BREWPI_TFT

#include "Display.h"
#include "Menu.h"
#include "TempControl.h"
#include "TemperatureFormats.h"
#include "Pins.h"
#include "TFTDisplay.h"

// Constant strings used multiple times
static const char STR_Beer_[] PROGMEM = "Beer ";
static const char STR_Fridge_[] PROGMEM = "Fridge ";
static const char STR_Const_[] PROGMEM = "Const.";
static const char STR_Cool[] PROGMEM = "Cool";
static const char STR_Heat[] PROGMEM = "Heat";
static const char STR_ing_for[] PROGMEM = "ing for";
static const char STR_Wait_to_[] PROGMEM = "Wait to ";
static const char STR__time_left[] PROGMEM = " time left";
static const char STR_empty_string[] PROGMEM = "";

TFT_eSPI * TFTDisplay::_display;
uint8_t TFTDisplay::_font;
uint8_t TFTDisplay::_textSize;
uint8_t TFTDisplay::_fontHeight;
int16_t TFTDisplay::_background;
uint8_t TFTDisplay::stateOnDisplay;
uint8_t TFTDisplay::flags;

TFTDisplay::TFTDisplay(void){
	;
}


TFTDisplay::~TFTDisplay(void){
	;
}

void TFTDisplay::init(void){
	_display = new TFT_eSPI();
	_display->init();
    _display->setRotation(1);
	_background = TFT_DARKGREY;
    _display->setTextColor(TFT_GREEN, _background);
    _font = 1;
    _display->setTextFont(_font);
    _textSize = 2;
    _display->setTextSize(2);
	_fontHeight = _display->fontHeight();
    _display->setTextWrap(true);
	_display->fillScreen(_background);
}

#ifndef UINT16_MAX
#define UINT16_MAX 65535
#endif

void TFTDisplay::printAt_P(uint8_t x, uint8_t y, const char* text){
	_display->drawString(text, x, y * _fontHeight);
	printMode();
	printState();
}


//print all temperatures on the screen
void TFTDisplay::printAllTemperatures(void){
	// alternate between beer and room temp
	if (flags & LCD_FLAG_ALTERNATE_ROOM) {
		bool displayRoom = ((ticks.seconds()&0x08)==0) && !BREWPI_SIMULATE && tempControl.ambientSensor->isConnected();
		if (displayRoom ^ ((flags & LCD_FLAG_DISPLAY_ROOM)!=0)) {	// transition
			flags = displayRoom ? flags | LCD_FLAG_DISPLAY_ROOM : flags & ~LCD_FLAG_DISPLAY_ROOM;
			//;
		}
	}

	printBeerTemp();
	printFridgeTemp();
	printState();
	printMode();
}

void TFTDisplay::setDisplayFlags(uint8_t newFlags) {
	flags = newFlags;
	printStationaryText();
	printAllTemperatures();
}

uint8_t TFTDisplay::getDisplayFlags(){ 
	return flags; 
};

void TFTDisplay::printBeerTemp(void){
	uint8_t xpos = _display->drawString(STR_Beer_, 0, _fontHeight);
	xpos += printTemperatureAt(xpos, 1, tempControl.getBeerTemp());
	xpos += printTemperatureAt(xpos, 1, tempControl.getBeerSetting());
}


void TFTDisplay::printFridgeTemp(void){
	int16_t xpos = _display->drawString((flags & LCD_FLAG_DISPLAY_ROOM) ?  PSTR("Room  ") : STR_Fridge_,  0, 2 * _fontHeight);
	xpos += printTemperatureAt(xpos,2, flags & LCD_FLAG_DISPLAY_ROOM ?
		tempControl.ambientSensor->read() :
		tempControl.getFridgeTemp());
	
	temperature fridgeSet = tempControl.getFridgeSetting();
	if(flags & LCD_FLAG_DISPLAY_ROOM) // beer setting is not active
		fridgeSet = INVALID_TEMP;
	printTemperatureAt(xpos, 2, fridgeSet);
}

// TODO: Not implemented
void TFTDisplay::getLine(uint8_t lineNumber, char * buffer){
    const char* src = PSTR("Not implemented");
    for(uint8_t i = 0; i < strlen(src);i++){
        char c = src[i];
        buffer[i] = (c == 0b11011111) ? 0xB0 : c;
    }
    buffer[strlen(src)] = '\0'; // NULL terminate string
}

void TFTDisplay::updateBacklight(void) {
    // TODO?
}

void TFTDisplay::setAutoOffPeriod(uint32_t period){
	// TODO
}

uint8_t TFTDisplay::printTemperatureAt(uint8_t x, uint8_t line, temperature temp){

	if (temp==INVALID_TEMP) {
		return _display->drawString(PSTR(" --.-"), x, line*_fontHeight);
	}
	char tempString[9];
	tempToString(tempString, temp, 1 , 9);
	uint8_t xdiff = _display->drawString(tempString, x, line * _fontHeight);
	xdiff += _display->drawChar(0x00B0, x+xdiff, line * _fontHeight);
	xdiff += _display->drawChar(tempControl.cc.tempFormat, x+xdiff, line*_fontHeight);
	return xdiff;
}

void TFTDisplay::printStationaryText(void){
	;
}
/**
//print degree sign + temp unit
uint8_t TFTDisplay::printDegreeUnit(uint8_t x, uint8_t y){
	uint8_t x1 = _display->drawChar(0b11011111, x, y*_fontHeight);
	return x1 + _display->drawChar(tempControl.cc.tempFormat, x+x1, y*_fontHeight);
}
**/

// print mode on the right location on the first line, after "Mode   "
void TFTDisplay::printMode(void){
	uint8_t xpos = _display->drawString("Mode ", 0, 0);
	// Factoring prints out of switch has negative effect on code size in this function
	switch(tempControl.getMode()){
		case MODE_FRIDGE_CONSTANT:
			xpos += _display->drawString(STR_Fridge_, xpos, 0);
			xpos += _display->drawString(STR_Const_, xpos, 0);
			break;
		case MODE_BEER_CONSTANT:
			xpos += _display->drawString(STR_Beer_, xpos, 0);
			xpos += _display->drawString(STR_Const_, xpos, 0);
			break;
		case MODE_BEER_PROFILE:
			xpos += _display->drawString(STR_Beer_, xpos, 0);
			xpos += _display->drawString(PSTR("Profile"), xpos, 0);
			break;
		case MODE_OFF:
			xpos += _display->drawString(PSTR("Off"), xpos, 0);
			break;
		case MODE_TEST:
			xpos += _display->drawString(PSTR("** Testing **"), xpos, 0);
			break;
		default:
			xpos += _display->drawString(PSTR("Invalid mode"), xpos, 0);
			break;
	}
	_display->fillRect(xpos, 0, 200, _fontHeight, _background);
}


#ifdef EARLY_DISPLAY
void TFTDisplay::clear(void){
	_display->fillScreen(_background);
}
#endif

// print the current state on the last line of the lcd
void TFTDisplay::printState(void){
	uint16_t time = UINT16_MAX; // init to max
	uint8_t state = tempControl.getDisplayState();
	uint8_t xpos = 0;
	if(state != stateOnDisplay){ //only print static text when state has changed
		stateOnDisplay = state;
		// Reprint state and clear rest of the line
		const char * part1 = STR_empty_string;
		const char * part2 = STR_empty_string;
		switch (state){
			case IDLE:
				part1 = PSTR("Idl");
				part2 = STR_ing_for;
				break;
			case WAITING_TO_COOL:
				part1 = STR_Wait_to_;
				part2 = STR_Cool;
				break;
			case WAITING_TO_HEAT:
				part1 = STR_Wait_to_;
				part2 = STR_Heat;
				break;
			case WAITING_FOR_PEAK_DETECT:
				part1 = PSTR("Waiting for peak");
				break;
			case COOLING:
				part1 = STR_Cool;
				part2 = STR_ing_for;
				break;
			case HEATING:
				part1 = STR_Heat;
				part2 = STR_ing_for;
				break;
			case COOLING_MIN_TIME:
				part1 = STR_Cool;
				part2 = STR__time_left;
				break;
			case HEATING_MIN_TIME:
				part1 = STR_Heat;
				part2 = STR__time_left;
				break;
			case DOOR_OPEN:
				part1 = PSTR("Door open");
				break;
			case STATE_OFF:
				part1 = PSTR("Temp. control OFF");
				break;
			default:
				part1 = PSTR("Unknown status!");
				break;
		}
		xpos += _display->drawString(part1, xpos, 3*_fontHeight);
		xpos += _display->drawString(part2, xpos, 3*_fontHeight);
		
	}
	uint16_t sinceIdleTime = tempControl.timeSinceIdle();
	if(state==IDLE){
		time = 	min(tempControl.timeSinceCooling(), tempControl.timeSinceHeating());
	}
	else if(state==COOLING || state==HEATING){
		time = sinceIdleTime;
	}
	else if(state==COOLING_MIN_TIME){
		#if SettableMinimumCoolTime
		time =(tempControl.cc.minCoolTime > sinceIdleTime)? (tempControl.cc.minCoolTime -sinceIdleTime):0;
		#else
		time = MIN_COOL_ON_TIME-sinceIdleTime;
		#endif
	}

	else if(state==HEATING_MIN_TIME){
		#if SettableMinimumCoolTime
		time = (tempControl.cc.minHeatTime > sinceIdleTime)? (tempControl.cc.minHeatTime-sinceIdleTime):0;
		#else
		time = MIN_HEAT_ON_TIME-sinceIdleTime;
		#endif
	}
	else if(state == WAITING_TO_COOL || state == WAITING_TO_HEAT){
		time = tempControl.getWaitTime();
	}
	if(time != UINT16_MAX){
		char timeString[10];
#if DISPLAY_TIME_HMS  // 96 bytes more space required.
		unsigned int minutes = time/60;
		unsigned int hours = minutes/60;
		int stringLength = sprintf_P(timeString, PSTR("%dh%02dm%02d"), hours, minutes%60, time%60);
		char * printString = timeString;
		if(!hours){
			printString = &timeString[2];
			stringLength = stringLength-2;
		}
		xpos += _display->drawString(printString, xpos, 3 * _fontHeight);
#else
		xpos += _display->drawString(timeString, xpos, 3 * _fontHeight);
#endif
//_display->fillRect(xpos, 3*_fontHeight, 100, _fontHeight, _background);
	}
}
#endif