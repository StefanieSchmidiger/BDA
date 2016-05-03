//------------------------------------------------------------------------------
//! \file
//! Declares the IO-Link class
//------------------------------------------------------------------------------ 
#ifndef iolink_h__
#define iolink_h__

#include <avr/pgmspace.h>

/**
 * Helper class for supporting IO-Link standard
 */
class IoLink
{
public:
    /**
     * M-sequence control (MC) octet
     */
    enum MSeqCtrl
    {
        MC_ADDRESS_MASK = 0x1f,     //< Bitmask for address bits in MC octet
        MC_CHANNEL_MASK = 0x60,     //< Bitmask for channel bits in MC octet
        MC_RW_MASK      = 0x80,     //< Bitmask for R/W bits in MC octet

        MC_ADDR_ISDU_COUNT_MASK = 0x0f, //< Element counter within an ISDU 
        MC_ADDR_ISDU_START = 0x10,  //< Start of an ISDU I-Service (Table 50)
        MC_ADDR_ISDU_IDLE1 = 0x11,  //< No request for ISDU transmission (Table 50)
        MC_ADDR_ISDU_IDLE2 = 0x12,  //< No request for ISDU transmission (Table 50)
        MC_ADDR_ISDU_ABORT = 0x1f,  //< Abort entire service

        MC_CHNL_PROC    = 0 << 5,   //< Process data channel
        MC_CHNL_PAGE    = 1 << 5,   //< Direct parameter page channel
        MC_CHNL_DIAG    = 2 << 5,   //< Diagnosis channel
        MC_CHNL_ISDU    = 3 << 5,   //< ISDU channel

        MC_WRITE        = 0 << 7,   //< Write access (tx of user data from Master to Device) 
        MC_READ         = 1 << 7    //< Read access (tx of user data from Device to Master)
    };

    /**
     * Checksum / M-sequence type (CKT) octet
     */
    enum MSeqCkt
    {
        CKT_CHECKSUM_MASK = 0x3f,   //< Bitmask for checksum bits in CKT octet
        CKT_TYPE_MASK     = 0xc0,   //< Bitmask for type bits in CKT octet

        CKT_TYPE_0      = 0 << 6,   //< Type 0
        CKT_TYPE_1      = 1 << 6,   //< Type 1
        CKT_TYPE_2      = 2 << 6,   //< Type 2
        CKT_TYPE_NONE   = 3 << 6    //< Reserved
    };

    /**
     * Checksum / status (CKS) octet
     */
    enum MSeqCks
    {
        CKS_CHECKSUM_MASK   = 0x3f, //< Bitmask for checksum bits in CKS octet        
        CKS_PD_STATUS_MASK  = 0x40, //< Bitmask for PD status bit
        CKS_EVENT_FLAG_MASK = 0x80, //< Bitmask for event flag bit

        CKS_PD_VALID      = 0 << 6, //< Process data valid
        CKS_PD_INVALID    = 1 << 6, //< Process data invalid

        CKS_NO_EVENT      = 0 << 7, //< No event
        CKS_EVENT         = 1 << 7  //< Event
    };

