//------------------------------------------------------------------------------
//! \file
//! Declares the PhyDriver
//------------------------------------------------------------------------------
#ifndef phyDriver_h__
#define phyDriver_h__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "spi.h"

// declare the existence of the interrupt service routine.  This is
// required for the friend statement in the MiniStack.
ISR(PCINT1_vect);

/**
 * Static class implementing register access to the PHY
 * 
 * \param SpiSsHndlr    Class for asserting / deasserting SPI SS/ line
 */
template <class SpiSsHndlr = DefaultSsHandler>
class PhyDriver 
{
public:
    /**
     * LED currents
     */
    enum LedLevel
    {
        LED_LEVEL_OFF,    //!< LED off
        LED_LEVEL_1,      //!< ~0.5mA
        LED_LEVEL_2,      //!< ~1.0mA
        LED_LEVEL_3,      //!< ~1.5mA
        LED_LEVEL_4,      //!< ~2.0mA
        LED_LEVEL_5,      //!< ~2.5mA
        LED_LEVEL_6,      //!< ~3.0mA
        LED_LEVEL_7,      //!< ~3.5mA
        LED_LEVEL_MAX = LED_LEVEL_7,
        LED_LEVEL_INVALID //!< not a LED level
    };

protected:
    /**
     * Default constructor
     */
    PhyDriver() {}

    /**
     * PHY MSEQ register flags
     */
    enum MseqRegister
    {
        MSEQ_M2CNT_SHIFT = 2,

        MSEQ_OD2_1       = 0 << 0,
        MSEQ_OD2_2       = 1 << 0,
        MSEQ_OD2_8       = 2 << 0,
        
        MSEQ_OD1_1       = 0 << 6,
        MSEQ_OD1_2       = 1 << 6,
        MSEQ_OD1_8       = 2 << 6
    };

    /**
     * PHY CFG register flags
     */
    enum CfgRegister { 
        CFG_NONE        = 0,
        CFG_UVT_18_0V   = 0 << 5,
        CFG_UVT_16_3V   = 1 << 5,
        CFG_UVT_15_0V   = 2 << 5,
        CFG_UVT_13_9V   = 3 << 5,
        CFG_UVT_12_0V   = 4 << 5,
        CFG_UVT_10_0V   = 5 << 5,
        CFG_UVT_8_6V    = 6 << 5,
        CFG_UVT_7_2V    = 7 << 5,
        CFG_BD_38400    = 0 << 4,   //!< COM2
        CFG_BD_230400   = 1 << 4,   //!< COM3
        CFG_RF_ABS      = 0 << 3, 
        CFG_RF_REL      = 1 << 3,
        CFG_S5V_SS      = 0 << 0,
        CFG_S5V_3_3V    = 2 << 0,
        CFG_S5V_5_0V    = 3 << 0,
    };

    /**
     * PHY CTL register flags
     */
    enum CtlRegister {
        CTL_NONE        = 0,
        CTL_TRNS_MODE   = 1 << 7, 
        CTL_SCT_190MA   = 0 << 4,
        CTL_SCT_210MA   = 1 << 4,
        CTL_SCT_230MA   = 2 << 4,
        CTL_SCT_250MA   = 3 << 4,
        CTL_SCT_110MA   = 4 << 4,
        CTL_SCT_130MA   = 5 << 4,
        CTL_SCT_150MA   = 6 << 4,
        CTL_SCT_170MA   = 7 << 4,
        CTL_SGL_MODE    = 1 << 3,
        CTL_IEN_MODE    = 1 << 3,
        CTL_IOLINK_MODE = 0 << 2,
        CTL_DIO         = 1 << 2,
        CTL_JOIN        = 0 << 2,
        CTL_SIO_MODE    = 1 << 2, 
        CTL_HS          = 1 << 1, 
        CTL_LS          = 1 << 0
    };

