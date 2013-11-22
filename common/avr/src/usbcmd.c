#include "usbcmd.h"


static CallbackArray usbRead;
static CallbackArray usbWrite;
static struct messageBuffer usbMessage;

void dummyFunc(char* buffer, int size){}

void initCallbackArray(CallbackArray* carray)
{
	carray->calls= malloc(ALLOCSIZE*sizeof(commandFunc));
	carray->number= 0;
	carray->allocatedMemory= ALLOCSIZE;
}

void usbCmdInit(void)
{
	/* We fake an USB disconnect by pulling D+ and D- to 0 during reset. This is
     * necessary if we had a watchdog reset or brownout reset to notify the host
     * that it should re-enumerate the device. Otherwise the host's and device's
     * concept of the device-ID would be out of sync.
     */
    DDRB = 0xFF;          /* output SE0 for faked USB disconnect */

    DDRB |= USBMASK;
    PORTB &= ~USBMASK;
    uint8_t i = 0;
    while (--i)         /* fake USB disconnect for > 500 ms */
    {
        wdt_reset();
        _delay_ms(2);
    }
    DDRB  &= ~USBMASK;    /* all outputs except USB data */

	usbInit();
	usbMessage.size=0;
	usbMessage.data= NULL;

	initCallbackArray(&usbRead);
	initCallbackArray(&usbWrite);
}

int defineCommand(unsigned int command, commandFunc call, CallbackArray* carray)
{
	if(command >= carray->allocatedMemory)
	{
		carray->allocatedMemory= (command/ALLOCSIZE+1) *ALLOCSIZE;
		commandFunc* dummy= malloc(carray->allocatedMemory*sizeof(commandFunc));  //ToDo: Überprüfung einfügen
		memcpy(dummy, carray->calls, carray->number*sizeof(commandFunc));
		free(carray->calls);
		carray->calls= dummy;
	}
	for(int i=carray->number; i< command; ++i)					//evtl. vorhanden Zwischenplätze werden mit dummys aufgefült.
		carray->calls[i]= dummyFunc;
	carray->calls[command]= call;
	if( carray->number < command+1) carray->number= command+1;
	return 1;
}
int defineReadCommand(unsigned int command, commandFunc call)
{
	return defineCommand(command, call, &usbRead);
}
int defineWriteCommand(unsigned int command, commandFunc call)
{
	return defineCommand(command, call, &usbWrite);
}

USB_PUBLIC uchar usbFunctionSetup(uchar data[8])
{
    usbRequest_t    *rq = (void *)data;
    if(rq->bRequest == 0x10)
    {
    	if(usbMessage.size != rq->wLength.word)
    	{
    		usbMessage.size=rq->wLength.word;
    		//free(usbMessage.data);
    		usbMessage.data= realloc(usbMessage.data, rq->wLength.word);
    	}
    	usbMessage.count= rq->wLength.word;
    	usbMessage.command= rq->wValue.word;

    	return 0xFF;
    }
	return 0;
}
uchar usbFunctionRead(uchar *data, uchar len)
{
    if(usbMessage.count < len)
    	return 0xff;
    if(usbMessage.count == usbMessage.size && usbMessage.command < usbRead.number)
    	(usbRead.calls[usbMessage.command])(usbMessage.data, usbMessage.size);
    int i;
    for(i=0; i <len; ++i)
    {
        data[i]= usbMessage.data[i+(usbMessage.size-usbMessage.count)];
    }
    usbMessage.count-=len;
    return len;
}
uchar usbFunctionWrite(uchar *data, uchar len)
{
    if(usbMessage.count < len)
    	return 0xff;
    int i;
    for(i=0; i <len; ++i)
    {
    	usbMessage.data[i+(usbMessage.size-usbMessage.count)] =data[i];
    }
    usbMessage.count-=len;
    if(!usbMessage.count && usbMessage.command < usbWrite.number)
    	(usbWrite.calls[usbMessage.command])(usbMessage.data, usbMessage.size);
    return len;
}

