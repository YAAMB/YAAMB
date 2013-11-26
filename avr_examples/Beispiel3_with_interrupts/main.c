/*************************************************

	Beispiel 3_with_interrupts
	Tiefpaß 5.Ordnung Chebyshev type II IIR Filter
	siehe Artikel "Octave"

	Es ist möglich die Filterrate bis etwa 13kHz zu steigern,
	indem der DAC Wandler schneller als erlaubt mit 600kHz
	betrieben wird. Die Genauigkeit von 8bit ist dann nicht mehr
	garantiert.

**************************************************/
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "lcd.h"
#include "peripheral_devices.h"

volatile int8_t x_buf[16];
volatile int8_t y_buf[16];

#define x(p) x_buf[(p) & 0x0F]
#define y(p) y_buf[(p) & 0x0F]

ISR(TIMER1_COMPA_vect) //10kHz Abtastrate
{
  static uint8_t p;

  x(p)=ADCH-128;		//ADC einlesen
  ADCSRA|= _BV(ADSC); //neue ADC Wandlung starten

  int16_t temp=	(14*x(p)+28*x(p-1)+44*x(p-2)+44*x(p-3)+28*x(p-4)+14*x(p-5) \
                 +68* y(p-1) - 112*y(p-2) + 16*y(p-3) - 15* y(p-4) -1 *y(p-5));
  y(p)=temp/128;
  set_onboard_DAC(y(p++)+128);		//DAC setzen
}


int main(void)
{
  lcd_init(LCD_DISP_ON);
  lcd_command(LCD_DISP_ON_CURSOR_BLINK);
  lcd_clrscr();
  lcd_puts_P("Bsp. 3 INT  v0.3\n");
  lcd_puts_P("TP ChebyII 5.Ord\n");

  //Timer 1
  TCCR1A = 0;
  TCCR1B = _BV(CS11) |_BV(WGM12); //prescaler=8, clear on match
  OCR1A = 199;
  TIMSK = _BV(OCIE1A);

  ADCSRA = _BV(ADEN) | _BV(ADPS0) | _BV(ADPS2); //ADC PRescaler 32 = 500Khz ADC Clock
  ADMUX = _BV(REFS0) | _BV(ADLAR); 	/*wir wollen nur 8bits auswerten, daher left adjusted*/
  ADCSRA|= _BV(ADSC); //ein mal laufen lassen

  periphery_init();
  sei();			//enable global interrupts

  for (;;)    /* main event loop */
    {
      process_periphery();
    }

  return 0;
}
