#include "TTGODisplay.h"

#include "Brewpi.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <inttypes.h>
#include "Arduino.h"

#if BREWPI_TTGO

#include "font_cousine_10.h"

#define TOP_MARGIN (2)
#define LEFT_MARGIN 4

#define STATUS_TOP  52
#define STATUS_LEFT 4

#define STATUS_BAR_TOP  52
#define STATUS_BAR_LEFT 0
#define STATUS_BAR_HEIGHT 12
#define STATUS_BAR_WIDTH  128

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
    _backlightTime = 0;
    _fontHeight=18;
    _fontWidth=9;
}
void TTGODisplay::begin(uint8_t cols, uint8_t lines){
    _display.begin();
    
  	_cols = cols;
  	_rows = lines;
    clear();
}

/********** high level commands, for the user! */
void TTGODisplay::clear(){
	_display.fillScreen(TFT_BLACK);
    _display.setTextColor(TFT_WHITE, TFT_BLACK);

  	_currline = 0;
    _currpos = 0;
}

void TTGODisplay::home(){
}

void TTGODisplay::setCursor(uint8_t col, uint8_t row){
	if ( row > _rows ) {
		row = _rows-1;    // we count rows starting w/0
	}

    _currline = row;
    _currpos = col;
}


/*********** mid level commands, for sending data/cmds */

inline int16_t TTGODisplay::xpos(void)
{
	return LEFT_MARGIN + _fontWidth * _currpos;
}

inline int16_t  TTGODisplay::ypos(void)
{
	return TOP_MARGIN + _fontHeight * _currline;
}

inline void TTGODisplay::internal_write(uint8_t value) {
    content[_currline][_currpos] = value;

    if (!_bufferOnly) {
    	int16_t x= xpos();
    	int16_t y= ypos();

	    String chstr=(value == 0b11011111)? String("°"):String((char)value);
       _display.drawString(chstr, x, y);
    }
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
}

void TTGODisplay::print(char * str){


    char *p=str;
    int16_t x=xpos();
    int16_t y=ypos();
    int16_t width=0;

 //   DBG_PRINTF("%d,%d, %s\n",_currline,_currpos,str);

    while(*p !='\0' && _currpos < _cols){
	    content[_currline][_currpos] = *p;
    	_currpos++;
    	p++;
    	width += _fontWidth;
    }

    if (!_bufferOnly) {

    	_display.fillRect(x,y, width ,_fontHeight, TFT_BLACK);
	    String strstr=String(str);
        _display.setTextColor(TFT_WHITE);
        _display.drawString(strstr, x, y);
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
	//Serial.print("printStatus:");
	//Serial.println(str);
	_display.setColor(WHITE);
    _display.fillRect(STATUS_BAR_LEFT,STATUS_BAR_TOP,STATUS_BAR_WIDTH,STATUS_BAR_HEIGHT);

    _display.setColor(BLACK);
    _display.drawString(STATUS_LEFT,STATUS_TOP,str);
}
#endif

#endif
