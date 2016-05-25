//------------------------------------------------------------------------------
//! \file
//! Declares the Spi class
//------------------------------------------------------------------------------
#ifndef spi_h__
#define spi_h__

/**
 * Helper class for handling SPI communication with the HMT7742
 */
struct Spi
{
    /**
     * Configures the hardware resources for SPI communication with the PHY. 
     */
    inline static void  configure() __attribute__((always_inline));

    /**
     * Enable SPI
     */
    static void         enable()  __attribute__((always_inline))
    {
        SPCR |= _BV(SPE);
    }

    /**
     * Disable SPI
     */
    static void         disable() __attribute__((always_inline))
    {
        SPCR &= ~_BV(SPE);
    }

    /**
     * Waits until SPI transmission / reception complete
     */
    static void         wait()  __attribute__((always_inline))
    {        
        loop_until_bit_is_set(SPSR, SPIF);
    }

    /**
     * Send a byte to SPI
     * 
     * \note Call spiRx to read the byte received by the slave.
     * \param data  Byte to send
     */
    static void         tx(uint8_t data)  __attribute__((always_inline))
    {
        // put data into buffer, sends the data
        SPDR = data;
    }

    /**
     * Read a byte from SPI
     * 
     * \note Automatically calls \c wait()
     * \return Received byte
     */
    static uint8_t      rx() __attribute__((always_inline))
    {
        wait();

        // get received data from buffer
        return SPDR;
    }

    /**
     * Send and receive a byte by SPI
     * 
     * \param data  Byte to send
     * \return      Byte received
     */
    static uint8_t      txRx(uint8_t data) __attribute__((always_inline))
    {
        tx(data);
        return rx();
    }
};

/**
 * Default implementation of class for handling SPI SS/ line
 */
struct DefaultSsHandler
{
    /**
     * Assert SS/ to begin SPI communication
     */
    static void         assert() __attribute__((always_inline))
    {
        PORTB &= ~ _BV(PORTB2);
    };

    /**
     * Deassert SS/ to terminate SPI communication
     */
    static void         deassert() __attribute__((always_inline))
    {
        PORTB |= _BV(PORTB2);
    };

    /**
     * Test if SS/ asserted
     */
    static bool         asserted()
    {
        return (PORTB & _BV(PORTB2)) == 0;
    }
    
    inline static void configure()
    {
      /* Data direction in the SPI bits, PB2/SS, PB5/SCK
      and PB3/MOSI are outputs.  Drive SS high in advance */
      PORTB |= _BV(PORTB2) | _BV(PORTB5);
      PORTB &= ~(_BV(PORTB3)); 
      DDRB  |= _BV(DDB5) | _BV(DDB3) | _BV(DDB2);
      DDRB  &= ~(_BV(DDB4));

      /* Set the SPI configuration register
      SPIE   = 1'b0   - interrupt disabled
      SPE    = 1'b1   - enabled
      DORD   = 1'b0   - MSB first
      MSTR   = 1'b1   - Master
      CPOL   = 1'b1   - clock idles high
      CPHA   = 1'b1   - sample on trailing edge
      SPR    = 2'b00  - fosc/2 */
      SPCR = _BV(SPE) | _BV(MSTR) | _BV(CPOL) | _BV(CPHA);

      /* and the SPI2X bit to speed it up */
      SPSR = _BV(SPI2X);
    }
};

//------------------------------------------------------------------------------


#endif // spi_h__
