#include "Brewpi.h"
#include "BrewpiStrings.h"
#include "Display.h"
#include "DisplayLcd.h"
#include "mystrlib.h"
#include "TimeKeeper.h"
#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include "SharedLcd.h"

#define SWITCH_TIME 8000


SharedDisplayManager sharedDisplayManager;

uint8_t SharedDisplayManager::i2cLcdAddr = IIC_LCD_ADDRESS;


#if CustomGlyph

static const uint8_t BMP_WifiSignal1[8]  PROGMEM  = {B00000, B00000, B00000, B00000, B00000, B00000, B01000,B00000};
static const uint8_t BMP_WifiSignal2[8]  PROGMEM  = {B00000, B00000, B00000, B00000, B01100, B00000, B01000,B00000};
static const uint8_t BMP_WifiSignal3[8]  PROGMEM  = {B00000, B00000, B01110, B00000, B01100, B00000, B01000,B00000};
static const uint8_t BMP_WifiSignal4[8]  PROGMEM  = {B01111, B00000, B01110, B00000, B01100, B00000, B01000,B00000};

static const uint8_t BMP_Battery[8]  PROGMEM  = {B00000, B01100, B01100, B11110, B11110, B11110, B11110,B00000};
static const uint8_t BMP_Tilt[8]  PROGMEM  = {B00000, B00000, B00001, B00010, B00100, B01000, B11111, B00000};

void SharedDisplayManager::_createCustomChar(char ch, const uint8_t bmp[8]){
    uint8_t buf[8];
    memcpy_P(buf,bmp,8);
    _lcd.createChar(ch,buf); 
}
void SharedDisplayManager::_createAllCustomChars(){
//    DBG_PRINTF("_createAllCustomChars\n");

    _createCustomChar(CharSignal_1,BMP_WifiSignal1);
    _createCustomChar(CharSignal_2,BMP_WifiSignal2);
    _createCustomChar(CharSignal_3,BMP_WifiSignal3);
    _createCustomChar(CharSignal_4,BMP_WifiSignal4);
    _createCustomChar(CharBattery,BMP_Battery);
    _createCustomChar(CharTilt,BMP_Tilt);
}
#endif

//*****************************************************************
//SharedDisplayManager
SharedDisplayManager::SharedDisplayManager():
#if BREWPI_IIC_LCD
_lcd(20,4)
#else
#if BREWPI_OLED128x64_LCD
_lcd(OLED128x64_LCD_ADDRESS,PIN_SDA,PIN_SCL)
#else
_lcd()
#endif
#endif
{
    _head=_current=NULL;
    _isForcedPrimary = false;
    _isChangingMode =false;
}
void SharedDisplayManager::add(SharedLcdDisplay* display,bool isPrimary){
    display->setManager(this);

    if(_head == NULL){
        _head  = display;
        display->_next = display;
        display->_previous = display;
        display->setHidden(false);
        display->onShow();
        _current = _head;
    }else{
        // find tail
        SharedLcdDisplay *tail= _head;
        while(tail->_next != _head) tail= tail->_next;

        tail->_next = display;
        display->_previous = tail;
        display->_next = _head;

        if(isPrimary){
            _head= display;
            if(_current){
                _current->setHidden(true);
                _current->onHide();
            }
            _current = _head;
            _head->setHidden(false);
            _head->onShow();
            _current = _head;
        }
    }
}

void SharedDisplayManager::init(){
    // initialize LCD
#if BREWPI_IIC_LCD
	_lcd.init(i2cLcdAddr); // initialize LCD
#else
	_lcd.init();
#endif	
	_lcd.begin(20, 4);
	_lcd.clear();
#if BREWPI_IIC_LCD    
    _lcd.noCursor();
#endif
    #if CustomGlyph
    _createAllCustomChars();
    #endif
}
void SharedDisplayManager::setPrimary(SharedLcdDisplay* display){
    _head= display;
    if(_current){
        _current->setHidden(true);
        _current->onHide();
    }
    _current = _head;
    _current->setHidden(false);
    _current->onShow();
}
void SharedDisplayManager::setDisplayMode(uint8_t mode){
    _mode = mode;
    if(mode != ShareModeRotate) _isChangingMode = true;
}


void SharedDisplayManager::next(){
    if(_current != NULL){
        //DBG_PRINTF("switch to %u\n",(unsigned long) _current->_next);
        _switch(_current->_next);
    }
}

void SharedDisplayManager::previous(){
    if(_current != NULL){
        _switch(_current->_previous);
    }
}

void SharedDisplayManager::SharedDisplayManager::forcePrimary(bool primary){
    // forced to start to switch to _head
    if(primary){
        _switch(_head);
        _isForcedPrimary=true;
    }else{
        _isForcedPrimary=false;
        _switchTime= millis();
    }
}

void SharedDisplayManager::_switch(SharedLcdDisplay* newDisplay){
    if(_current != newDisplay){
//        DBG_PRINTF("Switching display\n");
        if(_current){
            _current->setHidden(true);
            _current->onHide();
        }

        _lcd.clear();

        _current = newDisplay;
        _current->setHidden(false);
        _current->onShow();
        _current->redraw();
        _switchTime= millis();
    }
}

