/********************************************************************/
/*
 * File:		fram_i2c_interrupt.h
 * Purpose:		driver for ramtron i2c frams using interrupting i2c driver
 *
 * Programmer:	Stefan Agner
 *				stefan.agner@leister.com
 *
 * Date:		05.02.2010
 *		
 */
/********************************************************************/

/********************************************************************/
/* Includes															*/
/********************************************************************/

#include "fram_i2c_interrupt.h"

/********************************************************************/
/* Definitions														*/
/********************************************************************/

#define FRAM_PREAMBLE 0xA
uint8 read_data_sync;
uint8 read_data_result;
uint16 read_length;
uint8* read_dest;

void i2c_handler(uint8 state, uint8 send_result, uint8 read_result, uint8* read_buffer);

/********************************************************************/
/* global variables													*/
/********************************************************************/
fram_handler_t global_fram_callback = NULL;

/********************************************************************/
/* Functions														*/
/********************************************************************/
	
/********************************************************************/
/* Async write operation											*/
/********************************************************************/
uint8 fram_write(uint8 dev_addr, uint16 address, uint16 length, uint8* data) {
	uint8 device_specific_addr;
	uint8 data_raw[2+10];
	uint16 i;
	
	//set the address
	device_specific_addr = (uint8)((FRAM_PREAMBLE << 3) | dev_addr);
	
	data_raw[0] = (uint8)(address >> 8);
	data_raw[1] = (uint8)(address);
	
	for(i=0;i<length;i++)
		data_raw[i+2] = data[i];
	
	return i2c_send(device_specific_addr, data_raw, (uint8)(length+2));
}

/********************************************************************/
/* Register Callback												*/
/********************************************************************/
void fram_register_callback(fram_handler_t fram_callback) {
	global_fram_callback = fram_callback;
}

/********************************************************************/
/* Handels Callback													*/
/********************************************************************/
void i2c_handler(uint8 state, uint8 send_result, uint8 read_result, uint8* read_buffer)
{
	uint16 i;
		
	if(state != I2C_STATE_IDLE || !read_length)
		return;
	
	read_data_result = read_result;
	if(send_result) // more important, overwrite overall state if failed!
		read_data_result = send_result;
	
	// Copy data only if there is no error!
	if(!read_data_result)
	{
		for(i=0;i<read_length;i++)
			read_dest[i] = read_buffer[i];
	}
	
	if(!read_data_sync)
	{
		if(global_fram_callback != NULL)
			global_fram_callback(read_data_result, read_length, read_dest);
	}
		
	read_length = 0;
	read_data_sync = 0;
	
	return;
}

/********************************************************************/
/* Blocking read operation											*/
/********************************************************************/
uint8 fram_read(uint8 dev_addr, uint16 address, uint16 length, uint8* data) {
	uint8 result;
	
	result = fram_read_async(dev_addr, address, length, data);
	
	if(result)
		return result;
	
	read_data_sync = 1;
	while(read_data_sync)  {};
	return read_data_result;
}

/********************************************************************/
/* Async read operation												*/
/********************************************************************/
uint8 fram_read_async(uint8 dev_addr, uint16 address, uint16 length, uint8* data) {
	uint8 device_specific_addr;
	uint8 data_addr[2];
	uint8 result;
	
	//set the address
	device_specific_addr = (uint8)((FRAM_PREAMBLE << 3) | dev_addr);
	
	data_addr[0] = (uint8)(address >> 8);
	data_addr[1] = (uint8)(address);
	
	i2c_register_callback(&i2c_handler);
	result = i2c_send_read(device_specific_addr, data_addr, 2, (uint8)length);
	if(result)
		return result;
	read_length = length;
	read_dest = data;
	return 0;
}

