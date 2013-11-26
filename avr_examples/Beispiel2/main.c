/*************************************************

  Beispiel 2
  Eine Commandline für den AVR.
  Bisher sind folgene Kommandos definiert:
    calc ein einfacher Taschenrechner.
    moo  Gibt einen Text aus.
    version Gibt die Version aus.
  Siehe auch cmdline.c

**************************************************/
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "cmdline.h"

int main(void)
{
  lcd_init(LCD_DISP_ON);
  lcd_command(LCD_DISP_ON_CURSOR_BLINK);
  lcd_clrscr();
  lcd_puts_P("Beispiel 2 v0.1");

  DDRA = _BV(PA4);

  cmdline_init();

  sei();

  for (;;)    /* main event loop */
    {
      process_cmdline();
    }
  return 0;
}
