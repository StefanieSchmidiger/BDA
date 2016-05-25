//------------------------------------------------------------------------------
//! \file
//! Declares the StackBase class
//------------------------------------------------------------------------------ 
#ifndef stackbase_h__
#define stackbase_h__

#include "debugpin.h"
#include "phydriver.h"
#include "iolink.h"
#include <stddef.h>
#include <util/atomic.h>

// Declare ISRs
ISR(TIMER0_COMPA_vect);
ISR(TIMER0_COMPB_vect);
ISR(PCINT0_vect);

// Declare the direct parameter and event pages as global fixed variables
extern uint8_t  directParameter[32];   //!< direct parameter page
extern uint8_t  eventPage[8];          //!< event buffer

/**
 * The StackBase is the base class for minimal IO-Link stacks
 * for different IO-Link devices.  It is expected that a derived version
 * of the MiniStack is used, with member functions redefined according
 * to the application.
 * 
 * One stack derivative is instantiated in the application.
 *
 * The stack is configured before operation, using configure().
 * This first configures the uC hardware resources to communicate with
 * the PHY, including the interrupt service routine.  The stack is
 * reset, defaulting to SIO mode.
 * 
 * The PHY itself is configured on demand.  If the PHY sees a reset,
 * and is ready for operation, this is indicated in the status bits
 * read by the MiniStack.  The configurePhy() function is called,
 * automatically and the stack is placed into SIO mode.
 * 
 * Operation of the stack is primarily interrupt driven.  When a PHY
 * event occurs, either because the master has sent an M-sequence, or
 * due to a local PHY event, the interrupt service routine (ISR) is
 * called. The ISR maintains the stack state (STACK_MODE_SIO or STACK_MODE_IOLINK),
 * and automatically passes process data in or out.
 * 
 * The parameter data page is maintained within the stack.  Where the
 * master reads from the page, the stack provides the information without
 * informing the application.  Where the master writes to the page, the
 * attempted operation to the page is reported in the stack state, and must be
 * passed back to the stack using setParameterData() to have an effect.
 * 
 * \attention ISDU's are not supported.
 * \attention Frame TYPE_1_1/1_2 (interleaved) is not supported
 * 
 * The MiniStack code is closely linked to the interrupt service routine,
 * ISR(PCINT1_vect), which performs much of the stack handling, and makes reference to
 * the global stack.
 *
 * \param T     Type of specific derived stack class, will be typedef'ed as \c StackT
 * \param PDI   Amount of input process data (in octets), will be assigned to \c PD_IN_SIZE
 * \param PDO   Amount of output process data (in octets), will be assigned to \c PD_OUT_SIZE
 * \param SpiSsHndlr Optional class handling switching between SPI and UART 
 *                   communication with the PHY when using transparent mode.
*/
template <class T, int PDI, int PDO, class SpiSsHndlr = DefaultSsHandler>
class StackBase : public PhyDriver<SpiSsHndlr>
{
public:
    //! Convenience typedef for PhyDriver
    typedef PhyDriver<SpiSsHndlr> Phy;

    //! Specific derived stack type
    typedef T StackT;

    //! Convenience typedef for this class
    typedef StackBase<T, PDI, PDO, SpiSsHndlr> BaseT;

    //! Amount of input process data (in octets)
    static const uint8_t PD_IN_SIZE = PDI;

    //! Amount of output process data (in octets)
    static const uint8_t PD_OUT_SIZE = PDO;

    /**
     * Parameter structure
     */
    struct Parameter 
    {
        uint8_t address;
        uint8_t value;
    };

    /**
     * Structure for holding process data and associated status flags
     */
    template <int SIZE>
    struct ProcessData
    {
        uint8_t buffer[SIZE];                   //!< Process data buffer
        bool    isValid;                        //!< Indicates if buffer data is valid
    };

    typedef ProcessData<PDI> ProcessDataIn;     //!< Input process data
    typedef ProcessData<PDO> ProcessDataOut;    //!< Output process data

    /**
     * LEDs
     */
    enum Led
    {
        LED_1,           //!< LED 1
        LED_2            //!< LED 2
    };

    /** Stack mode */
    enum StackMode 
    {
        STACK_MODE_SIO, 
        STACK_MODE_IOLINK
    };

    /** Drive capability in SIO mode */
    enum SioDriveMode
    {
        DRIVE_MODE_PUSH_PULL = 0,     //!< Push-pull, HS and LS active
        DRIVE_MODE_NPN,               //!< LS only used 
        DRIVE_MODE_PNP,               //!< HS only used
        DRIVE_MODE_INACTIVE           //!< neither switch used (typical for an actuator)
    };

    /** 
     * Configure sets up the hardware resources on the uC, and initializes 
     * the stack.  
     */
    void                configure();

    /**
     * \anchor canRunUserCode
     * Test if cyclic user code may run
     * 
     * Call this function immediately after being woken up in the application's
     * main() loop, or, if no sleep mode is being used, at least every 0.1ms.
     * 
     * \param lastWrittenParameter  Pointer reference in which the function
     *                      returns a Parameter structure. If the returned 
     *                      pointer is not NULL then the most recent message
     *                      completed a write access to the direct parameter page. 
     *                      The data is *not* automatically written to the direct 
     *                      parameter page, but needs to be manually committed by
     *                      calling parameterWrite(). The returned pointer may 
     *                      be NULL if no write access occurred.
     * \return          true: user code may run now; false otherwise     
     */
    bool                canRunUserCode(const Parameter*& lastWrittenParameter);

    /** 
     * Temporarily disable the ISR
     */
    static void         stopInterrupt()         { Phy::stopInterrupt(); }

    /** 
     * Restart the ISR
     */
    static void         restartInterrupt()      { Phy::restartInterrupt(); }

    /**
     * Get current stack mode
     * 
     * \return STACK_MODE_SIO or STACK_MODE_IOLINK
     */
    StackMode           stackMode() const { return _ddlMode == IoLink::DDL_MODE_IDLE ? STACK_MODE_SIO : STACK_MODE_IOLINK; }

    /**
     * Test if connection to master has been lost
     * 
     * \return \c True if no communication exchange took place for at least four
     *            cycles.
     */
    bool                masterLost() const { return _deadCycleCtr > 4; }

    /**
	 * Debugging flag
	 */
    bool                flag() const { return _flag; };

    /**
     * Specify the level of the CQ line in SIO mode (STACK_MODE_SIO) state. 
     * 
     * \param active          If Stack::SIO_DRIVE_MODE is Stack::DRIVE_MODE_PNP or
     *                        Stack::DRIVE_MODE_NPN the relevant switch is 
     *                        activated if \p active == true.  In 
     *                        Stack::DRIVE_MODE_PUSH_PULL (push-pull) CQ is 
     *                        driven high (\p active == true) or low (\p active 
     *                        == false).
     */
    void                setSioLevel(bool active);

    /**
     * Read value from direct parameter page
     * 
     * \param address   Parameter index
     * \return          Read value
     * 
     */
    uint8_t             parameterRead(uint8_t address) const { return directParameter[address]; }

    /**
     * Write value to direct parameter page
     * 
     * \param address   Parameter index
     * \param value     Value to write
     * 
     */
    void                parameterWrite(uint8_t address, uint8_t value);

