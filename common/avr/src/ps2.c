/*************************************************************************
 Title	:   AT-PS2 Keyboard library (ps2.c)
 Author:    Andreas Weber <info@tech-chat.de>
 File:	    ps2.c, 8.12.2007
 Software:  AVR-GCC 4.1
 Hardware:  Atmega32, perhaps any AVR device with USART

 DESCRIPTION
	Bibliothek um Daten von einer AT Tastatur über PS2 zu empfangen.
	Momentan wird nur lesen unterstützt, d.h. es ist nicht möglich den
	Scancode zu ändern oder die LEDs Power, Caps-Lock usw. anzusteuern.

	Die Fehlerbehandlung für FrameError, ParityError oder DataOverrun fehlt.

 USAGE
       See the C include ps2.h file for a description of each function

***************************************************************************/

#include <stdlib.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "ps2.h"

static volatile unsigned char PS2_RxBuf[PS2_RX_BUFFER_SIZE];
static volatile unsigned char PS2_RxHead;
static volatile unsigned char PS2_RxTail;
static volatile unsigned char PS2_LastRxError;

void ps2_init(void)
{
    DDRB &= ~_BV(PB0);	//XCK, USART External Clock
    DDRD &= ~_BV(PD0);  //Data

    PS2_RxHead=PS2_RxTail=0;
    state.shift=state.extended=state.release=state.vk_left=state.vk_up=state.vk_right=state.vk_down=0;
    PS2_LastRxError=0;

    UCSRA = 0;
    UCSRB = _BV(RXEN) | _BV(RXCIE);  //Receiver enable, Receive Complete Interrupt Enable
    //synchron, odd parity, RX falling edge, TX rising edge, 8bit
    UCSRC = _BV(URSEL) | _BV(UMSEL) |_BV(UPM0) | _BV(UPM1) | _BV(UCSZ1) | _BV(UCSZ0);
}

