#ifndef IicOledLcd_h
#define IicOledLcd_h

#include "Brewpi.h"
#include "BrewpiStrings.h"
#include <inttypes.h>
#include <Print.h>
#include "Ticks.h"
#include <Wire.h>

#if BREWPI_OLED128x64_LCD

#if BREWPI_OLED_SH1106   // New 
#include "SH1106.h"
#else
#include "SSD1306.h"
#endif
class IICOledLcd : public Print {
public:
  IICOledLcd(uint8_t lcd_Addr,uint8_t sda,uint8_t scl);
  ~IICOledLcd() {};
  void	begin(uint8_t cols, uint8_t lines);
  void init();

  void clear();
  void home();

  void noDisplay();
  void display();
//  void noBlink();
//  void blink();
//  void noCursor();
//  void cursor();
//  void scrollDisplayLeft();
//  void scrollDisplayRight();
  // void printLeft();
  // void printRight();
//  void leftToRight();
//  void rightToLeft();
  // void shiftIncrement();
  // void shiftDecrement();
  void noBacklight();
  void backlight();
//  void autoscroll();
//  void noAutoscroll();

//  void createChar(uint8_t, uint8_t[]);
  void setCursor(uint8_t, uint8_t);
  void print(char* str);

#ifdef STATUS_LINE
	void printStatus(char* str);
#endif

  virtual size_t write(uint8_t);

#define print_P_inline 1
#ifdef print_P_inline
  // print a string stored in PROGMEM
  void print_P(const char * str) {
    char buf[21]; // create buffer in RAM
    strcpy_P(buf, str); // copy string to RAM
    print(buf); // print from RAM
  }
#else
  void print_P(const char * str);
#endif

  void getLine(uint8_t lineNumber, char * buffer);

  //void readContent(void); // read the content from the display to the shadow copy buffer
  char readChar(void);

//  void command(uint8_t);

  void setBufferOnly(bool bufferOnly) { _bufferOnly = bufferOnly; }

  void resetBacklightTimer(void);

  void updateBacklight(void);

  uint8_t getCurrPos(void) {
    return _currpos;
  }

  uint8_t getCurrLine(void) {
    return _currline;
  }

  void printSpacesToRestOfLine(void);

  using Print::write;
  void setAutoOffPeriod(uint32 period){ backlightAutoOffPeriod = period; }

private:
  uint32_t backlightAutoOffPeriod;
#if BREWPI_OLED_SH1106
SH1106  _display;
#else
SSD1306  _display;
#endif
  uint8_t _Addr;
  uint8_t _currline;
  uint8_t _currpos;
  uint8_t _cols;
  uint8_t _rows;

  uint8_t _fontHeight;
  uint8_t _fontWidth;

  uint8_t _backlightval;
  uint16_t _backlightTime;
  bool _bufferOnly;

  char content[4][21]; // always keep a copy of the display content in this variable

  void internal_write(uint8_t);

  inline int16_t xpos(void);
  inline int16_t ypos(void);
};
#endif
#endif