    /** 
     * Get buffer for returning process input data from slave to master
     *
     * \return Process input data buffer (read/writable)
     * 
     */
    ProcessDataIn&      processInputData() { return _processDataIn; }

    /**
     * Get buffer for process output data received from master
     * 
     * \return Process output data buffer (read-only)
     * 
     */
    const ProcessDataOut& processOutputData() const { return _processDataOut; }

    /**
     * Set LED level
     * 
     * \param led       Selected LED
     * \param level     Desired level
     */
    void                setLedLevel(Led led, typename Phy::LedLevel level);

    /**
     * Get LED level
     * 
     * \param  led      Selected LED
     * \return          Current LED level
     */
    typename Phy::LedLevel ledLevel(Led led) const;

    /**
     * Get current measured temperature value.  In order to convert the
     * returned value to a Celsius reading, the following formula should
     * be applied:
     *
     * Temperature [Celsius] = (80 - temp) * 2.70
     * 
     * \return Current temperature value
     */
    uint8_t             temperature() const;


protected:
    /**
     * Result codes for ISR sub-handlers
     */
    enum HandlerResult 
    {
        ResultSuccess = 0,          // successfully handled message
        ResultNoData,               // did not receive any data
        ResultChecksumError,        // detected message checksum error
        ResultIllegalMessageType,   // received invalid message type
        ResultPhyReset              // detected PHY power-on reset
    };

    using Phy::registerWriteDone;   // required since no Koenig-lookup available

    /**
     * Default constructor
     */
    StackBase();

    /**
     * Helper function returning derived stack specialization
     * 
     * \return          Derived stack instance
     */
    StackT&             stack() { return static_cast<StackT&>(*this); }

    /**
     * Configure the stack base class
     */
    void                configureStackBase();

    /**
     * Empty default implementation of derived stack configuration
     */
    void                configureStack() {}

    /**
     * Configure the PHY
     * 
     * This function is called whenever the stack detects that the PHY has
     * been reset
     */
    void                configurePhy();

    /**
     * Put PHY in SIO-Active state
     *
     * The CQ line is driven according to the setting of the HS and LS bits, 
     * as specified by Stack::SIO_DRIVE_MODE.
     *
     * \return          PHY status register
     */
    uint8_t             setSioActive();

    /**
     * Put PHY in SIO-Listen mode
     * 
     * \return          PHY status register
     */
    static uint8_t      setSioListen();

    /**
     * Put PHY in IO-Link-Listen mode
     * 
     * \return          PHY status register
     */
    static uint8_t      setIoLinkListen();

    /**
     * Get number of OD octets
     * 
     * \tparam DDL_MODE  Applicable DDL mode (see IoLink::DeviceDLMode)
     * \return          Expected OD octet count
     */
    template <IoLink::DeviceDLMode DDL_MODE>
    static int8_t       getOdOctetCount();

    /**
     * Validate control octet
     */
    static bool         validateControlOctet(uint8_t mc);

    /**
     * Validate frame type
     * 
     * \tparam DDL_MODE  Applicable DDL mode (see IoLink::DeviceDLMode)
     * \param ckt       Received CKT frame octet.
     * \return          True if frame type valid, false otherwise
     */
    template <IoLink::DeviceDLMode DDL_MODE>
    static bool         validateFrameType(uint8_t ckt);

    /**
     * Write received on-demand data
     * 
     * \param channel   Message channel (IoLink::MC_CHNL_*)
     * \param address   Address within selected channel
     * \param data      Data octet to write to address
     */
    void                odWrite(uint8_t channel, uint8_t address, uint8_t data);

    /**
     * Read requested on-demand data
     * 
     * \param channel   Message channel (IoLink::MC_CHNL_*)
     * \param address   Address within selected channel
     * \return          Data octet at address
     */
    uint8_t             odRead(uint8_t channel, uint8_t address);

    /**
     * Calculate cycle period from MasterCycleTime
     */
    void                updateCyclePeriod();
    
    /**
     * Start / synchronize user-callback timer
     * 
     * Starts a timer which ensures that the user call-back will be called on
     * a regular basis, even if no master message has been received
     * 
     * \param delay     Delay until first call to user callback (in 1/10ms units)
     */
    void                startCallbackTimer(uint8_t delay = 0);

    /** ISR handler as member function */
    inline void         onTimer0CompBInterrupt();

    /** The ISR function can access the stack state, and is declared here as a friend. */
    friend void ::TIMER0_COMPB_vect();

private:
    /**
     * Get drive mode bits for PHY CTL register, according to 
     * Stack::SIO_DRIVE_MODE
     * 
     * \return Drive mode bits (Phy::CTL_LS / Phy::CTL_HS)
     */
    uint8_t             driveModeBits() const;

protected:
    IoLink::DeviceDLMode _ddlMode;               //!< Device DL-mode
    ProcessDataIn       _processDataIn;         //!< process input data buffers
    ProcessDataOut      _processDataOut;        //!< process output data buffer
    uint8_t             _deadCycleCtr;          //!< Count of cycles from last master exchange
    int8_t              _hiZCounter;            //!< Cycle counter for listening to the CQ
    bool                _sioLevel       : 1;    //!< Level of CQ line during SIOActive state

private:
    Parameter           _parameterWrite;        //!< Most recently written parameter
    uint16_t            _cyclePeriod;           //!< Master frame cycle in 1/10ms
    uint16_t            _cycleTimer;            //!< Count-down to next expected master sequence (1/10ms)
    uint8_t             _savedLedRegister;      //!< Prevous value of PHY LED register
    uint8_t             _ledRegister;           //!< Value to write to PHY LED register
    uint8_t             _fallbackCounter;       //!< Cycle counter for FALLBACK system command
    bool                _runUserCode    : 1;    //!< If true, user code may run
    bool                _prmWriteAvailable : 1; //!< True until parameter write is picked up
    bool                _flag : 1;              //!< DEBUGGING only
};

/**
 * Returns LSB of argument
 * 
 * \param w     Argument
 * \return      LSB (bits 0..7) of argument
 */
#define LOBYTE(w)           ((uint8_t)(((w)) & 0xff))

/**
 * Returns MSB of argument
 * 
 * \param w     Argument
 * \return      MSB (bits 8..15) of argument
 */
#define HIBYTE(w)           ((uint8_t)((((w)) >> 8) & 0xff))

/**
 * Returns bank byte of argument
 * 
 * \param w     Argument
 * \return      Bank byte (bits 16..23) of argument
 */
#define BANKBYTE(w)         ((uint8_t)((((w)) >> 16) & 0xff))

//------------------------------------------------------------------------------
template <class T, int PDI, int PDO, class SpiSsHndlr>
StackBase<T, PDI, PDO, SpiSsHndlr>::StackBase() 
    : _ddlMode(IoLink::DDL_MODE_IDLE) 
    , _deadCycleCtr(0xff) 
    , _cyclePeriod(0)
    , _cycleTimer(0)
    , _savedLedRegister(0xff)
    , _ledRegister(0)
    , _fallbackCounter(0)
    , _runUserCode(false)
    , _sioLevel(true)
    , _prmWriteAvailable(false)
    , _flag(false)
{
    _parameterWrite.address = IoLink::PAGE_NO_PARAMETER;
}

