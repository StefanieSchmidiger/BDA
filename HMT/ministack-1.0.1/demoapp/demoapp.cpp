//------------------------------------------------------------------------------
//! \file
//! Implements the DemoApp class
//------------------------------------------------------------------------------
#include "demoapp.h"

extern "C" 
{
	#include "i2c_master.h"
}

// Instantiate the demo app
DemoApp DemoApp::instance;

//------------------------------------------------------------------------------
void DemoApp::configure()
{
    // Configure a timer with 10ms cycle
    OCR2A   = F_CPU / 1024 / 100;

    /* Start the timer
       WGM    = 3'b010  - clear timer on match
       COMxx  = 2'b0    - pins not used
       CS     = 3'b111   - divide by 1024 */
    TCCR2A = _BV(WGM21);
    TCCR2B = _BV(CS22) | _BV(CS21) | _BV(CS20);

    // configure digital input
    DDRB &= ~_BV(DDB7);
	
	// configure I2C
	i2c_init();
}

//------------------------------------------------------------------------------
void DemoApp::run(const Stack::Parameter* param)
{
    // check for write access to direct parameter page
    if (param)
    {
        handleParameterWrite(param);
    }
    else if (Stack::instance.stackMode() == Stack::STACK_MODE_SIO)
    {
        // when in SIOActive mode, use digital input to control CQ line
    	Stack::instance.setSioLevel(isDigitalButtonPressed());
    }

    // update process data
    updateProcessInputData();
}

//------------------------------------------------------------------------------
void DemoApp::handleParameterWrite(const Stack::Parameter* param)
{
    // (this is the location to intercept the write access if desired)
    bool commit = true;

    switch (param->address)
    {
		case VendorParamMirrorOutput:  // mirror data to 0x11: 
			Stack::instance.parameterWrite(VendorParamMirrorInput, param->value);
			break;

		case VendorParamMirrorInput:  // copy of 0x11 (read only)
			// read-only access => ignore
			commit = false;
			break;

		case VendorParamPidMode:  // process input data selection
			switch (param->value)
			{
				case I2cData:
					// ToDo: save that I2C was selected
					break;
				case SpiData:
					// ToDo: save that SPI was selected
					break;
				default:
					break;
			}
		default:
			break;
    }

    if (commit)
    {
        // commit to stack
        Stack::instance.parameterWrite(param->address, param->value);
    }
}

//------------------------------------------------------------------------------
void DemoApp::updateProcessInputData()
{
    if (elapsed10ms())
    {
        // the green LED cycles if IO-Link comms are up, red if not
        ++_ctr;
        uint8_t level = ((_ctr >> 3) & 0x0f);
        if (level & 0x8) level = ((~level) & 0x7);

        if (Stack::instance.masterLost())
        {
            // flash the red LED
            _ledLevel1 = Stack::LED_LEVEL_OFF;
            _ledLevel2 = (Stack::LedLevel)(level);
        }
        else
        {
            // flash the green LED
            _ledLevel1 = (Stack::LedLevel)(level);
            _ledLevel2 = Stack::LED_LEVEL_OFF;
        };

        // the red LED is over-ridden if the push-button is pressed
        if (isDigitalButtonPressed())
        {
            _ledLevel2 = (Stack::LedLevel)(0xf);
        };

        Stack::instance.setLedLevel(Stack::LED_1, _ledLevel1);
        Stack::instance.setLedLevel(Stack::LED_2, _ledLevel2);
    }

    switch (Stack::instance.parameterRead(VendorParamPidMode))
    {
		case SpiData:
			// check digital sensor 
			// ToDo: update SPI data instead of sending digital button
			Stack::instance.processInputData().buffer[0] = isDigitalButtonPressed() ? (uint8_t) 0x01 : (uint8_t) 0x00;
			Stack::instance.processInputData().buffer[1] = 0xFF;
			Stack::instance.processInputData().isValid = true;
			break;

		case I2cData:
			// update i2cData
			static uint8_t i2cSensorValue[2];
			// i2c_receive(0b10011010, i2cSensorValue, 2);
			Stack::instance.processInputData().buffer[1] = i2cSensorValue[1];
			Stack::instance.processInputData().buffer[0] = i2cSensorValue[0];
			Stack::instance.processInputData().isValid = true;
			break;
		default:
			Stack::instance.processInputData().isValid = false;
			break;
    }
}

//------------------------------------------------------------------------------
bool DemoApp::elapsed10ms()
{
    if (!(TIFR2 & _BV(OCF2A)))
        return false;

    TIFR2 = _BV(OCF2A);
    return true;
}

//------------------------------------------------------------------------------
bool DemoApp::isDigitalButtonPressed()
{
    return !(PINB & _BV(PINB7));
}
