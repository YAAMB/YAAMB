/*************************************************

	Beispiel 3
	Tiefpaß 5.Ordnung Chebyshev type II IIR Filter
	siehe Artikel "Octave"
    verwendet:

    * analog-digital Wandler
    * digital-analog Wandler über TWI/I²C

**************************************************/
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include "lcd.h"
#include "i2cmaster.h"
#define F_CPU 16000000UL
#include <util/delay.h>
#include <util/twi.h>

//device address of MAX517 Nr.1 0101100x, see datasheet
#define DevMAX517  0x58

volatile int8_t x_buf[16];
volatile int8_t y_buf[16];

#define x(p) x_buf[(p) & 0x0F]
#define y(p) y_buf[(p) & 0x0F]

void setDAC(const uint8_t value)
{
    i2c_start(DevMAX517+I2C_WRITE);     // set device address and write mode
    i2c_write(0x00);
    i2c_write(value);					//write DAC value
    i2c_stop();                         // set stop conditon = release bus
}

ISR(TIMER1_COMPA_vect) //5kHz Abtastrate
{
	static uint8_t p;
	x(p)=ADCH-128;		//ADC einlesen
	ADCSRA|= _BV(ADSC); //neue ADC Wandlung starten

	int32_t temp=	(14*x(p)+28*x(p-1)+44*x(p-2)+44*x(p-3)+28*x(p-4)+14*x(p-5) \
					+68* y(p-1) - 112*y(p-2) + 16*y(p-3) - 15* y(p-4) -1 *y(p-5));
	y(p)=temp/128;
	setDAC(y(p++)+128);		//DAC setzen
}


int main(void)
{
    lcd_init(LCD_DISP_ON);
    lcd_command(LCD_DISP_ON_CURSOR_BLINK);
    lcd_clrscr();
    lcd_puts_P("Beispiel 3  v0.2\n");
    lcd_puts_P("TP ChebyII 5.Ord\n");
	i2c_init();     // init I2C interface

	//Timer 1
	TCCR1A = 0;
	TCCR1B = _BV(CS11) |_BV(WGM12); //prescaler=8, clear on match
	OCR1A = 399;
	TIMSK = _BV(OCIE1A);

	ADCSRA = _BV(ADEN) | _BV(ADPS0) | _BV(ADPS2); //ADC PRescaler 32 = 500Khz ADC Clock
	ADMUX = _BV(REFS0) | _BV(ADLAR); 	/*wir wollen nur 8bits auswerten, daher left adjusted*/
	ADCSRA|= _BV(ADSC); //ein mal laufen lassen

	i2c_init();     // init I2C interface

    sei();			//enable global interrupts

    for (;;);    /* main event loop */
    return 0;
}
