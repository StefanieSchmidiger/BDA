//------------------------------------------------------------------------------
//! \file
//! Declares the StackTransparent class
//------------------------------------------------------------------------------
#ifndef stacktransparent_h__
#define stacktransparent_h__

#include "stackbase.h"

// Declare SS/ handler
struct TransparentModeSsHandler;

// declare the existence of the interrupt service routine.  This is
// required for the friend statement in the MiniStack.
ISR(USART_TX_vect);
ISR(USART_RX_vect);

/**
 * Stack implementation using the PHY transparent mode
 * 
 * The internal UART of the AVR is used, which requires the connection
 * of an external crystal or oscillator to give the required frequency
 * stability.  A frequency of 18.432MHz is used as standard, which
 * is internally divided for 38.4kBaud and 230.4kBaud operation.
 * 
 * The general operation is the same as for the multi-octet stack
 * implementation, but now all of the low-level checks and most of
 * the timing must be carried out by the AVR.
 * 
 * The @ref StackTransparent::_ddlMode "_ddlMode" member defines the
 * state of the data-link layer.  In  @ref IoLink::DDL_MODE_IDLE "DDL_MODE_IDLE",
 * the device drives the CQ line according to the sioLevel set, and
 * the @ref StackBase::SioDriveMode "drive mode" defined .  The other states
 * support IO-Link communication.
 * 
 * @section section_implementation_details Implementation details
 * 
 * @subsection subsection_rx_uart Rx UART operation
 * 
 * The UART is enabled, except under the following conditions:
 * 
 * - in @ref IoLink::DDL_MODE_IDLE "DDL_MODE_IDLE" if CQ is driven 
 * - in @ref IoLink::DDL_MODE_IDLE "DDL_MODE_IDLE" for a period
 *   (dead cycles) after the CQ line was driven
 * - when transmitting using the AVR UART
 * 
 * Following receipt of a UART frame, a timer is started.  If more frames
 * are expected and the next frame is not received within
 * the expected time, then any partially received sequence is discarded.
 * Following a parity or checksum error, any partially received sequence
 * is similarly discarded.
 * 
 *  @subsection subsection_establish_comms Establish comms
 * 
 * If a short-circuit is detected by the PHY in
 * @ref IoLink::DDL_MODE_IDLE "DDL_MODE_IDLE", then the stack switches immediately
 * @ref IoLink::DDL_MODE_ESTABLISH_COM "DDL_MODE_ESTABLISH_COM", and starts listening
 * for UART frames.  Typically the end of the wake-up pulse and any
 * communications at higher frequencies than the operational frequency
 * of the device are rejected as invalid frames.  In
 * @ref IoLink::DDL_MODE_ESTABLISH_COM "DDL_MODE_ESTABLISH_COM" the CQ line is
 * not driven by the device.
 * 
 * If a complete, correct, master sequence is received in either mode
 * @ref IoLink::DDL_MODE_ESTABLISH_COM "DDL_MODE_ESTABLISH_COM" (following a
 * wake-up request, WURQ) or in @ref IoLink::DDL_MODE_IDLE "DDL_MODE_IDLE"
 *(when the line is not driven), then the
 * @ref StackTransparent::_ddlMode "_ddlMode" is
 * changed to  @ref IoLink::DDL_MODE_STARTUP "DDL_MODE_STARTUP".  The device
 * will now only leave IO-Link operation if a FALLBACK command is sent
 * by the master.
 * 
 * If the PHY reports that the short-circuit condition no longer exists
 * while we are in @ref IoLink::DDL_MODE_ESTABLISH_COM "DDL_MODE_ESTABLISH_COM",
 * then we return to SIO operation and
 * @ref IoLink::DDL_MODE_IDLE "DDL_MODE_IDLE".  The PHY reports a
 * short-circuit condition for ca. 100ms after a short-circuit condition
 * is detected on the line as part of the output self-protection.
 * 
 * @subsection subsection_spi_commms SPI communication
 * 
 * The PHY MOSI and MISO lines are used to transmit both SPI data
 * and to transmit and receive data from the AVR UART.  The AVR UART
 * RX and TX pins must be connected to the AVR MISO and MOSI pins
 * respectively, and suitable jumper resistor footprints are provided
 * on the GENIE Explorer boards.
 * 
 * Once IO-Link communication is established, the predictable timing
 * of the IO-Link exchanges means that a conflict on the lines can
 * be avoided.  SPI communication is carried out immediately after
 * sending on the UART TX line.
 * 
 * Following a recognised wake-up request (WURQ) the call-back timer
 * is stopped, suspending SPI communication while waiting for the
 * master.
 * 
 * In @ref IoLink::DDL_MODE_IDLE "DDL_MODE_IDLE" it is possible that an SPI transmission may
 * conflict with an incoming master sequence, which will
 * corrupt the sequence.  (Note, it is also possible that the device
 * may start to drive the CQ line in normal SIO operation, similarly
 * corrupting the sequence.  An inverted line state in a high
 * impedance condition is not considered sufficient evidence for
 * presence of a master to block SIO operation.)
 * 
 * For the StackTransparent, the PHY MOSI line is normally driven by the UART
 * TX pin.  This operation is only interrupted for actual
 * SPI communications.  It would be possible to use the AVR UART as an
 * SPI driver and so avoid any switching, but this has not been
 * implemented here.
 * 
 * The PORTD1 retains the state of the SIO output during SPI communication
 * and ensures that the correct level is reestablished before the
 * SS line is released (set to '1').  PORTD1 is also set to '1'
 * before enabling transmission through the PHY, so that the
 * line output level on CQ is correctly driven low while the output
 * is enabled before and after the UART itself is sending.
 * 
 * In transparent mode, the PHY permanently drives the MISO line.
 * This is incompatible with the PHY sharing the bus with other
 * SPI slaves, and also interferes with programming the AVR over the
 * SPI lines.  (Programming via the reset pin is still possible).
 * For stack development purposes we have inserted a 270ohm resistor
 * between the PHY and the AVR to allow programming
 * over the SPI lines.
 */
