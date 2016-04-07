#define F_CPU 8000000

#include <avr/io.h>
#include <util/delay.h>

#include "i2c_master.h"
#include "spi_master.h"

#define LED_BIT PC2
#define GND_BIT PB6
#define VCC_BIT PC1

//#define LED_HIGH PORTC |= _BV(LED_BIT)
//#define LED_LOW PORTC &= ~(_BV(LED_BIT))



void main(void) 
{
	//uint8_t* data;
	//(uint8_t*) malloc(sizeof(uint8_t) * 2);
	uint8_t data[2];
	
	i2c_init();
	spi_init();
	DDRC |= (1 << LED_BIT);
	DDRC |= (1 << VCC_BIT);
	DDRB |= (1 << GND_BIT);		// set pins to output
	
	PORTC |= _BV(VCC_BIT);		// set VCC pin to high
	PORTB &= ~(_BV(GND_BIT));	// set GND pin to low
	
	
	while (1) 
	{
		/*
		i2c_receive(0b10011010,data, 2);	// get 2 bytes of sensor data
		//i2c_stop();							// single conversion
		PORTC ^= _BV(LED_BIT);				// toggle LED
		_delay_ms(1);
		*/
		
		data[0]=0x49;
		data[1]=0x00;
		spi_writeSingle(0xFF);
		spi_writeBytes(data, 2);
		//data[0]=0x00;
		//data[1]=0x00;
		//spi_writeBytes(data, 2);
		
		PORTC ^= _BV(LED_BIT);				// toggle LED
		_delay_ms(10);
	}
	return 0;
}

