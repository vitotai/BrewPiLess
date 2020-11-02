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
#include "DisplayBase.h"
#include "NullLcdDriver.h"

#include "FS.h"
#include <SPI.h>
#include <TFT_eSPI.h>



class TFTDisplay DISPLAY_SUPERCLASS
{
	public:
	TFTDisplay(void);
	~TFTDisplay(void);

	DISPLAY_METHOD void init(void);
	DISPLAY_METHOD void printAll() {
		printStationaryText();
		printState();
		printAllTemperatures();
		printMode();
	}

	// print all temperatures 
	DISPLAY_METHOD void printAllTemperatures(void);

	// print the stationary text 
	DISPLAY_METHOD void printStationaryText(void);

	// print mode on the right location on the first line, after Mode:
	DISPLAY_METHOD void printMode(void);

	DISPLAY_METHOD void setDisplayFlags(uint8_t newFlags);
	DISPLAY_METHOD uint8_t getDisplayFlags(void);

	// print beer temperature at the right place on the display
	DISPLAY_METHOD void printBeerTemp(void);

	// print fridge temperature at the right place on the display
	DISPLAY_METHOD void printFridgeTemp(void);


	// print the current state on the last line of the LCD
	DISPLAY_METHOD void printState(void);

	DISPLAY_METHOD void printTemperatureAt(uint8_t xpos, uint8_t line, temperature temp);
	DISPLAY_METHOD uint8_t _printTemperatureAt(uint8_t xpos, uint8_t line, temperature temp);
	
	DISPLAY_METHOD void printAt_P(uint8_t x, uint8_t y, const char* text);

	DISPLAY_METHOD void setBufferOnly(bool bufferOnly);

	DISPLAY_METHOD void setAutoOffPeriod(uint32_t period);

	DISPLAY_METHOD void getLine(uint8_t lineNumber, char * buffer);

	DISPLAY_METHOD void updateBacklight(void);

	DISPLAY_METHOD uint8_t drawString_P(const char *, uint8_t, uint8_t);
	
#ifdef EARLY_DISPLAY
	DISPLAY_METHOD void clear() ;
#endif

	// print degree sign + C/F
	DISPLAY_METHOD uint8_t printDegreeUnit(uint8_t x, uint8_t y);

#ifdef STATUS_LINE
	DISPLAY_METHOD void printStatus(char* text){ lcd.printStatus(text);}
#endif
	private:
	DISPLAY_FIELD TFT_eSPI * _display;
	DISPLAY_FIELD uint8_t _font;
	DISPLAY_FIELD uint8_t _textSize;
	DISPLAY_FIELD uint8_t _fontHeight;
	DISPLAY_FIELD int16_t _background;
	DISPLAY_FIELD uint8_t stateOnDisplay;
	DISPLAY_FIELD uint8_t flags;
	DISPLAY_FIELD uint8_t _cols;
	DISPLAY_FIELD uint8_t _rows;

};