ISR(USART_RXC_vect)
{
    PORTA &= ~_BV(4);
	char debug_buffer[3];

    int8_t ascii_char=0;
    uint8_t lastRxError;

    if (UCSRA & (_BV(FE) | _BV(PE) | _BV(DOR))) //FrameError, Paritätsfehler oder Data-Overrun
        lastRxError=PS2_DATA_ERROR;

    uint8_t temp=UDR;
    switch (temp)
    {
    case 0xF0:
        state.release=1;
        return;
    case 0xE0:
        state.extended=(state.release)?0:1;
        state.release=0;
        return;
    case 0x12:
        state.shift=(state.release)?0:1;
        state.release=0;
        return;	//L Shift
    case 0x59:
        state.shift=(state.release)?0:1;
        state.release=0;
        return;	//R Shift

        //Extended wird nicht abgefragt, daher haben die KB2,KB4,KB6,KB8 gleiche Funktion wie Cursor
    case 0x75:
        state.vk_up=(state.release)?0:1;
        state.release=0;
        return;	//Up
    case 0x6b:
        state.vk_left=(state.release)?0:1;
        state.release=0;
        return;	//Left
    case 0x72:
        state.vk_down=(state.release)?0:1;
        state.release=0;
        return;	//Down
    case 0x74:
        state.vk_right=(state.release)?0:1;
        state.release=0;
        return;	//Right
    }
    if (!state.release)
        switch (temp)
        {
        case 0x5A:
            ascii_char='\r';
            break;
        case 0x66:	//Backspace
            ascii_char=0x08;
            break;
        case 0x71:	//Delete
            ascii_char=0x7F;
            break;
        case 0x29:
            ascii_char=' ';
            break;
        case 0x41:
            ascii_char=(state.shift)?';':',';
			break;
        case 0x5b:
            ascii_char=(state.shift)?'*':'+';
			break;
        case 0x4a:
            ascii_char=(state.shift)?'_':'-';
			break;
        case 0x49:
            ascii_char=(state.shift)?':':'.';
			break;
        case 0x1c:
            ascii_char=(state.shift)?'A':'a';
            break;
        case 0x32:
            ascii_char=(state.shift)?'B':'b';
            break;
        case 0x21:
            ascii_char=(state.shift)?'C':'c';
            break;
        case 0x23:
            ascii_char=(state.shift)?'D':'d';
            break;
        case 0x24:
            ascii_char=(state.shift)?'E':'e';
            break;
        case 0x2b:
            ascii_char=(state.shift)?'F':'f';
            break;
        case 0x34:
            ascii_char=(state.shift)?'G':'g';
            break;
        case 0x33:
            ascii_char=(state.shift)?'H':'h';
            break;
        case 0x43:
            ascii_char=(state.shift)?'I':'i';
            break;
        case 0x3b:
            ascii_char=(state.shift)?'J':'j';
            break;
        case 0x42:
            ascii_char=(state.shift)?'K':'k';
            break;
        case 0x4b:
            ascii_char=(state.shift)?'L':'l';
            break;
        case 0x3a:
            ascii_char=(state.shift)?'M':'m';
            break;
        case 0x31:
            ascii_char=(state.shift)?'N':'n';
            break;
        case 0x44:
            ascii_char=(state.shift)?'O':'o';
            break;
        case 0x4d:
            ascii_char=(state.shift)?'P':'p';
            break;
        case 0x15:
            ascii_char=(state.shift)?'Q':'q';
            break;
        case 0x2d:
            ascii_char=(state.shift)?'R':'r';
            break;
        case 0x1b:
            ascii_char=(state.shift)?'S':'s';
            break;
        case 0x2c:
            ascii_char=(state.shift)?'T':'t';
            break;
        case 0x3c:
            ascii_char=(state.shift)?'U':'u';
            break;
        case 0x2a:
            ascii_char=(state.shift)?'V':'v';
            break;
        case 0x1d:
            ascii_char=(state.shift)?'W':'w';
            break;
        case 0x22:
            ascii_char=(state.shift)?'X':'x';
            break;
        case 0x35:
            ascii_char=(state.shift)?'Z':'z';
            break;
        case 0x1a:
            ascii_char=(state.shift)?'Y':'y';
            break;
        case 0x45:
            ascii_char=(state.shift)?'=':'0';
            break;
        case 0x16:
            ascii_char=(state.shift)?'!':'1';
            break;
        case 0x1e:
            ascii_char=(state.shift)?'"':'2';
            break;
        case 0x26:
            ascii_char='3';
            break;	//Paragraph klappt nicht
        case 0x25:
            ascii_char=(state.shift)?'$':'4';
            break;
        case 0x2e:
            ascii_char=(state.shift)?'%':'5';
            break;
        case 0x36:
            ascii_char=(state.shift)?'&':'6';
            break;
        case 0x3d:
            ascii_char=(state.shift)?'/':'7';
            break;
        case 0x3e:
            ascii_char=(state.shift)?'(':'8';
            break;
        case 0x46:
            ascii_char=(state.shift)?')':'9';
            break;
		default:	//Für debugging

			itoa(temp,debug_buffer,16);
			//lcd_puts(debug_buffer);
        }

    state.release=0;
    if (ascii_char)	//Eine Taste wurde gedrückt
    {
        uint8_t tmphead = ( PS2_RxHead + 1) & PS2_RX_BUFFER_MASK; // calculate buffer index
        if ( tmphead == PS2_RxTail )
        {
            lastRxError |= PS2_BUFFER_OVERFLOW; // error: receive buffer overflow
        }
        else
        {
            PS2_RxHead = tmphead; // store new index
            PS2_RxBuf[tmphead] = ascii_char; // store received data in buffer
        }
    }
    PS2_LastRxError = lastRxError;
    PORTA |= _BV(4);
}

/*************************************************************************
Function: ps2_getc()
Purpose:  return byte from ringbuffer
Returns:  received byte from ringbuffer
**************************************************************************/
uint8_t ps2_getc(void)
{
    unsigned char tmptail;

    if ( PS2_RxHead == PS2_RxTail )
        return 0;   /* no data available */

    /* calculate /store buffer index */
    tmptail = (PS2_RxTail + 1) & PS2_RX_BUFFER_MASK;
    PS2_RxTail = tmptail;

    /* get data from receive buffer */
    return PS2_RxBuf[tmptail];
}/* uart_getc */


uint8_t ps2_getLastError(void)
{
	return PS2_LastRxError;
}
