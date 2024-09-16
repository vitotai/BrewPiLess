# PCF8574_ESP

This is a simple library for using the PCF8574/PCF8574A I/O - expanders over I2C. I took the code by Rob Tillaart from http://playground.arduino.cc/Main/PCF8574Class and modified it to allow the use of I2C on non-default pins and/or using a TwoWire - class of your own instead of the default "Wire" as defined in Wire.h.

This library does not supply any special handling for using the interrupt - pin on the PCF8574, just connect the pin to a pin on your ESP, set up an interrupt-handler for it and remember to use pinMode(pin, INPUT_PULLUP) to make use of it.

# Usage
```
class initializer PCF8574(uint8_t address, int sda = SDA, int scl = SCL, TwoWire UseWire = Wire)
uint8_t read8()
uint8_t read(uint8_t pin)
uint8_t value()
void write8(uint8_t value)
void write(uint8_t pin, uint8_t value)
void toggle(uint8_t pin)
void shiftRight(uint8_t n=1)
void shiftLeft(uint8_t n=1)
int lastError()
```

# Mixing INPUT- and OUTPUT-pins and write8

Due to the way the PCF8574 works you cannot just use read8() to read the pin-states, then change one pin and write8() the new states out because if you are using some pins as INPUT and the pin is being pulled low the moment you read8() its state you'll then be pulling the pin LOW when issuing write8() and it'll stop working as an INPUT. For this reason the library caches written values instead of relying on reading the pin-states when using write() or toggle(), and if you use write8() in your own code you need to remember to pull HIGH any pin you want to use as INPUT regardless of their current state.
