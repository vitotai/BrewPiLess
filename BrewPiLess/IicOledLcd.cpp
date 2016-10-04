#include "IicOledLcd.h"

#include "Brewpi.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <inttypes.h>
#include "Arduino.h"
#include "font_cousine_10.h"

IICOledLcd::IICOledLcd(uint8_t lcd_Addr,uint8_t sda,uint8_t scl)
:_display(lcd_Addr,sda,scl)
{
}

void IICOledLcd::init(){
    _display.init();
    delay(500);
    _backlightTime = 0;
    _fontHeight=12;
    _fontWidth=6;
}
void IICOledLcd::begin(uint8_t cols, uint8_t lines){
  	_cols = cols;
  	_rows = lines;
    _currline = 0;
    _currpos = 0;
	
    _display.flipScreenVertically();
    _display.clear();
    _display.display();

    _display.setFont(Cousine_10);
    _display.setTextAlignment(TEXT_ALIGN_LEFT);
    _display.setContrast(255);	
}

/********** high level commands, for the user! */
void IICOledLcd::clear(){
	_display.clear();

    for(uint8_t i = 0; i < _rows; i++){
        for(uint8_t j = 0; j < _cols; j++){
            content[i][j]=' '; // initialize on all spaces
        }
        content[i][_cols]='\0'; // NULL terminate string
    }

	delayMicroseconds(2000);  // this command takes a long time!
}

void IICOledLcd::home(){
}

void IICOledLcd::setCursor(uint8_t col, uint8_t row){
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if ( row > _rows ) {
		row = _rows-1;    // we count rows starting w/0
	}

    _currline = row;
    _currpos = col;
}

// Turn the display on/off (quickly)
void IICOledLcd::noDisplay() {
	_display.displayOff();
}
void IICOledLcd::display() {
	_display.displayOn();
}

// Turn the (optional) backlight off/on
void IICOledLcd::noBacklight(void) {
	_display.displayOff();
}

void IICOledLcd::backlight(void) {
	_display.displayOn();
}



/*********** mid level commands, for sending data/cmds */


inline void IICOledLcd::internal_write(uint8_t value) {


    content[_currline][_currpos] = value;
	uint8_t ch=(value == 0b11011111)? 0xB0 : value;

    if (!_bufferOnly) {
    	_display.setColor(BLACK);
    	_display.fillRect(_fontWidth * _currpos, _fontHeight * _currline,_fontWidth,_fontHeight);

    	_display.setColor(WHITE);    	
	    String chstr=String((char)ch);
       _display.drawString(_fontWidth * _currpos, _fontHeight * _currline,chstr);
    }
    _currpos++;
}

inline size_t IICOledLcd::write(uint8_t value) {
	internal_write(value);
    _display.display();
    return 0;
}

// This resets the backlight timer and updates the SPI output
void IICOledLcd::resetBacklightTimer(void) {
    _backlightTime = ticks.seconds();
}

void IICOledLcd::updateBacklight(void) {
	#if BACKLIGHT_AUTO_OFF_PERIOD == 0
	backlight();
	#else
    // True = OFF, False = ON
    bool backLightOutput = BREWPI_SIMULATE || ticks.timeSince(_backlightTime) > BACKLIGHT_AUTO_OFF_PERIOD;
    if(backLightOutput) {
        noBacklight();
    } else {
        backlight();
    }
    #endif
}

// Puts the content of one LCD line into the provided buffer.
void IICOledLcd::getLine(uint8_t lineNumber, char * buffer){
    const char* src = content[lineNumber];
    for(uint8_t i = 0; i < _cols;i++){
        char c = src[i];
        buffer[i] = (c == 0b11011111) ? 0xB0 : c;
    }
    buffer[_cols] = '\0'; // NULL terminate string
}

void IICOledLcd::printSpacesToRestOfLine(void){
    while(_currpos < _cols){
        internal_write(' ');
    }
    _display.display();
}

void IICOledLcd::print(char * str){


    char *p=str;
    int16_t x=_fontWidth * _currpos;
    int16_t y=_fontHeight *_currline;
    int16_t width=0;
    
    while(*p !='\0' && _currpos < _cols){
	    content[_currline][_currpos] = *p;
    	_currpos++;
    	p++;
    	width += _fontWidth;
    }

    if (!_bufferOnly) {

    	_display.setColor(BLACK);
    	_display.fillRect(x,y, width ,_fontHeight);
		_display.setColor(WHITE);
	    String strstr=String(str);
        _display.drawString(x, y,strstr);
        _display.display();
    }
}

#ifndef print_P_inline
void IICOledLcd::print_P(const char * str){ // print a string stored in PROGMEM
    char buf[21]; // create buffer in RAM
    strcpy_P(buf, str); // copy string to RAM
    print(buf); // print from RAM
}
#endif


