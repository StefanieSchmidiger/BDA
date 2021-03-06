/*
 * spi_master_bitbanging.c
 *
 * Created: 09.04.2016 16:34:23
 *  Author: Stefanie
 */ 
#ifndef F_CPU
	#define F_CPU 8000000UL
#endif
#include <avr/io.h>
#include <util/delay.h>
#include "spi_master_bitbanging.h"

#define MISO PD3
#define MOSI PD1
#define nSS	PB6
#define CLK PD0	

// ------- prototypes only needed in this file -----------
//void spi_selectSlave(void);
//void spi_unselectSlave(void);
unsigned char spi_sendSingleByte(unsigned char cData);
void spi_send16Bits(unsigned char* cData);
void spi_selectSlave(void);
void spi_unselectSlave(void);

void spi_init_bitbanging(void)
{
	DDRD |=  ((1 << MOSI) | (1<<CLK)) ;	// set MOSI, CLK as output
	DDRB |=  (1 << nSS); // set nSS as output
	DDRD &= (~(1 << MISO)); // set MISO as input
	PORTD &= ~((1 << MOSI) | (1 << CLK));	// set MOSI and CLK as idle low
	DDRB |= (1 << nSS);	// set nSS to high (slave not selected)
}

unsigned char spi_sendSingle_bitbanging(unsigned char cData) // MSB first, clk idle low and MOSI idle low
{
	unsigned char retData;
	spi_selectSlave();
	//_delay_us(1);
	retData = spi_sendSingleByte(cData);
	spi_unselectSlave();
	return retData;
}

void spi_sendBytes_bitbanging(unsigned char* cData, int length) // MSB first, clk idle low and MOSI idle low
{
	spi_selectSlave();
	//_delay_us(1);
	for(int i=0; i<length; i++)
	{
		cData[i] = spi_sendSingleByte(cData[i]);
		//if(i<(length-1))
			//_delay_us(10);
	}
	spi_unselectSlave();
}

void spi_send16Bits_bitbanging(unsigned char* cData) // MSB first, clk idle low and MOSI idle low
{
	spi_selectSlave();
	//_delay_us(1);
	spi_send16Bits(cData);
	//_delay_us(1);
	spi_unselectSlave();
}



unsigned char spi_sendSingleByte(unsigned char cData)
{
	unsigned char retData = 0x00;
	for(int i = 0; i<8; i++)
	{
		PORTD |= (1 << CLK);	// clock high (leading edge)
		//_delay_us(1);
		// set data bit
		if(cData & 0b10000000)
		PORTD |= (1 << MOSI);
		else
		PORTD &= (~(1 << MOSI));
		cData = (unsigned char) (cData * 2);	// set pointer to next data bit
		PORTD &= (~(1 << CLK));	// clock low (sampling edge)
		retData = retData * 2;
		if((PIND & (1 << MISO)))	// set retData bit according to MISO state
		retData ++;
		//_delay_us(1);
	}
	PORTD &= (~(1 << MOSI));	// set MOSI idle low
	return retData;
}

void spi_send16Bits(unsigned char* cData)
{
	unsigned char retData[2];
	retData[0]=0;
	retData[1]=0;
	for(int j=0 ; j<2 ; j++)
	{
		for(int i = 0; i<8; i++)
		{
			PORTD |= (1 << CLK);	// clock high (leading edge)
			// set data bit
			if(cData[j] & 0b10000000)
				PORTD |= (1 << MOSI);
			else
				PORTD &= (~(1 << MOSI));
			cData[j] = (unsigned char) (cData[j] * 2);	// set pointer to next data bit
			PORTD &= (~(1 << CLK));	// clock low (sampling edge)
			retData[j] = retData[j] * 2;
			if((PIND & (1 << MISO)))	// set retData bit according to MISO state
				retData[j] ++;
		}
		cData[j]=retData[j];
	}
	PORTD &= (~(1 << MOSI));	// set MOSI idle low
}




void spi_selectSlave(void)
{
	PORTB &= (~(1 << nSS));	// select slave
}

void spi_unselectSlave(void)
{
	PORTB |= (1 << nSS);	// unselect slave
}
