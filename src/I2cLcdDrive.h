#ifndef LiquidCrystal_I2C_h
#define LiquidCrystal_I2C_h

#include "Brewpi.h"
#include "BrewpiStrings.h"
#include <LiquidCrystal_I2C.h>

// commands


class I2cLcdDrive: public LiquidCrystal_I2C
{
public:
    I2cLcdDrive(uint8_t addr, uint8_t cols, uint8_t rows, uint8_t displayadapter=YWROBOT)
    :LiquidCrystal_I2C(addr,cols,rows,displayadapter){}

    void print_P(const char * str) {
        char buf[21]; // create buffer in RAM
        strcpy_P(buf, str); // copy string to RAM
        print(buf); // print from RAM
    }

    void printSpacesToRestOfLine(void){
        while(_currpos < _cols){
            print(' ');
        }
    }

  void getLine(uint8_t lineNumber, char * buffer){
    const char* src = content[lineNumber];
    for(uint8_t i = 0; i < _cols;i++){
        char c = src[i];
        buffer[i] = (c == 0b11011111) ? 0xB0 : c;
    }
    buffer[_cols] = '\0'; // NULL terminate string
}

// This resets the backlight timer and updates the SPI output
void IIClcd::resetBacklightTimer(void) {
    _backlightTime = ticks.seconds();
}

void IIClcd::updateBacklight(void) {
    // True = OFF, False = ON
    bool backLightOutput = (backlightAutoOffPeriod !=0) && (BREWPI_SIMULATE || ticks.timeSince(_backlightTime) > backlightAutoOffPeriod);
    if(backLightOutput) {
        noBacklight();
    } else {
        backlight();
    }
}


};