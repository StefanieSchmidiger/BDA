/*
 * serialCommunication.c
 *
 *  Created on: 14.03.2016
 *      Author: Stefanie
 */

#include "I2C.h"
#include "SPI.h"


char spiData[5];
char i2cData[5];
SPI_TComData spiTxData;
SPI_TComData spiRxData;

void updateSerial (void)
{
  word* i2cSize;
  spiTxData = 0;
  // 2 Bytes of SPI
  SPI_SendChar(spiTxData);
  spiData[0]=SPI_RecvChar(&spiRxData);
  SPI_SendChar(spiTxData);
  spiData[0]=SPI_RecvChar(&spiRxData);

  // 2 Bytes of I2C
  I2C_RecvBlock(i2cData, 2, i2cSize);
}
