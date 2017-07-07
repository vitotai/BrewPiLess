#include "VirtualSerial.h"

int QueueBuffer::read(void)
{
	if(_writePtr == _readPtr) return -1;
	int r=(int)_buffer[_readPtr];
	_readPtr++;
	if(_readPtr == _bufferSize) _readPtr=0;
	return r;
}

int QueueBuffer::available(void)
{
	 // avoid using %(mod) which takes time;
	return (_writePtr >= _readPtr)? (_writePtr - _readPtr):(_bufferSize  + _writePtr - _readPtr);
}

void QueueBuffer::print(char c)
{
	_buffer[_writePtr] = c;
	_writePtr++;
	if(_writePtr == _bufferSize) _writePtr=0;
//	if(_writePtr == _readPtr){
	//		DBGPRINT("Fatal Error: queue buffer overlap");
//	}
}

void QueueBuffer::print(const char* c)
{
	for(char* cp=(char*)c; *cp != '\0';cp++)
	{
		print(*cp);
	}
}
