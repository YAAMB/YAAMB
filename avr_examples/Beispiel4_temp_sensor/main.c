/*************************************************

  Beispiel 4
  Temperatursensor LM334 an ADC0.
  Jumper1 Stellung beachten !!!
  Wandlung in °C und Anzeige auf dem Display
  Pollender ADC Zugriff

  ADC=227µV/K/270R * 8,2K *1023/2.56* t[K]
  temp=ADC*3.61-2731.6;

  ADC= (5V - 227µV/K/270R * t[k] * 12KR) * 1023/2.56

  Autor: Andreas Weber 13.12.2007

  verwendet:
    * LCD zur Ausgabe der Temperatur
    * analog-digital Wandler
    * Temperatursensor LM334

**************************************************/
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "lcd.h"
#define F_CPU 16000000UL
#include <util/delay.h>

volatile uint16_t ICPcounts;

ISR(TIMER1_CAPT_vect)
{
  ICPcounts++;
}


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

int main(void)
{
  char myTemp[10];

  lcd_init(LCD_DISP_ON);
  lcd_clrscr();
  //lcd_puts_P("Bsp 4 v0.1 Temp.\n");
  //lcd_puts_P("Temp =");


  //INPUT CAPTURE PIN IPC
  /*Noise canceller, rising edge , without prescaler*/
  TCCR1B= _BV(ICNC1) | _BV(CS10) | _BV(CS01);
  //enable input capture interrupts
  TIMSK= _BV(TICIE1);


  //ADC PRescaler 128 = 125Khz ADC Clock (langsamste Wandlung)
  ADCSRA = _BV(ADEN) | _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2) | _BV(ADSC);
  //ADCSRA = _BV(ADEN) | _BV(ADSC);
  //Interne 2.56V Reference verwenden
  //Multiplexer auf Kanal0 (LM334)
  ADMUX = (_BV(REFS0) | _BV(REFS1)) + 6;

  sei();      //enable global interrupts

  float temp=0;
  uint8_t i=0;
  uint8_t n=0;

  /* main event loop */
  for (;;)
    {
      ADMUX = (_BV(REFS0) | _BV(REFS1)) + 6;    //Multiplexer auf Kanal 6
      ADCSRA |=_BV(ADSC);             //Start conversion
      loop_until_bit_is_set(ADCSRA,ADIF);     //wait for end of conversion
      //temp=(879-ADC)*2.44;
      temp=ADC*3.61-2731.6;   //OLD sensor on board ch 1
      lcd_gotoxy(14,0);
      lcd_putc(0xDF);               //0xDF is the degree symbol "°"
      lcd_putc('C');
      lcd_gotoxy(13,0);
      lcd_put_temp((int16_t)temp);        //Show upper temp

      ADMUX = (_BV(REFS0) | _BV(REFS1)) + 7;    //Multiplexer auf Kanal 7
      ADCSRA |=_BV(ADSC);             //Start conversion
      loop_until_bit_is_set(ADCSRA,ADIF);     //wait for end of conversion
      temp=(879-ADC)*2.44;
      lcd_gotoxy(14,1);
      lcd_putc(0xDF);               //0xDF is the degree symbol "°"
      lcd_putc('C');
      lcd_gotoxy(13,1);
      lcd_put_temp((int16_t)temp);        //Show bottom temp


      utoa(ICPcounts, myTemp, 10);
      lcd_gotoxy(0,0);
      lcd_puts(myTemp);
      while(i--)_delay_ms(2);
    }
  return 0;
}
