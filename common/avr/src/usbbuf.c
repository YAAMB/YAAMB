/*************************************************************************
 Title	:   USB Buffer for AVR USB driver
 Author:    Raimund Lehmann
 File:	    usbbuf.c, 4.06.2008
 Software:  AVR-GCC 4.1
 Hardware:  any AVR device

 DESCRIPTION
 	Bibliothek um Daten byteweise über USB zu Senden oder zu Empfangen.
 	Dabei werden die Daten in den entsprechenden Buffern(RX und TX) zwischen
	gespeichert.
	Beim Senden vom AVR zum USB Master muss der Master die Daten
	zyklisch pollen, damit der TX_Buffer geleert wird.
***************************************************************************/

#include "usbbuf.h"

volatile char rx_buffer[USB_RX_BUFFER_SIZE];
volatile uint8_t rx_head=0;
volatile uint8_t rx_tail=0;
volatile char tx_buffer[USB_TX_BUFFER_SIZE];
volatile uint8_t tx_head=0;
volatile uint8_t tx_tail=0;

void usbBufInit(void)
{

   // USB disconnect

    DDRB |= USBMASK;
    PORTB &= ~USBMASK;
    uint8_t i = 0;
    while (--i)
    {
        _delay_ms(2);
    }
    DDRB  &= ~USBMASK;

	usbInit();
}

USB_PUBLIC uchar usbFunctionSetup(uchar data[8])
{
    //PORTA^=(1<<PA5);
	usbRequest_t    *rq = (void *)data;
    if(rq->bRequest == 0x10)
    {
    	return 0xFF;
    }
	return 0;
}
uchar usbFunctionRead(uchar *data, uchar len)
{

	if(len != 1)
    	return 0xff;

    if(tx_head != tx_tail)
	{
		char tmp= (tx_tail+1) % USB_TX_BUFFER_SIZE;
		*data= tx_buffer[tx_tail];
		tx_tail= tmp;
	}
	else *data=0;

    return len;
}
uchar usbFunctionWrite(uchar *data, uchar len)
{
    if(len != 1)
    	return 0xff;

	char tmp= (rx_head +1) % USB_RX_BUFFER_SIZE;
	if( tmp != rx_tail)
	{
		 rx_buffer[rx_head]= *data;
		 rx_head= tmp;
	}
    return len;
}

char usb_getc(void)
{
	if(rx_head != rx_tail)
	{
		char tmp= (rx_tail+1) % USB_RX_BUFFER_SIZE;
		char data= rx_buffer[rx_tail];
		rx_tail= tmp;
		return data;
	}
	else return 0;
}
void usb_inTXBuf(char data)
{
	char tmp= (tx_head +1) % USB_TX_BUFFER_SIZE;
	if( tmp != tx_tail)
	{
		 tx_buffer[tx_head]= data;
		 tx_head= tmp;
	}
}

void usb_putc(char data)
{
	while(!usb_wirteBufSpace())
	{
		//PORTA^=(1<<PA5);
		//_delay_ms(0.5);
		usbPoll();
	}
	usb_inTXBuf(data);
}

void usb_puts(const char* s)
{
	char c;
    while((c= *s++))
    {
		usb_putc(c);
    }
}

void usb_puts_p(const char *s)
{
    char c;
    while ((c = pgm_read_byte(s++)))
    {
        usb_putc(c);
    }
}

uint8_t usb_wirteBufSpace()
{
	int16_t tmp= (int16_t)tx_head- tx_tail;
	if ( tmp < 0)
		tmp+= USB_TX_BUFFER_SIZE;
	return USB_TX_BUFFER_SIZE-1-tmp;
}
