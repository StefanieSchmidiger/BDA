/********************************************************************/
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
/********************************************************************/

/********************************************************************/
/* Includes															*/
/********************************************************************/

#include "fram_i2c.h"

/********************************************************************/
/* Definitions														*/
/********************************************************************/

#define FRAM_PREAMBLE 0xA /*!< FRAM specific address preamble. The FRAM assumes this value in bit 7-4 of the i2c address */

/********************************************************************/
/* global variables													*/
/********************************************************************/

/********************************************************************/
/* Functions														*/
/********************************************************************/


//! write data burst to fram
/*!
 *			
 *	This function sends data to the fram which has to be stored. The first byte 
 *	is stored at the address in the address parameter.	
 *
 *	\param dev_addr The i2c device address of the fram.
 *	\param address data start address where the fram has to start storing the data bytes.
 *	\param length number of bytes to be stored.
 *	\param data pointer to bytes to be stored.
 *	
 *	\return 
 *  0: success <br>
 *	1: bus allocation error<br>
 *	2: send error<br>
 *
 *	\warning data transfer to the fram uses blocking i2c routines. The normal program execution is 
 *	blocked during the accesses to the fram. Therefore the function should not be used in an
 *	interrupt context.
 */
	
uint8 fram_write(uint8 dev_addr, uint16 address, uint16 length, uint8* data) {
	
	uint8 device_specific_addr;
	uint8 data_addr[2];
	
	//set the address
	device_specific_addr = (FRAM_PREAMBLE << 3) | dev_addr;
	
	//allocate the i2c bus in transmit mode
	if(i2c_allocate(device_specific_addr, 0)) {
		i2c_release();
		return 1;	
	}
	
	
	data_addr[0] = (uint8)(address >> 8);
	data_addr[1] = (uint8)(address);
	
	//send data address
	if (i2c_send(2, (uint8*)data_addr)) {
		i2c_release();
		return 2;	
	}

	//send data bytes
	if (i2c_send(length, data)) {
		i2c_release();
		return 2;	
	}
	
	//release the bus
	i2c_release();
	
	
	
	return 0;
}

//! read data burst from fram
/*!
 *			
 *	This function read data which is stored in the fram. The first byte 
 *	is read at the address in the address parameter.	
 *
 *	\param dev_addr The i2c device address of the fram.
 *	\param address data start address where the fram has to start sending the data bytes.
 *	\param length number of bytes to be read.
 *	
 *	\return 
 *  pointer to received data bytes. NULL if no bytes were read or an error occured
 *
 *	\warning data transfer from the fram uses blocking i2c routines. The normal program execution is 
 *	blocked during the accesses to the fram. Therefore the function should not be used in an
 *	interrupt context.
 */
	
uint8 fram_read(uint8 dev_addr, uint16 address, uint16 length, uint8* data) {
	
	uint8 device_specific_addr;
	uint8 data_addr[2];
	//set the address
	device_specific_addr = (FRAM_PREAMBLE << 3) | dev_addr;
	
	//allocate the i2c bus in write mode
	if(i2c_allocate(device_specific_addr, 0)) {
		i2c_release();
		return 1;	
	}
	
	data_addr[0] = (uint8)(address >> 8);
	data_addr[1] = (uint8)(address);
	
	//send data address
	if (i2c_send(2, (uint8*)data_addr)) {
		i2c_release();
		return 2;	
	}
	
	//repeated start in read mode
	if(i2c_repeated_allocate(device_specific_addr, 1)) {
		i2c_release();
		return 3;
	}

	//read data bytes
	if(i2c_read(length, data)) {
		i2c_release();
		return 4;
	}
	
	//release the bus
	i2c_release();
		
	return 0;
}

