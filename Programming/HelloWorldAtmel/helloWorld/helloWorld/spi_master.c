#include <avr/io.h>
#include <util/delay.h>

#define MISO PB4
#define MOSI PB3
#define nSS	PB2
#define SCL	PB5

void spi_selectSlaveISR(void);
void spi_unselectSlaveISR(void);


void  spi_init( ) 
{
	DDRB |=  ((1<<MOSI) | (1<<SCL) | (1<<nSS)) ;	// set MOSI, SCL, SS as output
	DDRB &=  (~(1<<MISO));	// set MISO as input
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPI2X)|(1<<SPR0)|(1<<CPHA);    // setup SPI: Master mode, SCK phase low, SCK idle low, SCK = fcpu/2
	//SPCR &= ~((1<<CPOL) | (1<<DORD) | (1<<SPR1));				// setup SPI: MSB first, SCK idle low
	SPSR = 0x00;
	PORTB |= _BV(nSS);		// set SS to high -> slave not selected
}

uint8_t spi_writeSingle(uint8_t cData) 
{
	spi_selectSlaveISR();
	//PORTB &= ~(_BV(MISO));
	DDRB &=  (~(1<<MISO));	// set MISO as input
	SPCR |= (1<<MSTR);
	
	SPDR = cData;
	while ( !(SPSR & (1<<SPIF)));		// wait for transmission to complete
	spi_unselectSlaveISR();
	return SPDR;						// read data -> clear SPIF
}

void spi_writeBytes(uint8_t* cData, uint8_t numOfBytes)
{
	spi_selectSlaveISR();
	SPCR |= (1<<MSTR);
	int i;
	for(i=0;i<numOfBytes;i++)
	{
		SPDR = cData[i];
		while ( !(SPSR & (1<<SPIF)));		// wait for transmission to complete
		cData[i] = SPDR;					// read data -> clear SPIF
	}
	spi_unselectSlaveISR();
}

uint8_t spi_read(void) 
{
	SPDR = 0xff;
	while ( !(SPSR & (1<<SPIF)));		// wait for transmission to complete
	return SPDR;						// read data -> clear SPIF
}

void spi_selectSlaveISR(void)
{
	PORTB &= ~(_BV(nSS));		// set SS to low -> slave selected
}

void spi_unselectSlaveISR(void)
{
	PORTB |= _BV(nSS);		// set SS to high -> slave not selected
}