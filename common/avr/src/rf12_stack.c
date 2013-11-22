/************************************************************************************
	file: rf12_stack.c

	RF12 stack implemenation, see rf12.h for more informations

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
#include "rf12_stack.h"
#include "rf12_stack_config.h"

static uint8_t stack_address;
static volatile uint8_t stack_time;
uint8_t input_Buffer[30];

static uint8_t cmd_count;
static uint8_t cmd_list[20];
static handler handler_list[20];

void rf12_stack_init(void)
{
	rf12_init();
	rf12_cmd(RF12_POWER_SETTING, RF12_ER | RF12_DISABLE_CLOCK_OUTPUT);
	stack_address=RF12_ADR;
	stack_time=0;
}

void rf12_ms_timetick(void)
{
	stack_time++;
}

void rf12_process(void)
{
	//rf12_cmd(RF12_POWER_SETTING, RF12_ER | RF12_DISABLE_CLOCK_OUTPUT);
	//_delay_ms(1);
	if (rf12_read_status_MSB())
    {
       uint8_t len=rf12_read(input_Buffer,30);
       rf12_cmd(RF12_FIFO_RESET, RF12_FIFO_IT_LEVEL(8) | RF12_FIFO_FILL(0) | RF12_DR);
	   rf12_cmd(RF12_FIFO_RESET, RF12_FIFO_IT_LEVEL(8) | RF12_FIFO_FILL(0) | RF12_FF | RF12_DR);
	   if(len!=255)
		{
		   if(input_Buffer[0]==stack_address)
			{
				if(input_Buffer[2]==0x00)	//ping
				{
				 //PORTA^=_BV(4);
				 rf12_pong(input_Buffer[1]);	//Pong an Sender
				}
			    else if(input_Buffer[2]>=0x80)	//CFG
				{
				 //PORTA^=_BV(4);
				 rf12_cmd(input_Buffer[2],input_Buffer[3]);
				}
			    else
				{
				  for(uint8_t i=0;i<cmd_count;i++)
					if(input_Buffer[2]==cmd_list[i])
						handler_list[i](input_Buffer[1],input_Buffer+3,len);
					//Raimund: (*handler_list[i])(input_Buffer[1],input_Buffer+3,len);
				}
			}
		}
    }
	//rf12_cmd(RF12_POWER_SETTING, RF12_EX | RF12_ES | RF12_DISABLE_CLOCK_OUTPUT);
	//_delay_ms(0.5);
}

//returns Time in ms if message received
//returns 255 if timeout
uint8_t rf12_loop_until_message(uint8_t msg)
{
    rf12_cmd(RF12_POWER_SETTING, RF12_ER | RF12_DISABLE_CLOCK_OUTPUT);	//enable receiver
	_delay_ms(0.5);
	stack_time=0;
	//wait for message

	while(stack_time<TIMEOUT)
	{
		if(rf12_read_status_MSB())
		{

			uint8_t len=rf12_read(input_Buffer,30);
	   		//re-enable sync pattern matching
       		rf12_cmd(RF12_FIFO_RESET, RF12_FIFO_IT_LEVEL(8) | RF12_FIFO_FILL(0) | RF12_DR);
	   		rf12_cmd(RF12_FIFO_RESET, RF12_FIFO_IT_LEVEL(8) | RF12_FIFO_FILL(0) | RF12_FF | RF12_DR);

			if ((len!=255) && (input_Buffer[2]==msg) && (input_Buffer[0]==stack_address))  //CRC korrekt,msg empfangen, adresse korrekt
				return stack_time;
		}
	}
	return 255;
}


uint8_t rf12_ping(uint8_t target)
{
    rf12_cmd(RF12_POWER_SETTING, RF12_ET | RF12_DISABLE_CLOCK_OUTPUT);	//enable transmitter
	_delay_ms(0.5);
	uint8_t buf[3];
	buf[0]=target;
	buf[1]=stack_address;
	buf[2]=0x00; //PING
    rf12_send(buf,3);
   	rf12_loop_until_FFIT_RGIT();

	//wait for pong
	return rf12_loop_until_message(0x01);
}

uint8_t rf12_stack_send(uint8_t target, uint8_t command, const uint8_t* const data, uint8_t len)
{
    rf12_cmd(RF12_POWER_SETTING, RF12_ET | RF12_DISABLE_CLOCK_OUTPUT);	//enable transmitter
	_delay_ms(0.5);
	uint8_t buf[len+3];
	buf[0]=target;
	buf[1]=stack_address;
	buf[2]=command;
    memcpy(buf+3,data,len);
    rf12_send(buf,len+3);
   	rf12_loop_until_FFIT_RGIT();
	return 0;
}

uint8_t register_handler(uint8_t command, handler h)
{
  if(cmd_count<20){
	cmd_list[cmd_count]=command;
	handler_list[cmd_count]=h;
  }
  cmd_count++;
}

void rf12_pong(uint8_t target)
{
    rf12_cmd(RF12_POWER_SETTING, RF12_ET | RF12_DISABLE_CLOCK_OUTPUT);	//enable transmitter
	_delay_ms(0.5);
	uint8_t buf[3];
	buf[0]=target;
	buf[1]=stack_address;
	buf[2]=0x01;
    rf12_send(buf,3);
    rf12_loop_until_FFIT_RGIT();
    rf12_cmd(RF12_POWER_SETTING, RF12_ER | RF12_DISABLE_CLOCK_OUTPUT);	//enable receiver
}

void rf12_remote_cmd(uint8_t target, uint8_t highbyte, uint8_t lowbyte)
{
    rf12_cmd(RF12_POWER_SETTING, RF12_ET | RF12_DISABLE_CLOCK_OUTPUT);	//enable transmitter
	_delay_ms(0.5);
	uint8_t buf[4];
	buf[0]=target;
	buf[1]=stack_address;
	buf[2]=highbyte;
	buf[3]=lowbyte;
    rf12_send(buf,4);
    rf12_loop_until_FFIT_RGIT();
    rf12_cmd(RF12_POWER_SETTING, RF12_ER | RF12_DISABLE_CLOCK_OUTPUT);	//enable receiver
}