//------------------------------------------------------------------------------
template <class T, int PDI, int PDO, class SpiSsHndlr>
void StackBase<T, PDI, PDO, SpiSsHndlr>::parameterWrite(uint8_t address, uint8_t value)
{
    // write to direct parameter page channel
    switch (address)
    {
    case IoLink::PAGE_MASTER_CMD:
        switch (value)
        {
        case IoLink::MCMD_DEVICE_STARTUP:   
            _ddlMode = IoLink::DDL_MODE_STARTUP;  
            break;
        case IoLink::MCMD_DEVICE_PREOPERATE:
            _ddlMode = IoLink::DDL_MODE_PREOPERATE; 
            break;
        case IoLink::MCMD_DEVICE_OPERATE:   
            _processDataOut.isValid = false;
            _ddlMode = IoLink::DDL_MODE_OPERATE; 
            break;
        case IoLink::MCMD_PD_OUT_OPERATE:   // Process output data valid
            _processDataOut.isValid = true;
            _ddlMode = IoLink::DDL_MODE_OPERATE; 
            break;
        case IoLink::MCMD_FALLBACK:
            // switch to SIO mode after 3 cycles
            _fallbackCounter = 3;
            break;
        }
        break;

    case IoLink::PAGE_MASTER_CYCLE_TIME:
        updateCyclePeriod();
        //lint -fallthrough
    case IoLink::PAGE_SYSTEM_CMD:
    case IoLink::PAGE_DEVICE_SPECIFIC_10:
    case IoLink::PAGE_DEVICE_SPECIFIC_11:
    case IoLink::PAGE_DEVICE_SPECIFIC_12:
    case IoLink::PAGE_DEVICE_SPECIFIC_13:
    case IoLink::PAGE_DEVICE_SPECIFIC_14:
    case IoLink::PAGE_DEVICE_SPECIFIC_15:
    case IoLink::PAGE_DEVICE_SPECIFIC_16:
    case IoLink::PAGE_DEVICE_SPECIFIC_17:
    case IoLink::PAGE_DEVICE_SPECIFIC_18:
    case IoLink::PAGE_DEVICE_SPECIFIC_19:
    case IoLink::PAGE_DEVICE_SPECIFIC_1A:
    case IoLink::PAGE_DEVICE_SPECIFIC_1B:
    case IoLink::PAGE_DEVICE_SPECIFIC_1C:
    case IoLink::PAGE_DEVICE_SPECIFIC_1D:
    case IoLink::PAGE_DEVICE_SPECIFIC_1E:
    case IoLink::PAGE_DEVICE_SPECIFIC_1F:
        directParameter[address] = value;
        break;

    case IoLink::PAGE_REVISION_ID:
    case IoLink::PAGE_DEVICE_ID_1:
    case IoLink::PAGE_DEVICE_ID_2:
    case IoLink::PAGE_DEVICE_ID_3:
        if (_ddlMode == IoLink::DDL_MODE_STARTUP)
        {
            directParameter[address] = value;
        }
        break;
    }
}

//------------------------------------------------------------------------------
template <class T, int PDI, int PDO, class SpiSsHndlr>
void StackBase<T, PDI, PDO, SpiSsHndlr>::setLedLevel(Led led, typename Phy::LedLevel level)
{
    switch (led)
    {
    case LED_1:
        _ledRegister &= 0x0f;
        _ledRegister |= (level << 4);
        break;

    case LED_2:
        _ledRegister &= 0xf0;
        _ledRegister |= level;
        break;

    default:
        break;
    }
}

//------------------------------------------------------------------------------
template <class T, int PDI, int PDO, class SpiSsHndlr>
typename PhyDriver<SpiSsHndlr>::LedLevel StackBase<T, PDI, PDO, SpiSsHndlr>::ledLevel(Led led) const
{
    switch (led)
    {
    case LED_1:  return _ledRegister & 0x0f;
    case LED_2:  return _ledRegister >> 4;
    default:    return Phy::LedLevelInvalid;
    }
}


//------------------------------------------------------------------------------
template <class T, int PDI, int PDO, class SpiSsHndlr>
uint8_t StackBase<T, PDI, PDO, SpiSsHndlr>::temperature() const
{
    return StackBase::registerRead(Phy::REG_TEMP);
}

//------------------------------------------------------------------------------
template <class T, int PDI, int PDO, class SpiSsHndlr>
void StackBase<T, PDI, PDO, SpiSsHndlr>::configure()
{
    // configure hardware
    SpiSsHndlr::configure();
    configureStackBase();

    Phy::configure();

    // start timer responsible for calling the application "main loop"
    startCallbackTimer();
};

//------------------------------------------------------------------------------
template <class T, int PDI, int PDO, class SpiSsHndlr>
void StackBase<T, PDI, PDO, SpiSsHndlr>::configurePhy()
{
    StackBase::registerWriteBegin(Phy::REG_MSEQ);

    // calculate M2CNT based on sequence type
    // NOTE: The if-statement will be evaluated at compile time!
    if ((((StackT::MSEQ_CAPABILITY & IoLink::MSEQCAP_OP_MASK) == IoLink::MSEQCAP_OP_CODE_0) 
            || StackT::REVISION_ID == IoLink::REVISION_ID_1_0)
        && (PD_OUT_SIZE >= 3 || PD_IN_SIZE >= 3))
    {
        // sequence TYPE_1_1/1_2 interleaved => M2CNT is 2
        StackBase::registerWriteNext((2) << Phy::MSEQ_M2CNT_SHIFT);        // REG_MSEQ
    }
    else
    {
        uint8_t mseq = 0;
        switch (getOdOctetCount<IoLink::DDL_MODE_OPERATE>())
        {
        case 1: mseq = Phy::MSEQ_OD2_1; break;
        case 2: mseq = Phy::MSEQ_OD2_2; break;
        case 8: mseq = Phy::MSEQ_OD2_8; break;
        default:
            // only up to 8 OD octets supported
            break;
        }
        
        switch (getOdOctetCount<IoLink::DDL_MODE_PREOPERATE>())
        {
        case 1: mseq |= Phy::MSEQ_OD1_1; break;
        case 2: mseq |= Phy::MSEQ_OD1_2; break;
        case 8: mseq |= Phy::MSEQ_OD1_8; break;
        default:
            // only up to 8 OD octets supported
            break;
        }        

        mseq |= (2 + PD_OUT_SIZE) << Phy::MSEQ_M2CNT_SHIFT;

        // other sequence type => M2CNT is 2 + PD_OUT_SIZE + ODcount
        StackBase::registerWriteNext((typename Phy::MseqRegister)mseq);   // REG_MSEQ
    }

    // REG_CFG
    StackBase::registerWriteNext(StackT::PHY_CFG);
    
    // REG_CTL
    StackBase::registerWriteNext(StackT::PHY_CTL_SCT | 
                                 StackT::PHY_CTL_MODE | 
                                 Phy::CTL_SIO_MODE | 
                                 driveModeBits());
    
#ifdef USE_HMT7748
    // REG_DCTL
    StackBase::registerWriteNext(Phy::CTL_DIO);
#endif

    // REG_LINK (dummy write)
    StackBase::registerWriteNext(Phy::LINK_NONE);          
    
    // REG_THERM
    StackBase::registerWriteNext(ENCODE_THERMAL_SHUTDOWN(StackT::PHY_THERM_DEG));
    
    // REG_STATUS (HMT7742) / REG_TEMP (HMT7748) - (dummy write)
    Phy::registerWriteNext(0);        
    
    // LEDs
    StackBase::registerWriteNext(this->_ledRegister);        
    
#ifdef USE_HMT7748
    // DCDC
    StackBase::registerWriteNext(Phy::DCDC_NONE);
#endif

    registerWriteDone();

    // set the stack to SIO mode to be in sync with the PHY.
    _ddlMode = IoLink::DDL_MODE_IDLE;
}

