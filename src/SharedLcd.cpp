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

#define GravityInfoUpdatePeriod 60000
#define SWITCH_TIME 8000
#define DegreeSymbolChar 0b11011111


static const char STR_Gravity[] PROGMEM = "Gravity ";
static const char STR_Temperature[] PROGMEM = "Temperature";
static const char STR_Updated[] PROGMEM = "Updated";
static const char STR_ago[] PROGMEM = "ago";
static const char STR_HumidityChamber[] PROGMEM= "Humidity Chamber";
static const char STR_Room[] PROGMEM = "Room";

static const char STR_Pressure[] PROGMEM = "Pressure";
static const char STR_psi[] PROGMEM = "psi";

static const char STR_IP_[] PROGMEM = "IP ";

static const char STR_RH_C_R[] PROGMEM = "RH C   %      R   %";
static const char STR_Humidity_Chamber[] PROGMEM = "Humidity Chamber   % ";
static const char STR_Humidity_Room[] PROGMEM = "Humidity Room      %";
static const char STR_Unknown[] PROGMEM = "NA";
static const char STR_Less_1m[] PROGMEM = "<1m";
static const char STR___[] PROGMEM = "--";


SharedDisplayManager sharedDisplayManager;

uint8_t SharedDisplayManager::i2cLcdAddr = IIC_LCD_ADDRESS;
//*****************************************************************
// SharedLcdDisplay
 SharedLcdDisplay::SharedLcdDisplay(){
    _next = _previous = NULL;
    _hidden = true;
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
    if(_isForcedPrimary || _mode != ShareModeRotate ) return;
    if(millis() -_switchTime > SWITCH_TIME) next();
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
}
#endif
//*****************************************************************
//BrewPiLcd


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
    // getLcd()->refresh();
    //redraw();
    _manager->refresh();
}
#endif
//*****************************************************************
//SmartDisplay



SmartDisplay smartDisplay;

SmartDisplay::SmartDisplay(){
    _gravityInfoValid=false;
    _layout=0;
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
Updated          99m
 4.32 V

layout 2: P
01234567890123456789

Pressure     13.5psi


Layout 3:G,P   Gravity: L0, L1, Pressure: L2 
01234567890123456789
G 1.012      012.5°C 
  Updated    10m ago
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
Updated      99m ago
 4.23 V
*/
        case GravityMask: //1:
            lcd->setCursor(0,0);
            lcd->print_P(STR_Gravity);
            if(_plato){
                lcd->setCursor(18,0);
                lcd->write(DegreeSymbolChar);
                lcd->write('P');
            }
            lcd->setCursor(0,1);
            lcd->print_P(STR_Temperature);
            lcd->setCursor(18,1);
            lcd->write(DegreeSymbolChar);
            lcd->write(_tempUnit);
            //lcd->setCursor(0,2);
            //lcd->print_P(STR_Updated);
            lcd->setCursor(6,2);
            lcd->write('V');
            lcd->setCursor(17,2);
            lcd->print_P(STR_ago);
            break;
/*
01234567890123456789

Pressure    13.5 psi
*/

        case PressureMask: //2:
            pressureLine = 1;
            break;
/*
3
01234567890123456789
G 1.012      012.5°C 
  updated    10m ago
  4.32 V
Pressure    13.5 psi              
5
G 1.012      012.5°C 
  updated    10m ago
RH C 56%      R  75%  
*/

        case (PressureMask | GravityMask): //3:
        case (HumidityMask | GravityMask)://5:
            if(_layout == 3) pressureLine = 3;
            else singleLinedHumidity = 3;

            lcd->setCursor(0,0);
            lcd->write('G');
            lcd->setCursor(18,0);
            lcd->write(DegreeSymbolChar);
            lcd->write(_tempUnit);
            if(_plato){
                lcd->setCursor(7,0);
                lcd->write(DegreeSymbolChar);
                lcd->write('P');
            }


            //lcd->setCursor(2,1);
            //lcd->print_P(STR_Updated);
            lcd->setCursor(7,1);
            lcd->write('V');
            lcd->setCursor(17,1);
            lcd->print_P(STR_ago);
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
        case HumidityMask: //4:
        case (HumidityMask | PressureMask)://6:
            if(_layout == 6) pressureLine = 3;
            lcd->setCursor(0,0);
            lcd->print_P(STR_HumidityChamber);
            lcd->setCursor(19,0);
            lcd->write('%');
            lcd->setCursor(12,1);
            lcd->print_P(STR_Room);
            lcd->setCursor(19,1);
            lcd->write('%');
            break;

/*
01234567890123456789
G 1.012  012.5°C 10m
          4.32V
RH  C 56%     R  75%  => Humidity Chamber 56% => Humidity Room 99%
Pressure    13.5 psi
*/
        case (HumidityMask | PressureMask | GravityMask)://7:
            lcd->setCursor(0,0);
            lcd->write('G');
            pressureLine = 3;
            singleLinedHumidity = 2;
            if(_battery ==0){
                lcd->setCursor(14,0);
                lcd->write(DegreeSymbolChar);
                lcd->write(_tempUnit);
            }else{
                lcd->setCursor(14,0);
                lcd->write('V');
            }

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
        lcd->print_P(STR_Pressure);
        lcd->setCursor(17,(uint8_t)pressureLine);
        lcd->print_P(STR_psi);
    }
    if(singleLinedHumidity>=0){
    //RH C 56%      R  75%  => Humidity Chamber 56% => Humidity Room 99%
        if(_chamberHumidityAvailable  && _roomHumidityAvailable){
            lcd->setCursor(0,(uint8_t)singleLinedHumidity);
            lcd->print_P(STR_RH_C_R);
        }else if (_chamberHumidityAvailable){
            lcd->setCursor(0,(uint8_t)singleLinedHumidity);
            lcd->print_P(STR_Humidity_Chamber);
        }else if (_roomHumidityAvailable){
            lcd->setCursor(0,(uint8_t)singleLinedHumidity);
            lcd->print_P(STR_Humidity_Room);
        }

    }
    lcd->setCursor(0,3);
    lcd->print_P(STR_IP_); 
}