    /**
     * Page 1 ranges from 0x00 to 0x0F. It comprises the following categories of 
     * parameters:
     * - Communication control
     * - Identification parameter
     * - Application control
     */
    enum DirectParamPage
    {
        PAGE_MASTER_CMD         = 0x00, //< Master command to switch to operating states
        PAGE_MASTER_CYCLE_TIME  = 0x01, //< Actual cycle duration used by the Master to address the Device
        PAGE_MIN_CYCLE_TIME     = 0x02, //< Minimum cycle duration supported by a Device
        PAGE_MSEQ_CAPABILITY    = 0x03, //< Information about implemented options related to M-sequences and physical configuration
        PAGE_REVISION_ID        = 0x04, //< ID of the used protocol version for implementation (shall be set to 0x11)
        PAGE_PD_IN              = 0x05, //< Number and structure of input data (Process Data from Device to Master)
        PAGE_PD_OUT             = 0x06, //< Number and structure of output data (Process Data from Master to Device)
        PAGE_VENDOR_ID_1        = 0x07, //< Unique vendor identification (1)
        PAGE_VENDOR_ID_2        = 0x08, //< Unique vendor identification (2)
        PAGE_DEVICE_ID_1        = 0x09, //< Unique Device identification allocated by a vendor (1)
        PAGE_DEVICE_ID_2        = 0x0a, //< Unique Device identification allocated by a vendor (2)
        PAGE_DEVICE_ID_3        = 0x0b, //< Unique Device identification allocated by a vendor (3)
        PAGE_FUNCTION_ID_1      = 0x0c, //< Reserved (shall be 0x00)
        PAGE_FUNCTION_ID_2      = 0x0d, //< Reserved (shall be 0x00)
        PAGE_RESERVED           = 0x0e, //< Reserved
        PAGE_SYSTEM_CMD         = 0x0f, //< Command interface for end user applications
        PAGE_DEVICE_SPECIFIC_10 = 0x10, //< Device specific parameter 0x10
        PAGE_DEVICE_SPECIFIC_11 = 0x11, //< Device specific parameter 0x11
        PAGE_DEVICE_SPECIFIC_12 = 0x12, //< Device specific parameter 0x12
        PAGE_DEVICE_SPECIFIC_13 = 0x13, //< Device specific parameter 0x13
        PAGE_DEVICE_SPECIFIC_14 = 0x14, //< Device specific parameter 0x14
        PAGE_DEVICE_SPECIFIC_15 = 0x15, //< Device specific parameter 0x15
        PAGE_DEVICE_SPECIFIC_16 = 0x16, //< Device specific parameter 0x16
        PAGE_DEVICE_SPECIFIC_17 = 0x17, //< Device specific parameter 0x17
        PAGE_DEVICE_SPECIFIC_18 = 0x18, //< Device specific parameter 0x18
        PAGE_DEVICE_SPECIFIC_19 = 0x19, //< Device specific parameter 0x19
        PAGE_DEVICE_SPECIFIC_1A = 0x1a, //< Device specific parameter 0x1a
        PAGE_DEVICE_SPECIFIC_1B = 0x1b, //< Device specific parameter 0x1b
        PAGE_DEVICE_SPECIFIC_1C = 0x1c, //< Device specific parameter 0x1c
        PAGE_DEVICE_SPECIFIC_1D = 0x1d, //< Device specific parameter 0x1d
        PAGE_DEVICE_SPECIFIC_1E = 0x1e, //< Device specific parameter 0x1e
        PAGE_DEVICE_SPECIFIC_1F = 0x1f, //< Device specific parameter 0x1f

        PAGE_NO_PARAMETER       = 0xff  //< Special value indicating absence of a parameter
    };

    /**
     * The Master application is able to check the status of a Device or to 
     * control its behavior with the help of MasterCommands.
     */
    enum MasterCommand
    {
        MCMD_FALLBACK           = 0x5a, //< Transition from communication to SIO mode
        MCMD_MASTER_IDENT       = 0x95, //< Indicates a Master revision higher than 1.0
        MCMD_DEVICE_IDENT       = 0x96, //< Start check of Direct Parameter page for changed entries
        MCMD_DEVICE_STARTUP     = 0x97, //< Switches the Device from OPERATE or PREOPERATE to STARTUP
        MCMD_PD_OUT_OPERATE     = 0x98, //< Process output data valid
        MCMD_DEVICE_OPERATE     = 0x99, //< Process output data invalid or not available. Switches from STARTUP or PREOPERATE to OPERATE
        MCMD_DEVICE_PREOPERATE  = 0x9a  //< Switches the Device from STARTUP to state PREOPERATE
    };

    /**
     * MasterCycleTime and MinCycleTime
     * 
     * Recommended cycle times (based on frame type 2.1)
     * - COM1: 18.0 ms
     * - COM2: 2.3ms
     * - COM3: 0.4ms
     */
    enum CycleTime
    {
        CYC_MULTIPLIER_MASK     = 0x3f,   //< Bitmask for cycle time multiplier
        CYC_TIME_BASE_MASK      = 0xc0,   //< Bitmask for time base

        CYC_TIME_BASE_0_1_MS    = 0 << 6, //< cycle time = multiplier * 0.1ms
        CYC_TIME_BASE_0_4_MS    = 1 << 6, //< cycle time = multiplier * 0.4ms
        CYC_TIME_BASE_1_6_MS    = 2 << 6  //< cycle time = multiplier * 1.6ms
    };

