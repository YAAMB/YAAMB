/*************************************************************************
 Title	:   C include file for the USB Buffer (usbbuf.c)
 Author:    Raimund Lehmann
 File:	    usbbuf.h, 4.06.2008
 Software:  AVR-GCC 4.1
 Hardware:  any AVR device

***************************************************************************/

#ifndef USBBUF_H
#define USBBUF_H

#include <stdlib.h>
#include <avr/pgmspace.h>
#include "usbdrv/usbdrv.h"
#define F_CPU 16000000UL
#include <util/delay.h>

#define USB_RX_BUFFER_SIZE 16
#define USB_TX_BUFFER_SIZE 32

// Initialisierung, muss aufgerufen werden bevor die Interrupts aktiviert werden.
void usbBufInit(void);

//Sendet ein Byte an USB Master, wenn Buffer voll ist wird gewartet.
void usb_putc(char data);

//Sende einen String an USB Master
void usb_puts(const char* s);
//Sende einen String aus dem Programmspeicher an den USB Master
void usb_puts_p(const char* s);

//Empfängt ein Byte vom USB Master, wenn keine Daten vorhanden sind wird der Wert 0 zurückgegeben
char usb_getc(void);

//Gib die Größe des freien Speichers im TX_Buffer zurück
uint8_t usb_wirteBufSpace(void);

#define usb_puts_P(__s)         usb_puts_p(PSTR(__s))
#endif
