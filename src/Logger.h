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

#pragma once

#include <stdarg.h>
#include "TemperatureFormats.h"
#include "LogMessages.h"

// define error id variable type to make it easy to bump to uint16 when needed
#define LOG_ID_TYPE uint8_t

class Logger{
	public:
	Logger(){};
	~Logger(){};

	static void logMessageVaArg(const char type, LOG_ID_TYPE errorID, const char * varTypes, ...);
};
extern Logger logger;


#if BREWPI_LOG_ERRORS
	inline void logError(uint8_t debugId){
		logger.logMessageVaArg('E', debugId, "");
	}
	inline void logErrorInt(uint8_t debugId, int val){
		logger.logMessageVaArg('E', debugId, "d", val);
	}
	inline void logErrorString(uint8_t debugId, const char * val){
		logger.logMessageVaArg('E', debugId, "s", val);
	}
	inline void logErrorTemp(uint8_t debugId, temperature temp){
		logger.logMessageVaArg('E', debugId, "t", temp);
	}
	inline void logErrorIntInt(uint8_t debugId, int val1, int val2){
		logger.logMessageVaArg('E', debugId, "dd", val1, val2);
	}
	inline void logErrorIntIntInt(uint8_t debugId, int val1, int val2, int val3){
		logger.logMessageVaArg('E', debugId, "ddd", val1, val2, val3);
	}
#else
	#define logError(debugId) {}
	#define logErrorInt(debugId, val) {}
	#define logErrorString(debugId, val) {}
	#define logErrorTemp(debugId, temp) {}
	#define logErrorIntInt(debugId, val1, val2)	{}
	#define logErrorIntIntInt(debugId, val1, val2, val3) {}
#endif

#if BREWPI_LOG_WARNINGS
	inline void logWarning(uint8_t debugId){
		logger.logMessageVaArg('W', debugId, "");
	}
	inline void logWarningInt(uint8_t debugId, int val){
		logger.logMessageVaArg('W', debugId, "d", val);
	}
	inline void logWarningString(uint8_t debugId, const char * val){
		logger.logMessageVaArg('W', debugId, "s", val);
	}
	inline void logWarningTemp(uint8_t debugId, temperature temp){
		logger.logMessageVaArg('W', debugId, "t", temp);
	}
	inline void logWarningIntString(uint8_t debugId, int val1, const char *val2){
		logger.logMessageVaArg('W', debugId, "ds", val1, val2);
	}
#else
	#define logWarning(debugId) {}
	#define logWarningInt(debugId, val) {}
	#define logWarningString(debugId, val) {}
	#define logWarningTemp(debugId, temp) {}
	#define logWarningIntString(debugId, val1, val2) {}
#endif

#if BREWPI_LOG_INFO
		inline void logInfo(uint8_t debugId){
			logger.logMessageVaArg('I', debugId, "");
		}
		inline void logInfoInt(uint8_t debugId, int val){
			logger.logMessageVaArg('I', debugId, "d", val);
		}
		inline void logInfoString(uint8_t debugId, const char * val){
			logger.logMessageVaArg('I', debugId, "s", val);
		}
		inline void logInfoTemp(uint8_t debugId, temperature temp){
			logger.logMessageVaArg('I', debugId, "t", temp);
		}
		inline void logInfoIntString(uint8_t debugId, int val1, const char * val2){
			logger.logMessageVaArg('I', debugId, "ds", val1, val2);
		}
		inline void logInfoStringString(uint8_t debugId, const char * val1, const char * val2){
			logger.logMessageVaArg('I', debugId, "ss", val1, val2);
		}
		inline void logInfoIntStringTemp(uint8_t debugId, int val1, const char * val2, temperature val3){
			logger.logMessageVaArg('I', debugId, "dst", val1, val2, val3);
		}
		inline void logInfoTempTempFixedFixed(uint8_t debugId, temperature t1, temperature t2, temperature f1, temperature f2){
			logger.logMessageVaArg('I', debugId, "ttff", t1, t2, f1, f2);
		}
#else
	#define logInfo(debugId) {}
	#define logInfoInt(debugId, val) {}
	#define logInfoString(debugId, val) {}
	#define logInfoTemp(debugId, temp) {}
	#define logInfoStringString(debugId, val1, val2) {}
	#define logInfoIntString(debugId, val1, val2) {}
	#define logInfoIntStringTemp(debugId, val1, val2, val3) {}


#endif

#if BREWPI_LOG_DEBUG
	#include "PiLink.h"
	#define logDebug(string, ...) piLink.debugMessage(PSTR(string), ##__VA_ARGS__)
#else
	#define logDebug(string, ...) {}
#endif
