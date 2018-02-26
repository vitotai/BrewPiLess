/*
 * Copyright 2012-2013 BrewPi/Elco Jacobs.
 * Copyright 2013 Matthew McGowan
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
#include "BrewpiStrings.h"

#if BREWPI_MENU

#include "Menu.h"

#include <limits.h>
#include "Pins.h"
#include "Display.h"
#include "TempControl.h"
#include "TemperatureFormats.h"
#include "RotaryEncoder.h"
#include "PiLink.h"
#include "Ticks.h"

Menu menu;

#define MENU_TIMEOUT 10u

void Menu::pickSettingToChange(){
	// ensure beer temp is displayed
	uint8_t oldFlags = display.getDisplayFlags();
	display.setDisplayFlags(oldFlags & ~(LCD_FLAG_ALTERNATE_ROOM|LCD_FLAG_DISPLAY_ROOM));
	pickSettingToChangeLoop();
	display.setDisplayFlags(oldFlags);
}

/**
 * @return {@code true} if a value was selected. {@code false} on timeout.
 */
#define BLINKING_TIME 250

bool blinkLoop(
	void (*changed)(),	// called to update the value
	void (*show)(),		// called to show the current value
	void (*hide)(),	// called to blank out the current value
	void (*pushed)())	// handle selection
{
	uint16_t lastChangeTime = ticks.seconds();
	//uint8_t blinkTimer = 0;
	uint32_t switchTime=0;
	bool hidden=true;

	while(ticks.timeSince(lastChangeTime) < MENU_TIMEOUT){ // time out at 10 seconds
	//#if ESP8266
		yield();
	//#endif
		if(rotaryEncoder.changed()){
			lastChangeTime = ticks.seconds();
			//blinkTimer = 0;
			switchTime=0;
			hidden=true;
			changed();
		}
/*		if (blinkTimer==0)
			show();
		else if (blinkTimer==128)
			hide(); */
		uint32_t current=millis();
		if((current - switchTime) > BLINKING_TIME){
			if(hidden) show();
			else hide();
			hidden = ! hidden;
			switchTime = current;
		}

		if (rotaryEncoder.pushed()) {
			rotaryEncoder.resetPushed();
			show();
			pushed();
			return true;
		}

		//blinkTimer++;
		//wait.millis(3); // delay for blinking
	}
	return false;
}

void clearSettingText() {
	display.printAt_P(0, rotaryEncoder.read(), STR_6SPACES);
}

void settingChanged() {} // no -op - the only change is to update the display which happens already

void settingSelected() {
	switch(rotaryEncoder.read()){
		case 0:
			menu.pickMode();
			return;
		case 1:
			// switch to beer constant, because beer setting will be set through display
			tempControl.setMode(MODE_BEER_CONSTANT);
			display.printMode();
			menu.pickBeerSetting();
			return;
		case 2:
			// switch to fridge constant, because fridge setting will be set through display
			tempControl.setMode(MODE_FRIDGE_CONSTANT);
			display.printMode();
			menu.pickFridgeSetting();
			return;
	}
}

void Menu::pickSettingToChangeLoop(void) {
	rotaryEncoder.setRange(0, 0, 2); // mode setting, beer temp, fridge temp
	blinkLoop(
		settingChanged,
		display.printStationaryText,
		clearSettingText,
		settingSelected
	);
}

void changedMode() {
	const char lookup[] = {'b', 'f', 'p', 'o'};
	tempControl.setMode(lookup[rotaryEncoder.read()]);
}

void clearMode() {
	display.printAt_P(7, 0, PSTR("             ")); // print 13 spaces
}

