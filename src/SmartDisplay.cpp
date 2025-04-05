
//*****************************************************************
//SmartDisplay

#include "Brewpi.h"
#include "BrewpiStrings.h"
#include "Display.h"
#include "DisplayLcd.h"
//#include "mystrlib.h"
#include "TimeKeeper.h"
#include "BPLSettings.h"
#if ISPINDEL_DISPLAY
#include "IicOledLcd.h"
#include "font_cousine_10.h"
#include "font_large_number.h"
#endif

#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include "SharedLcd.h"

SmartDisplay smartDisplay;

#define GravityInfoUpdatePeriod 60000
#define DegreeSymbolChar 0b11011111


static const char STR_Gravity[] PROGMEM = "Gravity ";
static const char STR_Temperature[] PROGMEM = "Temperature";
static const char STR_Updated[] PROGMEM = "Last seen";
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



void SmartDisplay::_drawSignalAt(uint8_t col,uint8_t row,int8_t rssi){
    char ch;
    
    if(rssi > -67) ch = CharSignal_4;
    else if(rssi > -70) ch = CharSignal_3;
    else if(rssi > -80)  ch = CharSignal_2;
    else if(rssi > -90)  ch = CharSignal_1;
    else ch = '!';

    PhysicalLcdDriver *lcd=getLcd();
    lcd->setCursor(col,row);
    lcd->write(ch);
}

SmartDisplay::SmartDisplay(){
    _gravityInfoValid=false;
    _layout=0;
}



