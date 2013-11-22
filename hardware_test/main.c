/*!
  \file main.c
  \brief Hautprogrammschleife des Hardware Tests
  \defgroup hwtest Hardware Test
  Dieses Programm dient der Inbetriebnahme des Motherboards um den korrekten Aufbau zu testen

  ACHTUNG! Die verwendete Hardware Revision ist im Makefile unter "CDEFS" einzutragen.
  Beispiel für Rev 1.0: "CDEFS = -DREV1" oder "CDEFS = -DREV2" für Rev 2.0
*/
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <lcd.h>
#include <i2cmaster.h>
#include <rc5.h>
#include <ps2.h>
#include <usbdrv.h>
#define F_CPU 16000000UL
#include <util/delay.h>

//device address of MAX517 Nr.1 0101100x, see datasheet
#define DevMAX517  0x58

const char PROGMEM text0[]="1 LED\nTest starten S2";
const char PROGMEM text1[]="2 Fototransistor\nTest starten S2";
const char PROGMEM text2[]="3 Tso17 + Servo\nTest starten S2";
const char PROGMEM text3[]="4 Leistungsaus.\nTest starten S2";
const char PROGMEM text4[]="5 Sample\nTest starten S2";
const char PROGMEM text5[]="6 Temperaturs.\nTest starten S2";
const char PROGMEM text6[]="7 PS2\nTest starten S2";
const char PROGMEM text7[]="8 USB\nTest starten S2";


void rc5test(void);
void sampleTest(void);
void lm334Test(void);

const char* textarray[]= { text0, text1, text2, text3, text4, text5, text6, text7 };


void LEDtest(void)
{
	lcd_clrscr();
	lcd_puts_P("LED Test\nPeriodendauer 1s");
	DDRA |= _BV(PA4) | _BV(PA5);
	PORTA |= _BV(PA4);
	uint8_t i=50;
	while(PIND & _BV(PD3))
	{
		i--;
		_delay_ms(10);
		if(i==0)
		{
			PORTA ^=  _BV(PA4) | _BV(PA5);
			i = 50;
		}
	}
	DDRA &= ~_BV(PA4) & ~_BV(PA5);
	PORTA&= ~_BV(PA4) & ~_BV(PA5);
}


void FototransistorTest(void)
{
	lcd_clrscr();

	lcd_puts_P("Fototrans. Test\nWert:");

	ADCSRA = _BV(ADEN) | _BV(ADSC) |  _BV(ADATE) | 7;
	ADMUX= _BV(REFS0) +1;
	//SFIOR &= ~(_BV(ADTS2) | _BV(ADTS1) | _BV(ADTS0));
	char buffer[5];
	uint16_t d=0, d2=0;
	while(PIND & _BV(PD3))
	{
		while(~ADCSRA & _BV(ADIF));
		ADCSRA |= _BV(ADIF);
		d= ADCL;
		d+= ADCH<<8;
		d2= (999*d2 + d)/1000;
		dtostrf(d, 4, 0, buffer);
		lcd_gotoxy(5,1);
		lcd_puts(buffer);
	}
	ADCSRA= 0;
	ADMUX=0;
}

void potiTest(void)
{
	lcd_clrscr();
	lcd_puts_P("Poti\nWert:");

	ADCSRA = _BV(ADEN) | _BV(ADPS0) | _BV(ADPS2) | _BV(ADATE) | _BV(ADSC);
	ADMUX = (_BV(REFS0) | _BV(REFS1) | _BV(ADLAR)) + 2;
	//SFIOR &= ~(_BV(ADTS2) | _BV(ADTS1) | _BV(ADTS0));
	//Fast PWM Mode, clear OC0 on compare match,Prescaler=256 -> clk=62,5kHz
	TCCR0 = _BV(WGM00) | _BV(COM01) |_BV(CS00);
	char buffer[6];
	while(PIND & _BV(PD3))
	{
		while(~ADCSRA & _BV(ADIF));
		ADCSRA |= _BV(ADIF);
		OCR0= ADCH;
		dtostrf(ADCH, 4, 0, buffer);
		lcd_gotoxy(5,1);
		lcd_puts(buffer);
	}
	ADCSRA = 0;
	ADMUX = 0;
	TCCR0 = 0;
	PORTB &= ~_BV(PD3);
}
void ps2Test(void)
{
	static const char PROGMEM text[]= "PS2 Test\n";
	lcd_clrscr();
	lcd_puts_p(text);
	lcd_command(LCD_DISP_ON_CURSOR_BLINK);
	uint8_t i=0;
	ps2_init();
	sei();
	while(PIND &_BV(PD3))
	{
		uint8_t c=ps2_getc();
		if(c)
		{
			lcd_putc(c);
			i++;
			if(i >= 16)
			{
				i= 0;
				lcd_clrscr();
				lcd_puts_p(text);
			}
		}

	}
	cli();
	UCSRA =0;
	UCSRB =0;
	UCSRC = 0x86;
	lcd_command(LCD_DISP_ON);
}

void usbTest(void)
{
    lcd_clrscr();
	lcd_puts_P("USB Test\nSOF packets:");
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
    sei();
    while(PIND &_BV(PD3))
	{
		usbPoll();
	}
	cli();
	DDRB &= ~USBMASK;
}
USB_PUBLIC uchar usbFunctionSetup(uchar data[8])
{
	return 0;
}

int main(void)
{
    lcd_init(LCD_DISP_ON);
    //lcd_command(LCD_DISP_ON_CURSOR_BLINK);
    lcd_clrscr();
    lcd_puts_P("Hadware Tests\n");
    lcd_puts_P("S1 drueken.");
//	i2c_init();     // init I2C interface
//    sei();			//enable global interrupts

	PORTD |= _BV(PD3) | _BV(PD1);
	DDRB |= _BV(PB3);  // PB3 Ausgang
	PORTB &= ~_BV(PB3);
	uint8_t i=0, i2=4;

    loop_until_bit_is_clear(PIND, PD3);
    uint8_t mpind=PIND;
    for (;;)    // main event loop /
    {
		if(~PIND & mpind & _BV(PD3))
		{
			++i;
		}
		if(i != i2)
		{
			lcd_clrscr();
			lcd_puts_p(textarray[i]);
			i2= i;
		}
    	if(~PIND & mpind & _BV(PD1))
    	{
    		 switch (i)
    		 {
    		 	case 0:
    		 		LEDtest();
    		 		break;
    		 	case 1:
    		 		FototransistorTest();
    		 		break;
    		 	case 2:
    		 		rc5test();
    		 		break;
    		 	case 3:
    		 		potiTest();
    		 		break;
    		 	case 4:
    		 		sampleTest();
    		 		break;
    		 	case 5:
    		 		lm334Test();
    		 		break;
    		 	case 6:
    		 		ps2Test();
    		 		break;
    		 	case 7:
    		 		usbTest();
    		 		break;
    		 }
    		 i++;
    	}
    	if(i > 7)	i=0;
    	mpind= PIND;
    	_delay_ms(10);
    }
    return 0;
}

