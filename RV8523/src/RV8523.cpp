/*
 * RV8523.cpp
 *
 * Created: 11/23/2023 9:38:39 AM
 *	Reference: Watterott electronic
 *  Author: KMAA
 */ 

#include "RV8523.h"
#include "I2C.h"

#define I2C_ADDR (0xD0>>1)


//-------------------- Constructor --------------------


RV8523::RV8523(void)
{
	return;
}


//-------------------- Public --------------------


void RV8523::begin(void)
{
	I2C_Setup(); //init I2C lib
}


void RV8523::start(void)
{
	uint8_t val;
	I2C_PacketReceive(I2C_ADDR, 0x00, &val, 1); //Read reg Control 1
	
	if(val & (1<<5))
	{
		val = val & ~(1<<5); //clear bit 5 (STOP)
		I2C_PacketTransmit(I2C_ADDR, 0x00, &val, 1); //Write reg Control 1
	}

	return;
}


void RV8523::stop(void)
{
	uint8_t val;
	I2C_PacketReceive(I2C_ADDR, 0x00, &val, 1); //Read reg Control 1
	
	if(!(val & (1<<5)))
	{
		val = val | (1<<5); //set bit 5 (STOP)
		I2C_PacketTransmit(I2C_ADDR, 0x00, &val, 1); //Write reg Control 1
	}

	return;
}


void RV8523::set12HourMode(void) //set 12 hour mode
{
	uint8_t val;
	I2C_PacketReceive(I2C_ADDR, 0x00, &val, 1); //Read reg Control 1
	
	if(!(val & (1<<3)))
	{
		val = val | (1<<3); //Set bit 3 (12HR mode)
		I2C_PacketTransmit(I2C_ADDR, 0x00, &val, 1); //Write reg Control 1
	}
	
	return;
}


void RV8523::set24HourMode(void) //set 24 hour mode
{
	uint8_t val;
	I2C_PacketReceive(I2C_ADDR, 0x00, &val, 1); //Read reg Control 1
	
	if(val & (1<<3))
	{
		val = val & ~(1<<3); //Clear Bit 3 (24HR mode)
		I2C_PacketTransmit(I2C_ADDR, 0x00, &val, 1); //Write reg Control 1
	}

	return;
}


void RV8523::batterySwitchOver(int on) //activate/deactivate battery switch over mode
{
	uint8_t val;
	I2C_PacketReceive(I2C_ADDR, 0x02, &val, 1); //Read reg Control 3
	
	if(val & 0xE0)
	{
		if(on)
		{
			val = val  & ~0xE0; //clear bits7-5 (Battery switchover in standard mode)
		}
		else
		{
			val = val | 0xE0; //set bits7-5 (Battery switchover disabled)
		}
		I2C_PacketTransmit(I2C_ADDR, 0x02, &val, 1); //write reg Control 3
	}

	return;
}


void RV8523::get(uint8_t *sec, uint8_t *min, uint8_t *hour, uint8_t *day, uint8_t *month, uint16_t *year)
{
	uint8_t val[7];
	I2C_PacketReceive(I2C_ADDR, 0x03, val, 7); //Read reg Control 1
	
	*sec   = bcd2bin(val[0] & 0x7F);
	*min   = bcd2bin(val[1] & 0x7F);
	*hour  = bcd2bin(val[2] & 0x3F); //24 hour mode
	*day   = bcd2bin(val[3] & 0x3F);
	*month = bcd2bin(val[5] & 0x1F);
	*year  = bcd2bin(val[6]) + 2000;

	return;
}


//void RV8523::get(int *sec, int *min, int *hour, int *day, int *month, int *year)
//{
	////Wire.beginTransmission(I2C_ADDR);
	////Wire.write(byte(0x03));
	////Wire.endTransmission();
////
	////Wire.requestFrom(I2C_ADDR, 7);
	////*sec   = bcd2bin(Wire.read() & 0x7F);
	////*min   = bcd2bin(Wire.read() & 0x7F);
	////*hour  = bcd2bin(Wire.read() & 0x3F); //24 hour mode
	////*day   = bcd2bin(Wire.read() & 0x3F);
	////bcd2bin(Wire.read() & 0x07); //day of week
	////*month = bcd2bin(Wire.read() & 0x1F);
	////*year  = bcd2bin(Wire.read()) + 2000;
//
	//return;
//}


void RV8523::set(uint8_t sec, uint8_t min, uint8_t hour, uint8_t day, uint8_t month, uint16_t year)
{
	
	if(year > 2000)
	{
		year -= 2000;
	}
	
	sec = bin2bcd(sec);
	min = bin2bcd(min);
	hour = bin2bcd(hour);
	day = bin2bcd(day);
	month = bin2bcd(month);
	year = bin2bcd(year);
	
	uint8_t val[7] = {sec, min, hour, day, 0, month, (uint8_t)year};
		
	I2C_PacketTransmit(I2C_ADDR, 0x03, val, 7); //Set time
	

	return;
}


//void RV8523::set(int sec, int min, int hour, int day, int month, int year)
//{
	//return set((uint8_t)sec, (uint8_t)min, (uint8_t)hour, (uint8_t)day, (uint8_t)month, (uint16_t)year);
//}


//-------------------- Private --------------------


uint8_t RV8523::bin2bcd(uint8_t val)
{
	return val + 6 * (val / 10);
}


uint8_t RV8523::bcd2bin(uint8_t val)
{
	return val - 6 * (val >> 4);
}