//------------------------------------------------------------------------------
//! \file
//! Defines the StackMultiByte class
//------------------------------------------------------------------------------
#ifdef STACK_MODE_MULTI_BYTE

#include "stackmultibyte.h"

// Declare the direct parameter and event pages as global fixed variables
uint8_t   directParameter[32];   //!< direct parameter page
uint8_t   eventPage[8];          //!< event buffer

StackMultiByte StackMultiByte::instance;

//------------------------------------------------------------------------------
template <IoLink::DeviceDLMode DDL_MODE>
StackMultiByte::HandlerResult StackMultiByte::handleOperatingMode(uint8_t mc)
{
    // get access mode, channel and address from MC octet
    const uint8_t access  = mc & IoLink::MC_RW_MASK;
    const uint8_t channel = mc & IoLink::MC_CHANNEL_MASK;
    const uint8_t address = mc & IoLink::MC_ADDRESS_MASK;
    const int8_t odCount = getOdOctetCount<DDL_MODE>();
    const uint8_t pdInSize = DDL_MODE == IoLink::DDL_MODE_OPERATE ? PD_IN_SIZE : 0;
    const uint8_t pdOutSize = DDL_MODE == IoLink::DDL_MODE_OPERATE ? PD_OUT_SIZE : 0;

    if (access == IoLink::MC_WRITE)
    {
        // get checksum/type octet
        uint8_t ckt = registerReadNext();
        if (!validateFrameType<DDL_MODE>(ckt))
            return ResultIllegalMessageType;

        // read process output data
        for (uint8_t i = 0; i < pdOutSize; ++i)
        {
            _processDataOut.buffer[i] = registerReadNext();
        }

        // read OD octet
        uint8_t od;
        if (odCount == 1)
        {
            od = registerReadLast();
        }
        else
        {
            od = registerReadNext();
            for (int8_t i = 1; i < odCount-1; ++i)
            {
                registerReadNext();
            }
            registerReadLast();
        }

        // start sending reply
        registerWriteBegin(REG_FR0);

        // store OD octet
        odWrite(channel, address, od);

        // send PDin data
        for (uint8_t i = 0; i < pdInSize; ++i)
        {
            registerWriteNext(_processDataIn.isValid ? _processDataIn.buffer[i] : 0);
        }

        // send the CKS octet, note the checksum field is set to 0
        registerWriteNext((pdInSize > 0 && _processDataIn.isValid) ? 
            IoLink::CKS_PD_VALID : IoLink::CKS_PD_INVALID);
        registerWriteDone();

        // trigger the transmission of the device sequence and update status
        uint8_t octetCount = pdInSize + 1;
        registerWrite(REG_LINK, (octetCount << LINK_CNT_SHIFT) | LINK_SND);

        // start user call-back timer after sending last byte 
        startCallbackTimerAfterReply(octetCount);
    }
    else // IoLink::MC_READ
    {
        // get checksum/type octet
        uint8_t ckt = (pdOutSize > 0) ? registerReadNext() : registerReadLast();

        if (!validateFrameType<DDL_MODE>(ckt))
            return ResultIllegalMessageType;

        // read process output data
        if (pdOutSize > 0)
        {
            for (uint8_t i = 0; i < pdOutSize-1; ++i)
            {
                _processDataOut.buffer[i] = registerReadNext();
            }
            _processDataOut.buffer[pdOutSize-1] = registerReadLast();
        }

        // start sending the return data
        registerWriteBegin(REG_FR0);

        // send OD data
        registerWriteNext(odRead(channel, address));
        for (int8_t i = 1; i < odCount; ++i)
        {
            registerWriteNext(0);
        }

        // send PDin data
        for (uint8_t i = 0; i < pdInSize; ++i)
        {
            registerWriteNext(_processDataIn.isValid ? _processDataIn.buffer[i] : 0);
        }

        // send the CKS octet, note the checksum field is set to 0
        registerWriteNext((pdInSize > 0 && _processDataIn.isValid) ? 
            IoLink::CKS_PD_VALID : IoLink::CKS_PD_INVALID);
        registerWriteDone();

        // trigger the transmission of the device sequence and update status
        uint8_t octetCount = pdInSize + odCount  + 1;
        registerWrite(REG_LINK, (octetCount << LINK_CNT_SHIFT) | LINK_SND);

        // start user call-back timer after sending last byte 
        startCallbackTimerAfterReply(octetCount);
    }

    if ((DDL_MODE == IoLink::DDL_MODE_STARTUP || DDL_MODE == IoLink::DDL_MODE_IDLE) 
        && _ddlMode != IoLink::DDL_MODE_STARTUP)
    {
        // if IDLE, enter STARTUP state
        _ddlMode = IoLink::DDL_MODE_STARTUP;
        setIoLinkListen();
    }

    return ResultSuccess;
}

