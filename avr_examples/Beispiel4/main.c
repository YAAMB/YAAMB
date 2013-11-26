/*************************************************

  Beispiel 4
  Servocontroller für 4 Servos über eine
  RC5 Fernbedienung

**************************************************/
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "rc5.h"
#define F_CPU 16000000UL
#include <util/delay.h>


volatile uint8_t servo_pos[4];
#define MIN_PULSE 250

ISR(TIMER2_COMP_vect)
{
  static uint8_t state=0;
  static int16_t repeat=250*20; //20ms
  switch(state)
    {
    case 0:
      PORTA |= _BV(4);
      OCR2=MIN_PULSE;   //wait 1ms
      break;
    case 1:
      OCR2=servo_pos[0];
      break;
    case 2:
      PORTA ^= _BV(4) | _BV(5);
      OCR2=MIN_PULSE;
      break;
    case 3:
      OCR2=servo_pos[1];
      break;
    case 4:
      PORTA ^= _BV(5) | _BV(6);
      OCR2=MIN_PULSE;
      break;
    case 5:
      OCR2=servo_pos[2];
      break;
    case 6:
      PORTA ^= _BV(6) | _BV(7);
      OCR2=MIN_PULSE;
      break;
    case 7:
      OCR2=servo_pos[3];
      break;
    case 8:
      PORTA ^= _BV(7);
      OCR2=100;
      break;
    default:
      if (repeat<0)
        {
          state=0xFF;
          repeat=250*19;
        }
    }
  state++;
  repeat-=OCR2;
}
int main(void)
{
  lcd_init(LCD_DISP_ON);
  lcd_puts_P("Beispiel 4 v0.2");
  _delay_ms(1000);

  //Port PA4-PA7 are outputs
  DDRA = 0xF0;

  //Timer2 konfigurieren
  TCNT2=0;
  OCR2=100;
  //CTC = Clear Timer on Compare match S.80
  //Normal port operation, OC2 disconnected
  //Prescaler=64 -> clk=250kHz
  TCCR2 = _BV(WGM21) | _BV(CS22);
  //On Compare match Interrupt Enable for timer 2
  TIMSK |= _BV(OCIE2);

  rc5_init(); // activate RC5 detection

  //Init servo_pos
  uint8_t i;
  for (i=0; i<4; ++i)
    servo_pos[i]=120; //middle pos

  //enable global interrupts
  sei();

  for(;;)    /* main event loop */
    {
      if (new_rc5_data())
        {

          char buf[17];
          uint16_t temp=get_rc5_code();

          if ( (temp&0xFF)==0x10 )
            {
              servo_pos[0]+=2;
              servo_pos[1]+=2;
              servo_pos[2]+=2;
              servo_pos[3]+=2;
            }
          else if ( (temp&0xFF)==0x11 )
            {
              servo_pos[0]-=2;
              servo_pos[1]-=2;
              servo_pos[2]-=2;
              servo_pos[3]-=2;
            }
          else if ( (temp&0xFF)==0x01 )
            {
              servo_pos[0]=25;
              servo_pos[1]=25;
              servo_pos[2]=25;
              servo_pos[3]=25;
            }
          else if ( (temp&0xFF)==0x02 )
            {
              servo_pos[0]=200;
              servo_pos[1]=200;
              servo_pos[2]=200;
              servo_pos[3]=200;
            }
          lcd_gotoxy(0,1);
          lcd_puts_P("S: ");
          itoa(servo_pos[0],buf,10);
          uint8_t j=3;
          while(buf[j] && j)
            {
              j--;
              lcd_putc(' ');
            }
          lcd_puts(buf);

          lcd_gotoxy(8,1);
          lcd_puts_P("C: ");
          itoa(temp,buf,16);
          lcd_puts(buf);

        }

    }
}

