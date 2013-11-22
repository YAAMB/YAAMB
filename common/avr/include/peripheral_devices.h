/*
	file: peripheral_devices.h

	Funktionen um periphere Hardware anzusprechen.
	Die Funktionen blockieren alle, falls der Commandbuffer voll ist.

	Momentan unterstützt:
		onboard DAC MAX517 über TWI
	Geplante Unterstützung:
		L298 und PowerBridge auf Expansion-Board1
		433MHz Funkmodule von Farnell

	Changes:
		16.12.2007 aw: first implementation
*/

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "TWI_Master.h"

#define TWI_GEN_CALL   			0x00

//device address of MAX517 Nr.1 0101100x, see datasheet
#define TWI_SlaveAddress_DAC 	0x2C

#define TWI_CMD_DAC_WRITE 0x01

void periphery_init(void);

void process_periphery(void);
uint8_t Add_Command_Byte(uint8_t c);
uint8_t Get_Command_Byte(void);
void process_TWI(void);
unsigned char TWI_Act_On_Failure_In_Last_Transmission ( unsigned char TWIerrorMsg );

/*   FUNCTIONS    */

void set_onboard_DAC(uint8_t value);

/*
Beim Expansion Board können die 2 Kanäle für
die L298 Brücke oder einer Leistungsbrücke mit IRFZ44N Modfets konfiguriert werden

channel: Kanal 0 oder 1
type: 0=L298, 1=PowerBridge
*/
void config_expansion_Board(uint8_t address, uint8_t channel, uint8_t type);
void set_expansion_Board_PWM(uint8_t address, uint8_t channel, int16_t value);
void get_expansion_Board_ADC(uint8_t address, uint8_t channel, int16_t *pvalue);