void SmartDisplay::_drawGravity(){
//    PhysicalLcdDriver *lcd=getLcd();

    _gravityInfoLastPrinted = millis();
    switch(_layout){
/*
01234567890123456789
Gravity       15.6 P
Gravity        1.045
Temperature  012.5°C
Updated      99m ago
 4.32 V
*/
        case GravityMask: //1:
            if(_plato) _printFloatAt(14,0,4,1,_gravity);
            else _printFloatAt(15,0,5,3,_gravity);
            _printFloatAt(13,1,5,1,_temperature);
            _printGravityTimeAt(13,2);
            _printBatteryAt(1,2);
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

        case (PressureMask | GravityMask): //3:
        case (HumidityMask | GravityMask)://5:
            if(_plato) _printFloatAt(2,0,4,1,_gravity);
            else _printFloatAt(2,0,5,3,_gravity);
            _printFloatAt(13,0,5,1,_temperature);
            _printGravityTimeAt(13,1);
            _printBatteryAt(1,1);
            break;

/*
01234567890123456789
G 1.012  012.5°C 10m
G 1.012   4.32V  10m
RH C 56%      R  75%  => Humidity Chamber 56% => Humidity Room 99%
Pressure    13.5 psi
*/
        case (HumidityMask | PressureMask | GravityMask)://7:
            if(_plato) _printFloatAt(2,0,4,1,_gravity);
            else _printFloatAt(2,0,5,3,_gravity);
            // ignore temperature if _battery is valid(not zero)
            if(_battery ==0.0) _printFloatAt(9,0,5,1,_temperature);
            else _printBatteryAt(10,0);
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

        case PressureMask: //2:
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

        case (PressureMask | GravityMask): //3:
        case (HumidityMask | PressureMask)://6:
        case (HumidityMask | PressureMask | GravityMask)://7:
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

        case (HumidityMask | GravityMask)://5:
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
        case  HumidityMask: //4:
        case (HumidityMask | PressureMask)://6:
            _printHumidityValueAt(17,0,_chamberHumidity);
            _printHumidityValueAt(17,1,_roomHumidity);
            break;

/*
01234567890123456789
G 1.012  012.5°C 10m
RH  C 56%     R  75%  => Humidity Chamber 56% => Humidity Room 99%
Pressure    13.5 psi
*/
        case (HumidityMask | PressureMask | GravityMask)://7:
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
//    DBG_PRINTF("_printFloatAt %d,%d,%s\n",space,digitNum,buffer);

    if(space > digitNum){
        uint8_t i=space - (uint8_t)digitNum;
        while( i-- > 0) lcd->write(' ');
    }else{
        digitNum = space;
    }
    buffer[digitNum]='\0';

    for( uint8_t i=0;i< digitNum;i++)
        lcd->write(buffer[i]);
}

void SmartDisplay::_printBatteryAt(uint8_t col,uint8_t row){
    
    PhysicalLcdDriver *lcd=getLcd();
    lcd->setCursor(col,row);
    int fraction = (int) (_battery * 100.0 + 0.5);
    int ipart = fraction / 100;
    fraction -= ipart * 100;

    lcd->write('0' + ipart);
    lcd->write('.');
    lcd->write('0' + fraction/10);
    lcd->write('0' + fraction%10);

}

void SmartDisplay::_printGravityTimeAt(uint8_t col,uint8_t row){

    PhysicalLcdDriver *lcd=getLcd();
    lcd->setCursor(col,row);

    uint32_t diff =TimeKeeper.getTimeSeconds() - _updateTime;
//    DBG_PRINTF("gravity Time: diff=%d",diff);
    if(diff > 30* 86400){ // greater than 10 days
        lcd->print_P(STR_Unknown);
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
        lcd->print_P(STR_Less_1m);
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

    if(value >=100) lcd->print_P(STR___);
    else{
        lcd->write( value < 10? ' ':'0' + value/10);
        lcd->write('0' + value%10);
    }
}

bool SmartDisplay::_updatePartial(uint8_t mask){
    uint8_t newLayout = _layout | mask;
    if(_hidden){
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

void SmartDisplay::gravityDeviceData(float gravity,float temperature, uint32_t update,char tunit,bool usePlato,float battery){
    _gravity = gravity;
    _temperature = temperature;
    _updateTime = update;
    _tempUnit = tunit;
    _plato = usePlato;
    _battery = battery;
    _gravityInfoValid = true;
    _gravityInfoLastPrinted =0; // forced to update
   // if(_updatePartial(GravityMask)) _drawGravity();
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
    if(!_hidden) _drawIp();
}

void SmartDisplay::update(){
   if(_gravityInfoValid){
       if(millis() - _gravityInfoLastPrinted > GravityInfoUpdatePeriod){
           if(_updatePartial(GravityMask)) _drawGravity();
       }
   }
}
