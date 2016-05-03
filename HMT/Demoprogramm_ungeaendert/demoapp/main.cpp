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
    }
}
