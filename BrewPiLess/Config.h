/*
 * Copyright 2012-2013 BrewPi/Elco Jacobs.
 * Copyright 2013 Matthew McGowan.
 *
 * This file is part of BrewPi.
 * 
 * BrewPi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * BrewPi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with BrewPi.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once



//////////////////////////////////////////////////////////////////////////
//
// Set verbosity of debug messages 0-3
// 0: means no debug messages
// 1: is typical debug messages required for end users
// 2-3: more verbose debug messages
//
// #ifndef BREWPI_DEBUG
// #define BREWPI_DEBUG 1
// #endif
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
// Define which brewpi shield is used.
// BREWPI_SHIELD_REV_A The RevA shield (ca. Feb 2013), two OneWire buses, door, heat, cool.
// BREWPI_SHIELD_REV_C The RevC shield (ca. May 2013). One common ONeWire bus, 4 actuators. Dynaconfig.
//
#ifndef BREWPI_STATIC_CONFIG
// #define BREWPI_STATIC_CONFIG BREWPI_SHIELD_REV_A
#define BREWPI_STATIC_CONFIG BREWPI_SHIELD_DIY
#endif
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//
// Enable the simulator. Real sensors/actuators are replaced with simulated versions. In particular, the values reported by
// temp sensors are based on a model of the fridge/beer.
//
// #ifndef BREWPI_SIMULATE
// #define BREWPI_SIMULATE 0
// #endif
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//
// Enable DS2413 Actuators. 
//
// #ifndef BREWPI_DS2413
// #define BREWPI_DS2413 0
// #endif
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//
// This flag virtualizes as much of the hardware as possible, so the code can be run in the AvrStudio simulator, which
// only emulates the microcontroller, not any attached peripherals.
//
// #ifndef BREWPI_EMULATE
// #define BREWPI_EMULATE 0
// #endif
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//
// Flag to control use of cascaded filter
//
// #ifndef TEMP_SENSOR_CASCADED_FILTER
// #define TEMP_SENSOR_CASCADED_FILTER 1
// #endif
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//
// Flag to control implementation of TempControl as a static class.
// Should normally be left alone unles you are experimenting with multi-instancing.
//
// #ifndef TEMP_CONTROL_STATIC
// #define TEMP_CONTROL_STATIC 1
// #endif
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//
// Flag to control use of Fast digital pin functions
// 
// #ifndef FAST_DIGITAL_PIN 
// #define FAST_DIGITAL_PIN 0
// #endif
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//
// Enable the LCD menu.
//
// #ifndef BREWPI_MENU
// #define BREWPI_MENU 1
// #endif
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//
// Enable the LCD display. Without this, a NullDisplay is used
//
#ifndef BREWPI_LCD
#define BREWPI_LCD 1
#endif

//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//
#ifndef BREWPI_BUZZER
#define BREWPI_BUZZER 1
#endif
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//
// #ifndef BREWPI_ROTARY_ENCODER
#define BREWPI_ROTARY_ENCODER 0
// #endif
//
//////////////////////////////////////////////////////////////////////////

#define WAKEUP_BUTTON 0

//////////////////////////////////////////////////////////////////////////
//
// #ifndef BREWPI_EEPROM_HELPER_COMMANDS
// #define BREWPI_EEPROM_HELPER_COMMANDS BREWPI_DEBUG || BREWPI_SIMULATE
// #endif
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//
// BREWPI_SENSOR_PINS - can be disabled if only using onewire devices
//
// #ifndef BREWPI_SENSOR_PINS
// #define BREWPI_SENSOR_PINS 1
// #endif
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//
// BREWPI_ACTUATOR_PINS - can be disabled if only using onewire devices
// #ifndef BREWPI_ACTUATOR_PINS
// #define BREWPI_ACTUATOR_PINS 1
// #endif
//
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
//
// Pin Configuration - Change the below to match your individual pinout
//
// pins

#define NODEMCU_PIN_A0 17	// Analog 

#define NODEMCU_PIN_D0 16	// No interrupt, do not use for rotary encoder
#define NODEMCU_PIN_D1 5	// Generally used for I2C
#define NODEMCU_PIN_D2 4	// Generally used for I2C
#define NODEMCU_PIN_D3 0
#define NODEMCU_PIN_D4 2    // Also controls the LED on the ESP8266 module
#define NODEMCU_PIN_D5 14
#define NODEMCU_PIN_D6 12
#define NODEMCU_PIN_D7 13
#define NODEMCU_PIN_D8 15

#define NODEMCU_PIN_D9 3	// Do not use - USB
#define NODEMCU_PIN_D10 1	// Do not use - USB

#define PIN_SDA NODEMCU_PIN_D2
#define PIN_SCL NODEMCU_PIN_D1

//#define SONOFF true

#if SONOFF
#define oneWirePin NODEMCU_PIN_D5  // If oneWirePin is specified, beerSensorPin and fridgeSensorPin are ignored
#define coolingPin NODEMCU_PIN_D6
#define heatingPin NODEMCU_PIN_D0
#define doorPin    NODEMCU_PIN_D7
#define BuzzPin NODEMCU_PIN_D3
#define BREWPI_LCD false
#else
#define oneWirePin NODEMCU_PIN_D6  // If oneWirePin is specified, beerSensorPin and fridgeSensorPin are ignored
// actuators
#define coolingPin NODEMCU_PIN_D5
#define heatingPin NODEMCU_PIN_D7
#define doorPin    NODEMCU_PIN_D4
#define BuzzPin NODEMCU_PIN_D0

#endif

#if BREWPI_LCD
// LCD configurations:
//#define BREWPI_OLED128x64_LCD 1
#define BREWPI_IIC_LCD 1
#endif

#define IIC_LCD_ADDRESS 0x27
#define LCD_AUTO_ADDRESSING true

#ifdef BREWPI_OLED128x64_LCD 
#define OLED128x64_LCD_ADDRESS 0x3c
#define STATUS_LINE 1
#endif


#if BREWPI_ROTARY_ENCODER || WAKEUP_BUTTON
#define BACKLIGHT_AUTO_OFF_PERIOD 180
#else
#define BACKLIGHT_AUTO_OFF_PERIOD 0 // disabled
#endif
// Pay attention when changing the pins for the rotary encoder.
// They should be connected to external interrupt INT0, INT1 and INT3
//#define rotaryAPin 2 // INT1
//#define rotaryBPin 1 // INT3
//#define rotarySwitchPin 0 // INT2

#if WAKEUP_BUTTON
#define WakeupButtonPin NODEMCU_PIN_D3
#if BREWPI_ROTARY_ENCODER
#error "No wakeup button with rotary encoder"
#endif
#endif

#if BREWPI_ROTARY_ENCODER

#define RotaryViaPCF8574 1

#ifdef RotaryViaPCF8574

#define rotaryAPin 0
#define rotaryBPin 1
#define rotarySwitchPin 2

#define PCF8574_INT NODEMCU_PIN_D3
#define PCF8574_ADDRESS 0x20

#else
#error "invalid setting"
#define rotaryAPin NODEMCU_PIN_D8
#define rotaryBPin NODEMCU_PIN_D7
#define rotarySwitchPin NODEMCU_PIN_D4
#endif

#endif //BREWPI_ROTARY_ENCODER

#ifdef ESP8266
//#define ESP8266_WiFi 1			// This disables Serial and enables WiFi support 
//#define ESP8266_WiFi_Control 1	// This adds the headers for WiFi support (so you can disconnect from WiFi via serial)
#define ESP8266_ONE 1
#endif


/*
// Note - LCD module pins aren't used yet.
#define DISP_RS 9
#define DISP_RW 8
#define DISP_EN 7
#define DISP_D4 6
#define DISP_D5 5
#define DISP_D6 4
#define DISP_D7 3
*/

// BREWPI_INVERT_ACTUATORS
// TODO - FIgure out what the hell this actually does
#define BREWPI_INVERT_ACTUATORS 0

#define BUFFER_PILINK_PRINTS 1

#define EARLY_DISPLAY 1
