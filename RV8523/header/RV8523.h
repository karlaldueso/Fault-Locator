/*
 * RV8523.cpp
 *
 * Created: 11/23/2023 9:38:39 AM
 *	Reference: Watterott electronic
 *  Author: KMAA
 */ 


#ifndef RV8523_H_
#define RV8523_H_

#include <inttypes.h>


class RV8523
{
	public:
	RV8523();

	void begin(void);
	void start(void);
	void stop(void);
	void get(uint8_t *sec, uint8_t *min, uint8_t *hour, uint8_t *day, uint8_t *month, uint16_t *year);
	//void get(int *sec, int *min, int *hour, int *day, int *month, int *year);
	void set(uint8_t sec, uint8_t min, uint8_t hour, uint8_t day, uint8_t month, uint16_t year);
	//void set(int sec, int min, int hour, int day, int month, int year);
	void set12HourMode(void);
	void set24HourMode(void);
	void batterySwitchOver(int on);
	
	private:
	uint8_t bin2bcd(uint8_t val);
	uint8_t bcd2bin(uint8_t val);
};



#endif /* RV8523_H_ */