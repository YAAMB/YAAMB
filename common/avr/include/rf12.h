/***********************************************************************************
	file: rf12.h

	Version 0.3 alpha

	!!! Do not edit this file, use rf12_config.h instead !!!

	Header für den RF12 Modul-Treiber von http://www.integration.com/index.php
	HopeRF bondet diese auf eine SMD Platine: http://www.hoperf.com/rf_fsk/rf12.htm
	Pollin z.B. verkauft diese Module als RFM12 Best.Nr. 810 049 für 7,95 EUR

	Links:
	http://www.mikrocontroller.net/articles/AVR_RFM12
	http://www.mikrocontroller.net/articles/RFM12
	http://www.roboternetz.de/phpBB2/zeigebeitrag.php?t=29202&highlight=rf12

	Chiphersteller INTEGRATION, siehe DB IA4421.pdf


	Anmerkungen:
		fast inline macros are upper-case, functions lower-case
		this library makes use of the AVR build in SPI harware.
		if your selected avr device lacks SPI hw, I am sorry...

		Aus dem Datenblatt: Umschaltung RX/TX
        Transmitter - Receiver turnover time
		(Synthesizer and crystal oscillator on during TX/RX change with 10 MHz step) 425 μs
		Transmitter turnover time
		(Synthesizer and crystal oscillator on during RX/TX change with 10 MHz step) 300 µs

		Die korekte Ausrichtung der Antennen bringt gewaltigen Gewinn.

		Eine Tabelle für die Bandbreite und FSK Settings findet sich auf Seite 33 von IA4421.pdf

	ToDo:
		Überlegen ob man die Kommunikation über Interrupts machen soll.
		War anfangs so gedacht mit INT1 dann aber verworfen weil INTs oft knapp sind.

		Befehle die nicht in der Hope-RF Doku stehen hinzufügen und testen, siehe IA442x.pdf
		'=== PLL Setting ===
		'Hex = 198 + y
		'Bit-Syntax: 110011000 | ob1 | ob0 | lpx | ddy | ddit | bw1 | bw0
		'ob... = ... (00= 5 oder 10MHz [standard] / 01=3.3MHz / 1x=2.5MHz oder weniger)
		'lpx = Wählt den 'Low-Power-Mode' für den Quarz-Oszilator aus. (0=1ms [620µA] / 1=2ms [460µA])
		'ddy = ...
		'ddi = Schaltet das Dithering in PLL Schleife ab. (1=Abgeschaltet / 0 = Eingeschaltet)
		'bw... = Wählt die Bandbreite des PLL Signals aus. (00=86.2kbps [-107dBc/Hz] / 01=256kbps [-102dBc/Hz]) Bei 1MHz offset Phasenrauschen.

		'=== Syncron Pattern ===
		'Hex = ce & xx
		'Bit-Syntax: 11001110 | b7 | b6 | b5 | b4 | b3 | b2 | b1 | b0
		'b... = Legt den Wert fest, der als Sycronisations Byte für die Datenfilterung verwendet werden soll.

	Author: Andreas Weber

	changelog: 	27.12.2007 aw: first implementation
				30.12.2007 aw: option RF12_USE_CRC implemented

    Copyright (C) 2007 Andreas Weber (info@tech-chat.de)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

***********************************************************************************/

#ifndef RF12_H
#define RF12_H

#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <rf12_config.h>
#if (RF12_USE_CRC==1)
	#include <util/crc16.h>
#endif

/***********************************************************
	1. Configuration Setting Command
***********************************************************/

#define RF12_CFG_SETTING 0x80
#define RF12_EL _BV(7)		// enables the internal data register for TX Buffered Data Transmission
#define RF12_EF _BV(6)		// enables FIFO mode. If EF=0 DATA and DCLK clock are used for output
#define RF12_315MHz	(0<<4)
#define RF12_433MHz	(1<<4)
#define RF12_868MHz (2<<4)
#define RF12_915MHz (3<<4)
#define RF12_LOAD_C(v) ((char)((v-8.5)*2))	//crystal load capacitance 8.5pF to 16.0pF

/***********************************************************
	2. Power Managment Command
***********************************************************/

//Anmerkung: ich gehe davon aus dass PLL synonym für "RF synthesizer" verwendet wird

#define RF12_POWER_SETTING 0x82
#define RF12_ER  _BV(7)		//Enable the whole receiver chain. (RF synthesizer, RF front end, baseband circuits, oscillator)
#define RF12_EBB _BV(6)		//Enable baseband circuits only
#define RF12_ET  _BV(5)		//Enable the whole transmitter chain (PowerAmplifier, synthesizer, oscillator) and starts TX
#define RF12_ES  _BV(4)		//Enables the PLL/Synthesizer
#define RF12_EX  _BV(3)		//Enables the oscillator
#define RF12_EB  _BV(2)		//Enables the low battery detector
#define RF12_EW  _BV(1)		//Enables the wake-up timer

