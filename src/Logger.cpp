/*
 * Copyright 2012-2013 BrewPi/Elco Jacobs.
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

#include "Brewpi.h"
#include "BrewpiStrings.h"
#include "Logger.h"
#include "PiLink.h"
#include "TemperatureFormats.h"
#include "JsonKeys.h"

static const char PROGMEM LOG_STRING_FORMAT[] = "\"%s\"";

void Logger::logMessageVaArg(char type, LOG_ID_TYPE errorID, const char * varTypes, ...){
	va_list args;
	piLink.printResponse('D');
	piLink.sendJsonPair(JSONKEY_logType, type);
	piLink.sendJsonPair(JSONKEY_logID, errorID);
	piLink.print_P(PSTR(",\"V\":["));
	va_start (args, varTypes);
	uint8_t index = 0;
	char buf[9];
	while(varTypes[index]){
		switch(varTypes[index]){
			case 'd': // integer, signed or unsigned
				piLink.print_P(STR_FMT_D, va_arg(args, int));
				break;
			case 's': // string
				piLink.print_P(LOG_STRING_FORMAT, va_arg(args, char*));
				break;
			case 't': // temperature in fixed_7_9 format
				piLink.print_P(LOG_STRING_FORMAT, tempToString(buf, va_arg(args,int), 1, 12));
			break;
			case 'f': // fixed point value
				piLink.print_P(LOG_STRING_FORMAT, fixedPointToString(buf, (temperature) va_arg(args,int), 3, 12));
			break;
		}
		if(varTypes[++index]){
			piLink.print(',');
		}
	}
	va_end (args);
	piLink.print_P(PSTR("]"));
	piLink.sendJsonClose();
}

Logger logger;