    /**
     * M-sequence Capability
     * 
     * \see IO-Link Interface and System Specification, V1.1.1, B.1.5.
     */
    enum MSeqCapability
    {
        MSEQCAP_ISDU_MASK           = 0x01,     //< Bitmask for ISDU bit
        MSEQCAP_OP_MASK             = 0x0e,     //< Bitmask for OPERATE M-sequence type
        MSEQCAP_PREOP_MASK          = 0x30,     //< Bitmask for PREOPERATE M-sequence type

        MSEQCAP_ISDU_SUPPORTED      = 1 << 0,   //< ISDU (SPDUs) supported
        MSEQCAP_ISDU_NOT_SUPPORTED  = 0 << 0,   //< ISDU (SPDUs) supported

        // see table A.10
        MSEQCAP_OP_CODE_0           = 0 << 1,   //< OPERATE mode code 0 (various)
        MSEQCAP_OP_CODE_1           = 1 << 1,   //< OPERATE mode code 1 (TYPE_1_2 / 2 OD) => NOT SUPPORTED!
        MSEQCAP_OP_CODE_4           = 4 << 1,   //< OPERATE mode code 4 (TYPE_2_V / 1 OD / 0 .. 32 PDin/out)
        MSEQCAP_OP_CODE_5           = 5 << 1,   //< OPERATE mode code 5 (TYPE_2_V / 2 OD / PDin/out) => NOT SUPPORTED!
        MSEQCAP_OP_CODE_6           = 6 << 1,   //< OPERATE mode code 6 (TYPE_2_V / 8 OD / PDin/out) => NOT SUPPORTED!
        MSEQCAP_OP_CODE_7           = 7 << 1,   //< OPERATE mode code 7 (TYPE_2_V / 32 OD / PDin/out) => NOT SUPPORTED!

        // see table A.8
        MSEQCAP_PREOP_CODE_0        = 0 << 4,   //< PREOPERATE mode code 0 (TYPE_0 / 1 OD byte)
        MSEQCAP_PREOP_CODE_1        = 1 << 4,   //< PREOPERATE mode code 1 (TYPE_1_2 / 2 OD byte) => NOT SUPPORTED!
        MSEQCAP_PREOP_CODE_2        = 2 << 4,   //< PREOPERATE mode code 2 (TYPE_1_V / 8 OD bytes) => NOT SUPPORTED!
        MSEQCAP_PREOP_CODE_3        = 3 << 4    //< PREOPERATE mode code 3 (TYPE_1_V / 16 OD bytes) => NOT SUPPORTED!
     };

    /**
     * Revision ID (RID)
     * 
     * The RevisionID parameter is the two-digit version number of the SDCI 
     * protocol implemented within the Device.
     * 
     * \note The RevisionID can be overwritten (see 10.6.3). An accepted 
     *       different RevisionID shall be volatile.
     */
    enum RevisionId
    {
        REVISION_ID_1_0             = 0x10,     //< Revision 1.0
        REVISION_ID_1_1             = 0x11,     //< Revision 1.1
    };

    /**
     * ProcessDataIn parameter
     * 
     * \see IO-Link Interface and System Specification, V1.1.1, B.1.7
     */
    enum ProcessDataIn
    {
        PDIN_LENGTH_MASK            = 0x1f,     //< Bitmask for PDin length

        PDIN_SIO_SUPPORTED          = 1 << 6,   //< SIO mode supported
        PDIN_SIO_NOT_SUPPORTED      = 0 << 6,   //< SIO mode not supported

        PDIN_BITS                   = 0 << 7,   //< PDin = length bits
        PDIN_BYTES_PLUS_1           = 1 << 7    //< PDin = length + 1 bytes
    };

    /**
     * ProcessDataOut parameter
     * 
     * \see IO-Link Interface and System Specification, V1.1.1, B.1.8
     */
    enum ProcessDataOut
    {
        PDOUT_LENGTH_MASK           = 0x1f,     //< Bitmask for PDout length

        PDOUT_BITS                  = 0 << 7,   //< PDout = length bits
        PDOUT_BYTES_PLUS_1          = 1 << 7    //< PDout = length + 1 bytes
    };

    /**
     * Device DL-mode
     * 
     * \see IO-Link Interface and System Specification V1.1.1, 7.3.2.5
     */
    enum DeviceDLMode
    {
        DDL_MODE_IDLE,          //!< Stack in SIO mode
        DDL_MODE_ESTABLISH_COM, //!< Stack in ESTABLISH_COM mode
        DDL_MODE_STARTUP,       //!< Stack in STARTUP mode
        DDL_MODE_PREOPERATE,    //!< Stack in PREOPERATE mode
        DDL_MODE_OPERATE        //!< Stack in OPERATE mode
    };

