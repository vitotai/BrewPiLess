#include "TTGODisplay.h"

#include "Brewpi.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <inttypes.h>
#include "Arduino.h"

#if BREWPI_TTGO

#include "font_cousine_10.h"

#if SerialDebug == true
#define DebugPort Serial
#define DBG_PRINTF(...) DebugPort.printf(__VA_ARGS__)
#else
#define DBG_PRINTF(...)
#endif

TTGODisplay::TTGODisplay()
:_display()
{ 
}

void TTGODisplay::init(){
    _display.init();
    _display.setRotation(1);
    _display.setTextColor(TFT_WHITE, TFT_BLACK);
    _font = 1;
    _display.setTextFont(_font);
    _textSize = 2;
    _display.setTextSize(2);
    _display.setTextWrap(true);
    _backlightTime = 0;
    _fontHeight=_display.fontHeight(_font);
    
}
void TTGODisplay::begin(uint8_t cols, uint8_t lines){
    _cols = cols;
  	_rows = lines;
    clear();
    for(uint8_t r = 0; r < _rows; r++){
        for(uint8_t c = 0; c < _cols; c++)
            _content[r][c] = ' ';
    }
}

/********** high level commands, for the user! */
void TTGODisplay::clear(){
	_display.fillScreen(TFT_BLACK);
    

  	_currline = 0;
    _currpos = 0;
}

void TTGODisplay::home(){
    _currline = 0;
	_currpos = 0;
}

void TTGODisplay::setCursor(uint8_t col, uint8_t row){
	if ( row > _rows ) {
		row = _rows-1;    // we count rows starting w/0
	}

    _currline = row;
    _currpos = col;
}


/*********** mid level commands, for sending data/cmds */

/**
 * Resets screen and fills with "content" private variable
 **/
void TTGODisplay::printContent(){
    
    for(int r = 0; r < _rows; r++){
        _display.drawString(content[r],0,r*_fontHeight);
    }
}

inline void TTGODisplay::internal_write(uint8_t value) {
    content[_currline][_currpos] = value;
    _currpos++;
}

inline size_t TTGODisplay::write(uint8_t value) {
	internal_write(value);
    return 0;
}

// This resets the backlight timer and updates the SPI output
void TTGODisplay::resetBacklightTimer(void) {
    _backlightTime = ticks.seconds();
}

void TTGODisplay::updateBacklight(void) {
    // True = OFF, False = ON
    bool backLightOutput = (backlightAutoOffPeriod !=0) && (BREWPI_SIMULATE || ticks.timeSince(_backlightTime) > backlightAutoOffPeriod);
    if(backLightOutput) {
        ;
    } else {
        ;
    }
}

// Puts the content of one LCD line into the provided buffer.
void TTGODisplay::getLine(uint8_t lineNumber, char * buffer){
    const char* src = content[lineNumber];
    for(uint8_t i = 0; i < _cols;i++){
        char c = src[i];
        buffer[i] = (c == 0b11011111) ? 0xB0 : c;
    }
    buffer[_cols] = '\0'; // NULL terminate string
}

void TTGODisplay::printSpacesToRestOfLine(void){
    while(_currpos < _cols){
        internal_write(' ');
    }
    printContent();
}

void TTGODisplay::print(char * str){
    char *p=str;
 
 //   DBG_PRINTF("%d,%d, %s\n",_currline,_currpos,str);

    while(*p !='\0' && _currpos < _cols){
	    content[_currline][_currpos] = *p;
    	_currpos++;
    	p++;
    }

    if (!_bufferOnly) {

    	printContent();
    }
}

#ifndef print_P_inline
void TTGODisplay::print_P(const char * str){ // print a string stored in PROGMEM
    char buf[21]; // create buffer in RAM
    strcpy_P(buf, str); // copy string to RAM
    print(buf); // print from RAM
}
#endif

#ifdef STATUS_LINE
void TTGODisplay::printStatus(char* str)
{
    strcpy(_content[0], str);
    _printContent();
}
#endif

#endif
