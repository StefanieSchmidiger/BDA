//------------------------------------------------------------------------------
//! \file
//! Declares the DebugPin class
//------------------------------------------------------------------------------
#ifndef debugpin_h__
#define debugpin_h__

#include <avr/io.h>

/**
 * Class to control pins on JP4 extension connector
 * 
 * This class is used for debugging purposes only. 
 * It controls the pin PD4 (JP6:1) and PD5 (JP6:2)
 */
class DebugPin
{
public:
    //! setup the HW configuration
    static void configure()
    {
        // Port PD4 and PD5 as output
        DDRD  |= _BV(DDD4) | _BV(DDD5);

        set(false);
    }

    //! Control debug pin PD4
    static void set(bool level)
    {
        if (level)
        {
            PORTD |= _BV(PORTD4);
        }
        else
        {
            PORTD &= ~(_BV(PORTD4));
        }
    }

    //! Toggle pin
    static void toggle(int8_t count)
    {
        while (count--)
        {
            PORTD |= _BV(PORTD4);
            PORTD &= ~(_BV(PORTD4));
        }
    }

    //! Control debug pin PD4
    static void set2(bool level)
    {
        if (level)
        {
            PORTD |= _BV(PORTD5);
        }
        else
        {
            PORTD &= ~(_BV(PORTD5));
        }
    }
};

#endif // debugpin_h__
