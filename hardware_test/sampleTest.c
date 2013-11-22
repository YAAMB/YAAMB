#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <lcd.h>
#include <i2cmaster.h>
#define F_CPU 16000000UL
#include <util/delay.h>

//device address of MAX517 Nr.1 0101100x, see datasheet
#define DevMAX517  0x58

void setDAC(const uint8_t value)
{
    i2c_start(DevMAX517+I2C_WRITE);     // set device address and write mode
    i2c_write(0x00);
    i2c_write(value);					//write DAC value
    i2c_stop();                         // set stop conditon = release bus
}

ISR(TIMER1_COMPA_vect) //10kHz Abtastrate
{
//	static uint8_t p;
	uint8_t x =ADCH-128;		//ADC einlesen
	ADCSRA|= _BV(ADSC); //neue ADC Wandlung starten

//	int32_t temp=	(14*x(p)+28*x(p-1)+44*x(p-2)+44*x(p-3)+28*x(p-4)+14*x(p-5) \
					+68* y(p-1) - 112*y(p-2) + 16*y(p-3) - 15* y(p-4) -1 *y(p-5));
//	y(p)=temp/128;
	setDAC(x+128);		//DAC setzen
}


int sampleTest(void)
{
    lcd_clrscr();
    lcd_puts_P("Sample Test\nChannel 0     S2");

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
	uint8_t mpind= PIND;
    while(PIND & _BV(PD3))  /* main event loop */
    {
    	if(mpind & _BV(PD1) & ~(mpind=PIND))
   		{
    		ADMUX ^= _BV(MUX0);
    		lcd_gotoxy(8,1);
    		if( ADMUX & _BV(MUX0))
    			lcd_putc('1');
    		else
    			lcd_putc('0');
    	}
    	//mpind= PIND;
    	//_delay_ms(10);
    }

	cli();
	TIMSK = 0;
	TCCR1B = 0;
	ADCSRA= 0;
	ADMUX= 0;
	return 0;
}