//------------------------------------------------------------------------------
template <class T, int PDI, int PDO, class SpiSsHndlr>
void StackBase<T, PDI, PDO, SpiSsHndlr>::configureStackBase()
{
    static const PROGMEM_ uint8_t directDefaultConfig[32] =
    {
        0, 
        0,
        ENCODE_CYCLE_TIME(StackT::MIN_CYC_TIME),        // PAGE_MIN_CYCLE_TIME
        StackT::MSEQ_CAPABILITY,                        // PAGE_MSEQ_CAPABILITY
        StackT::REVISION_ID,                            // PAGE_REVISION_ID
        ENCODE_PD_BYTES(PD_IN_SIZE) | IoLink::PDIN_SIO_SUPPORTED, // PAGE_PD_IN
        ENCODE_PD_BYTES(PD_OUT_SIZE),                   // PAGE_PD_OUT
        HIBYTE(StackT::VENDOR_ID),                      // PAGE_VENDOR_ID_1
        LOBYTE(StackT::VENDOR_ID),                      // PAGE_VENDOR_ID_2
        BANKBYTE(StackT::DEVICE_ID),                    // PAGE_DEVICE_ID_1
        HIBYTE(StackT::DEVICE_ID),                      // PAGE_DEVICE_ID_2
        LOBYTE(StackT::DEVICE_ID)                       // PAGE_DEVICE_ID_3
    };

    // copy the default parameters from flash to RAM
    (void)memcpy_P(directParameter, directDefaultConfig, sizeof(directDefaultConfig));

    // stack initial state is STACK_MODE_SIO operation
    _ddlMode = IoLink::DDL_MODE_IDLE;

    // set up the process data buffers and default process data
    _processDataIn.isValid = false;
    _processDataOut.isValid = false;

    // configure derived stack implementation
    stack().configureStack();
}

//------------------------------------------------------------------------------
template <class T, int PDI, int PDO, class SpiSsHndlr>
void StackBase<T, PDI, PDO, SpiSsHndlr>::setSioLevel(bool active)
{
    _sioLevel = active;

    // in SIO-mode, immediately switch CQ line
    if (_ddlMode == IoLink::DDL_MODE_IDLE)
    {
        stack().setSioActive();
    }
}

//------------------------------------------------------------------------------
template <class T, int PDI, int PDO, class SpiSsHndlr>
uint8_t StackBase<T, PDI, PDO, SpiSsHndlr>::driveModeBits() const
{
    switch (StackT::SIO_DRIVE_MODE)
    {
    case DRIVE_MODE_PUSH_PULL:
        return _sioLevel ? Phy::CTL_HS : Phy::CTL_LS;

    case DRIVE_MODE_NPN:
        if (_sioLevel)
            return Phy::CTL_LS;
        else
            return 0;

    case DRIVE_MODE_PNP:
        if (_sioLevel)
            return Phy::CTL_HS;
        else
            return 0;

    case DRIVE_MODE_INACTIVE:
    default:
        return 0;
    }
}

//------------------------------------------------------------------------------
template <class T, int PDI, int PDO, class SpiSsHndlr>
uint8_t StackBase<T, PDI, PDO, SpiSsHndlr>::setSioActive()
{
    static bool wasDriven = false;
    uint8_t hslsBits = driveModeBits();
    bool driven = (hslsBits != 0);

    // we need to switch to IO-Link mode after 1ms to detect a wake-up
    if (driven)
    {
        _hiZCounter = -1;
    }
    else if (wasDriven)  
    {
        _hiZCounter = 10;
    }

    wasDriven = driven;

    uint8_t sioBit = (!driven && (_hiZCounter < 0)) 
        ? Phy::CTL_IOLINK_MODE 
        : Phy::CTL_SIO_MODE;

    return StackBase::registerWrite(Phy::REG_CTL, 
        StackT::PHY_CTL_SCT | StackT::PHY_CTL_MODE | sioBit | hslsBits);
}

//------------------------------------------------------------------------------
template <class T, int PDI, int PDO, class SpiSsHndlr>
uint8_t StackBase<T, PDI, PDO, SpiSsHndlr>::setSioListen()
{
    return registerWrite(Phy::REG_CTL, StackT::PHY_CTL_SCT | StackT::PHY_CTL_MODE | Phy::CTL_SIO_MODE);
}

//------------------------------------------------------------------------------
template <class T, int PDI, int PDO, class SpiSsHndlr>
uint8_t StackBase<T, PDI, PDO, SpiSsHndlr>::setIoLinkListen()
{
    return StackBase::registerWrite(Phy::REG_CTL, StackT::PHY_CTL_SCT | StackT::PHY_CTL_MODE | Phy::CTL_IOLINK_MODE);
}

