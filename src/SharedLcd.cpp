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
#define DegreeSymbolChar 0b11011111

SharedDisplayManager sharedDisplayManager;

uint8_t SharedDisplayManager::i2cLcdAddr = IIC_LCD_ADDRESS;
//*****************************************************************
// SharedLcdDisplay
 SharedLcdDisplay::SharedLcdDisplay(){
    _next = _previous = NULL;
}
inline  PhysicalLcdDriver * SharedLcdDisplay::getLcd(){return _manager->getLcd(); }

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
    _isRotateMode =false;
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
}
void SharedDisplayManager::setPrimary(SharedLcdDisplay* display){
    _head= display;
    _current = _head;
    _current->onShow();

}

void SharedDisplayManager::setDisplayMode(uint8_t mode){
    if(mode == 0 ){
        _isRotateMode = true;
    }else{
        _isRotateMode = false;
        uint8_t count= mode -1;
        SharedLcdDisplay* display= _head;
        while( count-- > 0) display = display->_next;
        _switch(display);
    }
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
    _bufferOnly = false;
}

void BrewPiLcd::onShow(){
    _hiding = false;
}

void BrewPiLcd::onHide(){
    _hiding = true;
}
void BrewPiLcd::redraw(){
    //redraw all
    PhysicalLcdDriver *lcd = getLcd();
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
    if(!_bufferOnly && !_hiding) getLcd()->write(value);
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

#ifdef STATUS_LINE
void BrewPiLcd::printStatus(char* str){
    getLcd()->printStatus(str);
}
#endif

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
    getLcd()->refresh();
}
#endif
//*****************************************************************
//SmartDisplay



SmartDisplay smartDisplay;

SmartDisplay::SmartDisplay(){
    _shown = false;
}



void SmartDisplay::onShow(){
    _shown = true;
}

void SmartDisplay::onHide(){
    _shown = false;
}

void SmartDisplay::redraw(){
    //redraw all
    _drawFixedPart();
    if(_layout & GravityMask) _drawGravity();
    if(_layout & PressureMask) _drawPressure();
    if(_layout & HumidityMask) _drawHumidity();
    _drawIp();
}

