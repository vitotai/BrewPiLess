#if ISPINDEL_DISPLAY
#include "IicOledLcd.h"
#include "DisplayIspindel.h"
#include "font_cousine_10.h"
#include "font_large_number.h"
#include "mystrlib.h"
#include "TimeKeeper.h"

//extern const uint8_t Cousine_10[] PROGMEM;

#define BackgroundColor BLACK
#define TextColor WHITE

DisplayIspindel displayIspindel;

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

void DisplayIspindel::redraw(){
    DBG_PRINTF("DisplayIspindel::redraw()\n");
    _display = getLcd()->getDisplayDriver();
    _showFixedParts();
    _showUpdates();
    _display->display();
}

void DisplayIspindel::loop(){
    if(millis() - _lastUpdated > 60000){
        _showUpdates();
        _display->display();
   }
}

void DisplayIspindel::updateInfo(float gravity,float temperature,char unit,float battery,float tilt,int8_t wifiStrength){
     DBG_PRINTF("DisplayIspindel::updateInfo()\n");
    _gravity = gravity;
    _temperature=temperature;
    _battery=battery;
    _tilt=tilt;
    _wifiStrength=wifiStrength;
    _unit=unit;;

    _lastSeen=TimeKeeper.getTimeSeconds();
    // for update in next loop()
    _lastUpdated=0;
}


void DisplayIspindel::_showUpdates(){
     DBG_PRINTF("DisplayIspindel::_showUpdates()\n");
    _lastUpdated = millis();
    _display->setColor(TextColor);
    _showGravity();
    _showTemperature();
    _showBattery();
    _showTilt();
    _showLastSeen();
    _showIp();

    _showSignalAt(BPL_SIGNAL,WiFi.RSSI());
    _showSignalAt(Ispindel_SIGNAL_POS,_wifiStrength);
}

void DisplayIspindel::_showSignalAt(int16_t x, int16_t y,int8_t strength){

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

void DisplayIspindel::_showFixedParts(){
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
    _display->drawString(LB_VOLT_POS,"V");

    _display->drawRect(0,0,128,64);
}

void DisplayIspindel::_drawFloatAt(int16_t x, int16_t y, float value, uint8_t space, uint8_t precision, uint16_t fontWidth,uint16_t fontHeight){
    char buffer[32];
    
    _display->setColor(BackgroundColor);
    _display->fillRect(x,y,fontWidth * space,fontHeight);
    _display->setColor(TextColor);
    // 1.xxx
    sprintFloat((char*)buffer,value,precision);
    int dig=sprintFloat((char*)buffer,value,precision);
    _display->drawString(x + (space - dig) * fontWidth,y,buffer);
}

void DisplayIspindel::_showGravity(){
    // 1.076
    _display->setFont(Font_Large_Number);
    _drawFloatAt(SG_POS,_gravity,5,3,LargeFontWidth,LargeFontHeight);
    _display->setFont(Cousine_10);
}

void DisplayIspindel::_showTemperature(){
    // 123.5 or 23.5 or 9.8
    _display->setFont(Font_Large_Number);
    _drawFloatAt(TEMP_POS,_temperature,5,1,LargeFontWidth,LargeFontHeight);

    _display->setFont(Cousine_10);
    _display->drawString(TEMP_UNIT_POS,String(_unit));
}

void DisplayIspindel::_showBattery(){
    // 4.20 ~ 3.30
    _drawFloatAt(BAT_POS,_battery,4,2,SmallFontWidth,SmallFontHeight);
}

void DisplayIspindel::_showTilt(){
    // 90.4 ~ 8.91
    _drawFloatAt(TILT_POS,_tilt,4,2,SmallFontWidth,SmallFontHeight);
}

void DisplayIspindel::_showLastSeen(){
    char buffer[32];
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
    buffer[idx++]='\0';

    // clear
    _display->setColor(BackgroundColor);
    _display->fillRect(LASTSEEN_POS,LASTSEEN_WIDTH,SINGAL_HEIGHT);
    _display->setColor(TextColor);
    _display->drawString(LASTSEEN_POS,buffer);
}

void DisplayIspindel::_showIp(){
    IPAddress ip = WiFi.localIP();
	char buf[21];
	sprintf(buf,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);

    _display->setColor(BackgroundColor);
    _display->fillRect(IP_POS,IP_WIDTH,SINGAL_HEIGHT);
    _display->setColor(TextColor);

	_display->drawString(IP_POS,buf);
}
#endif //ISPINDEL_DISPLAY