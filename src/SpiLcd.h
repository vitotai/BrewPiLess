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

/* This is an adaptation of the Arduino LiquidCrystal library for the
 * NHD-0420DZW-AY5-ND lcd display, made by NewHaven. The display should be HD44780 compatible but isn't.
 * Differences are some of the control commands (cursor on/off), language setting and especially initialization sequence.
 */

#pragma once

#include "Brewpi.h"
#include "BrewpiStrings.h"
#include <stdint.h>
#include <Print.h>
#include "Ticks.h"

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#ifdef LCD_FUNCTIONSET
#undef LCD_FUNCTIONSET
#endif
#define LCD_FUNCTIONSET 0x28
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_ENGLISH_JAPANESE   0x00
#define LCD_WESTERN_EUROPEAN_1 0x01
#define LCD_ENGLISH_RUSSIAN    0x02
#define LCD_WESTERN_EUROPEAN_2 0x03

// Define shift register pins outputs
#define LCD_SHIFT_RS 0 // QA, LOW= command.  HIGH= character.
#define LCD_SHIFT_ENABLE 1 // QB, activated by a HIGH pulse.
#define LCD_SHIFT_BACKLIGHT 2 // QC, LOW = backlight on
#define LCD_SHIFT_QD 3 // unused QD pin
#define LCD_SHIFT_DATA_MASK 0xF0 // Data bits, QE = D4, QF = D5, QG = D6, QH = D7

// Backlight is switched with a P-channel MOSFET, so signal is inverted.
#ifndef BACKLIGHT_AUTO_OFF_PERIOD
#define BACKLIGHT_AUTO_OFF_PERIOD 600
#endif

class SpiLcd : public Print {
	public:
	// Constants are set in initializer list of constructor
	SpiLcd(){};
	~SpiLcd(){};

	void init();

	void begin(uint8_t cols, uint8_t rows);

	void clear();
	void home();

	void noDisplay();
	void display();
	void noBlink();
	void blink();
	void noCursor();
	void cursor();
	void scrollDisplayLeft();
	void scrollDisplayRight();
	void leftToRight();
	void rightToLeft();
	void autoscroll();
	void noAutoscroll();

	void createChar(uint8_t, uint8_t[]);
	void setCursor(uint8_t, uint8_t);

	virtual size_t write(uint8_t);

#define print_P_inline 1
#ifdef print_P_inline
	void print_P(const char * str){ // print a string stored in PROGMEM
		char buf[21]; // create buffer in RAM
		strcpy_P(buf, str); // copy string to RAM
		print(buf); // print from RAM
	}
#else
	void print_P(const char * str);
#endif
	// copy a line from the shadow copy to a string buffer and correct the degree sign
	void getLine(uint8_t lineNumber, char * buffer);

	void readContent(void); // read the content from the display to the shadow copy buffer

	void command(uint8_t);
	char readChar(void);

	void setBufferOnly(bool bufferOnly) { _bufferOnly = bufferOnly; }

	void resetBacklightTimer(void);

	void updateBacklight(void);

	uint8_t getCurrPos(void){
		return _currpos;
	}
	uint8_t getCurrLine(void){
		return _currline;
	}

	// Write spaces from current position to line end.
	void printSpacesToRestOfLine(void);

	using Print::write;

	private:
	void spiOut(void);
	void initSpi(void);
	void send(uint8_t, uint8_t);
	void write4bits(uint8_t);
	void pulseEnable();
	void waitBusy();

	// Define shift register byte, keep pin state in this byte and send it out for each write.
	volatile uint8_t _spiByte;

	uint8_t _displayfunction;
	uint8_t _displaycontrol;
	uint8_t _displaymode;
	uint8_t _currline;
	uint8_t _currpos;
	uint8_t _numlines;

	bool	_bufferOnly;
	uint16_t _backlightTime;

	char content[4][21]; // always keep a copy of the display content in this variable

};