//------------------------------------------------------------------------------
template <class T, int PDI, int PDO, class SpiSsHndlr>
template <IoLink::DeviceDLMode DDL_MODE>
int8_t StackBase<T, PDI, PDO, SpiSsHndlr>::getOdOctetCount()
{
    // this switch is evaluated at compile time!
    switch (DDL_MODE)
    {
        // see table A.7
    case IoLink::DDL_MODE_IDLE:
    case IoLink::DDL_MODE_STARTUP:
        return 1;                                           // TYPE_0

        // see table A.8
    case IoLink::DDL_MODE_PREOPERATE:
        switch (StackT::MSEQ_CAPABILITY & IoLink::MSEQCAP_PREOP_MASK)
        {
        default:
        case IoLink::MSEQCAP_PREOP_CODE_0: 
            return 1;                                       // TYPE_0

        case IoLink::MSEQCAP_PREOP_CODE_1:  
            return 2;                                       // TYPE_1_2

        case IoLink::MSEQCAP_PREOP_CODE_2:  
            return 8;                                       // TYPE_1_V

        case IoLink::MSEQCAP_PREOP_CODE_3:  
            return 32;                                      // TYPE_1_V
        }
        break;

        // see table A.9
    case IoLink::DDL_MODE_OPERATE:
        switch (StackT::MSEQ_CAPABILITY & IoLink::MSEQCAP_OP_MASK)
        {
        case IoLink::MSEQCAP_OP_CODE_0:
            if (PD_OUT_SIZE == 0 && PD_IN_SIZE == 0) 
                return 1;                                   // TYPE_0
            else if ((StackT::REVISION_ID == IoLink::REVISION_ID_1_0) && (PD_OUT_SIZE + PD_IN_SIZE <= 2))
                return 1;                                   // TYPE_2_1-2_5
            else if ((StackT::REVISION_ID == IoLink::REVISION_ID_1_1) && (PD_OUT_SIZE + PD_IN_SIZE <= 4))
                return 1;                                   // TYPE_2_1-2_6
            else
                return 2;                                   // TYPE_1_1/1_2
            break;

        case IoLink::MSEQCAP_OP_CODE_1:
            if (PD_OUT_SIZE == 0 && PD_IN_SIZE == 0)  
                return 2;                                   // TYPE_1_2
            else if ((StackT::REVISION_ID == IoLink::REVISION_ID_1_0) && (PD_OUT_SIZE + PD_IN_SIZE <= 2))
                return 1;                                   // TYPE_2_1-2_5
            else
                return 2;                                   // TYPE_1_1/1_2
            break;

        case IoLink::MSEQCAP_OP_CODE_4:
            if ((StackT::REVISION_ID == IoLink::REVISION_ID_1_1) && (PD_OUT_SIZE >= 3 || PD_IN_SIZE >= 3))
                return 1;                                   // TYPE_2_V
            else if ((StackT::REVISION_ID == IoLink::REVISION_ID_1_0) && (PD_OUT_SIZE + PD_IN_SIZE <= 2))
                return 1;                                   // TYPE_2_1-2_5
            else
                return 2;                                   // TYPE_1_1/1_2
            break;

        case IoLink::MSEQCAP_OP_CODE_5:
            if ((StackT::REVISION_ID == IoLink::REVISION_ID_1_1) && (PD_OUT_SIZE + PD_IN_SIZE > 0))
                return 2;                                   // TYPE_2_V
            else if ((StackT::REVISION_ID == IoLink::REVISION_ID_1_0) && (PD_OUT_SIZE + PD_IN_SIZE <= 2))
                return 1;                                   // TYPE_2_1-2_5
            else
                return 2;                                   // TYPE_1_1/1_2
            break;

        case IoLink::MSEQCAP_OP_CODE_6:
            if (StackT::REVISION_ID == IoLink::REVISION_ID_1_1)
                return 8;                                   // TYPE_2_V
            else if ((StackT::REVISION_ID == IoLink::REVISION_ID_1_0) && (PD_OUT_SIZE + PD_IN_SIZE <= 2))
                return 1;                                   // TYPE_2_1-2_5
            else
                return 2;                                   // TYPE_1_1/1_2
            break;

        case IoLink::MSEQCAP_OP_CODE_7:
            if (StackT::REVISION_ID == IoLink::REVISION_ID_1_1)
                return 32;                                  // TYPE_2_V
            else if ((StackT::REVISION_ID == IoLink::REVISION_ID_1_0) && (PD_OUT_SIZE + PD_IN_SIZE <= 2))
                return 1;                                   // TYPE_2_1-2_5
            else
                return 2;                                   // TYPE_1_1/1_2
            break;

        default:
            if ((StackT::REVISION_ID == IoLink::REVISION_ID_1_0) && (PD_OUT_SIZE >= 3 || PD_IN_SIZE >= 3))
                return 2;                                   // TYPE_1_1/1_2
            break;
        }
        break;
    }

    return -1;
}

//------------------------------------------------------------------------------
template <class T, int PDI, int PDO, class SpiSsHndlr>
bool StackBase<T, PDI, PDO, SpiSsHndlr>::validateControlOctet(uint8_t mc)
{
    if ((mc & IoLink::MC_CHANNEL_MASK) == IoLink::MC_CHNL_ISDU)
    {
        if ((mc && IoLink::MC_ADDRESS_MASK) > IoLink::MC_ADDR_ISDU_IDLE2)
            return false;
    }

    return true;
}

//------------------------------------------------------------------------------
template <class T, int PDI, int PDO, class SpiSsHndlr>
template <IoLink::DeviceDLMode DDL_MODE>
bool StackBase<T, PDI, PDO, SpiSsHndlr>::validateFrameType(uint8_t ckt)
{
    uint8_t frameType = (ckt & IoLink::CKT_TYPE_MASK);

    // this switch is evaluated at compile time!
    switch (DDL_MODE)
    {
        // see table A.7
    case IoLink::DDL_MODE_IDLE:
    case IoLink::DDL_MODE_STARTUP:
        if (frameType == IoLink::CKT_TYPE_0) 
            return true;                                           // TYPE_0
        break;

        // see table A.8
    case IoLink::DDL_MODE_PREOPERATE:
        switch (StackT::MSEQ_CAPABILITY & IoLink::MSEQCAP_PREOP_MASK)
        {
        case IoLink::MSEQCAP_PREOP_CODE_0: 
            if (frameType == IoLink::CKT_TYPE_0) 
                return true;                                       // TYPE_0
            break;

        case IoLink::MSEQCAP_PREOP_CODE_1:                         // TYPE_1_2
        case IoLink::MSEQCAP_PREOP_CODE_2:                         // TYPE_1_V
        case IoLink::MSEQCAP_PREOP_CODE_3:                         // TYPE_1_V
            if (frameType == IoLink::CKT_TYPE_1) 
                return true;                                       // TYPE_1_2
            break;
        }
        break;

        // see table A.9
    case IoLink::DDL_MODE_OPERATE:
        switch (StackT::MSEQ_CAPABILITY & IoLink::MSEQCAP_OP_MASK)
        {
        case IoLink::MSEQCAP_OP_CODE_0:
            if (PD_OUT_SIZE == 0 && PD_IN_SIZE == 0) 
            {
                if (frameType == IoLink::CKT_TYPE_0)
                    return true;                                   // TYPE_0
            }
            else if ((StackT::REVISION_ID == IoLink::REVISION_ID_1_0) && (PD_OUT_SIZE + PD_IN_SIZE <= 2))
            {
                if (frameType == IoLink::CKT_TYPE_2)
                    return true;                                   // TYPE_2_1-2_5
            }
            else if ((StackT::REVISION_ID == IoLink::REVISION_ID_1_1) && (PD_OUT_SIZE + PD_IN_SIZE <= 4))
            {
                if (frameType == IoLink::CKT_TYPE_2)
                    return true;                                   // TYPE_2_1-2_6
            }
            else if (PD_OUT_SIZE >= 3 || PD_IN_SIZE >= 3)
            {
                if (frameType == IoLink::CKT_TYPE_1)
                    return true;                                   // TYPE_1_1/1_2
            }
            break;

        case IoLink::MSEQCAP_OP_CODE_1:
            if (PD_OUT_SIZE == 0 && PD_IN_SIZE == 0)  
            {
                if (frameType == IoLink::CKT_TYPE_1)
                    return true;                                   // TYPE_1_2
            }
            else if ((StackT::REVISION_ID == IoLink::REVISION_ID_1_0) && (PD_OUT_SIZE + PD_IN_SIZE <= 2))
            {
                if (frameType == IoLink::CKT_TYPE_2)
                    return true;                                   // TYPE_2_1-2_5
            }
            else if ((StackT::REVISION_ID == IoLink::REVISION_ID_1_0) && (PD_OUT_SIZE >= 3 || PD_IN_SIZE >= 3))
            {
                if (frameType == IoLink::CKT_TYPE_1)
                    return true;                                   // TYPE_1_1/1_2
            }
            break;

        case IoLink::MSEQCAP_OP_CODE_4:
            if ((StackT::REVISION_ID == IoLink::REVISION_ID_1_1) && (PD_OUT_SIZE >= 3 || PD_IN_SIZE >= 3))
            {
                if (frameType == IoLink::CKT_TYPE_2)
                    return true;                                   // TYPE_2_V
            }
            else if ((StackT::REVISION_ID == IoLink::REVISION_ID_1_0) && (PD_OUT_SIZE + PD_IN_SIZE <= 2))
            {
                if (frameType == IoLink::CKT_TYPE_2)
                    return true;                                   // TYPE_2_1-2_5
            }
            else if ((StackT::REVISION_ID == IoLink::REVISION_ID_1_0) && (PD_OUT_SIZE >= 3 || PD_IN_SIZE >= 3))
            {
                if (frameType == IoLink::CKT_TYPE_1)
                    return true;                                   // TYPE_1_1/1_2
            }
            break;

        case IoLink::MSEQCAP_OP_CODE_5:
            if ((StackT::REVISION_ID == IoLink::REVISION_ID_1_1) && (PD_OUT_SIZE + PD_IN_SIZE > 0))
            {
                if (frameType == IoLink::CKT_TYPE_2)
                    return true;                                   // TYPE_2_V
            }
            else if ((StackT::REVISION_ID == IoLink::REVISION_ID_1_0) && (PD_OUT_SIZE + PD_IN_SIZE <= 2))
            {
                if (frameType == IoLink::CKT_TYPE_2)
                    return true;                                   // TYPE_2_1-2_5
            }
            else if ((StackT::REVISION_ID == IoLink::REVISION_ID_1_0) && (PD_OUT_SIZE >= 3 || PD_IN_SIZE >= 3))
            {
                if (frameType == IoLink::CKT_TYPE_1)
                    return true;                                   // TYPE_1_1/1_2
            }
            break;

        case IoLink::MSEQCAP_OP_CODE_6:
        case IoLink::MSEQCAP_OP_CODE_7:
            if (StackT::REVISION_ID == IoLink::REVISION_ID_1_1)
            {
                if (frameType == IoLink::CKT_TYPE_2)
                    return true;                                   // TYPE_2_V
            }
            else if ((StackT::REVISION_ID == IoLink::REVISION_ID_1_0) && (PD_OUT_SIZE + PD_IN_SIZE <= 2))
            {
                if (frameType == IoLink::CKT_TYPE_2)
                    return true;                                   // TYPE_2_1-2_5
            }
            else if ((StackT::REVISION_ID == IoLink::REVISION_ID_1_0) && (PD_OUT_SIZE >= 3 || PD_IN_SIZE >= 3))
            {
                if (frameType == IoLink::CKT_TYPE_1)
                    return true;                                   // TYPE_1_1/1_2
            }
            break;

        default:
            if ((StackT::REVISION_ID == IoLink::REVISION_ID_1_0) && (PD_OUT_SIZE >= 3 || PD_IN_SIZE >= 3))
            {
                if (frameType == IoLink::CKT_TYPE_1)
                    return true;                                   // TYPE_1_1/1_2
            }
            break;
        }
        break;
    }

    return false;
}

