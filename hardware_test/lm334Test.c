/*************************************************

	Beispiel 4
	Temperatursensor LM334 an ADC0.
	Jumper1 Stellung beachten !!!
	Wandlung in Â°C und Anzeige auf dem Display
	Pollender ADC Zugriff

	ADC=227µV/K/270R * 8,2K *1023/2.56* t[K]

	Autor: Andreas Weber 13.12.2007
	Geändert: Stefan Staiger 07.01.2008
	Äanderung
    ADC= ( 5V - 227µV/K/270R * t[K] * 12K) *1023/2.56

	Für Rev2.0, Geändert: Stefan Staiger 06.05.2009
    ADC= ( 227(µV/K)/100R * t[K] * 2700R ) * 1023/2.56

**************************************************/
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <lcd.h>
#define F_CPU 16000000UL
#include <util/delay.h>

void lcd_put_temp(int16_t v)
{
#define LEN 3
#define SEP 1

	lcd_command(LCD_ENTRY_DEC);
	uint8_t sign=v<0;
	if (sign)
		v=-v;
	uint8_t i=LEN;
	while(i--)
	{
		if (v || i)
		{
			lcd_putc('0'+v%10);
			v/=10;
		}
		else
			lcd_putc(' ');
		if (i==LEN-SEP) lcd_putc(',');
	}
	if(sign)
		lcd_putc('-');
	else
		lcd_putc('+');

	lcd_command(LCD_ENTRY_INC_);
}

void lm334Test(void)
{
    lcd_clrscr();
    lcd_puts_P("Thermometer\n");
    lcd_puts_P("Temp =");

	//ADC PRescaler 128 = 125Khz ADC Clock (langsamste Wandlung)
	ADCSRA = _BV(ADEN) | _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2) | _BV(ADSC);
	//Interne 2.56V Reference verwenden
	//Multiplexer auf Kanal0 (LM334)
	ADMUX = (_BV(REFS0) | _BV(REFS1)) + 0;

    //sei();			//enable global interrupts

	float temp=0;
	uint8_t i=1;
#ifdef REV2
	char buffer[10];
#endif

    while(PIND & _BV(PD3))    /* main event loop */
    {
		if(i--)
		{
			ADCSRA |=_BV(ADSC); 				//Start conversion
			loop_until_bit_is_set(ADCSRA,ADIF);	//wait for end of conversion
			ADCSRA |= _BV(ADIF);
#ifdef REV1
			temp=(879-(int16_t)ADC)*2.44;
#endif
#ifdef REV2
			temp=(ADC*0.4079)-273;
#endif
			lcd_gotoxy(14,1);
			lcd_putc(0xDF);
			lcd_putc('C');
			lcd_gotoxy(13,1);
#ifdef REV1
			lcd_put_temp((int16_t)temp);
#endif
#ifdef REV2
			dtostrf(temp, 6, 1,buffer);
			lcd_gotoxy(8,1);
			lcd_puts(buffer);
			lcd_put_temp((int16_t)temp);
#endif

		}
    }
    ADCSRA=0;
    ADMUX=0;
}
