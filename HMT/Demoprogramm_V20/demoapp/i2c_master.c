/*
 * i2c_master.c
 *
 * Created: 31.03.2016 15:28:58
 *  Author: Stefanie
 */ 
#include <avr/io.h>
#include <util/twi.h>
#include <avr/interrupt.h>

void i2c_init(void)
{
	OSCCAL = 0xFF;				// select  highest possible frequency with RC generator
	CLKPR = 0b10000000;
	CLKPR = 0b10000000;
	TWBR = 10;		// SCL frequency 400kHz
	// sei();			// global interrupt enable
}

void i2c_start(void)
{
	// transmit START condition
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);
}

