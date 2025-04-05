#include "IicOledLcd.h"

#include "Brewpi.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <inttypes.h>
#include "Arduino.h"

#if BREWPI_OLED128x64_LCD

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

#if OLED128x64_LCD_ORIENTATION
    _display.flipScreenVertically();
#endif
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

inline int16_t IICOledLcd::xpos(void)
{
	return LEFT_MARGIN + _fontWidth * _currpos;
}

inline int16_t  IICOledLcd::ypos(void)
{
	return TOP_MARGIN + _fontHeight * _currline;
}

inline void IICOledLcd::internal_write(uint8_t value) {
    content[_currline][_currpos] = value;

    if (!_bufferOnly) {
    	int16_t x= xpos();
    	int16_t y= ypos();
    	_display.setColor(BLACK);
    	_display.fillRect(x,y,_fontWidth,_fontHeight);
    	_display.setColor(WHITE);

	    String chstr=(value == 0b11011111)? String("°"):String((char)value);
       _display.drawString(x,y,chstr);
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
    // True = OFF, False = ON
    bool backLightOutput = (backlightAutoOffPeriod !=0) && (BREWPI_SIMULATE || ticks.timeSince(_backlightTime) > backlightAutoOffPeriod);
    if(backLightOutput) {
        noBacklight();
    } else {
        backlight();
    }
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

void IICOledLcd::print(const char * str){


    const char *p=str;
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

#ifdef STATUS_LINE
void IICOledLcd::printStatus(char* str)
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
