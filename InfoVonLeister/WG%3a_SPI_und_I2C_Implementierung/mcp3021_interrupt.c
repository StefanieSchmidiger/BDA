/********************************************************************/
/*
 * File:		mcp3021_interrupt.c
 * Purpose:		driver for I2C MCP3021 10-Bit A/D converter
 *
 * Programmer:	Stefan Agner
 *				stefan.agner@leister.com
 *
 * Date:		20.12.2011
 *		
 */
/********************************************************************/

/********************************************************************/
/* Includes															*/
/********************************************************************/

#include "mcp3021_interrupt.h"

/********************************************************************/
/* Definitions														*/
/********************************************************************/

#define MCP3021_PREAMBLE 0x9
uint8 mcp3021_read_data_sync;
uint8 mcp3021_read_data_received = 1;
uint8 mcp3021_read_data_result;
uint16 mcp3021_read_length = 2;
uint8 mcp3021_read_dest[2];

/********************************************************************/
/* global variables													*/
/********************************************************************/
mcp3021_handler_t global_mcp3021_callback = NULL;

/********************************************************************/
/* Functions														*/
/********************************************************************/

/********************************************************************/
/* Register Callback												*/
/********************************************************************/
void mcp3021_register_callback(mcp3021_handler_t mcp3021_callback) {
	global_mcp3021_callback = mcp3021_callback;
}

/********************************************************************/
/* Handels Callback													*/
/********************************************************************/
void mcp3021_i2c_handler(uint8 state, uint8 send_result, uint8 read_result, uint8* read_buffer)
{
	uint16 i;
		
	if(state != I2C_STATE_IDLE || !mcp3021_read_length)
		return;
	
	// Since we call i2c_read, we must not check send_result!!! Only read_result is valid...
	mcp3021_read_data_result = read_result;
	
	// Copy data only if there is no error!
	if(!mcp3021_read_data_result)
	{
		for(i=0;i<mcp3021_read_length;i++)
			mcp3021_read_dest[i] = read_buffer[i];
	}
	
	if(!mcp3021_read_data_sync)
	{
		uint16 data = (uint16)(read_buffer[0] << 8 | read_buffer[1]);
		
		if(global_mcp3021_callback != NULL)
			global_mcp3021_callback(mcp3021_read_data_result, data >> 2); // Data are shifted by to according to datasheet
	}
	
	mcp3021_read_data_received = 1;
	mcp3021_read_data_sync = 0;
	
	return;
}

/********************************************************************/
/* Blocking read operation											*/
/********************************************************************/
uint8 mcp3021_read(uint8 dev_addr, uint16* data) {
	uint8 result;
	
	result = mcp3021_read_async(dev_addr);
	
	if(result)
		return result;
	
	// What until the callback is beeing called
	mcp3021_read_data_sync = 1;
	while(mcp3021_read_data_sync)  {};
	
	if(!mcp3021_read_data_result)
	{
		// Copy data if there was no error
		uint16 tmp = mcp3021_read_dest[0] << 8 | mcp3021_read_dest[1];
		*data = tmp >> 2; // Data are shifted by to according to datasheet
	}
	return mcp3021_read_data_result;
}

/********************************************************************/
/* Async read operation												*/
/********************************************************************/
uint8 mcp3021_read_async(uint8 dev_addr) {
	uint8 device_specific_addr;
	uint8 result;
	
	//set the address
	device_specific_addr = (MCP3021_PREAMBLE << 3) | dev_addr;
	
	
	// This happens on AM-Einkopplung (EMV-Tests), we don't receive any data from I2C
	// reset the bus solves the problem...
	if(!mcp3021_read_data_received || mcp3021_read_data_result != I2C_RESULT_TRANSFER_OK)
	{
		// If async handler was not called since last call of this function
		// execute the callback anyway and report that we did not received a result 
		if(global_mcp3021_callback != NULL)
			global_mcp3021_callback(I2C_RESULT_NO_RESULT, 0);
		
		i2c_reset_bus_state();
	}
	
	i2c_register_callback(&mcp3021_i2c_handler);
	
	result = i2c_read(device_specific_addr, mcp3021_read_length);
	
	// Reset data received flag only if read succeeded...
	if(result == I2C_RESULT_TRANSFER_OK)
		mcp3021_read_data_received = 0;
	
	return result;
}