void selectMode() {
	char mode = tempControl.getMode();
	if(mode ==  MODE_BEER_CONSTANT){
		menu.pickBeerSetting();
	}
	else if(mode == MODE_FRIDGE_CONSTANT){
		menu.pickFridgeSetting();
	}
	else if(mode == MODE_BEER_PROFILE){
#ifdef ESP8266
		piLink.printTemperaturesJSON("Changed to profile mode in menu.", 0);
#else
		piLink.printBeerAnnotation(PSTR("Changed to profile mode in menu."));
#endif
	}
	else if(mode == MODE_OFF){
#ifdef ESP8266
		piLink.printTemperaturesJSON("Temp control turned off in menu.", 0);
#else
		piLink.printBeerAnnotation(PSTR("Temp control turned off in menu."));
#endif
	}
}

void Menu::pickMode(void) {
	char oldSetting = tempControl.getMode();
	uint8_t startValue=0;
	const char* LOOKUP = "bfpo";
	startValue = indexOf(LOOKUP, oldSetting);
	rotaryEncoder.setRange(startValue, 0, 3); // toggle between beer constant, beer profile, fridge constant

	if (!blinkLoop(changedMode, display.printMode, clearMode, selectMode))
		tempControl.setMode(oldSetting);
}

typedef void (* PrintAnnotation)(const char * annotation, ...);
typedef void (* DisplayUpdate)(void);
typedef temperature (* ReadTemp)();
typedef void (* WriteTemp)(temperature);

void pickTempSetting(ReadTemp readTemp, WriteTemp writeTemp, const char* tempName, PrintAnnotation printAnnoation, int row) {

	temperature oldSetting = readTemp();
	temperature startVal = oldSetting;
	if(oldSetting == INVALID_TEMP){	 // previous temperature was not defined, start at 20C
		startVal = intToTemp(20);
	}

	rotaryEncoder.setRange(fixedToTenths(oldSetting), fixedToTenths(tempControl.cc.tempSettingMin), fixedToTenths(tempControl.cc.tempSettingMax));

	//uint8_t blinkTimer = 0;
	uint32_t switchTime=0;
	bool hidden=true;

	uint16_t lastChangeTime = ticks.seconds();
	while(ticks.timeSince(lastChangeTime) < MENU_TIMEOUT){ // time out at 10 seconds
//		#if ESP8266
		yield();
//		#endif

		if(rotaryEncoder.changed()){
			lastChangeTime = ticks.seconds();
			//blinkTimer = 0;
			switchTime=0;
			hidden=true;

			startVal = tenthsToFixed(rotaryEncoder.read());
			display.printTemperatureAt(12, row, startVal);

			if( rotaryEncoder.pushed() ){
				rotaryEncoder.resetPushed();
				writeTemp(startVal);
				char tempString[9];
				//printAnnoation(PSTR("%S temp set to %s in Menu."), tempName, tempToString(tempString,startVal,1,9));

				printAnnoation(PSTR("%s temp set to %s in Menu."), tempName, tempToString(tempString,startVal,1,9));
				return;
			}
		}
		else{
			/*
			if(blinkTimer == 0){
				display.printTemperatureAt(12, row, startVal);
			}
			if(blinkTimer == 128){
				display.printAt_P(12, row, STR_6SPACES); // only 5 needed, but 6 is okay to and lets us re-use the string
			}
			blinkTimer++;
			wait.millis(3); // delay for blinking
			*/
			uint32_t current=millis();
			if((current - switchTime) > BLINKING_TIME){
				if(hidden) display.printTemperatureAt(12, row, startVal);
				else display.printAt_P(12, row, STR_6SPACES); 
				hidden = ! hidden;
				switchTime = current;
			}
			
		}
	}
	// Time Out. Setting is not written
}

void Menu::pickFridgeSetting(void){
	// TODO - Fix this
	//pickTempSetting(tempControl.getFridgeSetting, tempControl.setFridgeTemp, PSTR("Fridge"), piLink.printFridgeAnnotation, 2);
	pickTempSetting(tempControl.getFridgeSetting, tempControl.setFridgeTemp, "Fridge", piLink.printFridgeAnnotation, 2);
}

void Menu::pickBeerSetting(void){
	// TODO - Fix This
	pickTempSetting(tempControl.getBeerSetting, tempControl.setBeerTemp, "Beer", piLink.printBeerAnnotation, 1);
}


#endif