/*
Layout, depends on presence of  (G)ravity, (P)ressure, and (H)umidity
layout 0: none

layout 1, gravity only  Gravity: L0,1,2
01234567890123456789
Gravity        1.045
Temperature  012.5°C
Update           99m

layout 2: P
01234567890123456789

Pressure     13.5psi


Layout 3:G,P   Gravity: L0, L1, Pressure: L2 
01234567890123456789
G 1.012      012.5°C 
  updated    10m ago
Pressure    13.5 psi              

layout 4: Humidity
01234567890123456789
Humidity Chamber 12%      
            Room 99%  

layout 5: gravity & Humidity   Gravity: L0, L1, humidity L2
G 1.012      012.5°C 
  updated    10m ago
RH C 56%      R  75%  

layout 6: P & H
01234567890123456789   Humidity L0, L1, pressure: L2
Humidity Chamber 12%      
            Room 99%  
Pressure     13.5psi


Layout 7: G,P,H；  Gravity: L0,  hmiity L1, pressrure, L2
01234567890123456789
G 1.012 012.5°C 010m
RH C 56%      R  75%  => 
Humidity Chamber 56% => 
Humidity Room    99%
Pressure    13.5 psi
*/
void SmartDisplay::_drawFixedPart(){
    PhysicalLcdDriver *lcd=getLcd();
    // pressure is fixed
    int pressureLine = -1;
    int singleLinedHumidity = -1;

    switch(_layout){
/*
01234567890123456789
Gravity        1.045
Temp.        012.5°C
Update       99m ago
*/
        case 1:
            lcd->setCursor(0,0);
            lcd->print("Gravity");
            if(_plato){
                lcd->setCursor(18,0);
                lcd->write(DegreeSymbolChar);
                lcd->write('P');
            }
            lcd->setCursor(0,1);
            lcd->print("Temperature");
            lcd->setCursor(18,1);
            lcd->write(DegreeSymbolChar);
            lcd->write(_tempUnit);
            lcd->setCursor(0,2);
            lcd->print("Updated");
            lcd->setCursor(17,2);
            lcd->print("ago");
            break;
/*
01234567890123456789

Pressure    13.5 psi
*/

        case 2:
            pressureLine = 1;
            break;
/*
3
01234567890123456789
G 1.012      012.5°C 
  updated    10m ago
Pressure    13.5 psi              
5
G 1.012      012.5°C 
  updated    10m ago
RH C 56%      R  75%  
*/

        case 3:
        case 5:
            if(_layout == 3) pressureLine = 3;
            else singleLinedHumidity = 3;

            lcd->setCursor(0,0);
            lcd->print("G");
            lcd->setCursor(18,0);
            lcd->write(DegreeSymbolChar);
            lcd->write(_tempUnit);
            if(_plato){
                lcd->setCursor(7,0);
                lcd->write(DegreeSymbolChar);
                lcd->write('P');
            }


            lcd->setCursor(2,1);
            lcd->print("update");
            lcd->setCursor(17,1);
            lcd->print("ago");
            break;
/*
layout 4: Humidity
01234567890123456789
Humidity Chamber 12%      
            Room 99%  

layout 6:
01234567890123456789   Humidity L0, L1, pressure: L2
Humidity Chamber 12%      
            Room 99%  
Pressure     13.5psi

*/
        case 4:
        case 6:
            if(_layout == 6) pressureLine = 3;
            lcd->setCursor(0,0);
            lcd->print("Humidity Chamber");
            lcd->setCursor(19,0);
            lcd->write('%');
            lcd->setCursor(12,1);
            lcd->print("Room");
            lcd->setCursor(19,1);
            lcd->write('%');
            break;

/*
01234567890123456789
G 1.012  012.5°C 10m
RH  C 56%     R  75%  => Humidity Chamber 56% => Humidity Room 99%
Pressure    13.5 psi
*/
        case 7:
            lcd->setCursor(0,0);
            lcd->print("G");
            pressureLine = 3;
            singleLinedHumidity = 2;
            if(_plato){
                lcd->setCursor(6,0);
                lcd->write(DegreeSymbolChar);
                lcd->write('P');
            }

            break;

        default:
            break;
    }

    if(pressureLine >=0){
        lcd->setCursor(0,(uint8_t)pressureLine);
        lcd->print("Pressure");
        lcd->setCursor(17,(uint8_t)pressureLine);
        lcd->print("psi");
    }
    if(singleLinedHumidity>=0){
    //RH C 56%      R  75%  => Humidity Chamber 56% => Humidity Room 99%
        if(_chamberHumidityAvailable  && _roomHumidityAvailable){
            lcd->setCursor(0,(uint8_t)singleLinedHumidity);
            lcd->print("RH C   %      R   %");
        }else if (_chamberHumidityAvailable){
            lcd->setCursor(0,(uint8_t)singleLinedHumidity);
            lcd->print("Humidity Chamber   %");
        }else if (_roomHumidityAvailable){
            lcd->setCursor(0,(uint8_t)singleLinedHumidity);
            lcd->print("Humidity Room      %");
        }

    }
    lcd->setCursor(0,3);
    lcd->print("IP "); 
//    lcd->print(WiFi.localIP().toString().c_str());
}

