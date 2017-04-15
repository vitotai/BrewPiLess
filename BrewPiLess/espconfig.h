#ifndef ESPCONFIG_H
#define ESPCONFIG_H
#include "Config.h"
/**************************************************************************************/
/*  Configuration: 																	  */
/*  Only one setting: the serial used to connect to.                                  */
/*   if SoftwareSerial is used. RX/TX PIN must be defined.                            */
/*   else, UART0(Serial) is used.                                                     */
/**************************************************************************************/

#define SerialDebug false

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
#define DEVELOPMENT_OTA false
#define DEVELOPMENT_FILEMANAGER false
#else
#define DEVELOPMENT_OTA true
#define DEVELOPMENT_FILEMANAGER true
#endif

// for web interface update
#define UPDATE_SERVER_PORT 8008
#define FILE_MANAGEMENT_PATH "/filemanager"
#define SYSTEM_UPDATE_PATH "/systemupdate"

// don't change this.
#define MAX_PROFILE_LEN 1024
#define PROFILE_JSON_BUFFER_SIZE 1024
#endif