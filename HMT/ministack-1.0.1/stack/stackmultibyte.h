//------------------------------------------------------------------------------
//! \file
//! Declares the StackMultiByte class
//------------------------------------------------------------------------------
#ifndef stackmultibyte_h__
#define stackmultibyte_h__

#include "stackbase.h"

/**
 * Stack implementation using multi-byte mode
 */
class StackMultiByte : public StackBase<StackMultiByte, 
        2, // PD_IN_SIZE			// 8 bytes of sensor data
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
    static const uint32_t BAUD_RATE =       230400;		// COM3

    //! MinCycleTime in 0.1ms units
    static const uint8_t MIN_CYC_TIME =     5;  // 5x0.1ms

    //! M-sequence Capability (Direct Parameter 0x03)
    static const uint8_t MSEQ_CAPABILITY =  IoLink::MSEQCAP_ISDU_NOT_SUPPORTED | 
                                            IoLink::MSEQCAP_OP_CODE_0 | 
                                            IoLink::MSEQCAP_PREOP_CODE_0;

    //! PHY configuration
    static const uint8_t PHY_CFG =          CFG_UVT_16_3V | CFG_RF_ABS | CFG_S5V_3_3V
                                 | (BAUD_RATE == 38400 ? CFG_BD_38400 : CFG_BD_230400);

    //! PHY short-circuit threshold
    static const uint8_t PHY_CTL_SCT =      CTL_SCT_190MA;

    //! PHY mode (must be CTL_IOLINK_MODE)
    static const uint8_t PHY_CTL_MODE =     CTL_IOLINK_MODE;

    //! PHY drive mode to use in SIO mode
    static const enum SioDriveMode SIO_DRIVE_MODE = DRIVE_MODE_PUSH_PULL;

    //! PHY thermal shutdown temperature (in degrees centigrade)
    static const uint8_t PHY_THERM_DEG =    175; // ~175°C

    /**
     * The one and only stack instance.
     */
    static StackMultiByte instance;

private:

    /**
     * Private constructor
     * 
     * \note Use \c Stack::instance to access the stack instance
     */
    StackMultiByte() {}

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
    HandlerResult       handleOperatingMode(uint8_t mc);

    /**
     * Starts the user call-back timer after the last octet of the frame
     * reply has been sent
     * 
     * \warning Do NOT inline this function! Doing so will delay the start
     *          of SPI communication after receiving a frame.
     * 
     * \octetCnt        Number of octets in frame reply
     */
    void                startCallbackTimerAfterReply(uint8_t octetCnt);

    /** The ISR function can access the stack state, and is declared here as a friend. */
    friend void ::PCINT1_vect();
};

/**
 * Alias for selected stack type
 */
typedef StackMultiByte Stack;

#endif // stackmultibyte_h__
