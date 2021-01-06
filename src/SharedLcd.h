#include "Brewpi.h"
#include "DisplayBase.h"
#include "NullLcdDriver.h"
#include <inttypes.h>
#include <Print.h>

#if BREWPI_IIC_LCD
#include <Wire.h>
#include "IicLcd.h"
#endif

#if BREWPI_OLED128x64_LCD
#include "IicOledLcd.h"
#endif

#if BREWPI_OLED128x64_LCD
	typedef IICOledLcd	LcdDriver;
#else // BREWPI_OLED128x64_LCD
    #if defined(BREWPI_IIC_LCD)
    typedef IIClcd	LcdDriver;
    #else
        #if BREWPI_EMULATE || !BREWPI_LCD || !ARDUINO
	    typedef NullLcdDriver LcdDriver;
        #endif
    #endif
#endif //BREWPI_OLED128x64_LCD


class SharedDisplayManager;
// debate:
//  let SharedLcdDisplay determine if realy drainw should be done
//  or let the manager do.
// if the manager manages, then every call to LCD should be passed to
// the manager, and the manager need to check "calling" party against
// current active one.

class SharedLcdDisplay{
public:
    friend class SharedDisplayManager;

    SharedLcdDisplay();
    // sharing parts
    virtual void onShow();
    virtual void onHide();
    virtual void redraw();

    // linked list 
protected:
    void setManager(SharedDisplayManager *manager){ _manager=manager; }
    SharedLcdDisplay* _next;
    SharedLcdDisplay* _previous;
    SharedDisplayManager *_manager;
    LcdDriver *getLcd();
};

class SharedDisplayManager{
public:
    SharedDisplayManager();

    void add(SharedLcdDisplay* display);
    void loop();
    void init();
    void next();
    void previous();
    void forceHead();
    void endForceHead();

    LcdDriver *getLcd(){ return & _lcd;}
    
protected:
      SharedLcdDisplay* _head;
      SharedLcdDisplay* _current;      
    
      LcdDriver _lcd;
      uint32_t _switchTime;
      bool _isForcedPrimary;
      bool _isRotateMode;

      void _switch(SharedLcdDisplay* newDisplay);
};

class BrewPiLcd: public SharedLcdDisplay, public Print {
public:
    BrewPiLcd();
    void onShow();
    void onHide();
    void redraw();

    void init();
    void begin(uint8_t cols, uint8_t lines);
    void clear();
    void setCursor(uint8_t col, uint8_t row);
     // print a string stored in PROGMEM
    size_t write(uint8_t);
    void print(char* str);
    void printSpacesToRestOfLine(void);
    void getLine(uint8_t lineNumber, char * buffer);
    void resetBacklightTimer(void);
    void updateBacklight(void);
    void setAutoOffPeriod(uint32_t period);

    void print_P(const char * str) {
        #if ESP32
        print((char*)str);
        #else
        char buf[21]; // create buffer in RAM    
        strcpy_P(buf, str); // copy string to RAM
        print(buf); // print from RAM
        #endif
        }
protected:
    char content[4][21]; // always keep a copy of the display content in this variable
    bool _hiding;
    uint8_t _currline;
    uint8_t _currpos;
    uint8_t _cols;
    uint8_t _rows;

    void _clearBuffer();
};