#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

#include "usbdrv.h"
#include "oddebug.h"
#include "lcd.h"
#include "rc5.h"
#include "ps2.h"
#include "cmdline.h"
#include <util/delay.h>

#define EEPROM_LOCATION 37

/*
verwendet:
  * Software-USB
  * RC5 (Infrarot-Fernbedienung) basierend auf http://www.markh.de/software/rc5dec.c
  * PS2 (AT-Tastatur)
  * commandline, calculator
*/

static uchar    actionTimers[8];

static void outputByte(uchar b)
{
    PORTA =  (PORTA & ~(_BV(4) | _BV(5))) | (b & (_BV(4) | _BV(5)));
    PORTB =  (PORTB & ~(_BV(3)))  | (b & _BV(3));
}

static void eepromWrite(unsigned char addr, unsigned char val)
{
    while (EECR & (1 << EEWE));
    EEARL = addr;
    EEDR = val;
    cli();
    EECR |= 1 << EEMWE;
    EECR |= 1 << EEWE;  /* must follow within a couple of cycles -- therefore cli() */
    sei();
}

static uchar    eepromRead(uchar addr)
{
    while (EECR & (1 << EEWE));
    EEARL = addr;
    EECR |= 1 << EERE;
    return EEDR;
}

static uchar    computeTemporaryChanges(void)
{
    uchar   i, status = 0, mask = 1;

    for (i=0;i<8;i++)
    {
        if (actionTimers[i])
            status |= mask;
        mask <<= 1;
    }
    return status;
}

static void computeOutputStatus(void)
{
    uchar   status = eepromRead(EEPROM_LOCATION) ^ computeTemporaryChanges();

    outputByte(status);
}

/* We poll for the timer interrupt instead of declaring an interrupt handler
 * with global interrupts enabled. This saves recursion depth. Our timer does
 * not need high precision and does not run at a high rate anyway.
 */
static void timerInterrupt(void)
{
    static uchar    prescaler;
    uchar           i;

    if (!prescaler--)
    {
        prescaler = 8;  /* rate = 12M / 1024 * 256 * 9 */
        for (i=0;i<8;i++)
        {
            if (actionTimers[i])
                actionTimers[i]--;
        }
        computeOutputStatus();
    }
}

static int16_t s_test;

USB_PUBLIC uchar usbFunctionSetup(uchar data[8])
{
    return 0xFF;
}
uchar usbFunctionRead(uchar *data, uchar len)
{
    uchar* p= &s_test;
    if (len >=2)
    {
        s_test++;
        data[0]= p[0];
        data[1]= p[1];
        return len;
    }
    return 0xff;
}
uchar usbFunctionWrite(uchar *data, uchar len)
{
    char buffer[11];
    uchar* p= &s_test;
    if (len >=2)
    {
        p[0]=data[0];
        p[1]=data[1];
        lcd_clrscr();
        itoa( s_test , buffer, 10);
        lcd_puts(buffer);
        return len;
    }
    return 0xff;
}

/* allow some inter-device compatibility */
#if !defined TCCR0 && defined TCCR0B
#define TCCR0   TCCR0B
#endif
#if !defined TIFR && defined TIFR0
#define TIFR    TIFR0
#endif

int main(void)
{
    unsigned char i;

    DDRD &=~ (1 << PD1);        /* Pin PD1 input              */
    PORTD |= (1 << PD1);        /* Pin PD1 pull-up enabled    */

    lcd_init(LCD_DISP_ON);
    lcd_command(LCD_DISP_ON_CURSOR_BLINK);
    lcd_clrscr();
    lcd_puts_P("Beispiel 1 v0.1");

    wdt_enable(WDTO_1S);
    odDebugInit();
    //DDRD = 0;
    DDRA = _BV(4) | _BV(5); //gelb LEDs

    //PORTD = 0;
    PORTB = 0;          /* no pullups on USB pins */
    /* We fake an USB disconnect by pulling D+ and D- to 0 during reset. This is
     * necessary if we had a watchdog reset or brownout reset to notify the host
     * that it should re-enumerate the device. Otherwise the host's and device's
     * concept of the device-ID would be out of sync.
     */
    DDRB = 0xFF;          /* output SE0 for faked USB disconnect */
    computeOutputStatus();  /* set output status before we do the delay */
    i = 0;
    while (--i)         /* fake USB disconnect for > 500 ms */
    {
        wdt_reset();
        _delay_ms(2);
    }
    DDRB = ~USBMASK;    /* all outputs except USB data */
    TCCR0 = 5;          /* set prescaler to 1/1024 */
    usbInit();
    rc5_init(); // activate RC5 detection
    cmdline_init();
    sei();

    for (;;)    /* main event loop */
    {
        PORTA ^= _BV(5);
        wdt_reset();
        usbPoll();
        if (TIFR & (1 << TOV0))
        {
            TIFR |= 1 << TOV0;  /* clear pending flag */
            timerInterrupt();
        }
        if (new_rc5_data())
        {
            lcd_gotoxy(0,0);
            char buf[16];
            itoa(get_rc5_code(),buf,2);
            lcd_puts(buf);
        }
	process_cmdline();

    }
    return 0;
}
