/************************************************************************************
	RF12 driver implemenation, see rf12.h for more informations

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

************************************************************************************/
#include "rf12.h"

#ifdef RF12_USE_HAMMIMG_ECC
prog_uint8_t hammingC[16] =
{
    0x00,   /* 0 */
    0x71,   /* 1 */
    0x62,   /* 2 */
    0x13,   /* 3 */
    0x54,   /* 4 */
    0x25,   /* 5 */
    0x36,   /* 6 */
    0x47,   /* 7 */
    0x38,   /* 8 */
    0x49,   /* 9 */
    0x5A,   /* A */
    0x2B,   /* B */
    0x6C,   /* C */
    0x1D,   /* D */
    0x0E,   /* E */
    0x7F    /* F */
};

prog_uint8_t hammingD[128] =
{
    0x00, 0x00, 0x00, 0x03, 0x00, 0x05, 0x0E, 0x07,
    0x00, 0x09, 0x0E, 0x0B, 0x0E, 0x0D, 0x0E, 0x0E,
    0x00, 0x03, 0x03, 0x03, 0x04, 0x0D, 0x06, 0x03,
    0x08, 0x0D, 0x0A, 0x03, 0x0D, 0x0D, 0x0E, 0x0D,
    0x00, 0x05, 0x02, 0x0B, 0x05, 0x05, 0x06, 0x05,
    0x08, 0x0B, 0x0B, 0x0B, 0x0C, 0x05, 0x0E, 0x0B,
    0x08, 0x01, 0x06, 0x03, 0x06, 0x05, 0x06, 0x06,
    0x08, 0x08, 0x08, 0x0B, 0x08, 0x0D, 0x06, 0x0F,
    0x00, 0x09, 0x02, 0x07, 0x04, 0x07, 0x07, 0x07,
    0x09, 0x09, 0x0A, 0x09, 0x0C, 0x09, 0x0E, 0x07,
    0x04, 0x01, 0x0A, 0x03, 0x04, 0x04, 0x04, 0x07,
    0x0A, 0x09, 0x0A, 0x0A, 0x04, 0x0D, 0x0A, 0x0F,
    0x02, 0x01, 0x02, 0x02, 0x0C, 0x05, 0x02, 0x07,
    0x0C, 0x09, 0x02, 0x0B, 0x0C, 0x0C, 0x0C, 0x0F,
    0x01, 0x01, 0x02, 0x01, 0x04, 0x01, 0x06, 0x0F,
    0x08, 0x01, 0x0A, 0x0F, 0x0C, 0x0F, 0x0F, 0x0F
};
#endif

union
{
    unsigned char byte;
    struct
    {
	char ATS_RSSI:1;	//ATS=Antenna tuning circuit detected strong enough RF signal
						//RSSI=The strength of the incoming signal is above the pre-programmed limit
	char FFEM:1;		//FIFO is empty
	char LBD:1;			//Low battery detect, the power supply voltage is below the pre-programmed limit
	char EXT:1;			//Logic level on interrupt pin (pin 16) changed to low (Cleared after Status Read Command)
	char WKUP:1;		//Wake-up timer overflow (Cleared after Status Read Command )
	char RGUR_FFOV:1;	//RGUR=TX register under run, register over write (Cleared after Status Read Command )
        				//FFOV=RX FIFO overflow (Cleared after Status Read Command )
	char POR:1;			//Power-on reset (Cleared after Status Read Command )
	char RGIT_FFIT:1;	//RGIT=TX register is ready to receive the next byte
        				//(Can be cleared by Transmitter Register Write Command)
        				//FFIT=The number of data bits in the RX FIFO has reached the pre-programmed limit
        				//(Can be cleared by any of the FIFO read methods)
    }bits;
} status_H;

union
{
    unsigned char byte;
    struct
    {
	char OFFS:4;		//Offset value to be added to the value of the frequency control parameter (Four LSB bits)
	char OFFS6:1;		//MSB of the measured frequency offset (sign of the offset value)
	char ATGL:1;		//Toggling in each AFC cycle
	char CRL:1;			//Clock recovery locked
	char DQD:1;			//Data quality detector output
    }bits;
} status_L;


uint8_t rfc12_ATS_RSSI(void) { return status_H.bits.ATS_RSSI;}
uint8_t rfc12_FFEM(void) { return status_H.bits.FFEM;}
uint8_t rfc12_LBD(void) { return status_H.bits.LBD;}
uint8_t rfc12_EXT(void) { return status_H.bits.EXT;}
uint8_t rfc12_WKUP(void) { return status_H.bits.WKUP;}
uint8_t rfc12_RGUR_FFOV(void) { return status_H.bits.RGUR_FFOV;}
uint8_t rfc12_POR(void) { return status_H.bits.POR;}
uint8_t rfc12_RGIT_FFIT(void) { return status_H.bits.RGIT_FFIT;}
uint8_t rfc12_ATGL(void) { return status_L.bits.ATGL;}
uint8_t rfc12_CRL(void) { return status_L.bits.CRL;}
uint8_t rfc12_DQD(void) { return status_L.bits.DQD;}

