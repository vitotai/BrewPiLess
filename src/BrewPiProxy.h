#ifndef BrewPiProxy_H
#define BrewPiProxy_H

#include <Arduino.h>
#include "Config.h"

#define BUFF_SIZE 1024

#define LCD_CMD 'l'


class BrewPiProxy{
public:
	BrewPiProxy(void):_unit('C'),_lastLineLength(0),_readPtr(0){_lastLineBuff[0]='\0';}
	void begin(void (*readString)(const char*));

	void loop(void);
	void write(char ch);

	void putLine(const char* str);

	char* getLastLine(void){return _lastLineBuff;}
	float getBeerTemp(void);
	float getBeerSet(void);
	float getFridgeTemp(void);
	float getFridgeSet(void);
	char  getUnit(void);
	float getMinSetTemp(void);
	float getMaxSetTemp(void);
	char  getMode(void);
	uint8_t getState(void);
	float getRoomTemp(void);
	uint32_t getStatusTime(void);

	bool ambientSensorConnected(void);

	void setMode(char mode);
	void setBeerSet(float temp);
	void setFridgetSet(float temp);
protected:
	char _unit;
	char _lastLineBuff[BUFF_SIZE];
	int  _lastLineLength;
	char _buff[BUFF_SIZE];
	int   _readPtr;

	void (*_readString)(const char*);
};
extern BrewPiProxy brewPi;
#endif