    /** PHY LINK register flags  */
    enum LinkRegister { 
        LINK_NONE       = 0,
        LINK_CNT_MASK   = 0x3C,
        LINK_CNT_SHIFT  = 2,

        LINK_END        = 1 << 1, 
        LINK_SND        = 1 << 0
    };

    /** PHY STATUS register flags */
    enum StatusRegister { 
        STATUS_NONE     = 0,
        STATUS_RST      = 1 << 7, 
        STATUS_INT      = 1 << 6, 
        STATUS_UV       = 1 << 5, 
        STATUS_DINT     = 1 << 4, 
        STATUS_CHK      = 1 << 3, 
        STATUS_DAT      = 1 << 2, 
        STATUS_SSC      = 1 << 1, 
        STATUS_SOT      = 1 << 0
    };

    /** PHY TEMP register flags */
    enum TempRegister {
        TEMP_NONE       = 0
    };

    /** HMT7748 DCDC register flags */
    enum DcDcRegister {
        DCDC_NONE         = 0,
        DCDC_DIS          = 1 << 7,
        DCDC_BYP          = 1 << 6,
        DCDC_FSET_500kHz  = 4 << 3,
        DCDC_FSET_625kHz  = 5 << 3,
        DCDC_FSET_710kHz  = 6 << 3,
        DCDC_FSET_830kHz  = 7 << 3,
        DCDC_FSET_1000kHz = 0 << 3,
        DCDC_FSET_1250kHz = 1 << 3,
        DCDC_FSET_1670kHz = 2 << 3,
        DCDC_FSET_2000kHz = 3 << 3,
        DCDC_VSET_4V2     = 4,
        DCDC_VSET_4V5     = 5,
        DCDC_VSET_4V9     = 6,
        DCDC_VSET_5V4     = 7,
        DCDC_VSET_6V0     = 0,
        DCDC_VSET_6V7     = 1,
        DCDC_VSET_7V8     = 2,
        DCDC_VSET_9V5     = 3
    };

    /** HMT7748 DSTAT register flags */
    enum DstatRegister {
        DSTAT_NONE      = 0,
        DSTAT_LVL       = 1 << 2,
        DSTAT_SSC       = 1 << 1,
    };
    
#if (defined(USE_HMT7742) && defined(USE_HMT7748))
#  error Both USE_HMT7742 and USE_7748 are defined.  Can't use both PHY's at once!
#endif

#if (!defined(USE_HMT7742) && !defined(USE_HMT7748))
#  error Neither USE_HMT7742 or USE_7748 are defined.  Please select the PHY to use!
#endif
    
#if defined(USE_HMT7742)
    /** HMT7742 register addresses */
    enum Registers { 
        REG_MSEQ        = 0x00, 
        REG_CFG         = 0x01, 
        REG_CTL         = 0x02, 
        REG_LINK        = 0x03, 
        REG_THERM       = 0x04,
        REG_STATUS      = 0x05, 
        REG_LED         = 0x06, 
        REG_TEMP        = 0x0c, 
        REG_FR0         = 0x10, 
        REG_WRITE_BIT   = 0x80
    };
#elif defined(USE_HMT7748)
    /** HMT7748 register addresses */
    enum Registers { 
        REG_MSEQ        = 0x00, 
        REG_CFG         = 0x01, 
        REG_CTL         = 0x02,
        REG_CCTL        = 0x02, 
        REG_DCTL        = 0x03, 
        REG_LINK        = 0x04, 
        REG_THERM       = 0x05,
        REG_TEMP        = 0x06, 
        REG_LED         = 0x07, 
        REG_DCDC        = 0x08, 
        REG_DSTAT       = 0x09, 
        REG_STATUS      = 0x0a, 
        REG_FR0         = 0x10, 
        REG_WRITE_BIT   = 0x80
    };
#endif

    /** 
     * Configures the hardware resources for the ISR
     */
    static void         configure();

    /** 
     * Temporarily disable the ISR
     */
    static void         stopInterrupt();

