//------------------------------------------------------------------------------
//! \file
//! Implements the DemoApp class
//------------------------------------------------------------------------------
#include "demoapp.h"
#include "main.h"

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
        case PidAnalogInput:
            // configure ADC
            ADMUX = _BV(REFS0) | 7; 
            ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2) | _BV(ADEN) | _BV(ADSC);
            break;

        case PidDigitalInput:
        default:
            break;
        }
        break;

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

	// set default values for testing on oscilloscope
	Stack::instance.processInputData().buffer[1] = 0x12;
	Stack::instance.processInputData().buffer[2] = 0x34;
	Stack::instance.processInputData().buffer[3] = 0x56;
	
    switch (Stack::instance.parameterRead(VendorParamPidMode))
    {
    case PidDigitalInput:
        // check digital sensor 
        Stack::instance.processInputData().buffer[0] = isDigitalButtonPressed() ? 0x01 : 0x00;
        Stack::instance.processInputData().isValid = true;
        break;

    case PidAnalogInput:
        // check analog sensor
        if (!(ADCSRA & _BV(ADSC)))
        {
            uint16_t sensorValue = ADC;

            Stack::instance.processInputData().buffer[0] = sensorValue >> 2;
            Stack::instance.processInputData().isValid = true;

            // restart sampling
            ADCSRA |= _BV(ADSC);
        }
        break;

    case PidSawtooth:
        // copy counter to process input data
        Stack::instance.processInputData().buffer[0] = _ctr;
        Stack::instance.processInputData().isValid = true;
        break;
	
	case PidSensors:
		Stack::instance.processInputData().buffer[0] = i2cData[1];
		Stack::instance.processInputData().buffer[1] = i2cData[0];
		Stack::instance.processInputData().buffer[2] = spiData[3];
		Stack::instance.processInputData().buffer[3] = spiData[2];
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