//------------------------------------------------------------------------------
template <class T, int PDI, int PDO, class SpiSsHndlr>
void StackBase<T, PDI, PDO, SpiSsHndlr>::odWrite(uint8_t channel, uint8_t address, uint8_t data)
{
    if (channel == IoLink::MC_CHNL_PAGE)
    {
        // store data for use in application
        _parameterWrite.address = address;
        _parameterWrite.value = data;
        _prmWriteAvailable = true;
    }
    else
    {
        _prmWriteAvailable = false;
    }
}

//------------------------------------------------------------------------------
template <class T, int PDI, int PDO, class SpiSsHndlr>
uint8_t StackBase<T, PDI, PDO, SpiSsHndlr>::odRead(uint8_t channel, uint8_t address)
{
    uint8_t data;

    switch (channel)
    {
    case IoLink::MC_CHNL_PAGE:  // Direct parameter page channel
        data = directParameter[address];
        break;

    case IoLink::MC_CHNL_DIAG:  // Diagnosis channel
        if (address > 6)
            break;

        data = eventPage[address];
        break;

    default:            // SPDU and process data not handled
//        ASSERT(address == IoLink::MC_ADDR_ISDU_IDLE1 || address == IoLink::MC_ADDR_ISDU_IDLE2);
        data = 0;
    }

    return data;
}

//------------------------------------------------------------------------------
template <class T, int PDI, int PDO, class SpiSsHndlr>
void StackBase<T, PDI, PDO, SpiSsHndlr>::updateCyclePeriod()
{
    // get period
    uint8_t cycleParam = 0;
    if (_ddlMode == IoLink::DDL_MODE_OPERATE)
    {
        cycleParam = parameterRead(IoLink::PAGE_MASTER_CYCLE_TIME);
    }

    if (cycleParam == 0)
    {
        // MasterCycleTime hasn't been set yet => use MinCycleTime
        cycleParam = parameterRead(IoLink::PAGE_MIN_CYCLE_TIME);
    }

    // decode parameter into 1/10ms
    _cyclePeriod = DECODE_CYCLE_TIME(cycleParam);
}


//------------------------------------------------------------------------------
template <class T, int PDI, int PDO, class SpiSsHndlr>
void StackBase<T, PDI, PDO, SpiSsHndlr>::startCallbackTimer(uint8_t delay)
{
    // reset dead cycle timer
    _deadCycleCtr = 0;

    // disable timer
    TCCR0B = 0;
    TCNT0 = 0;

    // clear any outstanding interrupt requests
    TIFR0 |= _BV(OCF0B) | _BV(OCF0A) | _BV(TOV0);
    
    // check if we need to force recalculation of cycle period
    if (_cyclePeriod == 0)
    {
        updateCyclePeriod();
    }

    // initialize cycle timer
    _cycleTimer = delay + 1;

    if (_cycleTimer > 0)
    {      
        // Timer/Counter0 Output Compare Match B Interrupt Enable
        TIMSK0 = _BV(OCIE0B);

        // clear Timer on Compare Match A
        TCCR0A = _BV(WGM01);

        // generate 0.1ms cycles
#if F_CPU < 20000000
        OCR0B = OCR0A = F_CPU / 80000;
        TCCR0B = _BV(CS01);             // pre-scaler clkIO/8
#elif F_CPU < 160000000
        OCR0B = OCR0A = F_CPU / 640000;
        TCCR0B = _BV(CS01) | _BV(CS00); // pre-scaler clkIO/64
#else
# error Unsupported CPU frequency
#endif
    }

    if (delay == 0)
      ATOMIC_BLOCK( ATOMIC_RESTORESTATE)
      {
        onTimer0CompBInterrupt();
      };
}

