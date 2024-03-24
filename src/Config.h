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
 #define BREWPI_DS2413 true
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
// Enable humidity sensor
//
#ifndef EnableDHTSensorSupport
#define EnableDHTSensorSupport true
#endif
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//
// Enable humidity sensor
//
#ifndef EnableBME280Support
#define EnableBME280Support true
#endif
//
//////////////////////////////////////////////////////////////////////////
#ifndef EnableHumidityControlSupport
#define EnableHumidityControlSupport true
#endif

#if EnableHumidityControlSupport
#if !EnableBME280Support && !EnableDHTSensorSupport
#error "Humidity Sesonr is needed to support Humidity Control"
#endif
#else
#undef EnableDHTSensorSupport 
#define EnableDHTSensorSupport false
#undef EnableBME280Support 
#define EnableBME280Support false

#endif

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
#if ESP32
#define BREWPI_ROTARY_ENCODER 1
#else
#define BREWPI_ROTARY_ENCODER 0
#endif
#endif
//
//////////////////////////////////////////////////////////////////////////

// default supports 2 buttons
#ifndef BREWPI_BUTTONS
#if ESP8266
#define BREWPI_BUTTONS 1
#else
#define BREWPI_BUTTONS 0
#endif
#endif

#ifndef ButtonViaPCF8574 
#define ButtonViaPCF8574 0
#endif

#ifndef AUTO_CAP
#define  AUTO_CAP true
#endif

#ifndef PressureViaADS1115
#define PressureViaADS1115 true
#define ADS1115_ADDRESS 0x48
#define ADS1115_Transducer_ADC_NO 0
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
// BrewPiLess feature optoins
// #endif
//
//////////////////////////////////////////////////////////////////////////
//values of Front-end
#define ClassicFrontEnd 0
#define TomsFrontEnd 1

// default language
#ifndef WebPageLanguage
#define WebPageLanguage english
#endif


#ifndef UseClassicFrontEnd
#define FrontEnd TomsFrontEnd
#else
#define FrontEnd ClassicFrontEnd
#endif


#ifndef EanbleParasiteTempControl
#define EanbleParasiteTempControl true
#endif

#ifndef SupportPressureTransducer
#define SupportPressureTransducer true
#endif

#ifndef SupportMqttRemoteControl
#define SupportMqttRemoteControl true
#endif

#ifndef AUTO_CAP
#define  AUTO_CAP true
#endif


#ifndef DEVELOPMENT_OTA
#define DEVELOPMENT_OTA true
#endif

#define SaveWiFiConfiguration true

#ifndef DEVELOPMENT_FILEMANAGER
#define DEVELOPMENT_FILEMANAGER true
#endif

#define EnableGravitySchedule true
#define ENABLE_LOGGING 1
#define EARLY_DISPLAY 1

//#ifdef EnableGlycolSupport
//#define FridgeSensorFallBack true
// always true #define SettableMinimumCoolTime true
//#endif

#ifndef UseLittleFS

#define UseLittleFS true

#endif

#if ESP32
#define FS_EEPROM true
#endif

#define BPL_VERSION "4.4"


#ifndef MORE_PINS_CONFIGURATION
#define MORE_PINS_CONFIGURATION true
#endif

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
#ifdef ESP32

#define PIN_SDA 21
#define PIN_SCL 22


#define oneWirePin    23

#define actuatorPin1  16
#define actuatorPin2  17
#define actuatorPin3  19
#define actuatorPin4  27
#define actuatorPin5  26

#if MORE_PINS_CONFIGURATION

#define actuatorPin6  18

#define fanPin 14
#define doorPin 34

#define BuzzPin 4

#else
#define BuzzPin       18
#endif
// 34,35,66,39 input only
#define rotaryAPin      32
#define rotaryBPin      33
#define rotarySwitchPin 25

// Only ADC1 (pin 32~39) is allowed 
#define PressureAdcPin  36

#else // #ifdef ESP32
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

#if SONOFF_USE_AM2301
// free up pin 14, so that it can be assigned to AM2301
#define oneWirePin NODEMCU_PIN_D7  
#define doorPin    NODEMCU_PIN_D5