void SmartDisplay::_drawGravity(){
//    PhysicalLcdDriver *lcd=getLcd();

    switch(_layout){
/*
01234567890123456789
Gravity       15.6 P
Gravity        1.045
Temperature  012.5°C
Updated      99m ago
*/
        case 1:
            if(_plato) _printFloatAt(14,0,4,1,_gravity);
            else _printFloatAt(15,0,5,3,_gravity);
            _printFloatAt(13,1,5,1,_temperature);
            _printGravityTimeAt(13,2);
            break;
/*
3
01234567890123456789
G 15.6 P
G 1.012      012.5°C 
  updated    10m ago
Pressure    13.5 psi              
5
G 1.012      012.5°C 
  updated    10m ago
RH C 56%      R  75%  
*/

        case 3:
        case 5:
            if(_plato) _printFloatAt(2,0,4,1,_gravity);
            else _printFloatAt(2,0,5,3,_gravity);
            _printFloatAt(13,0,5,1,_temperature);
            _printGravityTimeAt(13,1);
            break;

/*
01234567890123456789
G 1.012  012.5°C 10m
RH C 56%      R  75%  => Humidity Chamber 56% => Humidity Room 99%
Pressure    13.5 psi
*/
        case 7:
            if(_plato) _printFloatAt(2,0,4,1,_gravity);
            else _printFloatAt(2,0,5,3,_gravity);
            _printFloatAt(9,0,5,1,_temperature);
            _printGravityTimeAt(17,1);
            break;

        default:
            break;
    }
}

void SmartDisplay::_drawPressure(){
     // pressure is fixed
    int pressureLine = -1;
    switch(_layout){
/*
01234567890123456789

Pressure    13.5 psi
*/

        case 2:
            pressureLine = 1;
            break;
/*
3
01234567890123456789
G 1.012      012.5°C 
  updated    10m ago
Pressure    13.5 psi              
5
G 1.012      012.5°C 
  updated    10m ago
RH C 56%      R  75%  
*/

        case 3:
        case 6:
        case 7:
            pressureLine = 3;
            break;
/*
layout 4: Humidity
01234567890123456789
Humidity Chamber 12%      
            Room 99%  

layout 6:
01234567890123456789   Humidity L0, L1, pressure: L2
Humidity Chamber 12%      
            Room 99%  
Pressure     13.5psi

*/

/*
01234567890123456789
G 1.012  012.5°C 10m
RH C 56%      R  75%  => Humidity Chamber 56% => Humidity Room 99%
Pressure    13.5 psi
*/

        default:
            break;
    }

    if(pressureLine >=0){
        _printFloatAt(12,pressureLine,4,1,_pressure);
    }

}

void SmartDisplay::_drawIp(){
    PhysicalLcdDriver *lcd=getLcd(); 
    lcd->setCursor(3,3);
    lcd->print(_ip.toString().c_str());
}

void SmartDisplay::_drawHumidity(){
//    PhysicalLcdDriver *lcd=getLcd();
    int singleLinedHumidity = -1;

    switch(_layout){
/*
5
G 1.012      012.5°C 
  updated    10m ago
RH C 56%      R  75%  
*/

        case 5:
            singleLinedHumidity = 3;
            break;
/*
layout 4: Humidity
01234567890123456789
Humidity Chamber 12%      
            Room 99%  

layout 6:
01234567890123456789   Humidity L0, L1, pressure: L2
Humidity Chamber 12%      
            Room 99%  
Pressure     13.5psi

*/
        case 4:
        case 6:
            _printHumidityValueAt(17,0,_chamberHumidity);
            _printHumidityValueAt(17,1,_roomHumidity);
            break;

/*
01234567890123456789
G 1.012  012.5°C 10m
RH  C 56%     R  75%  => Humidity Chamber 56% => Humidity Room 99%
Pressure    13.5 psi
*/
        case 7:
            singleLinedHumidity = 2;
            break;

        default:
            break;
    }

    if(singleLinedHumidity>=0){
    //RH C 56%      R  75%  => Humidity Chamber 56% => Humidity Room 99%
        if(_chamberHumidityAvailable  && _roomHumidityAvailable){
             _printHumidityValueAt(5,singleLinedHumidity,_chamberHumidity);
             _printHumidityValueAt(17,singleLinedHumidity,_roomHumidity);
        }else if (_chamberHumidityAvailable){
             _printHumidityValueAt(17,singleLinedHumidity,_chamberHumidity);
        }else if (_roomHumidityAvailable){
             _printHumidityValueAt(17,singleLinedHumidity,_roomHumidity);
        }

    }

}