    /**
     * Decode the MasterCycleTime or MinCycleTime parameter into 0.1ms units
     * 
     * \param cycleTimeParam Value of MasterCycleTime or MinCycleTime parameter
     * \return Cycle time in 1/10 of ms, or 0 if parameter invalid
     */
    uint16_t            decodeCycleTime(uint8_t cycleTimeParam)
    {
        uint16_t multiplier = cycleTimeParam & CYC_TIME_BASE_MASK;

        switch (cycleTimeParam & CYC_TIME_BASE_MASK)
        {
        case CYC_TIME_BASE_0_1_MS:  return multiplier;
        case CYC_TIME_BASE_0_4_MS:  return (multiplier << 2) + 64;
        case CYC_TIME_BASE_1_6_MS:  return (multiplier << 4) + 320;
        default: return 0;
        }
    }

    /**
     * Encode desired cycle time into MasterCycleTime or MinCycleTime parameter
     * 
     * The resulting parameter may not correspond exactly to the desired input
     * value. Use \c decodeCycleTime to obtain the exact value.
     * 
     * \see \c ENCODE_CYCLE_TIME()
     * 
     * \param cycleTimeDeciMs Cycle time in 1/10 of ms
     * \return Parameter value, or 0 if cycle time out of valid range
     */
    uint8_t         encodeCycleTime(uint16_t cycleTimeDeciMs)
    {
        if (cycleTimeDeciMs < 4)
            return 0;
        else if (cycleTimeDeciMs < 64)
            return cycleTimeDeciMs | CYC_TIME_BASE_0_1_MS;
        else if (cycleTimeDeciMs < 320)
            return ((cycleTimeDeciMs - 64) >> 2) | CYC_TIME_BASE_0_4_MS;
        else if (cycleTimeDeciMs <= 1328)
            return ((cycleTimeDeciMs - 320) >> 4) | CYC_TIME_BASE_1_6_MS;
        else
            return 0;
    }

    /**
     * Calculates CKT / CKS octet
     * 
     * Takes the XOR'ed octets of a message <b>excluding the CKT or CKS itself</b>
     * and returns the corresponding CKT / CKS octet.
     * 
     * \warning Do NOT include the seed value in the \c checksum8!
     * 
     * \param checksum8     Result of XORing all message octets, except CKT/CKS
     * \param ckt           Input CKT / CKS octet (only bits 6 and 7 are used)
     * \return              CKT / CKS octet with Checksum bits updated
     * 
     * \see IOL Interface Specs V1.1.1, A.1.6 
     */
    static uint8_t      calculateChecksum(uint8_t checksum8, uint8_t ckt)
    {
        // pre-calculated bits 0..6 of CKT for xor'ed input data (the table
        // automatically applies the seed value 0x52 to the data!)
        PROGMEM_ static const uint8_t checksum6[256] = 
        {
            0x2d, 0x3c, 0x0c, 0x1d, 0x3f, 0x2e, 0x1e, 0x0f, 0x0f, 0x1e, 0x2e, 0x3f, 0x1d, 0x0c, 0x3c, 0x2d,
            0x39, 0x28, 0x18, 0x09, 0x2b, 0x3a, 0x0a, 0x1b, 0x1b, 0x0a, 0x3a, 0x2b, 0x09, 0x18, 0x28, 0x39,
            0x09, 0x18, 0x28, 0x39, 0x1b, 0x0a, 0x3a, 0x2b, 0x2b, 0x3a, 0x0a, 0x1b, 0x39, 0x28, 0x18, 0x09,
            0x1d, 0x0c, 0x3c, 0x2d, 0x0f, 0x1e, 0x2e, 0x3f, 0x3f, 0x2e, 0x1e, 0x0f, 0x2d, 0x3c, 0x0c, 0x1d,
            0x35, 0x24, 0x14, 0x05, 0x27, 0x36, 0x06, 0x17, 0x17, 0x06, 0x36, 0x27, 0x05, 0x14, 0x24, 0x35,
            0x21, 0x30, 0x00, 0x11, 0x33, 0x22, 0x12, 0x03, 0x03, 0x12, 0x22, 0x33, 0x11, 0x00, 0x30, 0x21,
            0x11, 0x00, 0x30, 0x21, 0x03, 0x12, 0x22, 0x33, 0x33, 0x22, 0x12, 0x03, 0x21, 0x30, 0x00, 0x11,
            0x05, 0x14, 0x24, 0x35, 0x17, 0x06, 0x36, 0x27, 0x27, 0x36, 0x06, 0x17, 0x35, 0x24, 0x14, 0x05
        };

        // include CKT octet with Checksum bits (0..6) set to "0" (see A.1.6)
        ckt &= CKT_TYPE_MASK;
        checksum8 ^= ckt;

        // fold checksum8 at 0x80
        if (checksum8 >= 0x80)
        {
            checksum8 = ~checksum8;
        }

#ifdef NEVER
        // note: if PROGMEM is tight, further folding is possible, but doubles
        //       execution time from 1.64us to 3.28us @ 8Mhz
        if (checksum8 & 0x08)
        {
            // remove bit 3, invert bits 0-3
            checksum8 = ((checksum8 >> 1) & 0xf8) | ((~checksum8) & 0x07);
        }
        else
        {
            // remove bit 3
            checksum8 = ((checksum8 >> 1) & 0xf8) | (checksum8 & 0x07);
        }
#endif

        // calculate result checksum and combine with type bits (7..8) in original CKT
        ckt |=  pgm_read_byte(&checksum6[checksum8]);

        return ckt;
    }
};

