/*
 * spi_master_bitbanging.h
 *
 * Created: 09.04.2016 17:02:11
 *  Author: Stefanie
 */ 


#ifndef SPI_MASTER_BITBANGING_H_
#define SPI_MASTER_BITBANGING_H_


void spi_init_bitbanging(void);
unsigned char spi_sendSingle_bitbanging(unsigned char cData); // MSB first, clk idle low and MOSI idle low
unsigned char* spi_sendBytes_bitbanging(unsigned char* cData, int length); // MSB first, clk idle low and MOSI idle low


#endif /* SPI_MASTER_BITBANGING_H_ */