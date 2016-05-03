//------------------------------------------------------------------------------
//! \file
//! Declares the StackSingleByte class
//------------------------------------------------------------------------------
#ifndef singlebytestack_h__
#define singlebytestack_h__

#include "stackbase.h"

/**
 * Stack implementation using the PHY single-octet mode
 * 
 * The PHY UART is used to handle UART frames singly.
 * 
 * The general operation is the same as for the multi-octet stack
 * implementation, but now the checksum checks must be carried out
 * by the stack.
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
 * Following receipt of a UART frame, a timer is started in the PHY.  If
 * more frames are expected (no transmit is received) and the next frame
 * is not received within the expected time, then the PHY reports a
 * communication error and any partially received sequence is discarded.
 * Following a parity or checksum error, any partially received sequence
 * is similarly discarded.
 * 
 *  @subsection subsection_establish_comms Establish comms
 * 
 * If a short-circuit is detected by the PHY in SIO mode, (possible WURQ),
 * updates of the SIO are temporarily silenced by introducing a delay
 * before the next user code call.  If an exchange does not complete in
 * this time, then the stack will automatically switch back to SIO operation
 * after ca. 100ms.
 * 
 * @attention The single octet mode handshake requires the device to be
 * in IO-Link mode.  The device @b must be placed into IoLinkListen mode
 * before the first LINK::END command is returned following the reception
 * of the firs UART frame.
 * 
 * The PHY is switched to IoLinkListen mode after a delay once the line
 * is in at high impedance in SIO operation.  This allows for the reception
 * of UART frames despite the lack of a wake-up request.  This reception
 * may be interrupted if the device drives the CQ line, and a retry is
 * required.
 */
class StackSingleByte : public StackBase<StackSingleByte, 
        4, // PD_IN_SIZE
        0  // PD_OUT_SIZE
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
    static const uint32_t BAUD_RATE =       230400;

    //! MinCycleTime in 0.1ms units
    static const uint8_t MIN_CYC_TIME =     1;  // 1x0.1ms

    //! M-sequence Capability (Direct Parameter 0x03)
    static const uint8_t MSEQ_CAPABILITY =  IoLink::MSEQCAP_ISDU_NOT_SUPPORTED | 
                                            IoLink::MSEQCAP_OP_CODE_4 | 
                                            IoLink::MSEQCAP_PREOP_CODE_0;

    //! PHY configuration
    static const uint8_t PHY_CFG =          CFG_UVT_16_3V | CFG_RF_ABS | CFG_S5V_3_3V
                                 | (BAUD_RATE == 38400 ? CFG_BD_38400 : CFG_BD_230400);

    //! PHY short-circuit threshold
    static const uint8_t PHY_CTL_SCT =      CTL_SCT_190MA;

    //! PHY mode (must be CTL_SGL_MODE)
    static const uint8_t PHY_CTL_MODE =     CTL_SGL_MODE;

    //! PHY drive mode to use in SIO mode
    static const enum SioDriveMode SIO_DRIVE_MODE = DRIVE_MODE_PUSH_PULL;

    //! PHY thermal shutdown temperature (in degrees centigrade)
    static const uint8_t PHY_THERM_DEG =    175; // ~175�C

    /**
     * The one and only stack instance.
     */
    static StackSingleByte instance;

private:
    /**
     * Private constructor
     * 
     * \note Use \c Stack::instance to access the stack instance
     */
    StackSingleByte() : _frameState(FRAME_RESET),
                        _tccr0bRecord(0),
                        _receiveDdlMode(IoLink::DDL_MODE_IDLE) {}

    /** 
     * ISR handler as member function 
     */
    inline void         onPhyInterrupt() __attribute__((always_inline));

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
     * \param mc        Received MC frame octet
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
     * Restarts a stopped timer
     * 
     */
    void inline restartTimer()
    {
        if (TCCR0B == 0)
           TCCR0B = _tccr0bRecord;          
    }
    
    /**
     * Starts the user call-back timer after the last octet of the frame
     * reply has been sent
     * 
     * \warning Do NOT inline this function! Doing so will delay the start
     *          of SPI communication after receiving a frame.
     */
    void                startCallbackTimerAfterReply();

    /** The ISR function can access the stack state, and is declared here as a friend. */
    friend void ::PCINT1_vect();

    enum FrameState
    {
        FRAME_RESET,            //!< Initial machine state
        FRAME_RCV_MC,           //!< Expecting MC octet in message
        FRAME_RCV_CKT,          //!< Expecting CKT octet
        FRAME_RCV_CKT_CONT,     //!< Expecting CKT octet, continuation
        FRAME_RCV_PD,           //!< Expecting process output data
        FRAME_RCV_OD0,          //!< Expecting first on-demand octet in write-access message
        FRAME_RCV_ODX,          //!< Expecting remaining on-demand octet in write-access message
        FRAME_SND_OD0,          //!< Sending first on-demand octet in read-access message
        FRAME_SND_ODX,          //!< Sending remaining on-demand octet in read-access message
        FRAME_SND_PD,           //!< Send process input data
        FRAME_SND_CKS,          //!< Send CKS octet
        FRAME_SND_DONE          //!< Done sending
    };

    FrameState          _frameState;
    IoLink::DeviceDLMode _receiveDdlMode;
    uint8_t             _mc;
    uint8_t             _ckt;
    uint8_t             _cks;
    uint8_t             _checksum8;
    uint8_t             _odOut;
    uint8_t             _pdOut[PD_OUT_SIZE];
    uint8_t             _i; // general purpose counter
    uint8_t             _tccr0bRecord;
    bool                _frameTypeOk;
};

/**
 * Alias for selected stack type
 */
typedef StackSingleByte Stack;

#endif // singlebytestack_h__
