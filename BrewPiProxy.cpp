#include "BrewPiProxy.h"
#include "VirtualSerial.h"

#include "Brewpi.h"
#include <stdarg.h>

#include "stddef.h"
#include "PiLink.h"

#include "Version.h"
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
	*pBeerTemp=tempControl.getBeerTemp();
	*pBeerSet=tempControl.getBeerSetting();
	*pFridgeTemp = tempControl.getFridgeTemp();
	*pFridgeSet = tempControl.getFridgeSetting();
}

void BrewPiProxy::getControlParameter(char *pUnit,char *pMode,float *pBeerSet, float *pFridgeSet)
{
	*pUnit=tempControl.cc.tempFormat;
	*pMode=tempControl.cs.mode;
	*pBeerSet=tempControl.getBeerSetting();
	*pFridgeSet=tempControl.getFridgeSetting();

}



