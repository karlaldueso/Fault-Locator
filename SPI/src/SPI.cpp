/*
 * SPI.cpp
 *
 * Created: 11/21/2023 3:13:57 PM
 *  Author: KMAA
 */ 
#include <SPI.h>

SPIclass::SPIclass()
{
	
}

void SPIclass::begin(void)
{
	//MSB First, MODE0
	SPCR = (1<<SPE)|(1<<MSTR); 
}

uint8_t SPIclass::transmit(uint8_t data)
{
	SPDR = data; //transmit data
	while(!(SPSR&(1<<SPIF))); //Wait for transmission complete
	return SPDR;
}