/**********************************************************************/
/*
 * File:		fram_i2c.h
 * Purpose:		driver for ramtron i2c frams
 *
 * Programmer:	Christian Luethi
 *				christian.luethi@leister.com
 *
 * Date:		22.05.2007
 *		
 */
/**********************************************************************/

#ifndef _FRAM_I2C_DRV_H_
#define _FRAM_I2C_DRV_H_

/********************************************************************/
/* Includes															*/
/********************************************************************/

#include "common.h"
#include "i2c_interrupt.h"

/********************************************************************/
/* Definitions														*/
/********************************************************************/


/********************************************************************/
/* Functions														*/
/********************************************************************/

uint8 fram_write(uint8 dev_addr, uint16 address, uint16 length, uint8* data);
uint8 fram_read(uint8 dev_addr, uint16 address, uint16 length, uint8* data);

#endif //_FRAM_I2C_DRV_H_