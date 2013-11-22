#include "peripheral_devices.h"

//Commandbuffer
#define TWI_CMD_BUFFER_SIZE 16
#define TWI_CMD_BUFFER_MASK ( TWI_CMD_BUFFER_SIZE - 1)
static uint8_t TWI_command_buf[TWI_CMD_BUFFER_SIZE];  	// Puffer für TWI Kommandos
static volatile unsigned char TWI_commandHead;
static volatile unsigned char TWI_commandTail;

void process_TWI()
{
    uint8_t messageBuf[25];
    static uint8_t TWI_operation;
    if ( ! TWI_Transceiver_Busy() )
       {
            if (( TWI_commandHead != TWI_commandTail) && (!TWI_operation)) //Kommando im Puffer
            {
                switch (Get_Command_Byte())
                {
                //Schreiboperationen
				case (TWI_CMD_DAC_WRITE):
                    messageBuf[0] = (TWI_SlaveAddress_DAC<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
                    messageBuf[1] = 0;
					messageBuf[2] = Get_Command_Byte();
                    TWI_Start_Transceiver_With_Data( messageBuf, 3 );
                    break;
				/*
				case (TWI_CMD_Speed_WRITE):
                                messageBuf[0] = (TWI_SlaveAddress_Motorcontroller<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
                    messageBuf[1] = command;        	// The first byte is used for commands.

                    messageBuf[2] = Drive_Data[0].setpoint_Speed;
                    messageBuf[3] = Drive_Data[1].setpoint_Speed;

                    TWI_Start_Transceiver_With_Data( messageBuf, 4 );
                    break;

                    //Leseoperationen
                case (TWI_CMD_DATA_READ):
                                messageBuf[0] = (TWI_SlaveAddress_Motorcontroller<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
                    messageBuf[1] = command;
                    TWI_Start_Transceiver_With_Data( messageBuf, 2 );
                    TWI_operation = REQUEST_DATA;
                    break;
				*/
                }
            }
            // Check if the last operation was successful
            if ( TWI_statusReg.lastTransOK )
    		{
				if ( TWI_operation ) // Section for follow-up operations.
                {
					/*
                    // Determine what action to take now
                   	if (TWI_operation == REQUEST_DATA)
                    {
                        // Request/collect the data from the Slave

                        messageBuf[0] = (TWI_SlaveAddress_Motorcontroller<<TWI_ADR_BITS) | (TRUE<<TWI_READ_BIT);
                        TWI_Start_Transceiver_With_Data( messageBuf, 18 );
                        TWI_operation = READ_DATA_FROM_BUFFER; // Set next operation

                    }
                    else if (TWI_operation == READ_DATA_FROM_BUFFER)
                    {
                        // Get the received data from the transceiver buffer

                        TWI_Get_Data_From_Transceiver( messageBuf, 18 );
                        Drive_Data[0].actual_Speed =  (int8_t)messageBuf[1];        // Store data
                        Drive_Data[1].actual_Speed =  (int8_t)messageBuf[2];        // Store data
                        Drive_Data[0].actual_Current = messageBuf[3];
                        Drive_Data[1].actual_Current = messageBuf[4];
                        DriveOdoPosX= *(int32_t*)(messageBuf+5);
                        DriveOdoPosY= *(int32_t*)(messageBuf+9);
                        DriveOdoRotation= *(int32_t*)(messageBuf+13);
                        Drive_Status.byte=*(messageBuf+17);
                        TWI_operation = 0;        // Set next operation

                    }
					*/
                }
            }
            else
            {
                // Use TWI status information to detemine cause of failure and take appropriate actions.
                TWI_Act_On_Failure_In_Last_Transmission( TWI_Get_State_Info( ) );
            }

    }
}

unsigned char TWI_Act_On_Failure_In_Last_Transmission ( unsigned char TWIerrorMsg )
{
    // A failure has occurred, use TWIerrorMsg to determine the nature of the failure
    // and take appropriate actions.
    // See header file for a list of possible failures messages.
    // Here is a simple sample, where if received a NACK on the slave address,
    // then a retransmission will be initiated.

	//Muss nicht unbedingt ein Fehler sein, kann auch beim ersten Aufruf sein,
	//dass die letzte Übertragung nicht okay ist, da noch gar keine stattfand.
	//in diesem Fall Error 0xF8;

    if ( (TWIerrorMsg == TWI_MTX_ADR_NACK) | (TWIerrorMsg == TWI_MRX_ADR_NACK) )
		TWI_Start_Transceiver();
    return TWIerrorMsg;
}

//Zu sendendes TWI Kommando der Queue hinzufügen
uint8_t Add_Command_Byte(uint8_t c)
{
    uint8_t tmphead;
    tmphead  = (TWI_commandHead + 1) & TWI_CMD_BUFFER_MASK;
    if (tmphead==TWI_commandTail) //Command Puffer voll
    {
        return 0;
    }
    TWI_command_buf[tmphead] = c;
    TWI_commandHead = tmphead;
    return 1;
}

uint8_t Get_Command_Byte()
{
	TWI_commandTail = (TWI_commandTail + 1) & TWI_CMD_BUFFER_MASK;
	return TWI_command_buf[TWI_commandTail];
}

void periphery_init()
{
	TWI_Master_Initialise();
	TWI_commandHead=TWI_commandTail=0;
}

void process_periphery()
{
	process_TWI();
}
void set_onboard_DAC(uint8_t value)
{
	while(!Add_Command_Byte(TWI_CMD_DAC_WRITE));
	while(!Add_Command_Byte(value));
}

void config_expansion_Board(uint8_t address, uint8_t channel, uint8_t type)
{

}

void set_expansion_Board_PWM(uint8_t address, uint8_t channel, int16_t value)
{


}

void get_expansion_Board_ADC(uint8_t address, uint8_t channel, int16_t *pvalue)
{


}