//------------------------------------------------------------------------------
template <class T, int PDI, int PDO, class SpiSsHndlr>
void StackBase<T, PDI, PDO, SpiSsHndlr>::onTimer0CompBInterrupt()
{
    // decrement hiZ timer
    if (_hiZCounter >= 0 && --_hiZCounter < 0)
    {
        stack().setSioActive();
    }

    // check if cycle timer elapsed
    if (_cycleTimer > 0 && --_cycleTimer == 0)
    {
        // user code may run
        _runUserCode = true;

        // restart cycle timer
        _cycleTimer = _cyclePeriod;

        // advance the count of dead cycles
        if (_deadCycleCtr != 0xff)
        {
            ++_deadCycleCtr;
        }

        // check if we need to switch to SIO mode
        if (_fallbackCounter > 0 && --_fallbackCounter == 0)
        {
            _ddlMode = IoLink::DDL_MODE_IDLE;
        }

        // update LED register if necessary
        if (_savedLedRegister != _ledRegister)
        {
            _savedLedRegister = _ledRegister;

            StackBase::registerWrite(Phy::REG_LED, _ledRegister);
        }
    }
    else
    {
        // too late for user code
        _runUserCode = false;
    }
}

//------------------------------------------------------------------------------
template <class T, int PDI, int PDO, class SpiSsHndlr>
bool StackBase<T, PDI, PDO, SpiSsHndlr>::canRunUserCode(const Parameter*& lastWrittenParameter)
{
    // allow execution of user code?
    if (!_runUserCode)
        return false;

    // reset flag
    _runUserCode = false;

    // test for parameter write access
    if (_prmWriteAvailable)
    {
        lastWrittenParameter = &_parameterWrite;

        // reset write access
        _prmWriteAvailable = false;
    }
    else
    {
        lastWrittenParameter = NULL;
    }

    return true;
}

/**
 * \mainpage HMT Mini-stack software
 *
 * \author Roger Bostock at HMT microelectronics
 * \author Daniel Gehriger <gehriger@linkcad.com>
 * \date    28.01.14
 * \version 1.0.1
 *
 * The Mini-stack software is a minimal IO-Link protocol stack implemented
 * with the HMT PHY ICs and an Atmel ATMega328P micro controller.
 *
 * The function of all operating modes of the PHY's (multi-byte, single-byte
 * and transparent), all operating frequencies (COM2 and COM3) and all
 * SIO drive modes (NPN, PNP, Push-pull and Inactive) have been
 * demonstrated with a range of M-sequences with a device tester.</P>
 *
 * Please refer to the @ref page_change_history for modifications
 * between the software versions.
 *
 * <P>This Mini-stack software is primarily
 * designed to demonstrate the function of the HMT7742/HMT7748 PHY's,
 * and to provide a reference design for their use.  It <I>is</I>
 * intended that users inspect the internals of the code and understand
 * how it functions.  The stack is deliberately cut to a minimum number
 * of lines to give users a chance to follow the code.  In particular,
 * there is no software support for:</P>
 * <UL>
 *  <LI>Events
 *  <LI>ISDU's
 * </UL>
 * The stack is optimised to run efficiently on
 * the microcontroller, and care has been taken to avoid run-time
 * overhead in the interrupt service routines.  This demonstrates how
 * the HMT7742/HMT7748 PHY's can be used to reduce the load on the
 * micro-controller (MHz, mA and kBytes)</P>
 *
 * A demonstration application, see DemoApp, is supplied with the Mini-stack
 * software to facilitate a rapid development start in association
 * with one of the development boards (TM96.1 var. A or B, TM141.0 or TM142.0)
 * 
 * \section StackUsage Stack Usage
 * Three stack implementations are available, all derived from the templated
 * StackBase base class:
 * 
 * - StackMultiByte: Stack implementation using  Multi-Byte
 * mode.
 * - StackSingleByte: Stack implementation using Single-Byte mode.
 * - StackTransparent: Stack implementation using Transparent mode.
 * 
 * All implementations share the same API, and a typedef \c Stack is defined 
 * as an alias for the selected stack implementation (see \ref Compilation). The
 * stack instance is made available as \c Stack::instance.
 *
 * The main() function loop polls the \ref canRunUserCode "Stack::instance.canRunUserCode()"
 * function to determine when it is possible to call the application cyclic code.
 * \ref canRunUserCode "Stack::instance.canRunUserCode()" will return \c true exactly
 * once per IO-Link communication cycle, just after completion of the device response.
 * <b>Even in the absence of IO-Link communication</b> it is set to \c true once per
 * minimum cycle length.
 *
 * <p> \attention The timer 0 interrupts are important for the stack to track the 
 * communication timing, and should not be blocked for more than 50us (max. 
 * latency). The timer interrupt routine does not affect the data presented by 
 * the stack to the user cyclic code, but may affect the reported operating
 * mode.</p>
 *
 * <p> \attention In SIO operation, a PHY data interrupt may arrive at any time and
 * should not be blocked for more than 130us (38.4kBaud operation) or 10us 
 * (230.4kBaud operation).  The stack is idle in this mode, and will not update
 * or use the process data or write parameter fields returned by 
 * \ref canRunUserCode "Stack::canRunUserCode()".</p>
 * 
 * <p> \attention In established IO-Link operation, the PHY interrupt (only) should 
 * be blocked during the operation of the user cyclic code.  No data interrupt 
 * which requires rapid processing will be received in this time. </p>
 * 
 * <p> \attention The user cyclic code must complete in the gap between the last 
 * device transmission and the end of the master transmission (multi-byte
 * exchange) or between the last device transmission and the start of master 
 * transmission (single-byte and transparent exchange)</p>
 * 
 * \subsection Compilation
 * 
 * In order to select a stack implementation, include the corresponding stack 
 * header file from your application code and compile and link the stack source
 * file. 
 * 
 * The code in the stack source files will only be compiled if a 
 * corresponding preprocessor symbol has been defined, as shown below. This 
 * allows a project file to include all stack implementations and to select the 
 * desired stack type by defining the corresponding preprocessor symbol.
 * 
 * - Using the \b Multi-Byte (Io-Link) Mode Stack:
 *   - compile, and link with, \c "stack/stackmultibyte.cpp" with 
 *     \c STACK_MODE_MULTI_BYTE defined.
 *   - include the header file \c "stack/stackmultibyte.h":
 *     \code
 *       #include "stack/stackmultibyte.h"
 *     \endcode
 * 
 * - Using the \b Single-Byte Mode Stack:
 *   - compile, and link with, \c "stack/stacksinglebyte.cpp" with 
 *     \c STACK_MODE_SINGLE_BYTE defined.
 *   - include the header file \c "stack/stacksinglebyte.h":
 *     \code
 *       #include "stack/stacksinglebyte.h"
 *     \endcode
 *     
 * - Using the \b TransparentStack Mode Stack:
 *   - compile, and link with, \c "stack/stacktransparent.cpp" with 
 *     \c STACK_MODE_TRANSPARENT defined.
 *   - include the header file \c "stack/stacktransparent.h":
 *     \code
 *       #include "stack/stacktransparent.h"
 *     \endcode
 *
 * \subsection Hardware Hardware configuration for compilation
 *
 * Multi-byte and single-byte modes are insensitive to the CPU clock
 * frequency, and the internal RC oscillator running at 8MHz is used
 * by default.  The define  F_CPU=8000000UL should be set to achieve
 * this.
 *
 * Transparent mode requires an external oscillator running at 18.432MHz
 * and the define F_CPU=18432000UL should be set for this case.  The
 * additional define `USE_EXT_OSC` should be set to enable the correct
 * clock source, which is set by the ATMega fuses. <p>
 *
 * The correct PHY type, either HMT7742 or HMT7748, must be selected as a
 * preprocessor symbol define.  Define either `USE_HMT7742` to use the
 * HMT7742 PHY, or `USE_HMT7748` to use the HMT7748 PHY.
 *
 * The brown-out detector is enabled at the nominal 1.8V level, using
 * the ATMega fuses.  The ATmega shows correct behaviour at this level
 * even under conditions where the supply connection has considerable
 * contact bounce.
 *
 * The ATMega fuse codings are embedded in the source code, and contained
 * in the ".elf" file produced.
 *
 * \subsection StackConfiguration Stack Configuration
 * 
 * The selected stack is configured by adjusting the public class constants in its
 * header file, as shown below for the multi-byte mode stack:
 * \code
 * class StackMultiByte : public StackBase<StackMultiByte, 
 *         1, // PD_IN_SIZE
 *         1  // PD_OUT_SIZE
 * >
 * {
 * public:
 *     static const uint8_t REVISION_ID =      IoLink::REVISION_ID_1_1;
 *     static const uint16_t VENDOR_ID =       0x01a6; // HMT
 *     static const uint32_t DEVICE_ID =       0x123457;
 *     static const uint32_t BAUD_RATE =       230400;
 *     static const uint8_t MIN_CYC_TIME =     30;  // 30x0.1ms
 *     static const uint8_t MSEQ_CAPABILITY =  IoLink::MSEQCAP_ISDU_NOT_SUPPORTED | 
 *                                             IoLink::MSEQCAP_OP_CODE_0 | 
 *                                             IoLink::MSEQCAP_PREOP_CODE_0;
 *     static const uint8_t PHY_CFG =          CFG_UVT_16_3V | CFG_RF_ABS | CFG_S5V_3_3V
 *                                  | (BAUD_RATE == 38400 ? CFG_BD_38400 : CFG_BD_230400);
 *     static const uint8_t PHY_CTL_SCT =      CTL_SCT_190MA;
 *     static const uint8_t PHY_CTL_MODE =     CTL_IOLINK_MODE;
 *     static const enum SioDriveMode SIO_DRIVE_MODE = DRIVE_MODE_PUSH_PULL;
 *     static const uint8_t PHY_THERM_DEG =    175; // ~175 degrees Celsius
 * // [...]
 * };
 * \endcode
 * Most configuration parameters should be obvious and this documentation 
 * mentions the reference to the appropriate standard / data sheet documentation.
 * 
 * The <b>sequence size</b> and type is determined by the \c PD_IN_SIZE and \c PD_OUT_SIZE
 * template parameters, as well as the class constant \c MSEQ_CAPABILITY, as
 * described in the <i>"IO-Link Interface and System Specification"</i>, V1.1.1, 
 * section B.1.5.
 * 
 * The stacks all currently implement sequences TYPE_0, TYPE_2_1
 * through TYPE_2_5 on a Atmega328p with a 8MHz system clock. Larger sequence sizes
 * are also supported at this frequency when using single-byte mode, but other modes
 * may require a higher clock frequency.
 *
 * All modes both have been tested for the following sequence types at COM2 (38.4kBaud)
 * and COM3 (230.4kBaud):
 * - TYPE_0
 * - TYPE_2_1 through TYPE_2_5
 *
 * \attention Transparent mode requires an external oscillator running
 * 18.432 MHz and the connection of jumpers on the GENIE Explorer boards
 * to connect the ATMega hardware UART pins TXD (to MOSI) and RXD (to MISO).<p>
 *
 * \attention The PHY permanently drives the MISO line in transparent mode,
 * and this interferes with programming of the microcontroller on the SPI interface.
 * For development, a 470ohm resistor may be inserted between the PHY MISO and
 * micro-controller SPI input, to prevent contention when programming.  Alternatively
 * delay in startup (setting the PHY to transparent mode) may be added to provide
 * a window to start programming the device, or the debugWIRE may be used for
 * programming. <p>
 *
 * \attention Sequence TYPE_1_1/1_2 (interleaved) is not supported
 *
 * \subsection SampleApp Sample Application
 * A typical sample application looks like this:
 * \code
 * #include "stack/stackmultibyte.h"
 * #include <avr/sleep.h>
 *
 * void user_configure();
 * void user_run(const Stack::Parameter* param);
 * 
 * int main(void)
 * {
 *     // configure all software modules
 *     Stack::instance.configure();
 * 
 *     // configure user code
 *     user_configure();
 *     
 *     // enable interrupts
 *     sei();
 * 
 *     // select sleep mode
 *     set_sleep_mode(SLEEP_MODE_IDLE);
 * 
 *     // enter infinite loop: processing is interrupt controlled from now on
 *     for (;;)
 *     {
 *         // enter sleep until interrupt wakes us up
 *         sleep_mode();
 * 
 *         // check if it's time to run user code
 *         const Stack::Parameter* paramWrite;
 *         if (Stack::instance.canRunUserCode(paramWrite))
 *         {
 *             Stack::instance.stopInterrupt();
 *             user_run(paramWrite);
 *             Stack::instance.restartInterrupt();
 *         }
 *     }
 * }
 * 
 * void user_run(const Stack::Parameter* param)
 * {
 *     // check for write access to direct parameter page
 *     if (param)
 *     {
 *         // handle parameter write access
 *         // [...]
 *     }
 *     else if (Stack::instance.stackMode() == Stack::STACK_MODE_SIO)
 *     {
 *          // handle SIO mode
 *          // [...]
 *     }
 *     
 *     // update process data
 *     // [...]
 * }
 * \endcode
 */