//------------------------------------------------------------------------------
void StackMultiByte::onPhyInterrupt()
{
    // read the device status and start reading the frame register
    uint8_t status = registerReadBegin(REG_FR0);

    // test flags
    HandlerResult result;
    if (!(status & STATUS_RST))
    {
        result = ResultPhyReset;
    }
    else if (!(status & STATUS_DAT))
    {
        result = ResultNoData;
    }
    else if (status & STATUS_CHK)
    {
        result = ResultChecksumError;
    }
    else
    {
        // get master sequence control octet
        const uint8_t mc = registerReadNext();

        // call appropriate state handler
        switch (_ddlMode)
        {
        case IoLink::DDL_MODE_OPERATE:    
            result = handleOperatingMode<IoLink::DDL_MODE_OPERATE>(mc); 
            break;
        case IoLink::DDL_MODE_PREOPERATE:
            result = handleOperatingMode<IoLink::DDL_MODE_PREOPERATE>(mc);
            break;
        default:                  
            result = handleOperatingMode<IoLink::DDL_MODE_STARTUP>(mc); 
            break;
        }
    }

    // handle result codes
    switch (result)
    {
    case ResultSuccess:
        break;

    case ResultNoData:
        // terminate SPI access
        registerAbortAccess();
        break;

    case ResultChecksumError:
        // terminate SPI access
        registerAbortAccess();

        // bad data has been received, so decline to respond
        registerWrite(REG_LINK, LINK_END);
        break;

    case ResultIllegalMessageType:
        // terminate SPI access
        registerAbortAccess();

        // bad data has been received, so decline to respond
        registerWrite(REG_LINK, LINK_END);

        // revert to startup mode
        _ddlMode = IoLink::DDL_MODE_STARTUP;
        break;

    case ResultPhyReset:
        // terminate SPI access
        registerAbortAccess();

        // decline to respond
        registerWrite(REG_LINK, LINK_END);

        // reset the reset bit
        registerWrite(REG_STATUS, STATUS_RST);
        configurePhy(); // implicitly reverts to DDL_MODE_STARTUP
        break;

    default:
        break;
    }
}

//------------------------------------------------------------------------------
void StackMultiByte::startCallbackTimerAfterReply(uint8_t octetCnt)
{
    // start user call-back timer after sending last byte (each byte spaced
    // by 14 x T1)
    if (BAUD_RATE == 230400)
    {
        startCallbackTimer((octetCnt + 1) * 14 / 23 /* approx 230.4 / 10 */);
    }
    else // CFG_BD_38400
    {
        startCallbackTimer((octetCnt) * 14 / 4 /* approx. 38.4 / 10 */);
    }
}

/** \fn ISR(PCINT1_vect)
 * Interrupt service routine for the PHY interrupt.
 *
 * We immediately forward the call to onPhyInterrupt().
 */
ISR(PCINT1_vect)
{
    // run ISR handler until INT is cleared
    // 
    // (If INT isn't cleared after the handler, then the PHY state is probably
    //  out of sync with the one expected by the firmware. In that case, this
    //  ISR will never be called again. Looping the handler ensures we eventually
    //  resync before exiting the ISR)
    while (bit_is_set(PINC, PINC0))
    {
        StackMultiByte::instance.onPhyInterrupt();

        // clear interrupt
        PCIFR = _BV(PCIF1);
    }
}

// Interrupt on Timer0 compare match B
ISR(TIMER0_COMPB_vect)
{
    StackMultiByte::instance.onTimer0CompBInterrupt();
}

#endif // STACK_MODE_MULTI_BYTE
