#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include "i2cmaster.h"
#define Dev24C16  0xA0      // device address of EEPROM 24C16, see datasheet

#include "usbbuf.h"
#include "lcd.h"

#define UART_BAUD_RATE 38400				//Baudrate sertzen

int main(void)
{
    lcd_init(LCD_DISP_ON);
    lcd_clrscr();

	UCSRB =  (1 << TXEN);			//Bit TXEN in UCR setzen
	UBRRL = (F_CPU / (UART_BAUD_RATE * 16L) -1); //Baud setzen
	i2c_init();

	//enable global interrupts
    //sei();

	uint8_t data;

	//usb_puts_P("start\n");
	//while(usb_getc() != 'a') usbPoll();
    //_delay_ms(2000);
    //for(;;)
		i2c_start_wait(Dev24C16+I2C_WRITE);      // set device address and write mode
		i2c_write(0x00);
		i2c_rep_start(Dev24C16+I2C_READ);        // set device address and read mode

		for (int i=0; i < 60*16; ++i)    /* main event loop */
		{
			data= i2c_readAck();
			while(!(UCSRA & (1<<UDRE))){}
			UDR= data;
		 }
     	i2c_readNak();
     	i2c_stop();
    return 0;
}