class StackTransparent : public StackBase<StackTransparent, 
        1, // PD_IN_SIZE
        1, // PD_OUT_SIZE
        TransparentModeSsHandler
>
{
public:
    //! RevisionID of protocol implemented (Direct Parameter 0x04)
    static const uint8_t REVISION_ID =      IoLink::REVISION_ID_1_1;

    //! VendorID (Direct Parameters 0x07 and 0x08)
    static const uint16_t VENDOR_ID =       0x01a6; // HMT

    //! DeviceID (Direct Parameters 0x09 - 0x0b)
    static const uint32_t DEVICE_ID =       0x123456;

    //! Communication speed (must be either 38400 or 230400)
    static const uint32_t BAUD_RATE =       38400;

    //! MinCycleTime in 0.1ms units
    static const uint8_t MIN_CYC_TIME =     30;  // 30x0.1ms

    //! M-sequence Capability (Direct Parameter 0x03)
    static const uint8_t MSEQ_CAPABILITY =  IoLink::MSEQCAP_ISDU_NOT_SUPPORTED | 
                                            IoLink::MSEQCAP_OP_CODE_0 | 
                                            IoLink::MSEQCAP_PREOP_CODE_0;

    //! PHY configuration
    static const uint8_t PHY_CFG =          CFG_UVT_16_3V | CFG_RF_ABS | CFG_S5V_3_3V
                                 | (BAUD_RATE == 38400 ? CFG_BD_38400 : CFG_BD_230400);

    //! PHY short-circuit threshold
    static const uint8_t PHY_CTL_SCT =      CTL_SCT_190MA;

    //! PHY mode (must be CTL_TRNS_MODE)
    static const uint8_t PHY_CTL_MODE =     CTL_TRNS_MODE;

    //! PHY drive mode to use in SIO mode
    static const enum SioDriveMode SIO_DRIVE_MODE = DRIVE_MODE_PUSH_PULL;

    //! PHY thermal shutdown temperature (in degrees centigrade)
    static const uint8_t PHY_THERM_DEG =    175;

    /**
     * Configure the specific stack
     */
    void                configureStack();

    /** reimplementation */
    uint8_t setSioActive();
 
    /** reimplementation */
    uint8_t setSioListen();
    
    /** reimplementation */
    uint8_t setIoLinkListen();

    /**
     * The one and only stack instance.
     */
    static StackTransparent instance;

private:
    enum FrameState
    {
        FRAME_RCV_MC,                       //!< Expecting MC octet in message
        FRAME_RCV_CKT,                      //!< Expecting CKT octet
        FRAME_RCV_CKT_CONT,                 //!< Expecting CKT octet, continuation
        FRAME_RCV_PD,                       //!< Expecting process output data
        FRAME_RCV_OD0,                      //!< Expecting first on-demand octet in write-access message
        FRAME_RCV_ODX,                      //!< Expecting remaining on-demand octet in write-access message

        FRAME_SND_OD0,                      //!< Sending first on-demand octet
        FRAME_SND_ODX,                      //!< Sending remaining on-demand octet in read-access message
        FRAME_SND_PD,                       //!< Send process input data
        FRAME_SND_CKS,                      //!< Send CKS octet
        FRAME_SND_DONE                      //!< Done sending
    };

    /**
     * Private constructor
     * 
     * \note Use \c Stack::instance to access the stack instance
     */
    StackTransparent() {}

    /**
     * Handle status flags of PHY register
     */
    inline void         handleStatus(uint8_t status);

    /**
     * State machine handler for DL layer
     */
    inline void         onUpdateDlStateMachine();

    /** 
     * Frame handler
     * 
     * IO-Link frame handling depends on the current \ref IoLink::DeviceDLMode 
     * "operating mode" (IDLE, STARTUP, PREOPERATE and OPERATE), while sharing
     * most of the decision logic. This function implements frame handling for
     * all modes. The template parameter \c DDL_MODE acts as a compile-time
     * switch to include / exclude code as necessary. This is not done at run
     * time in order to reduce frame handling time.
     * 
     * \param DDL_MODE  Select operating mode for frame handling logic
     * \return          Result code
     */
    template <IoLink::DeviceDLMode DDL_MODE>
    HandlerResult       handleOperatingMode();
    
    /** 
     * Frame handler for the first UART frames
     * 
     * This function handles the common logic for the MC and CKT UART frames
     * 
     * \return          Result code
     */
    HandlerResult       handleFirstOperatingModes();

    /**
     * Start reception of byte over UART
     * 
     * The callback \c onRxDone() will be called after reception is complete
     */
     void               uartRxEnable();

    /**
     * Transmit byte over UART
     * 
     * The callback \c onTxDone() will be called after transmission is complete
     * 
     * \param ch        Data to transmit
     */
    void                uartTx(uint8_t ch);

    /**
     * Callback for handling wake-up interrupt
     */ 
    void                onWakeUp();

    /**
     * Callback called when done sending byte over UART
     */
    void                onTxDone();

    /**
     * Callback called when done receiving byte over UART
     * 
     * The received byte is available in _rxData
     */
    void                onRxDone();

    /**
     * Callback called when no data has been received for more than 13 * Tbit
     */ 
    void                onRxTimeout();

    /** The ISR function can access the stack state, and is declared here as a friend. */
    friend void ::TIMER0_COMPA_vect();
    friend void ::USART_TX_vect();
    friend void ::USART_RX_vect();
    friend void ::PCINT1_vect();

private:
    FrameState           _frameState;
    IoLink::DeviceDLMode _receiveDdlMode;
    uint8_t              _mc;
    uint8_t              _ckt;
    uint8_t              _checksum8;
    uint8_t              _odOut;
    uint8_t              _pdOut[PD_OUT_SIZE];
    uint8_t              _i; // general purpose counter
    bool                 _rxError;
    bool                 _frameTypeOk;
};

