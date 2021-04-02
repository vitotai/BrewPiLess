#pragma once

#include "Brewpi.h"
#include "DisplayBase.h"
#include "NullLcdDriver.h"
#include <inttypes.h>
#include <Print.h>
#if ESP8266
#include <ESP8266WiFi.h>
#endif
#if BREWPI_IIC_LCD
#include <Wire.h>
#include "IicLcd.h"
#endif

#if BREWPI_OLED128x64_LCD
#include "IicOledLcd.h"
#endif

#if BREWPI_OLED128x64_LCD
	typedef IICOledLcd	PhysicalLcdDriver;
#else // BREWPI_OLED128x64_LCD
    #if defined(BREWPI_IIC_LCD)
    typedef IIClcd	PhysicalLcdDriver;
    #else
        #if BREWPI_EMULATE || !BREWPI_LCD || !ARDUINO
	    typedef NullLcdDriver PhysicalLcdDriver;
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
    PhysicalLcdDriver *getLcd();
};

class SharedDisplayManager{
public:
    SharedDisplayManager();

    void add(SharedLcdDisplay* display);
    void setPrimary(SharedLcdDisplay* display);
    void loop();
    void init();
    void next();
    void previous();
    void forcePrimary(bool primary);
    void setRotateMode(bool rotate){ _isRotateMode = rotate;}
    
    void setDisplayMode(uint8_t mode);

    PhysicalLcdDriver *getLcd(){ return & _lcd;}
    static uint8_t i2cLcdAddr;

protected:
      SharedLcdDisplay* _head;
      SharedLcdDisplay* _current;      
    
      PhysicalLcdDriver _lcd;
      uint32_t _switchTime;
      bool _isForcedPrimary;
      bool _isRotateMode;

      void _switch(SharedLcdDisplay* newDisplay);
};

extern SharedDisplayManager sharedDisplayManager;

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
    void setBufferOnly(bool bufferOnly) { _bufferOnly = bufferOnly; }

    void print_P(const char * str) {
        #if ESP32
        print((char*)str);
        #else
        char buf[21]; // create buffer in RAM    
        strcpy_P(buf, str); // copy string to RAM
        print(buf); // print from RAM
        #endif
        }
 #ifdef STATUS_LINE
	void printStatus(char* str);
 #endif
#if EMIWorkaround
    void refresh();
#endif
protected:
    char content[4][21]; // always keep a copy of the display content in this variable
    bool _hiding;
    bool _bufferOnly;
    uint8_t _currline;
    uint8_t _currpos;
    uint8_t _cols;
    uint8_t _rows;

    void _clearBuffer();
};


#define GravityMask 1
#define PressureMask 2
#define HumidityMask 4

class SmartDisplay: public SharedLcdDisplay{
public:
    SmartDisplay();
    void onShow();
    void onHide();
    void redraw();

    
    void gravityDeviceData(float gravity,float temperature, uint32_t update,char tunit,bool usePlate);
    void pressureData(float pressure);
    void humidityData(bool chamberValid,uint8_t chamber,bool roomValid, uint8_t room);
    void setIp(IPAddress ip);
protected:
    bool _shown;
    uint8_t _layout;
    IPAddress _ip;

    char   _tempUnit;
    bool    _plato;
    float   _gravity;
    float   _temperature;
    uint32_t _updateTime;

    bool _chamberHumidityAvailable;
    bool _roomHumidityAvailable;

    uint8_t _chamberHumidity;
    uint8_t _roomHumidity;

    float _pressure;

    void _drawFixedPart();
    void _drawGravity();
    void _drawPressure();
    void _drawHumidity();
    void _drawIp();

    void _printFloatAt(uint8_t col,uint8_t row,uint8_t space,uint8_t precision,float value);
    void _printGravityTimeAt(uint8_t col,uint8_t row);
    void _printHumidityValueAt(uint8_t col,uint8_t row,uint8_t value);

    bool _updatePartial(uint8_t mask);
};

extern SmartDisplay smartDisplay;