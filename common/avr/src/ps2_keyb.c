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
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "ps2_keyb.h"

static volatile uint8_t PS2_RxBuf[PS2_RX_BUFFER_SIZE];
static volatile uint8_t PS2_RxHead;
static volatile uint8_t PS2_RxTail;
static volatile uint8_t PS2_LastRxError;
//+ AB
static volatile uint8_t PS2_ScBuf[PS2_RX_BUFFER_SIZE];
static volatile uint8_t PS2_ScHead;
static volatile uint8_t PS2_ScTail;
//- AB


void ps2_init(void)
{
    ps2_SetClkPortAsInput ();	//XCK, USART External Clock
    ps2_SetDataPortAsInput ();  //Data

    PS2_RxHead=PS2_RxTail=0;
    state.shift=state.extended=state.release=state.vk_left=state.vk_up=state.vk_right=state.vk_down=0;
    PS2_LastRxError=0;

    UCSRA = 0;
    UCSRB = _BV(RXEN) | _BV(RXCIE);  //Receiver enable, Receive Complete Interrupt Enable
    //synchron, odd parity, RX falling edge, TX rising edge, 8bit
    UCSRC = _BV(URSEL) | _BV(UMSEL) |_BV(UPM0) | _BV(UPM1) | _BV(UCSZ1) | _BV(UCSZ0);
//+ AB
    PS2_ScHead=PS2_ScTail=0;
//- AB
}

