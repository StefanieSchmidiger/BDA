#ifndef SPI_MASTER_H
#define SPI_MASTER_H


void  spi_init(void);
uint8_t spi_writeSingle(uint8_t cData);
void spi_writeBytes(uint8_t* cData, uint8_t numOfBytes);
uint8_t spi_read(void);

#endif // SPI_MASTER_H	