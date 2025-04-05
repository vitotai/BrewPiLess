//YWROBOT
//last updated on 21/12/2011
//Tim Starling Fix the reset bug (Thanks Tim)
//wiki doc http://www.dfrobot.com/wiki/index.php?title=I2C/TWI_LCD1602_Module_(SKU:_DFR0063)
//Support Forum: http://www.dfrobot.com/forum/
//Compatible with the Arduino IDE 1.0
//Library version:1.1


#include "IicLcd.h"

#include "Brewpi.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <inttypes.h>
#include "Arduino.h"

#if defined(ESP8266) || defined(ESP32)
#include <Wire.h>
#else
extern "C" {
  #include "Twi.h"
}
#endif

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set:
//    DL = 1; 8-bit interface data
//    N = 0; 1-line display
//    F = 0; 5x8 dot character font
// 3. Display on/off control:
//    D = 0; Display off
//    C = 0; Cursor off
//    B = 0; Blinking off
// 4. Entry mode set:
//    I/D = 1; Increment by 1
//    S = 0; No shift
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).


IIClcd::IIClcd(uint8_t lcd_cols,uint8_t lcd_rows)
{
  _cols = lcd_cols;
  _rows = lcd_rows;
  _backlightval = LCD_NOBACKLIGHT;
}

void IIClcd::init(uint8_t lcd_Addr){
	_Addr=lcd_Addr;
	init_priv();
    _backlightTime = 0;
}

void IIClcd::init_priv()
{
	#if defined(ESP8266) || defined(ESP32)

	Wire.begin(PIN_SDA,PIN_SCL);
#else
	twi_init();
#endif
	_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
	begin(_cols, _rows);
}

void IIClcd::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
	if (lines > 1) {
		_displayfunction |= LCD_2LINE;
	}
	_numlines = lines;
    _currline = 0;
    _currpos = 0;

	// for some 1 line displays you can select a 10 pixel high font
	if ((dotsize != 0) && (lines == 1)) {
		_displayfunction |= LCD_5x10DOTS;
	}

	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40ms after power rises above 2.7V
	// before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
	delay(50);

	// Now we pull both RS and R/W low to begin commands
	expanderWrite(_backlightval);	// reset expanderand turn backlight off (Bit 8 =1)
	delay(1000);

  	//put the LCD into 4 bit mode
	// this is according to the hitachi HD44780 datasheet
	// figure 24, pg 46

	  // we start in 8bit mode, try to set 4 bit mode
   write4bits(0x03 << 4);
   delayMicroseconds(4500); // wait min 4.1ms

   // second try
   write4bits(0x03 << 4);
   delayMicroseconds(4500); // wait min 4.1ms

   // third go!
   write4bits(0x03 << 4);
   delayMicroseconds(150);

   // finally, set to 4-bit interface
   write4bits(0x02 << 4);


	// set # lines, font size, etc.
	command(LCD_FUNCTIONSET | _displayfunction);

	// turn the display on with no cursor or blinking default
	_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	display();

	// clear it off
	command(LCD_CLEARDISPLAY); //clear();
	delayMicroseconds(2000);

	// Initialize to default text direction (for roman languages)
	_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

	// set the entry mode
	command(LCD_ENTRYMODESET | _displaymode);

	home();

}
#if EMIWorkaround
  void IIClcd::refresh(){
	begin(20,4);
    for(int i=0;i< 4;i++){
        setCursor(0,i);
		content[i][20]='\0';
        print(content[i]);
    }
  }
#endif
/********** high level commands, for the user! */
void IIClcd::clear(){
	command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero

    for(uint8_t i = 0; i < _rows; i++){
        for(uint8_t j = 0; j < _cols; j++){
            content[i][j]=' '; // initialize on all spaces
        }
        content[i][_cols]='\0'; // NULL terminate string
    }

	delayMicroseconds(2000);  // this command takes a long time!
}

void IIClcd::home(){
	command(LCD_RETURNHOME);  // set cursor position to zero
	delayMicroseconds(2000);  // this command takes a long time!
}

