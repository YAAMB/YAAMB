#ifndef PS2_H
#define PS2_H
/*************************************************************************
 Title	:   C include file for the AT-PS2 Keyboard library (ps2.c)
 Author:    Andreas Weber <info@tech-chat.de>
 File:	    ps2.h, 8.12.2007
 Software:  AVR-GCC 4.1
 Hardware:  Atmega32, perhaps any AVR device with USART
***************************************************************************/

/** Size of the circular receive buffer, must be power of 2 */
#ifndef PS2_RX_BUFFER_SIZE
#define PS2_RX_BUFFER_SIZE 32
#endif

#define PS2_RX_BUFFER_MASK ( PS2_RX_BUFFER_SIZE - 1)

#if ( PS2_RX_BUFFER_SIZE & PS2_RX_BUFFER_MASK )
#error RX buffer size is not a power of 2
#endif

#define PS2_DATA_ERROR   	 0x02              /* Overrun condition, Parity Error or Framing Error by USRT   */
#define PS2_BUFFER_OVERFLOW  0x01              /* receive ringbuffer overflow */

struct
{
	char release:	1;
	char shift:		1;
	char extended:	1;
	char vk_up:		1;
	char vk_left:	1;
	char vk_right:	1;
	char vk_down:	1;
} state;

void ps2_init(void);
uint8_t ps2_getc(void);
uint8_t ps2_getLastError(void);

#endif
