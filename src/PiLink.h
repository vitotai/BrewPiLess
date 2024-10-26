/*
 * Copyright 2012-2013 BrewPi/Elco Jacobs.
 * Copyright 2013 Matthew McGowan
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

#include "Brewpi.h"
#include "TemperatureFormats.h"
#include "DeviceManager.h"
#include "Logger.h"


#define PRINTF_BUFFER_SIZE 128

class DeviceConfig;


class PiLink{
	public:

	// There can only be one PiLink object, so functions are static
	static void init(void);
	static void receive(void);

#if 1 //#ifndef ESP8266 // There is a bug in the ESP8266 implementation that causes these not to work.
	static void printFridgeAnnotation(const char * annotation, ...);
	static void printBeerAnnotation(const char * annotation, ...);
#endif

	static void debugMessage(const char * message, ...);

	static void printTemperatures(void);

	typedef void (*ParseJsonCallback)(const char* key, const char* val, void* data);

	static void parseJson(ParseJsonCallback fn, void* data=NULL);

	static int read(void);  // Adding so we can completely abstract away piStream outside of piLink

	private:

	static void sendControlSettings(void);
	static void receiveControlConstants(void);
	static void sendControlConstants(void);
	static void sendControlVariables(void);

	static void receiveJson(void); // receive settings as JSON key:value pairs

	static void print(const char *fmt, ...); // use when format string is stored in RAM
#ifdef ARDUINO
	static void print(char c)       // inline for arduino
#if !defined(ESP8266) && !defined(ESP32)
	{ Serial.print(c); }
#else
		;
#endif
#endif

	static void test_functionality(void);
	static void print_P(const char *fmt, ...); // use when format string is stored in PROGMEM with PSTR("string")
	static void printNewLine(void);
	static void printChamberCount();
	static void printNibble(uint8_t n);

	private:
	static void soundAlarm(bool enabled);
	static void printResponse(char responseChar);
	static void printChamberInfo();

public:
	static void printTemperaturesJSON(const char * beerAnnotation, const char * fridgeAnnotation);
private:
	static void sendJsonPair(const char * name, const char * val); // send one JSON pair with a string value as name:val,
	static void sendJsonPair(const char * name, char val); // send one JSON pair with a char value as name:val,
	static void sendJsonPair(const char * name, uint16_t val); // send one JSON pair with a uint16_t value as name:val,
	static void sendJsonPair(const char * name, uint8_t val); // send one JSON pair with a uint8_t value as name:val,
	static void sendJsonAnnotation(const char* name, const char* annotation);
	static void sendJsonTemp(const char* name, temperature temp);

	static void processJsonPair(const char * key, const char * val, void* pv); // process one pair

	/* Prints the name part of a json name/value pair. The name must exist in PROGMEM */
	static void printJsonName(const char * name);
	static void printJsonSeparator();
	static void sendJsonClose();

	static void openListResponse(char type);
	static void closeListResponse();

	struct JsonOutput {
		const char* key;			// JSON key
		uint8_t offset;			// offset into TempControl class
		uint8_t handlerOffset;		// handler index
	};
	typedef void (*JsonOutputHandler)(const char* key, uint8_t offset);
	static void sendJsonValues(char responseType, const JsonOutput* /*PROGMEM*/ jsonOutputMap, uint8_t mapCount);


	// handler functions for JSON output
	static void jsonOutputUint8(const char* key, uint8_t offset);
	static void jsonOutputTempToString(const char* key, uint8_t offset);
	static void jsonOutputFixedPointToString(const char* key, uint8_t offset);
	static void jsonOutputTempDiffToString(const char* key, uint8_t offset);
	static void jsonOutputChar(const char* key, uint8_t offset);
	static void jsonOutputUint16(const char* key, uint8_t offset);
	static const JsonOutputHandler JsonOutputHandlers[];
	static const JsonOutput jsonOutputCCMap[];
	static const JsonOutput jsonOutputCVMap[];

	// Json parsing

	static void setMode(const char* val);
	static void setBeerSetting(const char* val);
	static void setFridgeSetting(const char* val);
	static void setTempFormat(const char* val);

	typedef void (*JsonParserHandlerFn)(const char* val, void* target);

	struct JsonParserConvert {
		const char* /*PROGMEM*/ key;
		void* target;
		JsonParserHandlerFn fn;
	};

	static const JsonParserConvert jsonParserConverters[];

#if BREWPI_SIMULATE
	static void updateInputs();
	public:
	static void printSimulatorSettings();
	private:
	static void sendJsonPair(const char* name, double val);
	static void printDouble(double val);
#endif

	private:
	static bool firstPair;
	friend class DeviceManager;
	friend class PiLinkTest;
	friend class Logger;
	static char printfBuff[PRINTF_BUFFER_SIZE];
#ifdef BUFFER_PILINK_PRINTS
	static String printBuf;
#endif
};

extern PiLink piLink;
