/*
MIT License

Copyright (c) 2017 Efthymios Koktsidis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */ 
#include "I2C.h"

#define __I2C_SLA_W(address)	(address<<1)
#define __I2C_SLA_R(address)	((address<<1) | (1<<0))

//----- Prototypes ------------------------------//

//-----------------------------------------------//

//----- Functions -------------//
//Setup TWI hardwarw.
void I2C_Setup(void)
{
	//TWI pins as outputs
	PinMode(I2C_SDA, Output);
	PinMode(I2C_SCL, Output);
	//Activate internal pull-up resistors
	//DigitalWrite(I2C_SDA, High);
	//DigitalWrite(I2C_SCL, High);

	//Initialize TWI prescaler and bit rate
	TWSR = 0x00;	//Prescaler = 1
	TWBR = (uint8_t)(((F_CPU / F_SCL) - 16) / 2);
	//F_SCL = F_CPU / (16 + 2 * TWBR * PRESCALER)
	//TWBR = ((F_CPU / F_SCL) - 16) / (2 * PRESCALER)

	//Enable TWI  module
	TWCR = (1<<TWEN);
}

//Get TWI status.
uint8_t I2C_Status(void)
{
	return (TWSR & 0xF8);
}

//Begin TWI transmission.
uint8_t I2C_BeginTransmission(void)
{
	//Transmit START condition
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

	//Wait for TWINT Flag set. This indicates that
	//the START condition has been transmitted.
	while (!BitCheck(TWCR, TWINT));

	//Return status register
	return (I2C_Status());
}

//End TWI transmission.
void I2C_EndTransmission(void)
{
	//Transmit STOP condition
	TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWEA) | (1<<TWINT) | (1<<TWSTO);

	//Wait for stop condition to be executed on bus
	//TWINT is not set after a stop condition!
	while(BitCheck(TWCR, TWSTO));
}

//Transmit data.
uint8_t I2C_Transmit(const uint8_t Data)
{
	//Data to be transmitted
	TWDR = Data;

	//Start transmission
	TWCR = (1<<TWINT) | (1<<TWEN);

	//Wait data to be shifted
	while (!BitCheck(TWCR, TWINT));

	//Return status register
	return (I2C_Status());
}

//Wait until ACK received.
uint8_t I2C_ReceiveACK(void)
{
	TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN);
	
	//Wait till reception is
	while (!BitCheck(TWCR, TWINT));

	//Return received data
	return TWDR;
}

//Wait until NACK received.
uint8_t I2C_ReceiveNACK(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN);

	//Wait till reception is
	while (!BitCheck(TWCR, TWINT));

	//Return received data
	return TWDR;
}

//Transmit packet to specific slave address.
enum I2C_Status_t I2C_PacketTransmit(const uint8_t SLA, const uint8_t SubAddress, uint8_t *Packet, const uint8_t Length)
{
	uint8_t i, status;
	
	do
	{
		//Transmit START signal
		status = I2C_BeginTransmission();
		if ((status != MT_START_TRANSMITTED) && ((status != MT_REP_START_TRANSMITTED)))
		{
			status = I2C_Error;
			break;
		}

		//Transmit SLA+W
		status = I2C_Transmit(__I2C_SLA_W(SLA));
		if ((status != MT_SLA_W_TRANSMITTED_ACK) && (status != MT_SLA_W_TRANSMITTED_NACK))
		{
			status = I2C_Error;
			break;
		}

		//Transmit write address
		status = I2C_Transmit(SubAddress);
		if ((status != MT_DATA_TRANSMITTED_ACK) && (status != MT_DATA_TRANSMITTED_NACK))
		{
			status = I2C_Error;
			break;
		}
		//Transmit DATA
		for (i = 0 ; i < Length ; i++)
		{
			status = I2C_Transmit(Packet[i]);
			if ((status != MT_DATA_TRANSMITTED_ACK) && (status != MT_DATA_TRANSMITTED_NACK))
			{
				status = I2C_Error;
				break;
			}
		}

		//Transmitted successfully
		status = I2C_Ok;
	}
	while (0);

	//Transmit STOP signal
	I2C_EndTransmission();

	return (I2C_Status_t)status;
}

//Receive packet from specific slave address.
enum I2C_Status_t I2C_PacketReceive(const uint8_t SLA, const uint8_t SubAddress, uint8_t *Packet, const uint8_t Length)
{
	uint8_t i = 0, status;

	do
	{
		//Transmit START signal
		status = I2C_BeginTransmission();
		if ((status != MT_START_TRANSMITTED) && (status != MT_REP_START_TRANSMITTED))
		{
			status = I2C_Error;
			break;
		}

		//Transmit SLA+W
		status = I2C_Transmit(__I2C_SLA_W(SLA));
		if ((status != MT_SLA_W_TRANSMITTED_ACK) && (status != MT_SLA_W_TRANSMITTED_NACK))
		{
			status = I2C_Error;
			break;
		}

		//Transmit read address
		status = I2C_Transmit(SubAddress);
		if ((status != MT_DATA_TRANSMITTED_ACK) && (status != MT_DATA_TRANSMITTED_NACK))
		{
			status = I2C_Error;
			break;
		}

		//Transmit START signal
		status = I2C_BeginTransmission();
		if ((status != MR_START_TRANSMITTED) && (status != MR_REP_START_TRANSMITTED))
		{
			status = I2C_Error;
			break;
		}

		//Transmit SLA+R
		status = I2C_Transmit(__I2C_SLA_R(SLA));
		if ((status != MR_SLA_R_TRANSMITTED_ACK) && (status != MR_SLA_R_TRANSMITTED_NACK))
		{
			status = I2C_Error;
			break;
		}

		//Receive DATA
		//Read all the bytes, except the last one, sending ACK signal
		for (i = 0 ; i < (Length - 1) ; i++)
		{
			Packet[i] = I2C_ReceiveACK();
			status = I2C_Status();
			if ((status != MR_DATA_RECEIVED_ACK) && (status != MR_DATA_RECEIVED_NACK))
			{
				status = I2C_Error;
				break;
			}
		}
		//Receive last byte and send NACK signal
		Packet[i] = I2C_ReceiveNACK();
		status = I2C_Status();
		if ((status != MR_DATA_RECEIVED_ACK) && (status != MR_DATA_RECEIVED_NACK))
		{
			status = I2C_Error;
			break;
		}

		//Received successfully
		status = I2C_Ok;
	}
	while (0);

	//Transmit STOP signal
	I2C_EndTransmission();
	
	return (I2C_Status_t)status;
}

//Set self slave address.
void I2C_SetAddress(const uint8_t Address)
{
	//Set TWI slave address (upper 7 bits)
	TWAR = Address<<1;
}
//-----------------------------//