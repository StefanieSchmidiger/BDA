/*
 * IsrSetupAtmel.c
 *
 * Created: 31.03.2016 15:28:27
 * Author : Stefanie
 */ 
#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <util/twi.h>
// ----- custom includes --------
#include "i2c_master.h"
#include "spi_master_bitbanging.h"
// ----- custom defines ---------
#define GND_BIT PC3
#define VCC_BIT PC2

// ----- prototypes -------
void pinSetup(void);
// ----- global variables ---------
uint8_t i2cData[2];
uint8_t spiData[2];

int main(void)
{
	pinSetup();	// setup VCC, GND and LED pins
	i2c_init();	// setup TWI
	spi_init_bitbanging();
	sei();			// enable global interrupts
	_delay_ms(1000);
    while (1) 
    {
		i2c_start();
		spiData[0]=0x49;
		spiData[1]=0x10;
		spi_sendBytes_bitbanging(spiData, 2);
		_delay_us(150);
    }
}

void pinSetup(void)
{
		DDRC |= (1 << VCC_BIT);
		DDRC |= (1 << GND_BIT);		// set pins to output
		
		PORTC |= _BV(VCC_BIT);		// set VCC pin to high
		PORTC &= ~(_BV(GND_BIT));	// set GND pin to low
}


ISR(TWI_vect)
{
	static uint8_t i2cDataTemp[2];
	if((TWSR & 0xF8) == TW_START)					// start condition successfully transmitted
	{
		TWDR = 0b10011011;							// load address(1001101x) and read(1) into data register
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);				// start transmission of address
	}
	else if ( ((TW_STATUS & 0xF8) == TW_MT_SLA_ACK) || ((TW_STATUS & 0xF8) == TW_MR_SLA_ACK) ) // address+read/write acknowledged
	{
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA) | (1<<TWIE); // start TWI module and acknowledge data after reception
	}
	else /*if((TW_STATUS & 0xF8) == TW_MR_DATA_ACK)*/			// data received and acknowledge sent
	{
		static uint8_t byteNumber=0;
		i2cDataTemp[byteNumber] = TWDR;	// data structure from sensor, B1: 0 0 0 0  b10 b9 b8 b7       B2: b6 b5 b4 b3  b2 b1 x x
		byteNumber++;
		if(byteNumber == 2)							// 2nd byte was sent -> send STOP
		{
			TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);	// transmit STOP condition. disable with endless data acquiry
			//TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA) | (1<<TWIE); // endless data acquiry enable, request 1st byte again
			byteNumber = 0;							// overwrite data the next time
			ATOMIC_BLOCK(ATOMIC_FORCEON)			// write i2cData into global array
			{
				i2cData[0]=(0x3F & (i2cDataTemp[0]/4))|((uint8_t)(i2cDataTemp[1]*64));	// store data in right format
				i2cData[1]=i2cDataTemp[1]/4;
			}
			
		}
		else	// after transmission of 1st data byte
		{
			TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE); // start TWI module for 2nd byte and NACK data after reception (signals end of reception)
		}
	}
}