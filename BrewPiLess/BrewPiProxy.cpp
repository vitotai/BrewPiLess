#include "BrewPiProxy.h"
#include "VirtualSerial.h"

#include "Brewpi.h"
#include <stdarg.h>

#include "stddef.h"
#include "PiLink.h"

#include "Version.h"
#include "TemperatureFormats.h"
#include "TempControl.h"
#include "Display.h"
#include "JsonKeys.h"
#include "Ticks.h"
#include "Brewpi.h"
#include "EepromManager.h"
#include "EepromFormat.h"
#include "SettingsManager.h"
#include "Buzzer.h"
#include "Display.h"

QueueBuffer brewPiRxBuffer(2048);
QueueBuffer brewPiTxBuffer(2048);


void BrewPiProxy::write(char ch)
{
	brewPiRxBuffer.print(ch);
}

void BrewPiProxy::putLine(const char* str)
{
	brewPiRxBuffer.print(str);
	brewPiRxBuffer.print('\n');
}

void BrewPiProxy::begin(void (*readString)(const char*))
{
	_readString=readString;
}

void BrewPiProxy::loop(void)
{
	while(brewPiTxBuffer.available()){
		char ch=brewPiTxBuffer.read();
		 if(ch == '\n'){
			_lastLineBuff[_readPtr]='\0';
			memcpy(_lastLineBuff,_buff,_readPtr);
			_readPtr=0;
			(*_readString)(_lastLineBuff);
		}
		else
		if(ch == 0xB0){
			//for safty
			if(_readPtr < (BUFF_SIZE-2)){
				_buff[_readPtr++]=0xC2;
				_buff[_readPtr++]=0xB0;
			}
		 }
		 else 
			 _buff[_readPtr++]=ch;
		if( _readPtr >= BUFF_SIZE) _readPtr=0; // just drop.
	}	
}

void BrewPiProxy::getTemperature(float *pBeerTemp,float *pBeerSet,float *pFridgeTemp, float *pFridgeSet)
{
	*pBeerTemp=temperatureFloatValue(tempControl.getBeerTemp());
	*pBeerSet=temperatureFloatValue(tempControl.getBeerSetting());
	*pFridgeTemp = temperatureFloatValue(tempControl.getFridgeTemp());
	*pFridgeSet = temperatureFloatValue(tempControl.getFridgeSetting());
}

void BrewPiProxy::getControlParameter(char *pUnit,char *pMode,float *pBeerSet, float *pFridgeSet)
{
	*pUnit=tempControl.cc.tempFormat;
	*pMode=tempControl.cs.mode;
	*pBeerSet=temperatureFloatValue(tempControl.getBeerSetting());
	*pFridgeSet=temperatureFloatValue(tempControl.getFridgeSetting());

}

void BrewPiProxy::getTemperatureSetting(char *pUnit,float *pMinSetTemp,float *pMaxSetTemp)
{
	*pUnit=tempControl.cc.tempFormat;
	*pMinSetTemp=temperatureFloatValue(tempControl.cc.tempSettingMin);
	*pMaxSetTemp=temperatureFloatValue(tempControl.cc.tempSettingMax);
}

void BrewPiProxy::getLogInfo(char *pUnit,uint8_t *pMode,uint8_t *pState)
{
	*pUnit=tempControl.cc.tempFormat;
	*pState = (uint8_t) tempControl.getState();
	*pMode = (uint8_t) tempControl.getMode();
}

void BrewPiProxy::getAllStatus(uint8_t *pState,uint8_t *pMode,float *pBeerTemp,float *pBeerSet,float *pFridgeTemp, float *pFridgeSet, float *pRoomTemp)
{
	*pBeerTemp=temperatureFloatValue(tempControl.getBeerTemp());
	*pBeerSet=temperatureFloatValue(tempControl.getBeerSetting());
	*pFridgeTemp = temperatureFloatValue(tempControl.getFridgeTemp());
	*pFridgeSet = temperatureFloatValue(tempControl.getFridgeSetting());
	*pRoomTemp =temperatureFloatValue(tempControl.getRoomTemp());
	*pState = (uint8_t) tempControl.getState();
	*pMode = (uint8_t) tempControl.getMode();
}

bool BrewPiProxy::ambientSensorConnected(void)
{
	return tempControl.ambientSensor->isConnected();
}











































































































































































































































































































































