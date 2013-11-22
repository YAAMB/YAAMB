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

#define PS2_PORT_CLK	(PORTB)
#define PS2_DDR_CLK		(DDRB)
#define PS2_PIN_CLK		(PINB)
#define PS2_PIN_NR_CLK	(PB0)
#define PS2_PORT_DATA	(PORTD)
#define PS2_DDR_DATA	(DDRD)
#define PS2_PIN_DATA	(PIND)
#define PS2_PIN_NR_DATA	(PD0)

inline void ps2_SetDataPortAsInput (void);
inline void ps2_SetDataPortAsOutput (void);
inline void ps2_SetDataToHi (void);
inline void ps2_SetDataToLo (void);

inline void ps2_SetClkPortAsInput (void);
inline void ps2_SetClkPortAsOutput (void);
inline void ps2_SetClkToHi (void);
inline void ps2_SetClkToLo (void);
inline uint8_t ps2_GetClkValue (void);

struct
{
	int8_t release:		1;
	int8_t shift:		1;
	int8_t extended:	1;
	int8_t vk_up:		1;
	int8_t vk_left:		1;
	int8_t vk_right:	1;
	int8_t vk_down:		1;
//+ AB
	int8_t ctrl:		1;
	int8_t alt:			1;
// Keyb LEDs
	int8_t newLedStatus:1;
	int8_t numLock:		1;
	int8_t scrollLock:	1;
	int8_t capsLock:	1;
//- AB
} state;

void ps2_init(void);
uint8_t ps2_getc(void);
uint8_t ps2_getLastError(void);
//+ AB
uint8_t ps2_getsc(void);
void ps2_wait_us (uint16_t ui16time);
uint8_t ps2_wait_for_clk_rising_edge (uint16_t ui16time);
uint8_t ps2_wait_for_clk_falling_edge (uint16_t ui16time);
uint8_t ps2_SendCmd (uint8_t ui8Cmd);
uint8_t ps2_SendCmdData (uint8_t ui8Cmd, uint8_t ui8Data);
uint8_t ps2_setLeds (void);
//- AB


#endif
