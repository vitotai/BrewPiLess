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

#if OLED_LCD
#define CustomGlyph false
#else
#define CustomGlyph true // triggering exception, have to re-think about it.
#endif

#define ShareModeRotate 0
#define ShareModeBrewPi 1
#define ShareModeAdditional 2

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



class SharedLcdDisplay;

class SharedDisplayManager{
public:
    SharedDisplayManager();

    void add(SharedLcdDisplay* display,bool isPrimary=false);
    void setPrimary(SharedLcdDisplay* display);
    void loop();
    void init();
    void next();
    void previous();
    void forcePrimary(bool primary);
    
    void setDisplayMode(uint8_t mode);

    PhysicalLcdDriver *getLcd(){ return & _lcd;}
    static uint8_t i2cLcdAddr;

    #if DebugSharedDisplay
    void debug(String& info);
    #endif

#if EMIWorkaround
    void refresh();
#endif

protected:
      SharedLcdDisplay* _head;
      SharedLcdDisplay* _current;      
    
      PhysicalLcdDriver _lcd;
      uint32_t _switchTime;
      bool _isForcedPrimary;
      bool _isChangingMode;
      uint8_t _mode;

      void _switch(SharedLcdDisplay* newDisplay);
    #if CustomGlyph
    void _createCustomChar(char ch, const uint8_t bmp[8]);
    void _createAllCustomChars();
    #endif
};


// debate:
//  let SharedLcdDisplay determine if realy drainw should be done
//  or let the manager do.
// if the manager manages, then every call to LCD should be passed to
// the manager, and the manager need to check "calling" party against
// current active one.

class SharedLcdDisplay{
public:
    friend class SharedDisplayManager;

    SharedLcdDisplay(){
        _next = _previous = NULL;
        _hidden = true;
    }
    // sharing parts
    virtual void onShow();
    virtual void onHide();
    virtual void redraw();
    virtual void loop();
    // linked list 
protected:
    void setHidden(bool hidden){ _hidden=hidden;}
    void setManager(SharedDisplayManager *manager){ _manager=manager; }
    SharedLcdDisplay* _next;
    SharedLcdDisplay* _previous;
    SharedDisplayManager *_manager;
    PhysicalLcdDriver *getLcd(){return _manager->getLcd();}
    bool _hidden;
};


extern SharedDisplayManager sharedDisplayManager;

class BrewPiLcd: public SharedLcdDisplay, public Print {
public:
    BrewPiLcd();
    void onShow(){}
    void onHide(){}
    void redraw();
    void loop();

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
    void _printTime(time_t now);
    #endif
    
    #if EMIWorkaround
    void refresh();
    #endif
protected:
    char content[4][21]; // always keep a copy of the display content in this variable
    bool _bufferOnly;
    uint8_t _currline;
    uint8_t _currpos;
    uint8_t _cols;
    uint8_t _rows;

#if STATUS_LINE
	time_t _displayTime;
#endif

    void _clearBuffer();
};


#define GravityMask 1
#define PressureMask 2
#define HumidityMask 4

class SmartDisplay: public SharedLcdDisplay{
public:
    SmartDisplay();
    void onShow(){}
    void onHide(){}
    void redraw();
    void loop();
    
    void gravityDeviceData(uint8_t type,float gravity,float temperature, uint32_t update,char tunit,bool usePlate,float battery,float tilt,int8_t rssi);
    void pressureData(float pressure);
    void humidityData(bool chamberValid,uint8_t chamber,bool roomValid, uint8_t room);
    void setIp(IPAddress ip);
protected:
    char _layout;
    IPAddress _ip;

    char   _tempUnit;
    bool    _plato;
    float   _gravity;
    float   _temperature;
    float _battery;
    uint8_t _batteryUnit;

    float _tilt;
    uint32_t _lastSeen;
    int8_t _rssi;

    bool _chamberHumidityAvailable;
    bool _roomHumidityAvailable;

    uint8_t _chamberHumidity;
    uint8_t _roomHumidity;

    float _pressure;
    bool _gravityInfoValid;
    uint32_t _gravityInfoLastPrinted;

    void _drawFixedPart();
    void _drawGravity();
    void _drawPressure();
    void _drawHumidity();
    void _drawIp();

    void _printFloatAt(uint8_t col,uint8_t row,uint8_t space,uint8_t precision,float value);
    void _printIntegerAt(uint8_t col,uint8_t row,uint8_t space,int value);
    
    void _printGravityTimeAt(uint8_t col,uint8_t row);
    void _printHumidityValueAt(uint8_t col,uint8_t row,uint8_t value);
    bool _updatePartial(uint8_t mask);
    #if CustomGlyph
    void _createCustomChar(PhysicalLcdDriver *lcd,char ch, const uint8_t bmp[8]);
    void _createAllCustomChars(PhysicalLcdDriver *lcd);
    #endif
    void _drawSignalAt(uint8_t col,uint8_t row,int8_t rssi);
    int _lastSeenString(char *buffer);
#if ISPINDEL_DISPLAY
    OledDisplay *_display;

    void _showGravityFixedParts();
    void _showGravityValues();

    void _showGravity();
    void _showTemperature();
    void _showBattery();
    void _showTilt();
    void _showLastSeen();
    void _showIp();
    void _showFloatAt(int16_t x, int16_t y, float value, uint8_t space, uint8_t precision, uint16_t fontWidth,uint16_t fontHeight);
    void _showIntegerAt(int16_t x, int16_t y, int value, uint8_t space, uint16_t fontWidth,uint16_t fontHeight);

    void _showSignalAt(int16_t x, int16_t y,int8_t strength);

#endif

};

extern SmartDisplay smartDisplay;

extern void makeTime(time_t timeInput, struct tm &tm);




#if CustomGlyph
#define CharSignal_1 1
#define CharSignal_2 2
#define CharSignal_3 3
#define CharSignal_4 4
#define CharBattery 5
#define CharTilt 6
#else
#define CharSignal_1 '1'
#define CharSignal_2 '2'
#define CharSignal_3 '3'
#define CharSignal_4 '4'
#define CharBattery 'B'
#define CharTilt 'A'

#endif
