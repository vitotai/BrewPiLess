//YWROBOT
#ifndef LiquidCrystal_I2C_h
#define LiquidCrystal_I2C_h

#include "Brewpi.h"
#include "BrewpiStrings.h"
#include <inttypes.h>
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
#define LCD_FUNCTIONSET 0x20
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
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En B00000100  // Enable bit
#define Rw B00000010  // Read/Write bit
#define Rs B00000001  // Register select bit

class IIClcd : public Print {
public:
  IIClcd(uint8_t lcd_cols,uint8_t lcd_rows);
  ~IIClcd() {};

  void init(uint8_t lcd_Addr);

  void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS );

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
  // void printLeft();
  // void printRight();
  void leftToRight();
  void rightToLeft();
  // void shiftIncrement();
  // void shiftDecrement();
  void noBacklight();
  void backlight();
  void autoscroll();
  void noAutoscroll();

  void createChar(uint8_t, uint8_t[]);
  void setCursor(uint8_t, uint8_t);

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

  void command(uint8_t);

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
#ifdef EMIWorkaround
  void refresh();
#endif

private:
  void init_priv();
  void send(uint8_t, uint8_t);
  void write4bits(uint8_t);
  void expanderWrite(uint8_t);
  void pulseEnable(uint8_t);
  
  uint32_t backlightAutoOffPeriod;
  uint8_t _Addr;
  uint8_t _displayfunction;
  uint8_t _displaycontrol;
  uint8_t _displaymode;
  uint8_t _numlines;
  uint8_t _currline;
  uint8_t _currpos;
  uint8_t _cols;
  uint8_t _rows;
  uint8_t _backlightval;
  uint16_t _backlightTime;
  bool _bufferOnly;

  char content[4][21]; // always keep a copy of the display content in this variable

};

#endif