    /** 
     * Restart the ISR
     */
    static void         restartInterrupt();

    /**
     * Start reading from PHY registers
     * 
     * Asserts SS/ and starts reading from PHY register at specified address.
     * This call must be followed by zero or more calls to \c registerReadNext()
     * and a final call to registerReadLast().
     * 
     * \warning Ensure interrupts disabled before calling!
     * 
     * \param address   PHY register
     * \return          PHY status register value
     * 
     * \see registerReadNext
     * \see registerReadLast
     */
    static uint8_t      registerReadBegin(Registers address) __attribute__((always_inline));

    /**
     * Read next PHY register value
     *
     * This function automatically request the following register value. 
     * Use \c registerReadLast when reading the last required PHY register value.
     * 
     * \warning Ensure interrupts disabled before calling!
     * 
     * \return          PHY register value
     * 
     * \see registerReadBegin
     * \see registerReadLast
     */
    static uint8_t      registerReadNext() __attribute__((always_inline));

    /**
     * Read final PHY register value
     * 
     * Reads final PHY register value and de-asserts SS/
     * 
     * \warning Ensure interrupts disabled before calling!
     * 
     * \return          PHY register value

     * \see registerReadBegin
     * \see registerReadNext
     */
    static uint8_t      registerReadLast() __attribute__((always_inline));

    /**
     * Read a single byte from a PHY register
     * 
     * \warning Ensure interrupts disabled before calling!
     * 
     * \param address   PHY register
     * \return          PHY register value
     * 
     * \see registerReadBegin
     * \see registerReadNext
     * \see registerReadLast
     */
    inline  static uint8_t registerRead(Registers address) __attribute__((always_inline));

    /**
     * Read the status register
     * 
     * \return          PHY status register value
     */
    static uint8_t      registerReadStatus() __attribute__((always_inline));

    /**
     * Start write operation to PHY registers
     * 
     * Asserts SS/ and starts writing to PHY register at specified address.
     * This call must be followed by one or more calls to \c registerWriteNext()
     * and a final call to registerWriteDone().
     * 
     * \warning Ensure interrupts disabled before calling!
     * 
     * \param address   PHY register
     * 
     * \see registerWriteNext
     * \see registerWriteDone
     */
    static void         registerWriteBegin(Registers address) __attribute__((always_inline));

    /**
     * Start write operation to PHY registers
     * 
     * Asserts SS/ and starts writing to PHY register at specified address.
     * This call must be followed by zero or more calls to \c registerWriteNext()
     * and a final call to registerWriteDone().
     * 
     * \warning Ensure interrupts disabled before calling!
     * 
     * \param address   PHY register
     * \param data      PHY register value to write at address
     * \return          PHY status register value
     * 
     * \see registerWriteNext
     * \see registerWriteDone
     */
    static uint8_t      registerWriteBegin(Registers address, uint8_t data) __attribute__((always_inline));

    /**
     * Write next PHY register value
     *
     * Use \c registerWriteDone to finish the write operation.
     * 
     * \warning Ensure interrupts disabled before calling!
     * 
     * \param data      PHY register value to write at next address
     * 
     * \see registerWriteBegin
     * \see registerWriteDone
     */
    static void         registerWriteNext(uint8_t data) __attribute__((always_inline));

    /**
     * Finish write access
     * 
     * Waits for SPI communication to complete and de-asserts SS/
     * 
     * \warning Ensure interrupts disabled before calling!
     * 
     * \see registerWriteBegin
     * \see registerWriteNext
     */
    static void         registerWriteDone() __attribute__((always_inline));

    /**
    * Write a single byte to a PHY register
    * 
    * \warning Ensure interrupts disabled before calling!
    * 
    * \param address   PHY register
    * \param data      Value to write
    * \return          PHY status register
    */
    inline  static uint8_t registerWrite(Registers address, uint8_t data) __attribute__((always_inline));