/**
 * Encodes desired number of process input data into ProcessDataIn / 
 * ProcessDataOut direct parameter
 * 
 * \param PDBYTES Number of bytes in process data (0 .. 32)
 * \return Value for ProcessDataIn parameter, or 0 if PDBYTES out of valid range
 */
#define ENCODE_PD_BYTES(PDBYTES)                                            \
   ((PDBYTES) == 0   ? (IoLink::PDOUT_BITS) :                               \
    (PDBYTES) == 1   ? (IoLink::PDOUT_BITS | 8) :                           \
    (PDBYTES) == 2   ? (IoLink::PDOUT_BITS | 16) :                          \
    (PDBYTES) <= 32  ? (IoLink::PDOUT_BYTES_PLUS_1 | ((PDBYTES) - 1)) :     \
    0)

/**
 * Encode desired cycle time into MasterCycleTime or MinCycleTime parameter
 * 
 * The resulting parameter may not correspond exactly to the desired input
 * value. Use \c decodeCycleTime to obtain the exact value.
 * 
 * \param DECIMS    Cycle time in 1/10 of ms
 * \return          Parameter value, or 0 if parameter \c DECIMS out of valid range
 */
#define ENCODE_CYCLE_TIME(DECIMS)                                                   \
   ((DECIMS) < 4      ? 0 :                                                         \
    (DECIMS) < 64     ? ((DECIMS) | IoLink::CYC_TIME_BASE_0_1_MS) :                 \
    (DECIMS) < 320    ? ((((DECIMS) - 64) >> 2) | IoLink::CYC_TIME_BASE_0_4_MS) :   \
    (DECIMS) < 1328   ? ((((DECIMS) - 320) >> 4) | IoLink::CYC_TIME_BASE_1_6_MS) :  \
    0)

/**
 * Decode MasterCycleTime or MinCycleTime parameter into cycle time in 1/10 of ms
 * 
 * \param CYC   Parameter value
 * \return      Cycle time in 1/10 of ms, or 0 if parameter \c CYC out of valid range
 */
#define DECODE_CYCLE_TIME(CYC)                                                      \
   ((CYC) < 4         ? (uint16_t)0 :                                               \
    (CYC) < 64        ? (uint16_t)(CYC) :                                           \
    ((CYC) & IoLink::CYC_TIME_BASE_MASK) == IoLink::CYC_TIME_BASE_0_4_MS ?          \
        ((uint16_t)((CYC) & IoLink::CYC_MULTIPLIER_MASK) << 2) + 64 :               \
    ((CYC) & IoLink::CYC_TIME_BASE_MASK) == IoLink::CYC_TIME_BASE_1_6_MS ?          \
        ((uint16_t)((CYC) & IoLink::CYC_MULTIPLIER_MASK) << 4) + 320 :              \
    0)


#endif // iolink_h__