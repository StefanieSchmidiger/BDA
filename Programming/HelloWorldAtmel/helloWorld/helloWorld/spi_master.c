#include <avr/io.h>

#define DD_MISO PB4
#define DD_MOSI PB3
#define DD_nSS	PB2
#define DD_SCL	PB5


void  spi_init( ) 
{
	DDRB |=  ((1<<DD_MOSI) | (1<<DD_SCL) | (1<<DD_nSS)) ;	// set MOSI, SCL, SS as output
	DDRB &=  ~(1<<DD_MISO) ;	// set MISO as input
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPI2X)|(1<<SPR0)|(1<<CPHA);    // setup SPI: Master mode, SCK phase low, SCK idle low, SCK = fcpu/2
	SPCR &= ~((1<<CPOL) | (1<<DORD) | (1<<SPR1));				// setup SPI: MSB first, SCK idle low
	SPSR = 0x00;
}

uint8_t spi_writeSingle(uint8_t cData) 
{
	SPDR = cData;
	while ( !(SPSR & (1<<SPIF)));		// wait for transmission to complete
	return SPDR;						// read data -> clear SPIF
}

void spi_writeBytes(uint8_t* cData, uint8_t numOfBytes)
{
	SPCR |= (1<<MSTR);
	int i;
	for(i=0;i<numOfBytes;i++)
	{
		SPDR = cData[i];
		while ( !(SPSR & (1<<SPIF)));		// wait for transmission to complete
		cData[i] = SPDR;					// read data -> clear SPIF
	}
}

uint8_t spi_read(void) 
{
	SPDR = 0xff;
	while ( !(SPSR & (1<<SPIF)));		// wait for transmission to complete
	return SPDR;						// read data -> clear SPIF
}