#include "Brewpi.h"
#include "BrewpiStrings.h"
#include "Display.h"
#include "DisplayLcd.h"

#include "SharedLcd.h"

#define SWITCH_TIME 6000

//*****************************************************************
// SharedLcdDisplay
 SharedLcdDisplay::SharedLcdDisplay(){
    _next = _previous = NULL;
}
inline  LcdDriver * SharedLcdDisplay::getLcd(){return _manager->getLcd(); }

//*****************************************************************
//SharedDisplayManager
SharedDisplayManager::SharedDisplayManager():
#if BREWPI_IIC_LCD
_lcd(20,4)
#else
_lcd()
#endif
{
    _head=_current=NULL;
    _isForcedPrimary = false;
}
void SharedDisplayManager::add(SharedLcdDisplay* display){
    display->setManager(this);
    if(_head == NULL){
        _head  = display;
        display->_next = _head;
        display->_previous = _head;
        return;
    }
    // find tail
    SharedLcdDisplay *tail= _head;

    while(tail->_next != _head) tail= tail->_next;

    tail->_next = display;
    display->_previous = tail;
    display->_next = _head;
}

void SharedDisplayManager::init(){
    if(! _head) return;
    _current = _head;
    // initialize LCD
#if BREWPI_IIC_LCD
	_lcd.init(LcdDisplay::i2cLcdAddr); // initialize LCD
#else
	lcd.init();
#endif	
	_lcd.begin(20, 4);
	_lcd.clear();

}

void SharedDisplayManager::next(){
    if(_current != NULL){
        _switch(_current->_next);
    }
}

void SharedDisplayManager::previous(){
    if(_current != NULL){
        _switch(_current->_previous);
    }
}

void SharedDisplayManager::SharedDisplayManager::forceHead(){
    // forced to start to switch to _head
    _switch(_head);
    _isForcedPrimary=true;
}
void SharedDisplayManager::SharedDisplayManager::endForceHead(){
    _isForcedPrimary=false;
    _switchTime= millis();
}

void SharedDisplayManager::_switch(SharedLcdDisplay* newDisplay){
    if(_current != newDisplay){
        if(_current) _current->onHide();

        _lcd.clear();

        _current = newDisplay;
        _current->onShow();
        _current->redraw();
        _switchTime= millis();
    }
}

void SharedDisplayManager::loop(){
    if(_isForcedPrimary || !_isRotateMode ) return;

    if(millis() -_switchTime > SWITCH_TIME) next();
}

//*****************************************************************
//BrewPiLcd


BrewPiLcd::BrewPiLcd():SharedLcdDisplay(){
    _hiding=true;
}

void BrewPiLcd::onShow(){
    _hiding = false;
}

void BrewPiLcd::onHide(){
    _hiding = true;
}
void BrewPiLcd::redraw(){
    //redraw all
    LcdDriver *lcd = getLcd();
    for(int i=0;i< 4;i++){
        lcd->setCursor(0,i);
        lcd->print(content[i]);
    }
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
    if(!_hiding) getLcd()->clear();
}

void BrewPiLcd::setCursor(uint8_t col, uint8_t row){
	if ( row > _rows ) {
		row = _rows-1;    // we count rows starting w/0
	}
    _currline = row;
    _currpos = col;
    if(! _hiding) getLcd()->setCursor(col,row);
}

size_t BrewPiLcd::write(uint8_t value){
    content[_currline][_currpos] = value;
    _currpos++;
    if(!_hiding) getLcd()->write(value);
    return 1;
}
//    void print(char* str);

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
