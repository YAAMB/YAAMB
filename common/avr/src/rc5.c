#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "rc5.h"

static volatile uint16_t rc5_code = 0;
static volatile uint8_t rc5_first_half;

uint8_t new_rc5_data(void)
{
    return rc5_code;
}

uint16_t get_rc5_code(void)
{
    uint16_t temp=rc5_code;
    rc5_code=0;
    return temp;
}

ISR (INT0_vect)
{
    /*	external interrupt handler
    	edge from IR receiver detected.
    	this is assumed to be the middle of a bit.
    */
    uint8_t i;

    // resample to filter out spikes
    if (MCUCR & (1<<ISC00))
    {
        // we were waiting for a rising edge, so cancel if we sample low
        for (i = RC5RESAMPLE; i; i--) if (!(PIND & (1<<PIND2))) return;
    }
    else
    {
        // vice versa...
        for (i = RC5RESAMPLE; i; i--) if ((PIND & (1<<PIND2))) return;
    }

    TCNT0 = RC5TIMERSECOND; // preset timer to sample (ovf int) at second half (3/4 bit time)
    rc5_first_half = 0; // next sample will be in the second half
    TIFR |= _BV(TOV0);  //manually clear timer interrupt
    TIMSK |= _BV(TOIE0); // sbi(TIMSK, TOIE0); // enable timer ovf int
    GICR &= ~_BV(INT0); //cbi(GICR, INT0); // disable ext int (this handler)
}

ISR (TIMER0_OVF_vect)
{
    /*	timer overflow handler
    	sample the present level of the interrupt line.
    */

    static uint16_t rc5_shift, rc5_temp;
    static uint8_t rc5_bit = 0, level, first_level = 1;
    uint8_t i = 0, temp;

    // resample to filter out spikes
    for (temp = RC5RESAMPLE; temp; temp--) if (PIND & (1<<PIND2)) i++;
    level = (i > ((uint8_t) (RC5RESAMPLE/2)));

    if (rc5_first_half == 2)
    {
        // this is a timeout, cancel operation
        goto rc5_cancel;
    }
    else if (rc5_first_half == 1)
    {
        // this sample is taken in the first half of the bit (1/4 bit time)
        first_level = level; // save current level
        if (level)
            MCUCR &= ~_BV(ISC00); //cbi(MCUCR, ISC10); // currently at high level, wait for falling edge
        else
            MCUCR |= _BV(ISC00); //sbi(MCUCR, ISC10); // currently at low level, wait for rising edge
        rc5_first_half = 2; // special code, means: next ovf int is a timeout
        GICR |= _BV(INT0); //sbi(GICR, INT0); // much better: wait for edge ;)
        TCNT0 = RC5TIMERCANCEL; // at timer ovf (in around 1/2 bit time) we should get a timeout
    }
    else
    {
        // this sample is taken in the second half of the bit (3/4 bit time)
        TCNT0 = RC5TIMERFIRST; // next sample at first half (1/4 bit time) of next bit
        rc5_first_half = 1;
        if (first_level != level)
        {
            // levels differ -> valid manchester encoded bit detected
            if (!rc5_bit)
            {
                // this is the first bit, reset values
                rc5_temp = 0;
                rc5_shift = 0x8000;
            }
            if (!level) rc5_temp |= rc5_shift; // low level (low-active) so store a '1'
            else if (rc5_bit < 2) goto rc5_cancel; // high level ('0' bit) as startbit -> error
            if (++rc5_bit == 14)
            {
                // reception complete, stop operation
                rc5_code = rc5_temp;
                goto rc5_cancel;
            }
            if (rc5_shift == 0x0100) rc5_shift = 0x0020;
            else rc5_shift >>= 1;
        }
        else
        {
            // error in manchester stream -> cancel operation
            goto rc5_cancel;
        }
    }
    return;

rc5_cancel:
    MCUCR &= ~_BV(ISC00); //cbi(MCUCR, ISC10); // falling edge
    GICR |= _BV(INT0); //sbi(GICR, INT0); // ext. int. enabled
    GIFR |= _BV(INTF0); // clear an eventually set ext. int. flag
    TIMSK &= ~_BV(TOIE0); //cbi(TIMSK, TOIE0); // disable timer int.
    first_level = 1; // as the next ext. int. appears at the falling edge, assume the first level as high
    rc5_bit = 0;
}


void rc5_init(void)
{
    /*	init for ext. int. and timer.
    	call this once to activate the rc5 decoder.
    */
    TCCR0 = TIMERCR;
    DDRD &= ~_BV(PD2);    //cbi(DDRD, PD3); // pin is input
    PORTD &= ~_BV(PD2);   //cbi(PORTD, PD3); // pullup disabled (receiver has defined output)
    //PORTD |= _BV(PD2);
    TIMSK &= ~_BV(TOIE0); //cbi(TIMSK, TOIE0); // timer ovf disabled. gets enabled by ext. int.
    MCUCR |= _BV(ISC01);  //sbi(MCUCR, ISC11); // ext. int. activated by falling edge
    MCUCR &= ~_BV(ISC00);  //cbi(MCUCR, ISC10);
    GIFR = _BV(INTF0); // clear ext. int. flag
    GICR |= _BV(INT0); //sbi(GICR, INT0); // enable ext. int.
}



// EOF