void IIClcd::setCursor(uint8_t col, uint8_t row){
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if ( row > _numlines ) {
		row = _numlines-1;    // we count rows starting w/0
	}

    _currline = row;
    _currpos = col;
	command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void IIClcd::noDisplay() {
	_displaycontrol &= ~LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void IIClcd::display() {
	_displaycontrol |= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void IIClcd::noCursor() {
	_displaycontrol &= ~LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void IIClcd::cursor() {
	_displaycontrol |= LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void IIClcd::noBlink() {
	_displaycontrol &= ~LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void IIClcd::blink() {
	_displaycontrol |= LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void IIClcd::scrollDisplayLeft(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void IIClcd::scrollDisplayRight(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void IIClcd::leftToRight(void) {
	_displaymode |= LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void IIClcd::rightToLeft(void) {
	_displaymode &= ~LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void IIClcd::autoscroll(void) {
	_displaymode |= LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void IIClcd::noAutoscroll(void) {
	_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void IIClcd::createChar(uint8_t location, uint8_t charmap[]) {
	location &= 0x7; // we only have 8 locations 0-7
	command(LCD_SETCGRAMADDR | (location << 3));
	for (int i=0; i<8; i++) {
		//write(charmap[i]);
		send(charmap[i], Rs);
	}
}

// Turn the (optional) backlight off/on
void IIClcd::noBacklight(void) {
	_backlightval=LCD_NOBACKLIGHT;
	expanderWrite(0);
}

void IIClcd::backlight(void) {
	_backlightval=LCD_BACKLIGHT;
	expanderWrite(0);
}



/*********** mid level commands, for sending data/cmds */

inline void IIClcd::command(uint8_t value) {
	send(value, 0);
}

inline size_t IIClcd::write(uint8_t value) {
    content[_currline][_currpos] = value;
    _currpos++;
    if (!_bufferOnly) {
        send(value, Rs);
    }
    return 1;
}

/************ low level data pushing commands **********/

// write either command or data
void IIClcd::send(uint8_t value, uint8_t mode) {
	uint8_t highnib=value&0xf0;
	uint8_t lownib=(value<<4)&0xf0;
       write4bits((highnib)|mode);
	write4bits((lownib)|mode);
}

void IIClcd::write4bits(uint8_t value) {
	expanderWrite(value);
	pulseEnable(value);
}

void IIClcd::expanderWrite(uint8_t _data) {
#if defined(ESP8266) || defined(ESP32)
#ifdef RotaryViaPCF8574
	noInterrupts();
#endif
	Wire.beginTransmission(_Addr);
	Wire.write((int)(_data) | _backlightval);
	Wire.endTransmission();
#ifdef RotaryViaPCF8574
	interrupts();
#endif
#else
    uint8_t data = ((uint8_t)(_data) | _backlightval);
    twi_writeTo(_Addr, &data, 1, true, true);
#endif
}

void IIClcd::pulseEnable(uint8_t _data){
	expanderWrite(_data | En);	// En high
	delayMicroseconds(1); //delayMicroseconds(1);		// enable pulse must be >450ns

	expanderWrite(_data & ~En);	// En low
	delayMicroseconds(50); //delayMicroseconds(50);		// commands need > 37us to settle
}

// This resets the backlight timer and updates the SPI output
void IIClcd::resetBacklightTimer(void) {
    _backlightTime = ticks.seconds();
}

void IIClcd::updateBacklight(void) {
    // True = OFF, False = ON
    bool backLightOutput = (backlightAutoOffPeriod !=0) && (BREWPI_SIMULATE || ticks.timeSince(_backlightTime) > backlightAutoOffPeriod);
    if(backLightOutput) {
        noBacklight();
    } else {
        backlight();
    }
}

// Puts the content of one LCD line into the provided buffer.
void IIClcd::getLine(uint8_t lineNumber, char * buffer){
    const char* src = content[lineNumber];
    for(uint8_t i = 0; i < _cols;i++){
        char c = src[i];
        buffer[i] = (c == 0b11011111) ? 0xB0 : c;
    }
    buffer[_cols] = '\0'; // NULL terminate string
}

void IIClcd::printSpacesToRestOfLine(void){
    while(_currpos < _cols){
        print(' ');
    }
}

#ifndef print_P_inline
void IIClcd::print_P(const char * str){ // print a string stored in PROGMEM
    char buf[21]; // create buffer in RAM
    strcpy_P(buf, str); // copy string to RAM
    print(buf); // print from RAM
}
#endif
