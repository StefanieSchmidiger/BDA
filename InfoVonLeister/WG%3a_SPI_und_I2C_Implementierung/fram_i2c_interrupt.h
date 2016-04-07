/**********************************************************************/
/*
 * File:		fram_i2c_interrupt.h
 * Purpose:		driver for ramtron i2c frams using interrupting spi driver
 *
 * Programmer:	Christian Luethi
 *				christian.luethi@leister.com
 *
 * Date:		27.11.2008
 *		
 */
/**********************************************************************/

#ifndef _FRAM_I2C_INTERRUPT_H_
#define _FRAM_I2C_INTERRUPT_H_

/********************************************************************/
/* Includes															*/
/********************************************************************/

#include "common.h"
#include "i2c_interrupt.h"

/********************************************************************/
/* Definitions														*/
/********************************************************************/
typedef void (*fram_handler_t)(uint8 result, uint16 length, uint8* data);

/********************************************************************/
/* Functions														*/
/********************************************************************/

uint8 fram_write(uint8 dev_addr, uint16 address, uint16 length, uint8* data);
uint8 fram_read(uint8 dev_addr, uint16 address, uint16 length, uint8* data);
uint8 fram_read_async(uint8 dev_addr, uint16 address, uint16 length, uint8* data);
void fram_register_callback(fram_handler_t fram_callback);

#endif //_FRAM_I2C_INTERRUPT_H_