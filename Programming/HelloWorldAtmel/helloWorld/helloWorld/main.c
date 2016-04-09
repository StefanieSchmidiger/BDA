#define F_CPU 8000000

#include <avr/io.h>
#include <util/delay.h>

#include "i2c_master.h"
#include "spi_master_bitbanging.h"

#define GND_BIT PC3
#define VCC_BIT PC2

//#define LED_HIGH PORTC |= _BV(LED_BIT)
//#define LED_LOW PORTC &= ~(_BV(LED_BIT))



void main(void) 
{
	//uint8_t* data;
	//(uint8_t*) malloc(sizeof(uint8_t) * 2);
	uint8_t data[2];
	
	i2c_init();
	spi_init_bitbanging();
	
	DDRC |= (1 << VCC_BIT);
	DDRC |= (1 << GND_BIT);		// set pins to output
	
	
	PORTC |= _BV(VCC_BIT);		// set VCC pin to high
	PORTC &= ~(_BV(GND_BIT));	// set GND pin to low
	
	
	while (1) 
	{
		/*
		i2c_receive(0b10011010,data, 2);	// get 2 bytes of sensor data
		//i2c_stop();							// single conversion
		PORTC ^= _BV(LED_BIT);				// toggle LED
		_delay_ms(1);
		*/
		data[0]=0x04;
		data[1]= 0x09;
		spi_sendBytes_bitbanging(data, 2);
		_delay_us(500);
		spi_sendBytes_bitbanging(data, 2);
		data[0] = 0x49;
		_delay_ms(10);
	}
	return 0;
}