ISR(USART_RXC_vect)
{
    int8_t ascii_char=0;
    uint8_t lastRxError;

    if (UCSRA & (_BV(FE) | _BV(PE) | _BV(DOR))) //FrameError, Paritätsfehler oder Data-Overrun
        lastRxError=PS2_DATA_ERROR;

    uint8_t temp=UDR;
//+ AB
//	ascii_char=temp;
    if (temp & 0x80)	// breakcode of Key Set 1
    {
        state.release=1;
    }
//- AB
    switch (temp)
    {
    case 0xF0:	// breakcode of Key Set 2 and Set 3
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

//+ AB
    case 0x14:
        state.ctrl=(state.release)?0:1;
        state.release=0;
        return;	//L Ctrl (extended: R Ctrl)

    case 0x11:
        state.alt=(state.release)?0:1;
        state.release=0;
        return;	//L Alt (extended: R Alt)

    case 0x77:
        if (!state.release)
		{
			state.numLock=!(state.numLock);	// invert selection
			state.newLedStatus = 1;
		}
		state.release=0;
        return;	//Num Lock
    case 0x58:
        if (!state.release)
		{
			state.capsLock=!(state.capsLock);	// invert selection
			state.newLedStatus = 1;
		}
		state.release=0;
        return;	//Caps Lock
    case 0x7e:
        if (!state.release)
		{
			state.scrollLock=!(state.scrollLock);	// invert selection
			state.newLedStatus = 1;
		}
		state.release=0;
        return;	//Caps Lock
//- AB
    }

//+ AB
    if (!state.numLock)
	{
//- AB
		switch (temp)
		{
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
//+ AB
	}
//- AB
    if (!state.release)
    {
		switch (temp)
        {
//+ AB
		case 0x7d:	// Num 9
			if (state.numLock)
				ascii_char='9';
			break;
		case 0x75:	// Num 8
			if (state.numLock)
				ascii_char='8';
			break;
		case 0x6c:	// Num 7
			if (state.numLock)
				ascii_char='7';
			break;
		case 0x74:	// Num 6
			if (state.numLock)
				ascii_char='6';
			break;
		case 0x73:	// Num 5
			if (state.numLock)
				ascii_char='5';
			break;
		case 0x6b:	// Num 4
			if (state.numLock)
				ascii_char='4';
			break;
		case 0x7a:	// Num 3
			if (state.numLock)
				ascii_char='3';
			break;
		case 0x72:	// Num 2
			if (state.numLock)
				ascii_char='2';
			break;
		case 0x69:	// Num 1
			if (state.numLock)
				ascii_char='1';
			break;
		case 0x70:	// Num 0
			if (state.numLock)
				ascii_char='0';
			break;
		case 0x79:	// Num +
			ascii_char='+';
			break;
		case 0x7b:	// Num -
			ascii_char='-';
			break;
		case 0x7c:	// Num *
			ascii_char='*';
			break;
		case 0x4a:	// Num / (extended)
			ascii_char='/';
			break;
		case 0x71:		//Delete (Num , (Delete))
            ascii_char=(state.extended)?0x7F:(state.numLock)?',':0x7F;
			break;
//- AB
       case 0x5A:	// Enter (extended: Num Enter)
            ascii_char='\r';
            break;
        case 0x66:	//Backspace
            ascii_char=0x08;
            break;
//        case 0x71:	//Delete
//            ascii_char=0x7F;
//            break;
        case 0x29:
            ascii_char=' ';
            break;
        case 0x41:
            ascii_char=(state.shift ^ state.capsLock)?';':',';
			break;
        case 0x5b:
            ascii_char=(state.shift ^ state.capsLock)?'*':'+';
			break;
        case 0x4e:
            ascii_char=(state.shift ^ state.capsLock)?'_':'-';
			break;
        case 0x49:
            ascii_char=(state.shift ^ state.capsLock)?':':'.';
			break;
        case 0x1c:
            ascii_char=(state.shift ^ state.capsLock)?'A':'a';
            break;
        case 0x32:
            ascii_char=(state.shift ^ state.capsLock)?'B':'b';
            break;
        case 0x21:
            ascii_char=(state.shift ^ state.capsLock)?'C':'c';
            break;
        case 0x23:
            ascii_char=(state.shift ^ state.capsLock)?'D':'d';
            break;
        case 0x24:
            ascii_char=(state.shift ^ state.capsLock)?'E':'e';
            break;
        case 0x2b:
            ascii_char=(state.shift ^ state.capsLock)?'F':'f';
            break;
        case 0x34:
            ascii_char=(state.shift ^ state.capsLock)?'G':'g';
            break;
        case 0x33:
            ascii_char=(state.shift ^ state.capsLock)?'H':'h';
            break;
        case 0x43:
            ascii_char=(state.shift ^ state.capsLock)?'I':'i';
            break;
        case 0x3b:
            ascii_char=(state.shift ^ state.capsLock)?'J':'j';
            break;
        case 0x42:
            ascii_char=(state.shift ^ state.capsLock)?'K':'k';
            break;
        case 0x4b:
            ascii_char=(state.shift ^ state.capsLock)?'L':'l';
            break;
        case 0x3a:
            ascii_char=(state.shift ^ state.capsLock)?'M':'m';
            break;
        case 0x31:
            ascii_char=(state.shift ^ state.capsLock)?'N':'n';
            break;
        case 0x44:
            ascii_char=(state.shift ^ state.capsLock)?'O':'o';
            break;
        case 0x4d:
            ascii_char=(state.shift ^ state.capsLock)?'P':'p';
            break;
        case 0x15:
            ascii_char=(state.shift ^ state.capsLock)?'Q':'q';
            break;
        case 0x2d:
            ascii_char=(state.shift ^ state.capsLock)?'R':'r';
            break;
        case 0x1b:
            ascii_char=(state.shift ^ state.capsLock)?'S':'s';
            break;
        case 0x2c:
            ascii_char=(state.shift ^ state.capsLock)?'T':'t';
            break;
        case 0x3c:
            ascii_char=(state.shift ^ state.capsLock)?'U':'u';
            break;
        case 0x2a:
            ascii_char=(state.shift ^ state.capsLock)?'V':'v';
            break;
        case 0x1d:
            ascii_char=(state.shift ^ state.capsLock)?'W':'w';
            break;
        case 0x22:
            ascii_char=(state.shift ^ state.capsLock)?'X':'x';
            break;
        case 0x35:
            ascii_char=(state.shift ^ state.capsLock)?'Z':'z';
            break;
        case 0x1a:
            ascii_char=(state.shift ^ state.capsLock)?'Y':'y';
            break;
        case 0x45:
            ascii_char=(state.shift ^ state.capsLock)?'=':'0';
            break;
        case 0x16:
            ascii_char=(state.shift ^ state.capsLock)?'!':'1';
            break;
        case 0x1e:
            ascii_char=(state.shift ^ state.capsLock)?'"':'2';
            break;
        case 0x26:
            ascii_char=(state.shift ^ state.capsLock)?0xa7:'3';	// § = 0xa7
//            ascii_char='3';
            break;	//Paragraph klappt nicht
        case 0x25:
            ascii_char=(state.shift ^ state.capsLock)?'$':'4';
            break;
        case 0x2e:
            ascii_char=(state.shift ^ state.capsLock)?'%':'5';
            break;
        case 0x36:
            ascii_char=(state.shift ^ state.capsLock)?'&':'6';
            break;
        case 0x3d:
            ascii_char=(state.shift ^ state.capsLock)?'/':'7';
            break;
        case 0x3e:
            ascii_char=(state.shift ^ state.capsLock)?'(':'8';
            break;
        case 0x46:
            ascii_char=(state.shift ^ state.capsLock)?')':'9';
            break;
//+ AB
        case 0x0e:
            ascii_char=(state.shift ^ state.capsLock)?'\'':'#';
            break;
        case 0x55:
            ascii_char=(state.shift ^ state.capsLock)?'?':'=';
            break;
        case 0x5d:
            ascii_char=(state.shift ^ state.capsLock)?'?':'\\';
            break;

//- AB
		default:	//Für debugging
			ascii_char=temp;
        }
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

    {	// set ps2 result to buffer
        uint8_t tmphead = ( PS2_ScHead + 1) & PS2_RX_BUFFER_MASK; // calculate buffer index
        if ( tmphead == PS2_ScTail )
        {
        }
        else
        {
            PS2_ScHead = tmphead; // store new index
            PS2_ScBuf[tmphead] = temp; // store received data in buffer
        }
    }
    PS2_LastRxError = lastRxError;
}

/*************************************************************************
Function: ps2_getc()
Purpose:  return byte from ringbuffer
Returns:  received byte from ringbuffer
**************************************************************************/
uint8_t ps2_getc(void)
{
    uint8_t tmptail;

	if (state.newLedStatus)
	{
		state.newLedStatus = 0;
		ps2_setLeds();
	}

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

//+ AB
uint8_t ps2_getsc(void)		// get last scan code
{
    uint8_t tmptail;

	if (state.newLedStatus)
	{
		state.newLedStatus = 0;
		ps2_setLeds();
	}

    if ( PS2_ScHead == PS2_ScTail )
        return 0;   /* no data available */

    /* calculate /store buffer index */
    tmptail = (PS2_ScTail + 1) & PS2_RX_BUFFER_MASK;
    PS2_ScTail = tmptail;

    /* get data from receive buffer */
    return PS2_ScBuf[tmptail];
}/* ps2_getsc */



uint8_t ps2_SendCmd (uint8_t ui8Cmd)
{
	uint8_t ui8parity;
	uint8_t ui8Index;
	uint8_t ui8RetVal;
	uint8_t ui8Error;

	UCSRB &= ~_BV(RXEN) & ~_BV(RXCIE);  //Receiver disable, Receive Complete Interrupt disable

	while (ps2_getsc() != 0);		// clear read buffer

    ui8parity = 1;	// odd parity
	ui8RetVal = 0;
	ui8Error = 0;

	ps2_SetClkPortAsOutput ();	//XCK, USART External Clock as output
	ps2_SetClkToLo ();
	ps2_wait_us (100);		// wait for 100 µs

    ps2_SetDataPortAsOutput ();  //Data as output (output = lo)
	ps2_SetDataToLo (); //  (output = lo)
	ps2_wait_us (25);

    ps2_SetClkPortAsInput ();	//XCK, USART External Clock as input
	ps2_SetClkToHi ();

	ui8Error |= ps2_wait_for_clk_falling_edge (1100);

	for (ui8Index = 0; ui8Index < 8; ui8Index ++)
	{                                                        // Datenbits LSB->MSB
		if (ui8Cmd & 0x01)
		{                                                   // Bit ist 1
			ui8parity++;                                    // Parityzähler erhöhen
			ps2_SetDataToHi (); //  (output = hi)
		}
		else
		{                                                   // Bit ist 0
			ps2_SetDataToLo (); //  (output = lo)
		}

		ui8Error |= ps2_wait_for_clk_rising_edge (50);
		ui8Error |= ps2_wait_for_clk_falling_edge (50);

		ui8Cmd = ui8Cmd >> 1;
	}

	if (ui8parity & 0x01)
	{                                                   // Bit ist 1
		ps2_SetDataToHi (); //  (output = hi)
	}
	else
	{                                                   // Bit ist 0
		ps2_SetDataToLo (); //  (output = lo)
	}

	ui8Error |= ps2_wait_for_clk_rising_edge (50);
	ui8Error |= ps2_wait_for_clk_falling_edge (50);

    ps2_SetDataToHi (); // Pull-Up on (output = hi)
	ps2_SetDataPortAsInput ();  //Data as input (output = hi)

	ui8Error |= ps2_wait_for_clk_rising_edge (50);
	ui8Error |= ps2_wait_for_clk_falling_edge (50);


    ps2_SetClkPortAsInput ();	//XCK, USART External Clock as input
    ps2_SetDataPortAsInput ();  //Data as input

    ps2_SetClkToLo ();	// pull-up off
    ps2_SetDataToLo ();	// pull-up off

    UCSRB = _BV(RXEN) | _BV(RXCIE);  //Receiver enable, Receive Complete Interrupt Enable

	while ((ui8RetVal = ps2_getsc()) == 0)		// wait for read one byte
	{
		_delay_us (1);
	}

	return ui8RetVal;	// 0 ERROR, other values send was OK
}

uint8_t ps2_SendCmdData (uint8_t ui8Cmd, uint8_t ui8Data)
{
	uint8_t ui8RetVal;

	ui8RetVal = ps2_SendCmd (ui8Cmd);	// send command

	if (ui8RetVal == 0xfa)		// Ack	0xfe: nack
	{
		ui8RetVal = ps2_SendCmd (ui8Data);	// send data
	}
	return ui8RetVal;
}

void ps2_wait_us (uint16_t ui16time)
{
	while (ui16time)
	{
		_delay_us (1);
		ui16time --;
	}
}

uint8_t ps2_wait_for_clk_rising_edge (uint16_t ui16time)
{
	while ((ui16time) && !ps2_GetClkValue ())		// wait for rising edge of clk
	{
		_delay_us (1);
		ui16time --;
	}
	return (ui16time == 0);
}

uint8_t ps2_wait_for_clk_falling_edge (uint16_t ui16time)
{
	while ((ui16time) && ps2_GetClkValue ())		// wait for falling edge of clk
	{
		_delay_us (1);
		ui16time --;
	}
	return (ui16time == 0);
}

uint8_t ps2_setLeds (void)
{
	uint8_t ui8LedStatus = 0;
	ui8LedStatus |= ((state.scrollLock!=0) << 0) | ((state.numLock!=0) << 1) | ((state.capsLock!=0) << 2);
	ui8LedStatus = ps2_SendCmdData (0xed, ui8LedStatus);
	return ui8LedStatus;
}


inline void ps2_SetDataPortAsInput (void) { PS2_DDR_DATA &= ~_BV(PS2_PIN_NR_DATA); };
inline void ps2_SetDataPortAsOutput (void) { PS2_DDR_DATA |= _BV(PS2_PIN_NR_DATA); };
inline void ps2_SetDataToHi (void) { PS2_PORT_DATA |= _BV(PS2_PIN_NR_DATA); };
inline void ps2_SetDataToLo (void) { PS2_PORT_DATA &= ~_BV(PS2_PIN_NR_DATA); };

inline void ps2_SetClkPortAsInput (void) { PS2_DDR_CLK &= ~_BV(PS2_PIN_NR_CLK); };
inline void ps2_SetClkPortAsOutput (void) { PS2_DDR_CLK |= _BV(PS2_PIN_NR_CLK); };
inline void ps2_SetClkToHi (void) { PS2_PORT_CLK |= _BV(PS2_PIN_NR_CLK); };
inline void ps2_SetClkToLo (void) { PS2_PORT_CLK &= ~_BV(PS2_PIN_NR_CLK); };
inline uint8_t ps2_GetClkValue (void) { return ((PS2_PIN_CLK & _BV(PS2_PIN_NR_CLK)) != 0); };




//- AB

