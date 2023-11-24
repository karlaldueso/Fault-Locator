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


#ifndef I2C_H_
#define I2C_H_
/*
||
||  Filename:	 		TWI.h
||  Title: 			    TWI Driver
||  Author: 			Efthymios Koktsidis
||	Email:				efthymios.ks@gmail.com
||  Compiler:		 	AVR-GCC
||	Description:
||	This library can utilize the TWI hardware of AVR microcontrollers.
||
*/

//----- Headers ------------//
#include <inttypes.h>
#include <avr/io.h>

#include "IO_Macros.h"
#include "I2C_Settings.h"
//--------------------------//

//----- Auxiliary data ---------------------------//
enum I2C_Status_t
{
	I2C_Error,
	I2C_Ok
};

enum I2C_Status_MT_t
{
	MT_START_TRANSMITTED				= 0x08,
	MT_REP_START_TRANSMITTED			= 0x10,
	MT_SLA_W_TRANSMITTED_ACK			= 0x18,
	MT_SLA_W_TRANSMITTED_NACK			= 0x20,
	MT_DATA_TRANSMITTED_ACK				= 0x28,
	MT_DATA_TRANSMITTED_NACK			= 0x30,
	MT_SLA_W_ARB_LOST					= 0x38
};

enum I2C_Status_MR_t
{
	MR_START_TRANSMITTED				= 0x08,
	MR_REP_START_TRANSMITTED			= 0x10,
	MR_SLA_R_ARB_LOST					= 0x38,
	MR_SLA_R_TRANSMITTED_ACK			= 0x40,
	MR_SLA_R_TRANSMITTED_NACK			= 0x48,
	MR_DATA_RECEIVED_ACK				= 0x50,
	MR_DATA_RECEIVED_NACK				= 0x58
};

enum I2C_Status_SR_t
{
	SR_SLA_W_RECEIVED_ACK				= 0x60,
	SR_SLA_RW_ARB_LOST					= 0x68,
	SR_GEN_ADDR_RECEIVED_ACK			= 0x70,
	SSR_SLA_RW_ARB_LOST_2				= 0x78,
	SR_DATA_RECEIVED_SLA_ACK			= 0x80,
	SR_DATA_RECEIVED_SLA_NACK			= 0x88,
	SR_DATA_RECEIVED_GEN_ADDR_ACK		= 0x90,
	SR_DATA_RECEIVED_GEN_ADDR_NACK		= 0x98,
	SR_STOP_REP_START					= 0xA0
};

enum I2C_Status_ST
{
	ST_SLA_R_RECEIVED_ACK				= 0xA8,
	ST_SLA_RW_ARB_LOST					= 0xB0,
	ST_DATA_TRANSMITTED_ACK				= 0xB8,
	ST_DATA_TRANSMITTED_NACK			= 0xC0,
	ST_LAST_DATA_TRANSMITTED_ACK		= 0xC8
};

//------------------------------------------------//

//----- Prototypes ---------------------------------------------------//
void I2C_Setup(void);
uint8_t I2C_BeginTransmission(void);
void I2C_EndTransmission(void);
uint8_t I2C_Status(void);

uint8_t I2C_Transmit(uint8_t Data);
uint8_t I2C_ReceiveACK(void);
uint8_t I2C_ReceiveNACK(void);

enum I2C_Status_t I2C_PacketTransmit(const uint8_t SLA, const uint8_t SubAddress, uint8_t *Packet, const uint8_t Length);
enum I2C_Status_t I2C_PacketReceive(const uint8_t SLA, const uint8_t SubAddress, uint8_t *Packet, const uint8_t Length);

void I2C_SetAddress(const uint8_t Address);
//--------------------------------------------------------------------//


#endif /* I2C_H_ */