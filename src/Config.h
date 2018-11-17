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
// Enable External temperature Sensor
//
#ifndef BREWPI_EXTERNAL_SENSOR
#define BREWPI_EXTERNAL_SENSOR true
#endif
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
#ifndef BREWPI_ROTARY_ENCODER
#define BREWPI_ROTARY_ENCODER 0
#endif
//
//////////////////////////////////////////////////////////////////////////

// default supports 2 buttons
#ifndef BREWPI_BUTTONS
#define BREWPI_BUTTONS 1
#endif

#ifndef ButtonViaPCF8574 
#define ButtonViaPCF8574 0
#endif

#ifndef AUTO_CAP
#define  AUTO_CAP 1
#endif


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
// Board Definition
//
// 
#define BrewShield 0
#define Sonoff 1
#define Thorrak_PCB 2

#ifndef BOARD
#define BOARD BrewShield
#endif

//////////////////////////////////////////////////////////////////////////
//
// Pin Configuration - Change the settings below to match your individual pinout
//
// pins

#define NODEMCU_PIN_A0 17	// Analog

#define NODEMCU_PIN_D0 16	// No interrupt, do not use for rotary encoder,
                            // Also controlls 2nd LED on NodeMCU board
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


#if  BOARD == Sonoff
// define this option to disable webserver.
#define SONOFF true 

#define oneWirePin NODEMCU_PIN_D5  // If oneWirePin is specified, beerSensorPin and fridgeSensorPin are ignored
#define coolingPin NODEMCU_PIN_D6
#define heatingPin NODEMCU_PIN_D0
#define doorPin    NODEMCU_PIN_D7
#define BuzzPin NODEMCU_PIN_D3
#ifdef BREWPI_LCD
#undef BREWPI_LCD 
#endif
// NO LCD, NO BUTTONs
#define BREWPI_LCD false
#undef BREWPI_MENU
#define BREWPI_MENU 0
#undef  BREWPI_BUTTONS 
#define  BREWPI_BUTTONS 0

#elif BOARD == Thorrak_PCB
#define oneWirePin NODEMCU_PIN_D6  // If oneWirePin is specified, beerSensorPin and fridgeSensorPin are ignored
// actuators
#define coolingPin NODEMCU_PIN_D5
#define heatingPin NODEMCU_PIN_D0
#define doorPin    NODEMCU_PIN_D7
#define BuzzPin NODEMCU_PIN_D3

#define UpButtonPin NODEMCU_PIN_D3
#define DownButtonPin NODEMCU_PIN_D4

#elif BOARD == BrewShield
// default
#define oneWirePin NODEMCU_PIN_D6  // If oneWirePin is specified, beerSensorPin and fridgeSensorPin are ignored
#define coolingPin NODEMCU_PIN_D5
#define heatingPin NODEMCU_PIN_D7
#if AUTO_CAP
#define doorPin    NODEMCU_PIN_D8
#define BuzzPin NODEMCU_PIN_D0
#else
#define doorPin    NODEMCU_PIN_D4
#define BuzzPin NODEMCU_PIN_D0
#endif

#define UpButtonPin NODEMCU_PIN_D3
#define DownButtonPin NODEMCU_PIN_D4

#else
#error "unknown board"
#endif


#if BREWPI_LCD
// LCD configurations:
#if OLED_LCD
#define BREWPI_OLED128x64_LCD 1
#else
#define BREWPI_IIC_LCD 1
#endif
#endif

#define IIC_LCD_ADDRESS 0x27
#define LCD_AUTO_ADDRESSING true

#ifdef BREWPI_OLED128x64_LCD
#define OLED128x64_LCD_ADDRESS 0x3c
#define STATUS_LINE 1
//////////////////////////////////////////////////////////////////////////
//
// OLED orientation
// 1: flipScreenVertically() will be called on init,
//    resulting in a 180° rotation. This is the default.
// 0: flipScreenVertically() will be omitted
//
#ifndef OLED128x64_LCD_ORIENTATION
#define OLED128x64_LCD_ORIENTATION 1
#endif
/////////////////////////////////////////////////////////////////////////

