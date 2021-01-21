/*
 * I2C_one_master.c
 *
 * Created: 05.01.2021 23:01:24
 *  Author: Serhii Senyk
 */ 
// дописати повідомлення про помилки
#include "I2C_one_master.h"

volatile uint8_t  I2C_BufOUT[I2C_BUF_SIZE_OUT], I2C_StartBufOUT = 0, I2C_EndBufOUT = 0;
volatile uint8_t  I2C_BufIN[I2C_BUF_SIZE_IN], I2C_StartBufIN = 0, I2C_EndBufIN = 0;
volatile uint8_t I2C_task = 0;   //counter start-stop packets

volatile uint8_t  I2C_status = 0;
volatile uint8_t  I2C_error = 0;
ISR(TWI_vect)
{
	switch(TWSR & 0b11111000)
	{
		case 0x08:  // START was formed
		case 0x10:  // repeat start
		case 0x18:  // Write was transmited and ACK received
		case 0x28:
		case 0x40:
			I2C_error = 1;
			I2C_Action(); 
			break;
		//--------------------------------------------------------------------------------------
		case 0x50:
		case 0x58:
			I2C_status |= 1 << I2C_NACK_ERROR;       //error : NACK
			I2C_error = 0;
			write_I2C_BufIN(TWDR); 
			I2C_Action(); 
			break;
		//--------------------------------------------------------------------------------------
		case 0x20:
		case 0x30:
		case 0x48:
			I2C_error = 1; //виправити
			I2C_SearchStartStop(); 
			I2C_Action(); 
			break;
		default: 
			break;
	}
}

void I2C_Init()
{
	TWBR = I2C_BIT_RATE_CALC; // Bit Rate
	TWSR = 0;  // prescalar TWPS = 0
	I2C_DDR &= ~(1 <<  SCL_PIN | 1 << SDA_PIN);//input
	I2C_PORT |= ((1 << SCL_PIN) | (1 << SDA_PIN));//pullup
	TWAR = 0;
}

void write_I2C_BufOUT(uint8_t value) //transmit
{
	I2C_BufOUT[I2C_EndBufOUT++] = value;
	I2C_EndBufOUT &= I2C_BUF_MASK_OUT;
	// check overflow
	if(I2C_StartBufOUT == I2C_EndBufOUT)
		I2C_status |= 1 << I2C_BufOUT_ERROR; // error
}

void write_I2C_BufIN(uint8_t value)  // receive
{
	I2C_BufIN[I2C_EndBufIN++] = value;
	I2C_EndBufIN &= I2C_BUF_MASK_IN;
	//check overflow
	if(I2C_StartBufIN == I2C_EndBufIN)
		I2C_status |= 1 << I2C_BufIN_ERROR; // error
}

uint8_t read_I2C_BufOUT()
{
	uint8_t value = I2C_BufOUT[I2C_StartBufOUT++];
	I2C_StartBufOUT &= I2C_BUF_MASK_OUT;
	return value;
}

uint8_t read_I2C_BufIN()
{
	uint8_t value = I2C_BufIN[I2C_StartBufIN++];
	I2C_StartBufIN &= I2C_BUF_MASK_IN;
	return value;
}

void I2C_SearchStartStop() //search and goto in buffer
{                                                              // ReStart  Stop
	uint8_t temp = 1, I2C_StartBufOUT_temp;
	do
	{
		I2C_StartBufOUT_temp = I2C_StartBufOUT;
		switch(read_I2C_BufOUT())
		{
			case 0x20: //if address command
			case 0x40: //if command  transmit bytes
			//then reading next byte
				read_I2C_BufOUT(); 
				break;
			case 0x80: 
				break;             //if command  reading bytes
			case 0x04:  //if stop commands
			case 0x10:  //if ReStart commands
				temp = 0;          //search complete
				//restore the address in the buffer
				I2C_StartBufOUT = I2C_StartBufOUT_temp;
				break;
		}
	}
	while(temp == 1);
}

void I2C_Start(uint8_t control)
{
	cli();
	write_I2C_BufOUT(control);                 //  whether to report about works
}

void I2C_ReStart(void)
{
	cli();//////
	write_I2C_BufOUT(0x10);                     // indicate about Restart
}

void I2C_Address(uint8_t address, uint8_t direction)
{
	write_I2C_BufOUT(0x20);              //indicates that the address will be transmitted
	write_I2C_BufOUT(address << 1 | direction);      // address + direction of transmission
}

void I2C_SendByte(uint8_t byte)
{
	write_I2C_BufOUT(0x40);     // indicates that a byte of data will be transmitted
	write_I2C_BufOUT(byte);      // byte of data
}

void I2C_ReadByte(uint8_t ack)
{
	write_I2C_BufOUT(0x80 + ack); //indicates that the data byte will be read
}                                                                 //indicates the state of ACK / NACK in mol. bits

void I2C_Stop()
{
	write_I2C_BufOUT(0x04);     // indicate that STOP
	if((I2C_status & (1 << I2C_Proccess)) == 0)
	{ // if there is no transmission on the TWI bus
		I2C_status |= 1 << I2C_Proccess; // TWI busy
		// command Start
		TWCR = (1 << TWINT)|(1 << TWSTA)|(1 << TWEN)|(1 << TWIE);
		if(read_I2C_BufOUT())
			I2C_status |= 1 << I2C_Control;
		else
			I2C_status &= ~(1 << I2C_Control);
	}
	sei();
}

void I2C_Action()
{
	switch(read_I2C_BufOUT())
	{
		case 0x04:  // stop command
			if(I2C_status & (1 << I2C_Control))  I2C_task++;
			// check for buffer emptying
			if(I2C_StartBufOUT == I2C_EndBufOUT)
			{
				I2C_status &= ~(1 << I2C_Proccess);
				// stop command
				TWCR = (1 << TWINT)|(1 << TWSTO)|(1 << TWEN);
			}
			else
			{
				// stop command
				TWCR = (1 << TWINT)|(1 << TWSTO)|(1 << TWEN);   // ??????? Start
				TWCR = (1 << TWINT)|(1 << TWSTA)|(1 << TWEN)|(1 << TWIE);
				if(read_I2C_BufOUT())
					I2C_status |= (1 << I2C_Control);
				else
					I2C_status &= ~(1 << I2C_Control);
			}
			break;
		case 0x10:  // command  ReStart
			TWCR = (1 << TWINT)|(1 << TWSTA)|(1 << TWEN)|(1 << TWIE);
			break;
		case 0x20:  // address transmission to the TWI bus
		case 0x40: // write a byte of data to the TWI bus
			TWDR = read_I2C_BufOUT();
			TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWIE);
			break;
		// reading a byte of data on the TWI bus
		case 0x80:
			TWCR = (1 << TWINT)|(1 << TWEA)|(1 << TWEN)|(1 << TWIE);
			break;
		case 0x81:
			TWCR = (1 << TWINT)|(0 << TWEA)|(1 << TWEN)|(1 << TWIE);
			break;
	}
}