void SharedDisplayManager::loop(){
    if(_isChangingMode){
        _isChangingMode = false;
        uint8_t count= _mode -1;
        SharedLcdDisplay* display= _head;
        while( count-- > 0) display = display->_next;
        _switch(display);
        DBG_PRINTF("*LCD changes to %d\n", _mode);
        return;
    }
    _current->loop();
    if(!_isForcedPrimary && _mode == ShareModeRotate ){
        if(millis() -_switchTime > SWITCH_TIME) next();
    }
}

#if DebugSharedDisplay
void SharedDisplayManager::debug(String& info){
    info = String("Pm:") + String(_isForcedPrimary) 
    + String(", mode:") +String(_mode)
    + String(", ST:") + String(_switchTime)
    + String(", H:") + String(_head->_hidden)
    + String(", S:") + String(_head->_next->_hidden)
    + String(", c:") + String((unsigned long)_current)
    + String(", c->next:") + String( (unsigned long)( _current? _current->_next:0) );
}
#endif
#if EMIWorkaround
void SharedDisplayManager::refresh(){
//    delay(500);
    _lcd.refresh();
//    delay(500);
//    _current->redraw();
    #if CustomGlyph
    _createAllCustomChars();
    #endif
}
#endif

//*****************************************************************
//BrewPiLcd
//*****************************************************************


BrewPiLcd::BrewPiLcd():SharedLcdDisplay(){
    _bufferOnly = false;
}

void BrewPiLcd::redraw(){
    //redraw all
//    DBG_PRINTF("redraw LCD:\n");
    PhysicalLcdDriver *lcd = getLcd();
    for(int i=0;i< 4;i++){
        lcd->setCursor(0,i);
        lcd->print(content[i]);
//        DBG_PRINTF(content[i]);
//        DBG_PRINTF("\n");
    }
    #if STATUS_LINE
    _printTime(TimeKeeper.getLocalTimeSeconds());
    #endif
    // recover cursor location
    lcd->setCursor(_currpos,_currline);
}

void BrewPiLcd::init(){

}

void BrewPiLcd::begin(uint8_t cols, uint8_t lines){
    _cols = cols;
    _rows=lines;
    _clearBuffer();
}

void BrewPiLcd::_clearBuffer(){
    for(uint8_t i = 0; i < _rows; i++){
        for(uint8_t j = 0; j < _cols; j++){
            content[i][j]=' '; // initialize on all spaces
        }
        content[i][_cols]='\0'; // NULL terminate string
    }

}

void BrewPiLcd::clear(){
    _clearBuffer();
    if(!_hidden) getLcd()->clear();
}

void BrewPiLcd::setCursor(uint8_t col, uint8_t row){
	if ( row > _rows ) {
		row = _rows-1;    // we count rows starting w/0
	}
    _currline = row;
    _currpos = col;
    if(! _hidden) getLcd()->setCursor(col,row);
}

size_t BrewPiLcd::write(uint8_t value){
    content[_currline][_currpos] = value;
    _currpos++;
    if(!_bufferOnly && !_hidden) getLcd()->write(value);
    return 1;
}

void BrewPiLcd::printSpacesToRestOfLine(void){
    while(_currpos < _cols){
        write(' ');
    }
}

void BrewPiLcd::getLine(uint8_t lineNumber, char * buffer){
    const char* src = content[lineNumber];
    for(uint8_t i = 0; i < _cols;i++){
        char c = src[i];
        buffer[i] = (c == 0b11011111) ? 0xB0 : c;
    }
    buffer[_cols] = '\0'; // NULL terminate string
}

// pass through. 
void BrewPiLcd::resetBacklightTimer(void){
    getLcd()->resetBacklightTimer();
}

void BrewPiLcd::updateBacklight(void){
    getLcd()->updateBacklight();
}

void BrewPiLcd::setAutoOffPeriod(uint32_t period){
    getLcd()->setAutoOffPeriod(period);
}

void BrewPiLcd::print(char * str){
    char* ptr=str;

    while(*ptr){
        write(*ptr);
        ptr++;
    }
}
#if EMIWorkaround
void BrewPiLcd::refresh(){
    // getLcd()->refresh();
    //redraw();
    _manager->refresh();
}
#endif

#ifdef STATUS_LINE
void BrewPiLcd::printStatus(char* str){
    if(! _hidden) getLcd()->printStatus(str);
}
void BrewPiLcd::_printTime(time_t now){
		struct tm t;
		if(_displayTime == now) return;
		_displayTime = now;
		makeTime(TimeKeeper.getLocalTimeSeconds(),t);
		char buf[21];
		sprintf(buf,"%d/%02d/%02d %02d:%02d:%02d",t.tm_year,t.tm_mon,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
		printStatus(buf);
}
#endif

void BrewPiLcd::loop(){
    #ifdef STATUS_LINE
    time_t now = TimeKeeper.getTimeSeconds();
    if(now != _displayTime ) _printTime(now);
    #endif
}