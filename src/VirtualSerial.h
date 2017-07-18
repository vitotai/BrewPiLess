#ifndef VirtualSerial_H
#define VirtualSerial_H

#include <Arduino.h>

class QueueBuffer
{
protected:
	char* _buffer;
	int _writePtr;
	int _readPtr;
	int  _bufferSize;
public:
	QueueBuffer(int size){_bufferSize=size; _buffer=(char*)malloc(size);  _writePtr=_readPtr=0;}
	~QueueBuffer(void){ free(_buffer);}

	void print(char c);
	void print(const char* c);
    void println(){ print('\n');}

	int read(void);
	int available(void);
};

#endif