    /**
     * Start write/read operation to PHY registers
     * 
     * Asserts SS/ and starts writing to PHY register at specified address.
     * This call must be followed by zero or more calls to \c registerWriteNext()
     * and a final call to registerWriteDone().
     * 
     * \warning Ensure interrupts disabled before calling!
     * 
     * \param address   PHY register
     * \param data      PHY register value to write at address
     * \return          PHY status register value
     * 
     * \see registerWriteNext
     * \see registerWriteDone
     */
    static uint8_t      registerReadWriteBegin(Registers address, uint8_t data) __attribute__((always_inline));

    /**
     * Write/read next PHY register value
     *
     * Use \c registerWriteDone to finish the write operation.
     * 
     * \warning Ensure interrupts disabled before calling!
     * 
     * \param data      PHY register value to write at next address
     * \return          PHY register value at previous address
     * 
     * \see registerWriteBegin
     * \see registerWriteDone
     */
    static uint8_t      registerReadWriteNext(uint8_t data) __attribute__((always_inline));

    /**
     * Finish write access and return final PHY register value
     * 
     * Reads final PHY register value and de-asserts SS/
     * 
     * \warning Ensure interrupts disabled before calling!
     * 
     * \return          PHY register value at previous address
     * 
     * \see registerWriteBegin
     * \see registerWriteNext
     */
    static uint8_t      registerReadWriteDone() __attribute__((always_inline));

    /**
     * Abort register access
     */
    static void         registerAbortAccess();

protected:
    static SpiSsHndlr   _ssHndlr;   //!< Handler functor for SS/ line
};

template <class SpiSsHndlr> SpiSsHndlr PhyDriver<SpiSsHndlr>::_ssHndlr;

//------------------------------------------------------------------------------
template <class SpiSsHndlr>
void PhyDriver<SpiSsHndlr>::registerWriteBegin(Registers address)
{
    _ssHndlr.assert();

    // send the address, with the write bit set, and read the status
    Spi::tx(address | REG_WRITE_BIT);
}

//------------------------------------------------------------------------------
template <class SpiSsHndlr>
uint8_t PhyDriver<SpiSsHndlr>::registerWriteBegin(Registers address, uint8_t data)
{
    _ssHndlr.assert();

    // send the address, with the write bit set, and read the status
    uint8_t status = Spi::txRx(address | REG_WRITE_BIT);

    // write data byte
    Spi::tx(data);

    return status;
}

//------------------------------------------------------------------------------
template <class SpiSsHndlr>
void PhyDriver<SpiSsHndlr>::registerWriteNext(uint8_t data)
{
    // wait until SPI exchange complete
    Spi::wait();

    // write data byte
    Spi::tx(data);
}

//------------------------------------------------------------------------------
template <class SpiSsHndlr>
void PhyDriver<SpiSsHndlr>::registerWriteDone()
{
    // wait until SPI exchange complete
    Spi::wait();

    _ssHndlr.deassert();
}

//------------------------------------------------------------------------------
template <class SpiSsHndlr> 
uint8_t PhyDriver<SpiSsHndlr>::registerWrite(Registers address, uint8_t data)
{
    uint8_t status = registerWriteBegin(address, data);
    registerWriteDone();
    return status;
}

//------------------------------------------------------------------------------
template <class SpiSsHndlr> 
uint8_t PhyDriver<SpiSsHndlr>::registerReadBegin(Registers address)
{
    _ssHndlr.assert();

    // send the address and read the status
    uint8_t status = Spi::txRx(address);

    // send dummy data byte to trigger next read (writing received value saves one MCU register)
    Spi::tx(status /* dummy byte */);
    return status;
}

//------------------------------------------------------------------------------
template <class SpiSsHndlr> 
uint8_t PhyDriver<SpiSsHndlr>::registerReadNext()
{
    // read next register value
    uint8_t result = Spi::rx();

    // send dummy data byte to trigger next read (writing received value saves one MCU register)
    Spi::tx(result /* dummy byte */);
    return result;
}