/*
ISR(INT1_vect)	//momentan nicht verwendet
{

}

ISR(SPI_STC_vect)	//SPI übertragung fertig, nicht verwendet
{

}
*/

void rf12_cmd(uint8_t highbyte, uint8_t lowbyte)
{
    RF12_CS();
    SPDR = highbyte;
    while (!(SPSR & _BV(SPIF)));
    SPDR = lowbyte;
    while (!(SPSR & _BV(SPIF)));
    RF12_NOT_CS();
}

void rf12_loop_until_FFIT_RGIT(void)
{
    while (!rf12_read_status_MSB());
}

/*	rf12_read_status_MSB
	RX Mode: FFIT = The number of data bits in the RX FIFO has reached the pre-programmed limit.
					Can be cleared by any of the FIFO read methods
	TX Mode: RGIT = TX register is ready to receive the next byte
					(Can be cleared by Transmitter Register Write Command)
*/
uint8_t rf12_read_status_MSB(void)
{
    RF12_CS();
    MOSI_LOW();
    asm("nop");
    return MISO_LEVEL();
}

void rf12_read_status(void)
{
    RF12_CS();
    SPDR = 0x00;	//Status Read Command
    while (!(SPSR & _BV(SPIF)));
    status_H.byte = SPDR;
    SPDR = 0x00; 	//Status Read Command
    while (!(SPSR & _BV(SPIF)));
    status_L.byte = SPDR;
    RF12_NOT_CS();
}

void rf12_TX(uint8_t aByte)
{
    //FFIT wird gepollt um zu erkennen ob das FIFO TX
    //Register bereit ist.
    //Alternativ ist es auch möglich(wenn verbunden)
    //den Interrupt Ausgang des RF12 zu pollen: while(INT1_LEVEL());
    rf12_loop_until_FFIT_RGIT();
    rf12_cmd(RF12_TRANSMIT,aByte);
}

void rf12_TX_hamming(uint8_t aByte)
{
    rf12_loop_until_FFIT_RGIT();
    rf12_cmd(RF12_TRANSMIT,pgm_read_byte(hammingC+(aByte & 0x0F)));
    rf12_loop_until_FFIT_RGIT();
    rf12_cmd(RF12_TRANSMIT,pgm_read_byte(hammingC+(aByte>>4)));
}

uint8_t rf12_RX(void)
{
    rf12_loop_until_FFIT_RGIT();
    RF12_CS();
    SPDR = 0xB0;
    while (!(SPSR & _BV(SPIF)));
    SPDR = 0x00;
    while (!(SPSR & _BV(SPIF)));
    RF12_NOT_CS();
    return SPDR;
}

uint8_t rf12_RX_hamming(void)
{
    rf12_loop_until_FFIT_RGIT();
    RF12_CS();
    SPDR = 0xB0;
    while (!(SPSR & _BV(SPIF)));
    SPDR = 0x00;
    while (!(SPSR & _BV(SPIF)));
    uint8_t lower_nibble=SPDR;
    RF12_NOT_CS();
    rf12_loop_until_FFIT_RGIT();
    RF12_CS();
    SPDR = 0xB0;
    while (!(SPSR & _BV(SPIF)));
    SPDR = 0x00;
    while (!(SPSR & _BV(SPIF)));
    RF12_NOT_CS();
    return pgm_read_byte_near(hammingD+(lower_nibble&0x7F)) | pgm_read_byte_near(hammingD+(SPDR&0x7F))<<4;
}

void rf12_send(const uint8_t* buf, uint8_t cnt)
{
    if (!cnt) return;
    rf12_TX(0xAA);  //PREAMBLE
    rf12_TX(0x2D);  //SYNC HI BYTE
    rf12_TX(0xD4);  //SYNC LOW BYTE
    //XOR mit 0xAA ist ein quick&dirty hack da der Empfänger bei Nullen
    //am Anfang wohl außer Tritt gerät

#if RF12_USE_HAMMIMG_ECC ==1
    rf12_TX_hamming(cnt^0xAA);
#else
    rf12_TX(cnt^0xAA);
#endif
#if (RF12_USE_CRC==1)
	uint16_t crc=_crc16_update(0,cnt);
#else
    uint8_t chksum=cnt;
#endif
    while (cnt--)
    {
#if RF12_USE_HAMMIMG_ECC ==1
        rf12_TX_hamming(*buf);
#else
        rf12_TX(*buf);
#endif
#if (RF12_USE_CRC==1)
		crc=_crc16_update(crc,*buf++);
#else
        chksum+=*buf++;
#endif
    }
#if RF12_USE_HAMMIMG_ECC ==1
	#if (RF12_USE_CRC==1)
		rf12_TX_hamming(crc&0xFF);
		rf12_TX_hamming(crc>>8);
	#else
        rf12_TX_hamming(chksum);
	#endif
#else
	#if (RF12_USE_CRC==1)
		rf12_TX(crc&0xFF);
		rf12_TX(crc>>8);
	#else
        rf12_TX(chksum);
	#endif
#endif
    rf12_TX(0x0);	//dummy byte
}