/*!
   @page page_change_history Change History

   @section section_v0_1_10 0.1.10
   <ul>
   <li> Initial release</li>
   </ul>

   @section section_v0_1_11 0.1.11
   <ul>
   <li> minimised update delay in SIO mode</li> 
   <li> initialisation of _parameterWrite address corrected</li>
   <li> multiple process and on-demand octets in all stacks</li> 
   <li> implemented non-response to incorrect M-sequence types</li> 
   <li> implemented SIO_DRIVE_MODE</li> 
   <li> implemented SIO switch to IO-Link listening after delay if HiZ</li> 
   <li> implemented masterLoss indication</li>
   </ul>

   @section section_v0_1_12 0.1.12
   <ul>
   <li> IODD file versions 1.01 (for IO-Link 1.0) are provided</li>
   </ul>

   @section section_v0_1_13 0.1.13
   <ul>
   <li> transparent mode uses the ATMega hardware internal UART</li>
   <li> updated IODD file versions 1.01 (for IO-Link 1.0) and 1.1 (for IO-Link 1.1) are provided</li> 
   </ul>

   @section section_v1_0_0 1.0.0

   Release date: 11.04.13
   <ul>
   <li> testing carried out with IO-Link master for v1.1 and v1.0
   <li> corrections to improve EMC performance
   <li> AVR6 support included
   </ul>

   @section section_v1_0_1 1.0.1

   Release date: 28.01.14
   <ul>
   <li> prefix StackBase:: added to SPI accesses in Stackbase.h, to satisfy modified AVR compiler requirement
   </ul>

*/

#endif // stackbase_h__