//------------------------------------------------------------------------------
template <class SpiSsHndlr> 
uint8_t PhyDriver<SpiSsHndlr>::registerReadLast()
{
    // read final register value
    uint8_t result = Spi::rx();

    _ssHndlr.deassert();
    return result;
}

//------------------------------------------------------------------------------
template <class SpiSsHndlr> 
uint8_t PhyDriver<SpiSsHndlr>::registerRead(Registers address)
{
    registerReadBegin(address);
    return registerReadLast();
}

//------------------------------------------------------------------------------
template <class SpiSsHndlr> 
uint8_t PhyDriver<SpiSsHndlr>::registerReadWriteBegin(Registers address, uint8_t data)
{
    _ssHndlr.assert();

    // send the address, with the write bit set, and read the status
    uint8_t status = Spi::txRx(address | REG_WRITE_BIT);

    // write data byte
    Spi::tx(data);

    return status;
}

//------------------------------------------------------------------------------
template <class SpiSsHndlr> 
uint8_t PhyDriver<SpiSsHndlr>::registerReadWriteNext(uint8_t data)
{
    // wait until SPI exchange done and read next value
    uint8_t result = Spi::rx();

    // write data byte
    Spi::tx(data);
    return result;
}

//------------------------------------------------------------------------------
template <class SpiSsHndlr> 
uint8_t PhyDriver<SpiSsHndlr>::registerReadWriteDone()
{
    // wait until SPI exchange done and read next value
    uint8_t result = Spi::rx();

    _ssHndlr.deassert();
    return result;
}

//------------------------------------------------------------------------------
template <class SpiSsHndlr> 
void PhyDriver<SpiSsHndlr>::registerAbortAccess()
{
    if (_ssHndlr.asserted())
    {
        Spi::wait();
        _ssHndlr.deassert();
    }
}

//------------------------------------------------------------------------------
template <class SpiSsHndlr> 
uint8_t PhyDriver<SpiSsHndlr>::registerReadStatus()
{
    _ssHndlr.assert();

    // send the address and read the status
    uint8_t status = Spi::txRx(REG_STATUS /* or anything else */);

    _ssHndlr.deassert();    
    return status;
}

//------------------------------------------------------------------------------
template <class SpiSsHndlr>
void PhyDriver<SpiSsHndlr>::configure()
{
    // Configure and enable interrupt
    // 
    // Only pin change interrupts are available, so the routine must check
    // the level before returning
    PCICR  |= _BV(PCIE1);
    PCMSK1 |= _BV(PCINT8);

    // call ISR handler
    PCINT1_vect();
}

//------------------------------------------------------------------------------
template <class SpiSsHndlr>
void PhyDriver<SpiSsHndlr>::stopInterrupt()
{
    PCMSK1 &= ~(_BV(PCINT8));
}

//------------------------------------------------------------------------------
template <class SpiSsHndlr>
void PhyDriver<SpiSsHndlr>::restartInterrupt()
{
    PCMSK1 |= _BV(PCINT8);

    // call ISR handler
    PCINT1_vect();
}

/**
 * Calculates set-point value of PHY THERM:TH[4:0] register
 * 
 * The next higher available set-point temperature will be selected.
 * 
 * \param  TEMPC    Set-point temperature in degree Celsius in range [-40°C .. +200°C]
 * \return          THERM:TH[4:0] register value
 */
#define ENCODE_THERMAL_SHUTDOWN(TEMPC)                                      \
    ((TEMPC) > 200 ? 0 :                                                    \
     (TEMPC) < -40 ? 23 :                                                   \
    (((8000 - 37 * (TEMPC)) / 400) & 0x1f))

/**
 * Alternative to PROGMEM storage class
 * 
 * Same effect as PROGMEM storage class, but avoiding erroneous warning by
 * GCC.
 * 
 * \see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=34734
 */
#define PROGMEM_ __attribute__((section(".progmem.data")))

#endif // phyDriver_h__
