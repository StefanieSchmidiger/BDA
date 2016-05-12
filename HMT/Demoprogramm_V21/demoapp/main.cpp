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
	#include "spi_master_bitbanging.h"
	#include <avr/io.h>
	#include <avr/interrupt.h>
	#include <util/atomic.h>
	#include <util/delay.h>
	#include <util/twi.h>
}

#define INDICATOR_BIT PC2

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
uint8_t spiData[4];

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
	spi_init_bitbanging();
	i2cData[0]=0;
	DDRC |= (1 << INDICATOR_BIT);		// set pins to output
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
			// update sensordata
			i2c_receive(0b10011010, i2cData, 2);
			// bring i2c data in right format
			uint8_t i2cDataTemp[2];
			i2cDataTemp[0]=i2cData[1];
			i2cDataTemp[1]=i2cData[0];
			i2cData[0]=(0x3F & (i2cDataTemp[0]/4))|((uint8_t)(0xC0 & (i2cDataTemp[1]*64)));	// store data in right format
			i2cData[1]=i2cDataTemp[1]/4;
			spiData[0]=0x31;
			spiData[1]=0;
			//spi_sendBytes_bitbanging(spiData, 4);
			PORTC = PORTC ^ 0x04;	// toggle indicator bit
			
            Stack::instance.stopInterrupt();
            theApp.run(paramWrite);	// update processdata
            Stack::instance.restartInterrupt();
        }
    }
}