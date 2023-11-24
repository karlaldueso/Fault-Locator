/*
 * Fault Locator.cpp
 *
 * Created: 11/21/2023 2:08:33 PM
 * Author : KMAA
 */ 

#define F_CPU 8000000UL
#define DEVICEID 17

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ADE9153A.h>
#include <ADE9153AAPI.h>
#include <RV8523.h>
#include <Serial.h>

//Fcn declarations
void resetADE(void);
void startTimer(void);
void stopTimer(void);
//For LED indicators
void init_signal_leds(void);
void init_RYG_leds(void);
void set_signal_leds(int on);
void set_RYG_leds(int on);

//Classes
ADE9153AClass ADE9153A;
SerialClass Serial;
RV8523 RTC;

//For ADE9153A
bool commscheck = false;
struct EnergyRegs energyVals;  //Energy register values are read and stored in EnergyRegs structure
struct PowerRegs powerVals;    //Metrology data can be accessed from these structures
struct RMSRegs rmsVals;
struct PQRegs pqVals;
struct AcalRegs acalVals;
struct Temperature tempVal;
double irms,vrms,rp,qp,sp,pf,freq,temp;

//For RTC
uint8_t sec, min, hour, day, month;
uint16_t year;
uint8_t i = 0;
uint16_t setTime[6];

//For data sending/receiving
char myString[100];
char incomingString[100];
char *ptrincomingString = incomingString;
char *tokenStrings;
volatile uint8_t d;
volatile bool sendData = false;
volatile bool dataComplete = false;
volatile int counter_rx = 0;
int counter = 0;
ISR(USART1_RX_vect)
{
	d = UDR1;
	Serial.sendByte(d);
	*ptrincomingString = d;
	ptrincomingString++;
	if (d == '@') 
	{
		*ptrincomingString = '\0';
		ptrincomingString = incomingString;
		dataComplete = true;
	}
}

//Timer Interrupt
bool timerStarted = false;
ISR(TIMER1_COMPA_vect)
{
	sendData = true;
	counter_rx++;
}

//-----------------------Main fcn--------------------------------
int main(void)
{
	CLKPR = 0b10000000; //Enable CLKPR change
	CLKPR = 0b00000000; //Set CLK prescaler to div 1
	
	//Serial Initialization
	Serial.begin(2400);
	
	//ADE9153A Initialization
	DDRF |= (1 << PINF0);//ADE RESET PIN AS OUTPUT
	PORTF |= (1 << PINF0);//Set ADE RESET PIN
	resetADE();
	_delay_ms(1000);
	commscheck = ADE9153A.SPI_Init(&PORTB, PINB2, PINB3, PINB1, PINB0);
	if (!commscheck) 
	{
		Serial.sendString("ADE9153A not detected!\r\n");
		while (!commscheck) 
		{    
			_delay_ms(1000);
		}
	}
	else
	{
		Serial.sendString("ADE9153A detected!\r\n");
	}
	ADE9153A.SetupADE9153A();
	ADE9153A.SPI_Write_32(REG_AIGAIN, -268435456); //AIGAIN to -1 to account for IAP-IAN swap
	
	//RTC Initialization
	RTC.begin();
	RTC.set24HourMode();
	RTC.batterySwitchOver(1);
	RTC.start();
	_delay_ms(500);
	
	//LED inidactors initialization
	init_signal_leds();
	init_RYG_leds();
	set_signal_leds(1);
	set_RYG_leds(1);
	sei();
    /* Replace with your application code */
    while (1) 
    {
		//GET ADE9153A VALUES
		ADE9153A.ReadRMSRegs(&rmsVals);
		vrms = rmsVals.VoltageRMSValue / 1000.0;
		
		//SENDING DATA
		if(vrms < 210)
		{
			sprintf(myString, "$,%d,%0.2f,@\r\n", DEVICEID, vrms);
			Serial.sendString(myString);
			if(timerStarted)
			{
				stopTimer();
				set_signal_leds(0);
				set_RYG_leds(0);
			}
		}
		else
		{
			//GET ADE9153A VALUES
			//ADE9153A.ReadPowerRegs(&powerVals);    //Template to read Power registers from ADE9000 and store data in Arduino MCU
			ADE9153A.ReadRMSRegs(&rmsVals);
			ADE9153A.ReadPQRegs(&pqVals);
			//ADE9153A.ReadTemperature(&tempVal);
			irms = rmsVals.CurrentRMSValue / 1000.0;
			vrms = rmsVals.VoltageRMSValue / 1000.0;
			//rp = powerVals.ActivePowerValue / 1000.0;
			//qp = powerVals.FundReactivePowerValue / 1000.0;
			//sp = powerVals.ApparentPowerValue / 1000.0;
			//pf = pqVals.PowerFactorValue;
			freq = pqVals.FrequencyValue;
			//temp = tempVal.TemperatureVal;
			//GET DATETIME
			RTC.get(&sec, &min, &hour, &day, &month, &year);
			if(!timerStarted)
			{
				startTimer();
			}
			if(sendData)
			{
				//sprintf(myString, "$,%d-%d-%d %d:%d:%d,@\n", year, month, day, hour, min, sec);
				sprintf(myString, "$,%d,%0.2f,%0.2f,%0.2f,%d-%d-%d %d:%d:%d,@\r\n", DEVICEID, vrms, irms, freq, year, month, day, hour, min, sec);
				Serial.sendString(myString);
				sendData = false;
				set_RYG_leds(2);
			}
		}
		
		//FOR SETTING RTC
		if(dataComplete)
		{
			counter = strlen(incomingString);
			if((*(incomingString) == '$') && (*(incomingString+counter-1) == '@'))
			{
				tokenStrings = strtok(incomingString, ",");
				while(tokenStrings)
				{
					if((*(tokenStrings) == '$') || (*(tokenStrings) == '@'))
					{
						tokenStrings = strtok(NULL, ",");
					}
					else
					{
						setTime[i] = atoi(tokenStrings);
						tokenStrings = strtok(NULL, ",");
						i++;
					}
				}
				strcpy(incomingString,"");
				dataComplete = false;
				i = 0;
				counter_rx = 0;
				RTC.set((uint8_t)setTime[0],(uint8_t)setTime[1],(uint8_t)setTime[2],(uint8_t)setTime[3],(uint8_t)setTime[4],setTime[5]);
			}
		}else if(counter_rx >= 3)
		{
			strcpy(incomingString,"");
			dataComplete = false;
			ptrincomingString = incomingString;
			i = 0;
			counter_rx = 0;
		}
    }
}
//----------------------------------end Main------------------------------