//returns -1(255) if there is a CRC or checksum Error
//else, returns number of received byte
uint8_t rf12_read(uint8_t* buf, const uint8_t max)
{
    uint8_t len,i;
#if RF12_USE_HAMMIMG_ECC == 1
    len=rf12_RX_hamming()^0xAA;
#else
    len=rf12_RX()^0xAA;
#endif

#if RF12_USE_CRC == 1
	uint16_t crc=_crc16_update(0,len);
#else
    uint8_t chksum=len;
#endif
	i=len;
    while (i--)
    {
#if RF12_USE_HAMMIMG_ECC ==1
        *buf=rf12_RX_hamming();
#else
        *buf=rf12_RX();
#endif
#if (RF12_USE_CRC==1)
		crc=_crc16_update(crc,*buf++);
#else
        chksum+=*buf++;
#endif
    }
#if RF12_USE_HAMMIMG_ECC == 1
	#if RF12_USE_CRC == 1
		crc=_crc16_update(crc,rf12_RX_hamming());
		crc=_crc16_update(crc,rf12_RX_hamming());
		return (crc)? -1:len;
	#else
    	chksum-=rf12_RX_hamming();
		return (chksum)? -1:len;
	#endif
#else
	#if RF12_USE_CRC == 1
    	crc=_crc16_update(crc,rf12_RX());
		crc=_crc16_update(crc,rf12_RX());
		return (crc)? -1:len;
	#else
		chksum-=rf12_RX();
		return (chksum)? -1:len;
	#endif
#endif
}

void rf12_init(void)
{
    //configure SPI (Atmega32 s.136)
    //SPI CLK auf F_CPU/16
    SPCR = /*_BV(SPIE) |*/ _BV(SPE) | _BV(MSTR) | /*_BV(SPR1) |*/ _BV(SPR0);

    uint8_t i;
    for (i=0;i<100;++i)	//300ms für PowerUp warten
        _delay_ms(3);

    //14. Low Battery Detector and Microcontroller Clock Divider Command
    //Clock Output selected in rf12_config.h
    //2.2V Threshold
    rf12_cmd(RF12_LOW_BAT_AND_CLK_DIV, RF12_LOW(0) | RF12_DIV(RF12_CLOCK_OUTPUT));

    //1.Configuration Setting Command
    //EL(internal data register), EF(FIFO mode), 433MHz, 12pF crystal Load Capacitance
    rf12_cmd(RF12_CFG_SETTING,RF12_EL | RF12_EF | RF12_433MHz | RF12_LOAD_C(12.0));

    //6. Data Filter Command
    rf12_cmd(RF12_DATA_FILTER, RF12_AUTOLOCK | RF12_FASTMODE | RF12_DQD_THRESHOLD(3));

    //7. FIFO and Reset ModeCommand
    rf12_cmd(RF12_FIFO_RESET, RF12_FIFO_IT_LEVEL(8) | RF12_FIFO_FILL(0) | RF12_FF | RF12_DR);

    //12. Wake-Up Timer Command
    //do not use wake-up timer
    rf12_cmd(RF12_WAKE_UP(0),0x00);

    //13. Low Duty.Cycle Command
    //do not use duty cycle
    rf12_cmd(RF12_DUTY_CYCLE(0),0x00);

    //9. AFC Command
	rf12_cmd(RF12_AFC, RF12_AFC_AUTO(2) | RF12_AFC_RANGE_LIMIT(3) | RF12_AFC_en | RF12_AFC_oe | RF12_AFC_fi);

    //3. Freqency Settings 433.92
    rf12_cmd(RF12_FREQ_SETTING_HIGH(RF12_FREQ_433Band(435.00)),RF12_FREQ_SETTING_LOW(RF12_FREQ_433Band(435.00)));

    //5. Receiver Control Command
	rf12_cmd(RF12_RECEIVER_CONTROL(RF12_VDI_FAST | RF12_P20_VDI),RF12_BW_134 | RF12_LNA_0dB | RF12_RSSI_73dBm);

    //4. Data Rate Command
    rf12_cmd(RF12_DATA_RATE, RF12_38400);

    //10. TX Configuration Control Command
	//see also page 33 IA4441.pdf for optimal settings!
    //!mp, freq-shift (5+1)*15kHz, MAX OUT
    rf12_cmd(RF12_TX_CFG, RF12_OUTPUT_FSK_SHIFT(5) | RF12_OUTPUT_ATTENUATON(0));
}