/***********************************************************
	3. Frequency Setting Command
***********************************************************/
//The 12-bit parameter F (bits f11 to f0) should be in the range of 96 and 3903. When F value sent is out of
//range, the previous value is kept. The synthesizer band center frequency f0 can be calculated as:
//f0 = 10 * C1 * (C2 + F/4000) [MHz]
//The constants C1 and C2 are determined by the selected band as:
//315MHz Band C1=1, C2=31
//433MHz Band C1=1, C2=43
//868MHz Band C1=2, C2=43
//915MHz Band C1=3, C2=30

#define RF12_FREQ_SETTING_HIGH(bits) (0xA0 | (bits>>8))
#define RF12_FREQ_SETTING_LOW(bits) (bits & 0xFF)

#define RF12_FREQ_315Band(v) (uint16_t)((v/10.0-31)*4000)
#define RF12_FREQ_433Band(v) (uint16_t)((v/10.0-43)*4000)
#define RF12_FREQ_868Band(v) (uint16_t)((v/20.0-43)*4000)
#define RF12_FREQ_915Band(v) (uint16_t)((v/30.0-30)*4000)

/***********************************************************
	4. Data Rate Command
***********************************************************/

//R= (10000 / 29 / (1+cs*7) / BR) – 1
//BR = 10000 / 29 / (R+1) / (1+cs*7) [kbps]BR = 10000 / 29 / (R+1) / (1+cs*7) [kbps]

//some predefines
#define RF12_4800 71
#define RF12_9600 35
#define RF12_19200 17
#define RF12_38400 8
#define RF12_43100 7
#define RF12_57600 5
#define RF12_115000 2

#define RF12_DATA_RATE 0xC6
#define RF12_DATA_RATE_CS _BV(7)

/***********************************************************
	5. Receiver Control Command
***********************************************************/

#define RF12_RECEIVER_CONTROL(bits) (0x90 | bits)

#define RF12_BW_400 (1<<5)
#define RF12_BW_340 (2<<5)
#define RF12_BW_270 (3<<5)
#define RF12_BW_200 (4<<5)
#define RF12_BW_134 (5<<5)
#define RF12_BW_67  (6<<5)

#define RF12_P20_INT 0
#define RF12_P20_VDI _BV(2)

#define RF12_VDI_FAST   (0)
#define RF12_VDI_MEDIUM (1)
#define RF12_VDI_SLOW   (2)
#define RF12_VDI_ON     (3)

#define RF12_LNA_0dB (0<<3)
#define RF12_LNA_6dB (1<<3)
#define RF12_LNA_14dB (2<<3)
#define RF12_LNA_20dB (3<<3)

#define RF12_RSSI_103dBm  (0)
#define RF12_RSSI_97dBm   (1)
#define RF12_RSSI_91dBm   (2)
#define RF12_RSSI_85dBm   (3)
#define RF12_RSSI_79dBm   (4)
#define RF12_RSSI_73dBm   (5)
#define RF12_RSSI_67dBm   (6)
#define RF12_RSSI_61dBm   (7)

/***********************************************************
	6. Data Filter Command
***********************************************************/

#define RF12_DATA_FILTER 0xC2
#define RF12_AUTOLOCK (_BV(7) | _BV(5) | _BV(3))
#define RF12_FASTMODE (_BV(6) | _BV(5) | _BV(3))
#define RF12_ANALOG_FILTER (_BV(4) | _BV(5) | _BV(3))
#define RF12_DQD_THRESHOLD(bits) (bits | _BV(5) | _BV(3))

/***********************************************************
	7. FIFO and Reset Mode Command
***********************************************************/

#define RF12_FIFO_RESET 0xCA
#define RF12_FIFO_IT_LEVEL(bits) (bits<<4)
#define RF12_FIFO_FILL(bit) (bit<<2)
#define RF12_FF _BV(1)		//FIFO fill after synchron pattern reception
//To restart the synchron pattern recognition, this bit should be cleared and set
#define RF12_DR _BV(0)		//Disables the highly sensitive RESET mode.
//If this bit is cleared, a 200 mV glitch in the power supply may cause a system reset.


/***********************************************************
	8. Receiver FIFO Read Command
***********************************************************/

#define RF12_FIFO_READ 0xB0

/***********************************************************
	9. AFC Command
***********************************************************/