#else
#define oneWirePin NODEMCU_PIN_D5  // If oneWirePin is specified, beerSensorPin and fridgeSensorPin are ignored
#define doorPin    NODEMCU_PIN_D7
#endif

#define coolingPin NODEMCU_PIN_D6
#define heatingPin NODEMCU_PIN_D0
#define BuzzPin NODEMCU_PIN_D3
/*
// NO LCD, NO BUTTONs
#ifdef BREWPI_LCD
#undef BREWPI_LCD 
#endif
#define BREWPI_LCD false
#undef BREWPI_MENU
#define BREWPI_MENU 0
#undef  BREWPI_BUTTONS 
#define  BREWPI_BUTTONS 0

//overwrite feature set
#undef EanbleParasiteTempControl
#define EanbleParasiteTempControl flase
#undef SupportPressureTransducer
//#define SupportPressureTransducer false
#undef SupportMqttRemoteControl
#define SupportMqttRemoteControl false
#undef AUTO_CAP
#define  AUTO_CAP false

#undef UseLittleFS 
#define UseLittleFS false

#ifdef NO_SPIFFS
    #undef DEVELOPMENT_OTA 
    #define DEVELOPMENT_OTA true
    #undef DEVELOPMENT_FILEMANAGER
    #define DEVELOPMENT_FILEMANAGER false
#else
    #undef DEVELOPMENT_OTA 
    #define DEVELOPMENT_OTA false
    #undef DEVELOPMENT_FILEMANAGER
    #define DEVELOPMENT_FILEMANAGER false
#endif

*/

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

#endif // #ifdef ESP32

#if BREWPI_LCD
// LCD configurations:
#define EMIWorkaround 1

#if OLED_LCD
#define BREWPI_OLED128x64_LCD 1
#else
#define BREWPI_IIC_LCD 1
#endif
#endif

#ifndef TWOFACED_LCD
#define TWOFACED_LCD true
#endif

#ifndef ISPINDEL_DISPLAY
#if OLED_LCD
#define ISPINDEL_DISPLAY true
#else
#define ISPINDEL_DISPLAY false
#endif
#endif

#if TWOFACED_LCD
    #define SMART_DISPLAY true
    #if ISPINDEL_DISPLAY
        #if ! OLED_LCD
        #error "ISPINDEL_DISPLAY is only available for OLED display"    
        #endif
    #endif
#endif

#define IIC_LCD_ADDRESS 0x27
#define LCD_AUTO_ADDRESSING true

#ifdef BREWPI_OLED128x64_LCD
#define OLED128x64_LCD_ADDRESS 0x3c
#define STATUS_LINE true
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


#ifdef ESP8266
#if ButtonViaPCF8574
#define PCF8574_INT NODEMCU_PIN_D3
#define PCF8574_ADDRESS 0x20
// use the same setting as BrewManiacEx
#define UpButtonBitMask   2  
#define DownButtonBitMask  1
#endif

#endif //#if ButtonViaPCF8574

#if defined(ESP8266) || defined(ESP32)
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


#if ESP32
#define SupportTiltHydrometer true
#define SupportBleHydrometer true
#define SupportPillHydrometer true
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

/**************************************************************************************/
/*  Advanced Configuration:  														  */
/*   URLs .										  									  */
/**************************************************************************************/


#define MINIMUM_TEMPERATURE_STEP 0.005
#define MINIMUM_TEMPERATURE_SETTING_PERIOD 60

// for web interface update
#define UPDATE_SERVER_PORT 8008
#define FILE_MANAGEMENT_PATH "/filemanager"
#define SYSTEM_UPDATE_PATH "/systemupdate"

#define DEFAULT_PAGE_TITLE "BrewPiLess"
#define DEFAULT_HOSTNAME "brewpiless"
#define DEFAULT_USERNAME "brewpiless"
#define DEFAULT_PASSWORD "brewpiless"


#if UseLittleFS
#if ESP32
#define FileSystem LittleFS
#else
#define FileSystem  LittleFS
#endif
#else
#define FileSystem SPIFFS
#endif


#if ESP32
// when read logs, ESP32, or AsyncTCP to be exact, would request 5623 bytes
// it seems stressful to SPIFFS. Using this option to read file in a small 
// portion, 1480. 
#define ReadFileByPortion true
#endif