//FUNCTIONS
void resetADE(void)
{
	PORTF &= ~(1 << PINF0);//Set ADE RESET PIN
	_delay_ms(500);
	PORTF |= (1 << PINF0);//Set ADE RESET PIN
}

void startTimer(void)
{
	/*	CTC Mode 
	2^16 - 1 = 65,535; TCNT1
	clkdiv = 256
	timerCPU = F_CPU/clkdiv = 31.250kHz
	one clock period = 1/timerCPU = 32us
	specified_delay = 1s
	(specified_delay/32us)-1 = 31249 //OCR1A value
	*/
	TCCR1B |= (1<<WGM12)|(1<<CS12); ////CTC mode & clkdiv = 256;
	TCNT1 = 0; //clear timer
	TIMSK1 |= (1<<OCIE1A); //Timer compare A interrupt enable
	OCR1A = 31249; //equivalent to 1 second
	timerStarted = true;
}

void stopTimer(void)
{
	TCCR1B &= ~((1<<WGM12)|(1<<CS12)); //disable timer;
	TIMSK1 &= ~(1<<OCIE1A); //Timer compare A interrupt disable
	timerStarted = false;
}

void init_signal_leds(void)
{
	DDRD |= (1<<PIND7)|(1<<PIND5);
	DDRB |= (1<<PINB7)|(1<<PINB6)|(1<<PINB5);
}

void init_RYG_leds(void)
{
	DDRD |= (1<<PIND6)|(1<<PIND4);
	DDRF |= (1<<PIND1);
}

void set_signal_leds(int on)
{
	if(on)
	{
		PORTD |= (1<<PIND7)|(1<<PIND5);
		PORTB |= (1<<PINB7)|(1<<PINB6)|(1<<PINB5);
	}
	else
	{
		PORTD &= ~((1<<PIND7)|(1<<PIND5));
		PORTB &= ~((1<<PINB7)|(1<<PINB6)|(1<<PINB5));
	}
}

void set_RYG_leds(int on)
{
	if(on == 1)
	{
		PORTD |= (1<<PIND6)|(1<<PIND4);
		PORTF |= (1<<PIND1);
	}
	else if(on == 2)
	{
		_delay_ms(10);
		PORTD |= (1<<PIND6)|(1<<PIND4);
		PORTF &= ~(1<<PIND1);
		_delay_ms(10);
		PORTD |= (1<<PIND4);
		PORTD &= ~(1<<PIND6);
		PORTF |= (1<<PIND1);
	}
	else
	{
		PORTD &= ~((1<<PIND6)|(1<<PIND4));
		PORTF &= ~((1<<PIND1));
	}
}