/*  Example sketch for the PCF8574.
    Attach an LED to PIN7 on the PCF8574, a button to PIN3 and the INT-pin to GPIO14 (Nodemcu D5) on the ESP8266.
    You may need to add a debounce - circuit to the button for this to work right.
*/

#include <pcf8574_esp.h>

// Wire.h already defines "Wire" which the PCF8574-class would use by default, but for the sakes of an example let's define our own instance of it and use that instead!
TwoWire testWire;
// Also, since I2C is emulated on the ESP8266 let's redefine what pins to use as SDA and SCL and instead swap them around! Just for the sakes of an example!
// DO NOT FORGET TO WIRE ACCORDINGLY, SDA GOES TO GPIO5, SCL TO GPIO4 (ON NODEMCU GPIO5 IS D1 AND GPIO4 IS D2)
PCF8574 pcf8574(0x20, 5, 4, testWire);

bool PCFFlag = false;

void PCFInterrupt() {
  PCFFlag = true;
}

void setup() {
  // Most ready-made PCF8574 - modules seem to lack an internal pullup-resistor, so you have to use the ESP8266 - internal one or else it won't work
  pinMode(14, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(14), PCFInterrupt, CHANGE);
}

void loop() {
  if (PCFFlag) {
    PCFFlag = false;
    pcf8574.write(7, pcf8574.read(3));
  }
  delay(1);
}