#endif //BREWPI_OLED128x64_LCD


//#if BREWPI_ROTARY_ENCODER  || BREWPI_BUTTONS
//#define BACKLIGHT_AUTO_OFF_PERIOD 180
//#else
#define BACKLIGHT_AUTO_OFF_PERIOD 0 // disabled
//#endif
// Pay attention when changing the pins for the rotary encoder.
// They should be connected to external interrupt INT0, INT1 and INT3
//#define rotaryAPin 2 // INT1
//#define rotaryBPin 1 // INT3
//#define rotarySwitchPin 0 // INT2


#if BREWPI_ROTARY_ENCODER

#define RotaryViaPCF8574 1

#ifdef RotaryViaPCF8574

#define rotaryAPin 0
#define rotaryBPin 1
#define rotarySwitchPin 2

#define PCF8574_INT NODEMCU_PIN_D3
#define PCF8574_ADDRESS 0x20

#else // #ifdef RotaryViaPCF8574

#error "invalid setting"
#define rotaryAPin NODEMCU_PIN_D3
#define rotaryBPin NODEMCU_PIN_D7
#define rotarySwitchPin NODEMCU_PIN_D4

#endif //#ifdef RotaryViaPCF8574

#endif //BREWPI_ROTARY_ENCODER


#if ButtonViaPCF8574
#define PCF8574_INT NODEMCU_PIN_D3
#define PCF8574_ADDRESS 0x20
// use the same setting as BrewManiacEx
#define UpButtonBitMask   2  
#define DownButtonBitMask  1

#endif //#if ButtonViaPCF8574

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

//#ifdef EnableGlycolSupport
#define FridgeSensorFallBack true
#define SettableMinimumCoolTime true
//#endif

#define EMIWorkaround 1
#define BPL_VERSION "3.4"

#ifndef EanbleParasiteTempControl
#define EanbleParasiteTempControl 0
#endif

/**************************************************************************************/
/*  Configuration: 																	  */
/*  Only one setting: the serial used to connect to.                                  */
/*   if SoftwareSerial is used. RX/TX PIN must be defined.                            */
/*   else, UART0(Serial) is used.                                                     */
/**************************************************************************************/

//#define SerialDebug false

#if SerialDebug == true
#define DebugPort Serial
#define DBG_PRINTF(...) DebugPort.printf(__VA_ARGS__)
#define DBG_PRINT(...) DebugPort.print(__VA_ARGS__)
#else
#define DBG_PRINTF(...)
#define DBG_PRINT(...)
#endif

#define ENABLE_LOGGING 1

/**************************************************************************************/
/*  Advanced Configuration:  														  */
/*   URLs .										  									  */
/**************************************************************************************/

#define EnableGravitySchedule true

#define MINIMUM_TEMPERATURE_STEP 0.005
#define MINIMUM_TEMPERATURE_SETTING_PERIOD 60
#if SONOFF

#ifdef NO_SPIFFS
#define DEVELOPMENT_OTA true
#else
#define DEVELOPMENT_OTA false
#endif

#define DEVELOPMENT_FILEMANAGER false
#else
#define DEVELOPMENT_OTA true
#define DEVELOPMENT_FILEMANAGER true
#endif

// for web interface update
#define UPDATE_SERVER_PORT 8008
#define FILE_MANAGEMENT_PATH "/filemanager"
#define SYSTEM_UPDATE_PATH "/systemupdate"

#define DEFAULT_PAGE_TITLE "BrewPiLess"
#define DEFAULT_HOSTNAME "brewpiless"
#define DEFAULT_USERNAME "brewpiless"
#define DEFAULT_PASSWORD "brewpiless"


#ifndef WebPageLanguage
#define WebPageLanguage english
#endif

#define ClassicFrontEnd 0
#define TomsFrontEnd 1

#ifndef UseNewFrontEnd
#define FrontEnd ClassicFrontEnd
#else
#define FrontEnd TomsFrontEnd
#endif