void SmartDisplay::_printFloatAt(uint8_t col,uint8_t row,uint8_t space,uint8_t precision,float value){
    PhysicalLcdDriver *lcd=getLcd();
    lcd->setCursor(col,row);

    char buffer[32];
    int digitNum=sprintFloat((char*)buffer,value,precision);
    buffer[digitNum]='\0';
//    DBG_PRINTF("_printFloatAt %d,%d,%s\n",space,digitNum,buffer);

    if(space > digitNum){
        uint8_t i=space - (uint8_t)digitNum;
        while( i-- > 0) lcd->write(' ');
    }else{
        digitNum = space;
    }
    for( uint8_t i=0;i< digitNum;i++)
        lcd->write(buffer[i]);
}

void SmartDisplay::_printGravityTimeAt(uint8_t col,uint8_t row){

    PhysicalLcdDriver *lcd=getLcd();
    lcd->setCursor(col,row);

    uint32_t diff =TimeKeeper.getTimeSeconds() - _updateTime;
//    DBG_PRINTF("gravity Time: diff=%d",diff);
    if(diff > 30* 86400){ // greater than 10 days
        lcd->print("???");
    }else if(diff >  99*60*60){  // greater than 99 hours, in days
        uint32_t days = diff/86400;
        lcd->write( days < 10? ' ':'0' + days/10);
        lcd->write('0' + days%10);
        lcd->write('D');
    }else if(diff >  100*60){  // greater than 100 minutes, in hours
        uint32_t hours = diff/3600;
        lcd->write( hours < 10? ' ':'0' + hours/10);
        lcd->write('0' + hours%10);
        lcd->write('H');
    }else if(diff <  60){
        // less than one minutes
        lcd->print("<1m");
    }else{
        // in minute
        uint32_t minutes = diff/60;
        lcd->write( minutes < 10? ' ':'0' + minutes/10);
        lcd->write('0' + minutes%10);
        lcd->write('m');
    }
}

void SmartDisplay::_printHumidityValueAt(uint8_t col,uint8_t row,uint8_t value){
    PhysicalLcdDriver *lcd=getLcd();
    lcd->setCursor(col,row);

    if(value >=100) lcd->print("--");
    else{
        lcd->write( value < 10? ' ':'0' + value/10);
        lcd->write('0' + value%10);
    }
}

bool SmartDisplay::_updatePartial(uint8_t mask){
    uint8_t newLayout = _layout | mask;
    if(!_shown){
        _layout = newLayout;
        return false;
    }
    //else, 
    // showing
    if(_layout == newLayout){
        return true;
    }else{
        // update the layout
        _layout = newLayout;
        redraw();
        return false;
    }
}

void SmartDisplay::gravityDeviceData(float gravity,float temperature, uint32_t update,char tunit,bool usePlato){
    _gravity = gravity;
    _temperature = temperature;
    _updateTime = update;
    _tempUnit = tunit;
    _plato = usePlato;
    if(_updatePartial(GravityMask)) _drawGravity();
}

void SmartDisplay::pressureData(float pressure){
    _pressure = pressure;
    if(_updatePartial(PressureMask)) _drawPressure();
}
void SmartDisplay::humidityData(bool chamberValid, uint8_t chamber,bool roomValid, uint8_t room){
    _chamberHumidityAvailable = chamberValid;
    _roomHumidityAvailable = roomValid;
    _chamberHumidity = chamber;
    _roomHumidity = room;
    if(_updatePartial(HumidityMask)) _drawHumidity();
}

void SmartDisplay::setIp(IPAddress ip){
    _ip = ip;
    if(_shown) _drawIp();
}
