//    FILE: PCF8574.H
//  ORIGINAL AUTHOR: Rob Tillaart
//    DATE: 02-febr-2013
// VERSION: 0.1.02
// PURPOSE: I2C PCF8574 library for Arduino
//     URL:
//
// Library modified by WereCatf

#ifndef _PCF8574_ESP_H
#define _PCF8574_ESP_H

#include <Arduino.h>
#include <Wire.h>

class PCF8574
{
  public:
    PCF8574(uint8_t address, int sda = SDA, int scl = SCL, TwoWire UseWire = Wire);

    uint8_t read8();
    uint8_t read(uint8_t pin);
    uint8_t value();

    void write8(uint8_t value);
    void write(uint8_t pin, uint8_t value);

    void toggle(uint8_t pin);
    void shiftRight(uint8_t n=1);
    void shiftLeft(uint8_t n=1);

    int lastError();

  private:
    TwoWire _Wire;
    uint8_t _address;
    uint8_t _data;
    uint8_t _pinModeMask = 255;
    int _error;
};

#endif
