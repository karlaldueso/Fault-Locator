/*
 * Serial.cpp
 *
 * Created: 11/21/2023 7:08:00 PM
 *  Author: User
 */ 
#include <Serial.h>

SerialClass::SerialClass()
{
	
}

//ISR(USART1_RX_vect)
//{
	////d = UDR1;
//}

//volatile const char *tempptrdata;
//volatile const char *ptrdata;

//ISR(USART1_UDRE_vect)
//{
	//if(UCSR1A&(1<<UDRE1))
	//{
		//UDR1 = *tempptrdata;
		//if (*tempptrdata == '\0')
		//{
			//tempptrdata = ptrdata;
			//UCSR1B &= ~(1<<UDRE1);
		//}
		//tempptrdata++;
	//}
//}

void SerialClass::begin(uint16_t baud)
{
	UBRR1 = F_CPU/16/baud-1;
	UCSR1B = (1<<TXEN1)|(1<<RXEN1)|(1<<RXCIE1); //Enable Tx and Rx
	UCSR1C = (1<<UCSZ11)|(1<<UCSZ10);	//8-bits, no parity, 1 stop bit
}

void SerialClass::enableRxinterrupt(void)
{
	UCSR1B |= (1<<RXCIE1); //Enable RX interrupt
	sei();
}

uint8_t SerialClass::readbyte(void)
{
	while(!(UCSR1A&(1<<RXC1))); //wait for data
	return UDR1;
}

void SerialClass::sendByte(uint8_t data)
{
	while(!(UCSR1A&(1<<UDRE1))); //wait for empty transmit buffer/data register
	UDR1 = data;
}

void SerialClass::sendString(const char *str)
{
	while (1)
	{
		if (*str == '\0')
		{
			break;
		}
		sendByte(*str);
		str++;
	}
}

bool SerialClass::available(void)
{
	bool available;
	
	if(!(UCSR1A&(1<<RXC1))) //check for data
		available = false;
	else
		available = true;
	return available;
}