#define RF12_AFC 0xC4
#define RF12_AFC_en _BV(0)	// Enables the calculation of the offset frequency by the AFC circuit
#define RF12_AFC_oe _BV(1)	// Enables the frequency offset register. It allows the addition of the offset register to
// the frequency control word of the PLL
#define RF12_AFC_fi _BV(2)	// Switches the circuit to high accuracy (fine) mode. In this case,
// the processing time is about twice longer, but the measurement uncertainty is about the half
#define RF12_AFC_st _BV(3)	// Stobe edge, when st goes to high, the actual latest calculated frequency error
// is stored into the offset register of the AFC block

#define RF12_AFC_RANGE_LIMIT(v) (v<<4)
// Limits the value of the frequency offset register to the next values:
// 0: No restriction
// 1: +15 fres to -16 fres
// 2: +7 fres to -8 fres
// 3: +3 fres to -4 fres
// fres=2.5 kHz in 315MHz and 433MHz Bands
// fres=5.0 kHz in 868MHz Band
// fres=7.5 kHz in 915MHz Band

#define RF12_AFC_AUTO(v) (v<<6)
// 0: Auto mode off (Strobe is controlled by microcontroller)
// 1: Runs only once after each power-up
// 2: Keep the foffset only during receiving(VDI=high)
// 3: Keep the foffset value independently trom the state of the VDI signal

/***********************************************************
	10. TX Config Control Command
***********************************************************/

//sign bit mp ist 0 by default
//fout = f0 + (-1)^FSK_SIGN * (M + 1) * (15 kHz)
//RF12_OUTPUT_FSK_SHIFT(0)=15kHz
//RF12_OUTPUT_FSK_SHIFT(1)=30kHz
//...
//RF12_OUTPUT_FSK_SHIFT(15)= 240kHz
//Attenuation*3dB:
//RF12_OUTPUT_ATTENUATON(2)= -6dB

#define RF12_TX_CFG 0x98
#define RF12_OUTPUT_FSK_SHIFT(m) ( (m&0x0F)<<4 )
#define RF12_OUTPUT_ATTENUATON(p) (p & 0x07)

/***********************************************************
	11. Transmitter Register Write Command
***********************************************************/

#define RF12_TRANSMIT 0xB8

/***********************************************************
	12. Wake-Up Timer Command
		(not fully implemented)
	see page 22 hope-rf rf12.pdf
***********************************************************/
#define RF12_WAKE_UP(R) (0xE0 | R)
//The wake-up time period can be calculated by t_wake_up=M * 2^R [ms]

/***********************************************************
	13. Low Duty.Cycle Command
		(not fully implemented),
	see page 22 hope-rf rf12.pdf
***********************************************************/

#define RF12_DUTY_CYCLE(r0) (0xC8 | r0)

/**********************************************************************
	14. Low Battery Detector and Microcontroller Clock Divider Command
**********************************************************************/

#define RF12_LOW_BAT_AND_CLK_DIV 0xC0
#define RF12_LOW(v)	(v)
//5 bit represents the value, which defines the threshold voltage of the detector
// 0:  2.2V
// 1:  2.3V
// ...
// 31: 5.3V
#define RF12_DIV(v) (v<<5)
// 0: 1    MHz Clock Output Frequency
// 1: 1.25 MHz Clock Output Frequency
// 2: 1.66 MHz Clock Output Frequency
// 3: 2    MHz Clock Output Frequency
// 4: 2.5  MHz Clock Output Frequency
// 5: 3.33 MHz Clock Output Frequency
// 6: 5    MHz Clock Output Frequency
// 7: 10   MHz Clock Output Frequency


/********************** FUNCTIONS *********************/

void rf12_cmd(uint8_t highbyte, uint8_t lowbyte);
void rf12_loop_until_FFIT_RGIT(void);
uint8_t rf12_read_status_MSB(void);
void rf12_TX(uint8_t aByte);
void rf12_TX_hamming(uint8_t aByte);
void rf12_read_status(void);
uint8_t rf12_RX(void);
uint8_t rf12_RX_hamming(void);
void rf12_send(const uint8_t* buf, uint8_t cnt);
uint8_t rf12_read(uint8_t* buf,const uint8_t max);
void rf12_init(void);

uint8_t rfc12_ATS_RSSI(void);
uint8_t rfc12_FFEM(void);
uint8_t rfc12_LBD(void);
uint8_t rfc12_EXT(void);
uint8_t rfc12_WKUP(void);
uint8_t rfc12_RGUR_FFOV(void);
uint8_t rfc12_POR(void);
uint8_t rfc12_RGIT_FFIT(void);
uint8_t rfc12_ATGL(void);
uint8_t rfc12_CRL(void);
uint8_t rfc12_DQD(void);

#endif