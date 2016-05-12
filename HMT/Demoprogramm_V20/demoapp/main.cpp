//------------------------------------------------------------------------------
//! \file
//! Minimal application to provide a simple sensor emulator and a response to a 
//! button press on the TM96.
//------------------------------------------------------------------------------
//#include "stacksinglebyte.h"
#include "demoapp.h"
#include <util/delay.h>
#include <avr/io.h>
#include <avr/signature.h>
#include <avr/sleep.h>

extern "C"
{
	#include "i2c_master.h"
	#include <avr/io.h>
	#include <avr/interrupt.h>
	#include <util/atomic.h>
	#include <util/delay.h>
	#include <util/twi.h>
}

// declare main as function without exit (saves a few bytes)
int main(void) __attribute__ ((OS_main));

// embed fuse configuration ELF file:
// - external clock, low startup time
// - enable SPI programming
FUSES =
{
#ifdef USE_EXT_OSC
    (FUSE_CKSEL0 & FUSE_CKSEL1 & FUSE_CKSEL2 & FUSE_CKSEL3 & FUSE_SUT0), // .low
#else
    (FUSE_CKSEL0 & FUSE_CKSEL2 & FUSE_CKSEL3 & FUSE_SUT0),               // .low
#endif
    (FUSE_SPIEN & FUSE_BOOTSZ0 & FUSE_BOOTSZ1),                          // .high
    ( FUSE_BODLEVEL0 )                                                   // .ext
};

// ----- global variables ---------
uint8_t i2cData[2];
uint8_t i2cConversionDone=1;

/**
 * Main application entry point
 */
int main(void)
{
    // give peripherals time to stabilize
    _delay_ms(200);

    // instantiate DemoApp
    DemoApp& theApp = DemoApp::instance;

    // configure all software modules
    DebugPin::configure();
    DemoApp::configure();   
    Stack::instance.configure();

	i2c_init();	// setup TWI
	i2cData[0]=0;
    // enable interrupts
    sei();
	

    // select sleep mode
    set_sleep_mode(SLEEP_MODE_IDLE);

    // enter infinite loop: processing is interrupt controlled from now on
    for (;;)
    {
        // enter sleep until interrupt wakes us up
        sleep_mode();

        // check if it's time to run user code
        const Stack::Parameter* paramWrite;
        if (Stack::instance.canRunUserCode(paramWrite))
        {
            Stack::instance.stopInterrupt();
            theApp.run(paramWrite);
            Stack::instance.restartInterrupt();
        }
		if(i2cConversionDone == 1)
		{
			i2c_start();
			i2cConversionDone=0;
		}
    }
}


ISR(TWI_vect)
{
	static uint8_t i2cDataTemp[2];
	if((TWSR & 0xF8) == TW_START)					// start condition successfully transmitted
	{
		TWDR = 0b10011011;							// load address(1001101x) and read(1) into data register
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);				// start transmission of address
	}
	else if ( ((TW_STATUS & 0xF8) == TW_MT_SLA_ACK) || ((TW_STATUS & 0xF8) == TW_MR_SLA_ACK) ) // address+read/write acknowledged
	{
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA) | (1<<TWIE); // start TWI module and acknowledge data after reception
	}
	else /*if((TW_STATUS & 0xF8) == TW_MR_DATA_ACK)*/			// data received and acknowledge sent
	{
		static uint8_t byteNumber=0;
		i2cDataTemp[byteNumber] = TWDR;	// data structure from sensor, B1: 0 0 0 0  b10 b9 b8 b7       B2: b6 b5 b4 b3  b2 b1 x x
		byteNumber++;
		if(byteNumber == 2)							// 2nd byte was sent -> send STOP
		{
			TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);	// transmit STOP condition. disable with endless data acquiry
			//TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA) | (1<<TWIE); // endless data acquiry enable, request 1st byte again
			byteNumber = 0;							// overwrite data the next time
			ATOMIC_BLOCK(ATOMIC_FORCEON)			// write i2cData into global array
			{
				i2cData[0]=(0x3F & (i2cDataTemp[0]/4))|((uint8_t)(i2cDataTemp[1]*64));	// store data in right format
				i2cData[1]=i2cDataTemp[1]/4;
				i2cConversionDone = 1;
			}
			
		}
		else	// after transmission of 1st data byte
		{
			TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE); // start TWI module for 2nd byte and NACK data after reception (signals end of reception)
		}
	}
}