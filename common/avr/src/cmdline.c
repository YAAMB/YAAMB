/*************************************************************************
 Title	:   cmdline library
 Author:    Andreas Weber <info@tech-chat.de>
 File:	    cmdline.c, 9.12.2007
 Software:  AVR-GCC 4.1
 Hardware:  any AVR device with PS2 Keyboard and LCD connected

 DESCRIPTION
	Eine Commandline für den AVR. Verwendet die ps2 library um Zeichen
	von einer AT PS2 Tastatur zu lesen und die lcd library um die Commandline
	auszugeben.

 USAGE
       See the C include cmdline.h file for a description of each function

***************************************************************************/

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000L
#include <util/delay.h>
#include <avr/wdt.h>
#include "ps2.h"
#include "lcd.h"
#include "cmdline.h"

static void cmd_remove_char(void);
static void cmd_insert_char(const uint8_t *c);
static void cmd_execute(void);

void process_cmdline(void)
{
    if (state.vk_left)	//Cursor links
    {
        if (cmdline_pos)
        {
            cmdline_pos--;
            lcd_command(LCD_MOVE_CURSOR_LEFT);
        }
        state.vk_left=0;
    }
    else if (state.vk_right)	//Cursor links
    {
        if (cmdline[cmdline_pos])
        {
            cmdline_pos++;
            lcd_command(LCD_MOVE_CURSOR_RIGHT);
        }
        state.vk_right=0;
    }

    uint8_t ps2_char = ps2_getc();
    if (ps2_char)
    {
        switch (ps2_char)
        {
        case 0x08:		//Backspace
			if (cmdline_pos)
			{
            cmdline_pos--;
            lcd_command(LCD_MOVE_CURSOR_LEFT);
			}
        case 0x7F:		//Delete
            cmd_remove_char();
            return;
        case '\r':		//Return
            cmd_execute();
            return;
        }
        cmd_insert_char(&ps2_char);
    }

}

//remove char at pos
void cmd_remove_char(void)
{
    uint8_t i=cmdline_pos;
    while (cmdline[i]) { cmdline[i]=cmdline[i+1]; i++; }

    //Könnte man ggf. optimieren
    lcd_gotoxy(0,CMDLINE_ROW);
    lcd_puts(cmdline);
    lcd_putc(' ');
    lcd_gotoxy(cmdline_pos,CMDLINE_ROW);
}

//insert char at pos
void cmd_insert_char(const uint8_t *c)
{
    if (cmdline_pos<CMDLINE_MAX_LENGTH-1)
    {
        //verschieben
        uint8_t i;
        for (i=CMDLINE_MAX_LENGTH-1;i>cmdline_pos;--i)
            cmdline[i]=cmdline[i-1];
        cmdline[cmdline_pos]=*c;
        lcd_gotoxy(cmdline_pos,CMDLINE_ROW);
        lcd_puts(cmdline+cmdline_pos);
        lcd_gotoxy(cmdline_pos+1,CMDLINE_ROW);
        cmdline_pos++;
    }
}

void cmd_execute(void)
{
    lcd_clrscr();	//nur für testing
    lcd_gotoxy(0,0);

    if (!strcmp_P(cmdline,PSTR("version")))
        lcd_puts_P("cmdline v0.1.3");
    else if (!strcmp_P(cmdline,PSTR("moo")))
        lcd_puts_P("Have you mooed  today?");
    else if (!strncmp_P(cmdline,PSTR("calc"),4))	//mini Rechner
		{
		 char* end;
       	 double value1=strtod(cmdline+4,&end);
		 uint8_t op=*end;
       	 double value2=strtod(end+1,NULL);
		 char buf[15];
		 switch(op)
		 {
			case '+': value1+=value2; break;
			case '-': value1-=value2; break;
			case '*': value1*=value2; break;
			case '/': value1/=value2; break;
		 }
		 sprintf(buf,"%f",value1);
		 lcd_puts(buf);
		}
    else
    {
        //Kommando nicht bekannt
        lcd_putc('?');
        lcd_puts(cmdline);
        lcd_putc('?');
    }

	//etwas warten
    uint8_t i=200;
    while (--i)
    {
        wdt_reset();
        _delay_ms(10);
    }
	//LCD löschen
    lcd_clrscr();
    lcd_gotoxy(0,CMDLINE_ROW);
	cmdline_pos=0;
    cmdline[0]=0;
}

void cmdline_init(void)
{
    ps2_init();
    cmdline_pos=cmdline[0]=0;
    lcd_gotoxy(0,CMDLINE_ROW);
}


