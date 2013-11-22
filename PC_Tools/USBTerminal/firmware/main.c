#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#define F_CPU 16000000UL
#include <util/delay.h>

#include "usbbuf.h"
#include "lcd.h"

int main(void)
{
    lcd_init(LCD_DISP_ON);
    lcd_clrscr();

	usbBufInit();

	//enable global interrupts
    sei();

	usb_puts_P("Hallo, dies ist eine Test.\n");
    for (;;)    /* main event loop */
    {
    	char data= usb_getc();
    	if(data)
    	{
    		lcd_putc(data);
    		usb_putc(data);
    	}
   		usbPoll();
     }
    return 0;
}

