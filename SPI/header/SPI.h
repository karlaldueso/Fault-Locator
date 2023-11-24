/*
 * SPI.h
 *
 * Created: 11/21/2023 3:13:43 PM
 *  Author: KMAA
 */ 


#ifndef SPI_H_
#define SPI_H_

#include <avr/io.h>
#include <util/delay.h>

class SPIclass
{
public:
	SPIclass(void);
	void begin(void);
	uint8_t transmit(uint8_t data);
};



#endif /* SPI_H_ */