void SmartDisplay::redraw(){
    //redraw all
    #if ISPINDEL_DISPLAY
    if(_layout == GravityMask){
        _display = getLcd()->getDisplayDriver();
        _showGravityFixedParts();
        _showGravityValues();
        _display->display();
        return;
    }
    #endif

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
   4.23V
*/
        case GravityMask: //1:
            #if ISPINDEL_DISPLAY
                //showGravityFixedParts();
            #else
                lcd->setCursor(0,0);
                lcd->print("G:");
                lcd->setCursor(10,0);
                lcd->print("T:");
                lcd->setCursor(17,0);
                lcd->write(DegreeSymbolChar);
                lcd->write(_tempUnit);
                lcd->setCursor(0,1);
                lcd->write(CharBattery);
                lcd->write(':');
                lcd->setCursor(6,1);
                lcd->write(_batteryUnit);
                lcd->setCursor(10,1);
                lcd->write(CharTilt);
                lcd->write(':');
                lcd->setCursor(0,2);
                lcd->print_P(STR_Updated);
                lcd->setCursor(17,2);
                lcd->print_P(STR_ago);
                #if !CustomGlyph
                lcd->setCursor(18,1);
                lcd->write('w');
                lcd->setCursor(18,3);
                lcd->write('w');
                #endif
            #endif
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
  4.32V
Pressure    13.5 psi              
5
G 1.012      012.5°C 
  updated    10m ago
RH C 56%      R  75%  
*/

        case (PressureMask | GravityMask): //3:
        case (HumidityMask | GravityMask)://5:
            if(_layout == 3) pressureLine = 2;
            else singleLinedHumidity = 2;

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

            lcd->setCursor(6,1);
            lcd->write(_batteryUnit);
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
                lcd->write(_batteryUnit);
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
  4.32V
*/
        case GravityMask: //1:
            #if ISPINDEL_DISPLAY
               // _showGravityValues();
            #else
            if(_plato) _printFloatAt(3,0,4,1,_gravity);
            else{
                /*DBG_PRINTF("print gravity:");
                DBG_PRINT(_gravity);
                DBG_PRINTF("\n"); */
                _printFloatAt(2,0,5,3,_gravity);
            }

            _printFloatAt(12,0,5,1,_temperature);
            _printGravityTimeAt(13,2);
            if(_batteryUnit == '%'){
                _printIntegerAt(2,1,4,(int)_battery);
            }else{
                _printFloatAt(2,1,4,2,_battery);
            }
            _printFloatAt(12,1,5,2,_tilt);
            _printGravityTimeAt(13,2);
            _drawSignalAt(19,1,_rssi);
            _drawSignalAt(19,3,WiFi.RSSI());
            #endif
            break;
/*
3
01234567890123456789
G 15.6 P
G 1.012      012.5°C 
  updated    10m ago
  4.32V
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
            if(_batteryUnit == '%'){
                _printIntegerAt(2,1,4,(int)_battery);
            }else{
                _printFloatAt(2,1,4,2,_battery);
            }

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
            else{
                if(_batteryUnit == '%'){
                _printIntegerAt(10,0,4,(int)_battery);
                }else{
                _printFloatAt(10,0,4,2,_battery);
                }
            }
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
            pressureLine = 2;
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
            singleLinedHumidity = 2;
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

void SmartDisplay::_printIntegerAt(uint8_t col,uint8_t row,uint8_t space,int value){
    PhysicalLcdDriver *lcd=getLcd();
    lcd->setCursor(col,row);

    char buffer[64];
    sprintf(buffer,"%d",value);
    size_t len=strlen(buffer);
    if(len > space) return;

    for(int i=0;i< (space-len);i++) lcd->write(' ');
    lcd->print(buffer);
}


void SmartDisplay::_printFloatAt(uint8_t col,uint8_t row,uint8_t space,uint8_t precision,float value){
    PhysicalLcdDriver *lcd=getLcd();
    lcd->setCursor(col,row);

    char buffer[64];
    char fmt[16];
    sprintf(fmt,"%%%d.%df",space,precision);
    sprintf(buffer,fmt,value);
//    DBG_PRINTF("_printFloatAt fmt:%s res:%s\n",fmt,buffer);
    lcd->print(buffer);
}
int SmartDisplay::_lastSeenString(char *buffer){
    int idx=0;
    uint32_t diff =TimeKeeper.getTimeSeconds() - _lastSeen;
    if(diff > 30* 86400){ // greater than 10 days
        strcpy(buffer,"long");
        idx = 4;
    }else if(diff >  99*60*60){  // greater than 99 hours, in days
        uint32_t days = diff/86400;
        buffer[idx++]= days < 10? ' ':'0' + days/10;
        buffer[idx++]='0' + days%10;
        buffer[idx++]='D';
    }else if(diff >  100*60){  // greater than 100 minutes, in hours
        uint32_t hours = diff/3600;
        buffer[idx++]= hours < 10? ' ':'0' + hours/10;
        buffer[idx++]= '0' + hours%10;
        buffer[idx++]='H';
    }else if(diff <  60){
        // less than one minutes
        strcpy(buffer,"<1m");
        idx = 3;
    }else{
        // in minute
        uint32_t minutes = diff/60;
        buffer[idx++]= minutes < 10? ' ':'0' + minutes/10;
        buffer[idx++]='0' + minutes%10;
        buffer[idx++]='m';
    }
    buffer[idx]='\0';
    return idx;
}

void SmartDisplay::_printGravityTimeAt(uint8_t col,uint8_t row){

    PhysicalLcdDriver *lcd=getLcd();
    lcd->setCursor(col,row);
    char buffer[32];
    _lastSeenString(buffer);
    lcd->print(buffer);
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

void SmartDisplay::gravityDeviceData(uint8_t type,float gravity,float temperature, uint32_t update,char tunit,bool usePlato,float battery,float tilt,int8_t rssi){
    if (type == GravityDevicePill) _batteryUnit = '%';
    else _batteryUnit = 'V';

    _gravity = gravity;
    _temperature = temperature;
    _lastSeen = update;
    _tempUnit = tunit;
    _plato = usePlato;
    _battery = battery;
    _rssi = rssi;
    _tilt = tilt;
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

void SmartDisplay::loop(){
   if(_gravityInfoValid){
       if(millis() - _gravityInfoLastPrinted > GravityInfoUpdatePeriod){
           if(_updatePartial(GravityMask)) _drawGravity();
       }
   }
}

#if ISPINDEL_DISPLAY
#define BackgroundColor BLACK
#define TextColor WHITE


#define Grvity_XBM_width  12
#define Grvity_XBM_height 21
static const uint8_t Grvity_XBM[] PROGMEM= {
 0x00,0xf0,0x00,0xf0,0xfe,0xff,0xfe,0xff,0x0e,0xfc,0x06,0xf8,
 0xe2,0xf9,0xf2,0xff,0xf2,0xff,0xf2,0xff,0x32,0xf8,0x32,0xf8,
 0xf2,0xf9,0xe2,0xf9,0x06,0xf8,0x0e,0xfc,0xfe,0xff,0xfe,0xff,
 0x00,0xf0,0x00,0xf0,0x00,0xf0};



#define Thermo_XBM_width  9
#define Thermo_XBM_height 21
 static const uint8_t Thermo_XBM[] PROGMEM= {
 0x00,0xfe,0x00,0xfe,0x10,0xfe,0x28,0xfe,0x28,0xfe,0x28,0xfe,
 0x28,0xfe,0x28,0xfe,0x28,0xfe,0x28,0xfe,0x38,0xfe,0x7c,0xfe,
 0xfe,0xfe,0xfe,0xfe,0x7c,0xfe,0x38,0xfe,0x00,0xfe,0x00,0xfe,
 0x00,0xfe,0x00,0xfe,0x00,0xfe};

#define BAT_XBM_width  12
#define BAT_XBM_height 12
static const uint8_t BAT_XBM[] PROGMEM= {
 0x00,0xf0,0x00,0xf0,0x60,0xf0,0x60,0xf0,0xf8,0xf1,0xf8,0xf1,
 0xf8,0xf1,0xf8,0xf1,0xf8,0xf1,0xf8,0xf1,0xf8,0xf1,0x00,0xf0};

#define TILT_XBM_width  12
#define TILT_XBM_height 12
static const uint8_t TILT_XBM[] PROGMEM= {
 0x00,0xf0,0x00,0xf0,0x00,0xf4,0x00,0xf2,0x00,0xf1,0x80,0xf0,
 0x40,0xf0,0x20,0xf0,0xf0,0xf7,0x00,0xf0,0x00,0xf0,0x00,0xf0};

#define NOWIFI_XBM_width  12
#define NOWIFI_XBM_height 12
static const uint8_t NOWIFI_XBM[] PROGMEM= {
 0x00,0xf0,0xf0,0xf0,0x08,0xf1,0x04,0xf3,0x82,0xf4,0x42,0xf4,
 0x22,0xf4,0x12,0xf4,0x0c,0xf2,0x08,0xf1,0xf0,0xf0,0x00,0xf0};

#define LargeFontWidth 10
#define LargeFontHeight 21
#define SmallFontWidth 6
#define SmallFontHeight 12

// line 0
#define Y0 2
#define LB_SG_POS 1,Y0
#define LB_SG_WIDTH 12
#define LB_SG_HEIGHT 21
#define SG_POS 14,Y0
#define LB_TEMP_POS 64,Y0
#define LB_TEMP_WIDTH 6
#define LB_TEMP_HEIGHT 21
#define TEMP_POS 71,Y0
#define TEMP_POS_X 71

#define TEMP_UNIT_POS 120,(Y0+6)

// line 1
#define Y1 24
#define LB_BAT_POS 1,Y1
#define LB_BAT_WIDTH 18
#define LB_BAT_HEIGHT 12
#define BAT_POS 20,Y1
#define LB_VOLT_POS 44,Y1


#define LB_TILT_POS 53,Y1
#define LB_TILT_WIDTH 24
#define LB_TILT_HEIGHT 12
#define TILT_POS  78,Y1
#define TILT_POS_X  78
#define Ispindel_SIGNAL_POS 108,Y1
// line 2
#define Y2 37
#define LB_LASTSEEN_POS 1,Y2
#define LASTSEEN_POS 66,Y2
#define LB_AGO_POS 108,Y2
#define LASTSEEN_WIDTH (108-66)

// line 3
#define Y3 50
#define LB_IP_POS 1,Y3
#define LB_IP_WIDTH 12
#define LB_IP_HEIGHT 12
#define IP_POS 16,Y3
#define BPL_SIGNAL 108,Y3
#define IP_WIDTH 90


// wifi signal 
#define SIGNAL_WIDTH 18
#define SINGAL_HEIGHT 12
#define SIGNAL_BAR_WIDTH 3
#define SIGNAL_BAR_GAP 2


void SmartDisplay::_showGravityValues(){
     DBG_PRINTF("DisplayIspindel::_showUpdates()\n");
    _display->setColor(TextColor);
    _showGravity();
    _showTemperature();
    _showBattery();
    _showTilt();
    _showLastSeen();
    _showIp();

    _showSignalAt(BPL_SIGNAL,WiFi.RSSI());
    _showSignalAt(Ispindel_SIGNAL_POS,_rssi);
}

void SmartDisplay::_showSignalAt(int16_t x, int16_t y,int8_t strength){

    int barNum;
    
    if(strength > -67) barNum = 4;
    else if(strength > -70) barNum = 3;
    else if(strength > -80) barNum = 2;
    else if(strength > -90) barNum = 1;
    else barNum = 0;    
    // clear
    _display->setColor(BackgroundColor);
    _display->fillRect(x,y,SIGNAL_WIDTH,SINGAL_HEIGHT);

    _display->setColor(TextColor);

    if(barNum ==0){
        _display->drawXbm(x, y, NOWIFI_XBM_width,NOWIFI_XBM_height, NOWIFI_XBM);
        return;
    }
    
    int16_t xpos = x;
    int16_t ypos = y + SINGAL_HEIGHT - SINGAL_HEIGHT/4;
    int16_t barHeight = SINGAL_HEIGHT/4;
    for(int i=1;i<=barNum;i++){
        _display->fillRect(xpos,ypos,SIGNAL_BAR_WIDTH,barHeight);

        xpos += SIGNAL_BAR_WIDTH +SIGNAL_BAR_GAP;
        ypos -= SINGAL_HEIGHT/4;
        barHeight += SINGAL_HEIGHT/4;
    }
}

void SmartDisplay::_showGravityFixedParts(){
    DBG_PRINTF("DisplayIspindel::_showFixedParts()\n");
    _display->setColor(TextColor);
    // Gravity
    _display->drawXbm(LB_SG_POS, Grvity_XBM_width, Grvity_XBM_height, Grvity_XBM);
    // temperature
    _display->drawXbm(LB_TEMP_POS, Thermo_XBM_width, Thermo_XBM_height, Thermo_XBM);
    // battery
    _display->drawXbm(LB_BAT_POS, BAT_XBM_width, BAT_XBM_height, BAT_XBM);
    // tilt
    _display->drawXbm(LB_TILT_POS, TILT_XBM_width, TILT_XBM_height, TILT_XBM);

    _display->drawString(LB_IP_POS,"IP");
    _display->drawString(LB_LASTSEEN_POS,"Last seen");
    _display->drawString(LB_AGO_POS,"ago");
    char unit[2]; unit[0]=_batteryUnit; unit[1]='\0';
    _display->drawString(LB_VOLT_POS,unit);

    _display->drawRect(0,0,128,64);
}

void SmartDisplay::_showFloatAt(int16_t x, int16_t y, float value, uint8_t space, uint8_t precision, uint16_t fontWidth,uint16_t fontHeight){
    char buffer[32];
    
    _display->setColor(BackgroundColor);
    _display->fillRect(x,y,fontWidth * space,fontHeight);
    _display->setColor(TextColor);

    char fmt[16];
    sprintf(fmt,"%%%d.%df",space,precision);
    sprintf(buffer,fmt,value);

    _display->drawString(x,y,buffer);
}
void SmartDisplay::_showIntegerAt(int16_t x, int16_t y, int value, uint8_t space, uint16_t fontWidth,uint16_t fontHeight){
    char buffer[32];
    
    _display->setColor(BackgroundColor);
    _display->fillRect(x,y,fontWidth * space,fontHeight);
    _display->setColor(TextColor);
    // 1.xxx    
    sprintf(buffer,"%d",value);
    
    int dig=strlen(buffer);
    _display->drawString(x + (space - dig) * fontWidth,y,buffer);
}

void SmartDisplay::_showGravity(){
    // 1.076
    _display->setFont(Font_Large_Number);

    if(_plato) _showFloatAt(SG_POS,_gravity,5,1,LargeFontWidth,LargeFontHeight);
    else _showFloatAt(SG_POS,_gravity,5,3,LargeFontWidth,LargeFontHeight);
    _display->setFont(Cousine_10);
}

void SmartDisplay::_showTemperature(){
    // 123.5 or 23.5 or 9.8
    _display->setFont(Font_Large_Number);
    _showFloatAt(TEMP_POS,_temperature,5,1,LargeFontWidth,LargeFontHeight);

    _display->setFont(Cousine_10);
    _display->drawString(TEMP_UNIT_POS,String(_tempUnit));
}

void SmartDisplay::_showBattery(){
    // 4.20 ~ 3.30, 
    if(_batteryUnit  == '%'){
         _showIntegerAt(BAT_POS,(int)_battery,4,SmallFontWidth,SmallFontHeight);
    }else{
        _showFloatAt(BAT_POS,_battery,4,2,SmallFontWidth,SmallFontHeight);
    }
}

void SmartDisplay::_showTilt(){
    // 90.4 ~ 8.91
    _showFloatAt(TILT_POS,_tilt,4,2,SmallFontWidth,SmallFontHeight);
}

void SmartDisplay::_showLastSeen(){
    char buffer[32];
    _lastSeenString(buffer);
    // clear
    _display->setColor(BackgroundColor);
    _display->fillRect(LASTSEEN_POS,LASTSEEN_WIDTH,SINGAL_HEIGHT);
    _display->setColor(TextColor);
    _display->drawString(LASTSEEN_POS,buffer);
}

void SmartDisplay::_showIp(){
    IPAddress ip = WiFi.localIP();
	char buf[21];
	sprintf(buf,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);

    _display->setColor(BackgroundColor);
    _display->fillRect(IP_POS,IP_WIDTH,SINGAL_HEIGHT);
    _display->setColor(TextColor);

	_display->drawString(IP_POS,buf);
}
#endif //ISPINDEL_DISPLAY