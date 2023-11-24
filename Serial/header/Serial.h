/*
 * Serial.h
 *
 * Created: 11/21/2023 7:07:48 PM
 *  Author: User
 */ 


#ifndef SERIAL_H_
#define SERIAL_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

class SerialClass
{
	public:
	SerialClass(void);
	void begin(uint16_t baud);
	void enableRxinterrupt(void);
	uint8_t readbyte(void);
	void sendByte(uint8_t data);
	void sendString(const char *str);
	bool available(void);
	
};




#endif /* SERIAL_H_ */