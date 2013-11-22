#ifndef CMDLINE_H
#define CMDLINE_H
/*************************************************************************
 Title	:   C include file for the cmdline library (cmdline.c)
 Author:    Andreas Weber <info@tech-chat.de>
 File:	    cmdline.h, 9.12.2007
 Software:  AVR-GCC 4.1
 Hardware:  any AVR device with PS2 Keyboard and LCD connected
***************************************************************************/

#define CMDLINE_MAX_LENGTH 16
#define CMDLINE_ROW 1
//Kommandozeile nullterminiert, also effektiv CMDLINE_MAX_LENGTH-1 Zeichen
char cmdline[CMDLINE_MAX_LENGTH];
int8_t cmdline_pos;	//Position

void cmdline_init(void);
//Muss zyklisch aufgerufen werden um anstehende Kommandos zu verarbeiten
void process_cmdline(void);

#endif