/**
 * Alias for selected stack type
 */
typedef StackTransparent Stack;

/**
 * Implementation of class for handling SPI SS/ line with TX save/restore
 * 
 * Caching of the TX line is done by the port register (PORTB) itself.
 * This class simply enables / disables SPI to toggle between SPI and PORTB register
 * value.
 */
class TransparentModeSsHandler : DefaultSsHandler
{
public:

    /**
     * Reimplementation so that PORTB3/MOSI is not active by default
     */
    void static configure()
    {
        /* Data direction in the SPI bits, PB2/SS and PB5/SCK
           Drive SS high by default */
        PORTB |= _BV(PORTB2) | _BV(PORTB5);
        DDRB  |= _BV(DDB5) | _BV(DDB2);
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
        //SPSR = _BV(SPI2X);
    }

    /**
     * Assert SS/ to begin SPI communication
     */
    static void assert()
    {
        // wait until byte transferred
        loop_until_bit_is_set(UCSR0A, UDRE0);

        if (_reading)
        {
            // disable USART receiver
            UCSR0B &= ~(_BV(RXCIE0) | _BV(RXEN0));
        }

        // assert SS/
        DefaultSsHandler::assert();
        
        // MOSI now controlled by PORTB3/MOSI rather than D1/TXD
        // any glitch on line will be ignored
        DDRD &= ~_BV(DDD1); // PD1/TX
        DDRB |= _BV(DDB3);  // PB3/MOSI

        // enable SPI => SPI controls MOSI
        Spi::enable();
    }

    /**
     * Deassert SS/ to terminate SPI communication
     */
    static void deassert()
    {
        // disable SPI => PORTB3 controls MOSI
        Spi::disable();
        
        // return MOSI control to D1/TXD rather than PORTB3/MOSI
        // any glitch on line will be ignored
        DDRB &= ~_BV(DDB3); // PB3/MOSI
        DDRD |= _BV(DDD1);  // PD1/TX
        
        // de-assert SS/
        DefaultSsHandler::deassert();

        if (_reading)
        {
            // re-enable USART receiver
            UCSR0B |= (_BV(RXEN0) | _BV(RXCIE0));
        }
    }

    //! Set read/write mode
    static void setReading(bool set = true) { _reading = set; }

    //! Test if reading
    bool        isReading() const  { return _reading; }

private:
    static bool _reading;
};

#endif // stacktransparent_h__
