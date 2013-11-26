/*************************************************

	Beispiel 3
	Gibt einen Sinus auf dem DAC aus.
	Die Frequenz kann mit dem Potentiometer an ADC2 verändert werden.
	(Jumper Settings beachten!)

	Man kann aus Jux auch mal auf Kanal 1
	(Phototransistor) wechseln ;-)
    verwendet:

    * analog-digital Wandler
    * digital-analog Wandler über TWI/I²C

**************************************************/
#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "lcd.h"
#include "i2cmaster.h"
#define F_CPU 16000000UL
#include <util/delay.h>

//device address of MAX517 Nr.1 0101100x, see datasheet
#define DevMAX517  0x58

uint8_t const PROGMEM sin_list[]= {55,65,74,83,90,97,101,104,105,104,101,97,90,83,74,65,55,45,36,27,20,13,9,6,5,6,9,13,20,27,36,45};

void setDAC(const uint8_t value)
{
  i2c_start(DevMAX517+I2C_WRITE);       // set device address and write mode
  i2c_write(0x00);
  i2c_write(value);					//write DAC Value
  i2c_stop();                         // set stop conditon = release bus
}

int main(void)
{
  lcd_init(LCD_DISP_ON);
  lcd_clrscr();
  lcd_puts_P("Beispiel 3  v0.1\n");
  lcd_puts_P("Sinus ueber DAC\n");
  i2c_init();     // init I2C interface

  //ADC PRescaler 32 = 500Khz ADC Clock, AutoTrigger
  ADCSRA = _BV(ADEN) | _BV(ADPS0) | _BV(ADPS2) | _BV(ADATE) | _BV(ADSC);
  //Interne 2.56V Reference verwenden
  //Multiplexer auf Kanal2 (Poti)
  //nur 8bit, left adjusted
  ADMUX = (_BV(REFS0) | _BV(REFS1) | _BV(ADLAR)) + 2;
  //ADC in Free Running mode
  SFIOR &= ~(_BV(ADTS2) | _BV(ADTS1) | _BV(ADTS0));

  //sei();			//enable global interrupts

  uint8_t i=0;
  uint16_t t=0;
  for (;;)    /* main event loop */
    {
      setDAC(pgm_read_byte(&sin_list[i & 0x1F]));
      t=ADCH;
      while(t--) _delay_us(5);
      i++;
    }
  return 0;
}
