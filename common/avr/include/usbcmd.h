/**************************************************************************************************

	File: usbcmd.h



	Bug: Bei dem Empfangen von 255 + n*256 tritt ein Fehler auf.


**************************************************************************************************/

#ifndef USBCMD_H
#define USBCMD_H

#include <stdlib.h>
#include <avr/pgmspace.h>
#include "usbdrv.h"
//#include "oddebug.h"
#include "string.h"
#include <avr/wdt.h>
#include <util/delay.h>


#define ALLOCSIZE 4
typedef void (*commandFunc)(char* buffer, int size);

typedef struct _CallbackArray
{
	commandFunc* calls;
	unsigned int number;
	unsigned int allocatedMemory;
} CallbackArray;


struct 	messageBuffer
{
	char* data;
	unsigned int size;
	unsigned int count;
	unsigned int command;
};

void initCallbackArray(CallbackArray* carray);

void usbCmdInit(void);

int defineCommand(unsigned int command, commandFunc call, CallbackArray* carray);
int defineReadCommand(unsigned int command, commandFunc call);
int defineWriteCommand(unsigned int command, commandFunc call);

//USB_PUBLIC uchar usbFunctionSetup(uchar data[8]);
//uchar usbFunctionRead(uchar *data, uchar len);
//uchar usbFunctionWrite(uchar *data, uchar len);

#endif
