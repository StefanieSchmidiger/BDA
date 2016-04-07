#ifndef __I2C_INTERRUPT_H__
#define __I2C_INTERRUPT_H__

/********************************************************************/
/* Low level includes												*/
/********************************************************************/
#include "common.h"

/********************************************************************/
/* Definitions														*/
/********************************************************************/

#define I2C_INTERRUP_LEVEL 		1 //low level
#define I2C_INTERRUP_PRIORITY 	3 //mid priority within level
#define I2C_BUF_LEN			  255

#define I2C_STATE_IDLE							0
#define I2C_STATE_MASTER_WRITE_SEND_ADDRESS		1
#define I2C_STATE_MASTER_WRITE_SEND_DATA		2
#define I2C_STATE_MASTER_READ_SEND_ADDRESS		3
#define I2C_STATE_MASTER_READ_RECEIVE_DATA		4
#define I2C_STATE_MASTER_SENDREAD_ADDRESS_SEND	5
#define I2C_STATE_MASTER_SENDREAD_DATA_SEND		6
#define I2C_STATE_MASTER_SENDREAD_ADDRESS_READ	7
#define I2C_STATE_MASTER_SENDREAD_DATA_READ		8

#define I2C_RESULT_TRANSFER_OK					0
#define I2C_RESULT_NO_RESULT					1
#define I2C_RESULT_ADRRESS_ACK_ERROR			2
#define I2C_RESULT_DATA_ACK_ERROR				3
#define I2C_RESULT_FREQUENCE_INVALID			4
#define I2C_RESULT_ADRRESS_INVALID				5
#define I2C_RESULT_LENGTH_INVALID				6
#define I2C_RESULT_BUSY							7
#define I2C_RESULT_INTERRUPT_BUSY				8

typedef void (*i2c_handler_t)(uint8 state, uint8 send_result, uint8 read_result, uint8* read_buffer);

/********************************************************************/
/* Structures														*/
/********************************************************************/


/********************************************************************/
/* Declarations	and prototypes										*/
/********************************************************************/

uint8 i2c_init(uint8 slv_addr, uint8 freq_div);
uint8 i2c_read(uint8 addr, uint8 len);
uint8 i2c_send(uint8 addr, uint8* data, uint8 len);
uint8 i2c_send_read(uint8 addr, uint8* data, uint8 sendlen, uint8 receivelen);
void i2c_register_callback(i2c_handler_t func);
void i2c_isr(void);
void i2c_reset_bus_state(void);

/********************************************************************/

#endif //__I2C_INTERRUPT_H__