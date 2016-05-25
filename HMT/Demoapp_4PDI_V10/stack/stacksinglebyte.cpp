//------------------------------------------------------------------------------
//! \file
//! Defines the StackSingleByte class
//------------------------------------------------------------------------------
#ifdef STACK_MODE_SINGLE_BYTE

#include "stacksinglebyte.h"
#include <string.h>

// Declare the direct parameter and event pages as global fixed variables
uint8_t   directParameter[32];   //!< direct parameter page
uint8_t   eventPage[8];          //!< event buffer

StackSingleByte StackSingleByte::instance;
//------------------------------------------------------------------------------
StackSingleByte::HandlerResult StackSingleByte::handleFirstOperatingModes()
{
    // 
    uint8_t status;

    //  state machine
    switch (_frameState)
    {
    case FRAME_RESET:
        return ResultPhyReset;
        
    case FRAME_RCV_MC:      // expecting MC octet
        {
            // request MC octet
            status = registerReadBegin(REG_FR0);
            if ((status & (STATUS_DAT | STATUS_CHK)) != STATUS_DAT)
                return ResultChecksumError;

            // get master sequence control (MC) octet
            _mc = registerReadLast();

            // ensure that we are in IoLinkListen from here on
            if (_ddlMode < IoLink::DDL_MODE_STARTUP)
                setIoLinkListen();

            // signal ready to receive next byte
            registerWrite(REG_LINK, LINK_END);

            // precalculate the checksum, assuming no returned PD or OD frames
            _cks = IoLink::calculateChecksum(0, (_processDataIn.isValid ? 
                                                 IoLink::CKS_PD_VALID : IoLink::CKS_PD_INVALID));

            // disable the cycle timer (Timer0): it will be restarted after this frame...
            _tccr0bRecord = TCCR0B;
            TCCR0B = 0;

            // ...and clear any outstanding timer interrupt requests
            TIFR0 |= _BV(OCF0B) | _BV(OCF0A) | _BV(TOV0);

            // enter next state
            _frameState = FRAME_RCV_CKT;
        }
        break;

    case FRAME_RCV_CKT: // expecting CKT octet in read access
        {
            // request CKT octet
            status = registerReadBegin(REG_FR0);
            if ((status & (STATUS_DAT | STATUS_CHK)) != STATUS_DAT)
                return ResultChecksumError;

            // get CKT octet
            _ckt = registerReadLast();

            // prepare checksum-8
            _checksum8 = _mc;
            _i = 0;

            // indicate continuation required
            _frameState     = FRAME_RCV_CKT_CONT;
            _frameTypeOk    = true;
            _receiveDdlMode = _ddlMode; 
            
            switch (_ddlMode)
            {
            case IoLink::DDL_MODE_PREOPERATE:
                if (validateFrameType<IoLink::DDL_MODE_PREOPERATE>(_ckt))
                    return ResultSuccess;
                break;

            case IoLink::DDL_MODE_OPERATE:
                if (validateFrameType<IoLink::DDL_MODE_OPERATE>(_ckt))
                    return ResultSuccess;
                break;
                
            default:
                 if (validateFrameType<IoLink::DDL_MODE_STARTUP>(_ckt))
                    return ResultSuccess;
                break;
            }
            
            _frameTypeOk    = false;
            
            // look for a viable alternative
            if (validateFrameType<IoLink::DDL_MODE_STARTUP>(_ckt))
            {
                _receiveDdlMode = IoLink::DDL_MODE_STARTUP;
            }
            else if (validateFrameType<IoLink::DDL_MODE_PREOPERATE>(_ckt))
            {
                _receiveDdlMode = IoLink::DDL_MODE_OPERATE;    
            }
            else if (validateFrameType<IoLink::DDL_MODE_OPERATE>(_ckt))
            {
                _receiveDdlMode = IoLink::DDL_MODE_OPERATE;    
            }
            else
            {
                _frameState     = FRAME_RCV_CKT;
                return ResultChecksumError;
            }
        }
        break;
        
    default:
        return ResultIllegalMessageType;
    }

    if (!(status & STATUS_RST))
        return ResultPhyReset;
    else
        return ResultSuccess;
}

//------------------------------------------------------------------------------
template <IoLink::DeviceDLMode DDL_MODE>
StackSingleByte::HandlerResult StackSingleByte::handleOperatingMode()
{
    // Durations measured for INT-high to INT-low for each state:
    // 
    // FRAME_RCV_MC:        24us
    // FRAME_RCV_CKT_READ:  27us
    // FRAME_RCV_CKT_WRITE: 24us
    // FRAME_RCV_OD:        29us
    // FRAME_SND_OD:        13us
    // FRAME_SND_CKS:       13us
    // FRAME_SND_DONE:      18us
    // 
    // It's possible to start SPI communication immediately upon entry of this
    // handler, but measured gain is only ~2us per state, and was not considered
    // worth the increased code complexity.
    // 
    uint8_t status = STATUS_RST;
    const int8_t odCount = getOdOctetCount<DDL_MODE>();
    const uint8_t pdInSize = DDL_MODE == IoLink::DDL_MODE_OPERATE ? PD_IN_SIZE : 0;
    const uint8_t pdOutSize = DDL_MODE == IoLink::DDL_MODE_OPERATE ? PD_OUT_SIZE : 0;

    //  state machine
    switch (_frameState)
    {
    case FRAME_RCV_CKT_CONT: // expecting CKT octet in read access
        {
            // determine next state
            if (pdOutSize > 0)
            {
                // signal ready to receive next byte
                registerWrite(REG_LINK, LINK_END);

                // expecting PDout data
                _frameState = FRAME_RCV_PD;               
            }
            else if (odCount > 0 && (_mc & IoLink::MC_RW_MASK) == IoLink::MC_WRITE)
            {
                // signal ready to receive next byte
                registerWrite(REG_LINK, LINK_END);

                // expecting OD data
                _frameState = FRAME_RCV_OD0;
            }
            else
            {
                // verify message checksum
                if (IoLink::calculateChecksum(_checksum8, _ckt) != _ckt)
                    return ResultChecksumError;

                if (!_frameTypeOk)
                    return ResultIllegalMessageType;

                // immediately enter next state
                // _frameState = FRAME_SND_OD;
                goto frame_snd_od;
            }
        }
        break;
        
    case FRAME_RCV_PD:
        {
            // request PD octet
            status = registerReadBegin(REG_FR0);
            if ((status & (STATUS_DAT | STATUS_CHK)) != STATUS_DAT)
                return ResultChecksumError;

            // get PD octet
            _checksum8 ^= _pdOut[_i] = registerReadLast();            
            
            if (++_i < pdOutSize) // get next PD octet
            {
                // signal ready to receive next byte
                registerWrite(REG_LINK, LINK_END);
            }
            else if (odCount > 0 // check for OD data
                && (_mc & IoLink::MC_RW_MASK) == IoLink::MC_WRITE)
            {
                // signal ready to receive next byte
                registerWrite(REG_LINK, LINK_END);

                // expecting OD data
                _frameState = FRAME_RCV_OD0;
                _i = 0;
            }
            else // done receiving
            {
                // verify message checksum
                if (IoLink::calculateChecksum(_checksum8, _ckt) != _ckt)
                    return ResultChecksumError;

                if (!_frameTypeOk)
                    return ResultIllegalMessageType;                    

                // commit process output data (use double-buffer and swap instead?)
                memcpy(_processDataOut.buffer, _pdOut, PD_OUT_SIZE);

                // immediately enter next state
                // _frameState = FRAME_SND_OD;
                _i = 0;
                goto frame_snd_od;
            }
        }
        break;
        
    case FRAME_RCV_OD0:  // expecting on-demand data in write access
        {
            // request OD octet
            status = registerReadBegin(REG_FR0);
            if ((status & (STATUS_DAT | STATUS_CHK)) != STATUS_DAT)
                return ResultChecksumError;

            // read OD octet
            _checksum8 ^= _odOut = registerReadLast();

            if (++_i < odCount) // get next OD octet
            {
                // signal ready to receive next byte
                registerWrite(REG_LINK, LINK_END);

                _frameState = FRAME_RCV_ODX;
            }
            else
            {
                // verify message checksum
                if (IoLink::calculateChecksum(_checksum8, _ckt) != _ckt)
                    return ResultChecksumError;

                if (!_frameTypeOk)
                    return ResultIllegalMessageType;
  
                // commit on-demand data
                const uint8_t channel = _mc & IoLink::MC_CHANNEL_MASK;
                const uint8_t address = _mc & IoLink::MC_ADDRESS_MASK;
                odWrite(channel, address, _odOut);

                // commit process output data (use double-buffer and swap instead?)
                memcpy(_processDataOut.buffer, _pdOut, PD_OUT_SIZE);

                // immediately enter next state
                // _frameState = FRAME_SND_OD;
                _i = 0;
                _checksum8 = 0;
                goto frame_snd_pd;
            }
        }
        break;

    case FRAME_RCV_ODX:  // expecting on-demand data in write access
        {
            // request OD octet
            status = registerReadBegin(REG_FR0);
            if ((status & (STATUS_DAT | STATUS_CHK)) != STATUS_DAT)
                return ResultChecksumError;

            // read OD octet
            _checksum8 ^= registerReadLast();

            if (++_i < odCount) // get next PD octet
            {
                // signal ready to receive next byte
                registerWrite(REG_LINK, LINK_END);
            }
            else
            {
                // verify message checksum
                if (IoLink::calculateChecksum(_checksum8, _ckt) != _ckt)
                    return ResultChecksumError;

                if (!_frameTypeOk)
                    return ResultIllegalMessageType;                    
                 
                // commit on-demand data
                const uint8_t channel = _mc & IoLink::MC_CHANNEL_MASK;
                const uint8_t address = _mc & IoLink::MC_ADDRESS_MASK;
                odWrite(channel, address, _odOut);

                // commit process output data (use double-buffer and swap instead?)
                memcpy(_processDataOut.buffer, _pdOut, PD_OUT_SIZE);

                // immediately enter next state
                // _frameState = FRAME_SND_OD;
                _i = 0;
                _checksum8 = 0;
                goto frame_snd_pd;
            }
        }
        break;

    case FRAME_SND_OD0:  // sending on-demand octet
frame_snd_od:
        // prepare checksum
        _checksum8 = 0;

        if (odCount > 0)
        {
            // get access mode, channel and address from MC octet
            const uint8_t channel = _mc & IoLink::MC_CHANNEL_MASK;
            const uint8_t address = _mc & IoLink::MC_ADDRESS_MASK;

            // get the data
            uint8_t odIn = odRead(channel, address);

            // send output on-demand data
            status = registerWrite(REG_FR0, odIn);

            // update checksum-8
            _checksum8 ^= odIn;

            // pre-calculate CKS
            _cks = IoLink::calculateChecksum(_checksum8, ((pdInSize > 0 && _processDataIn.isValid) ? 
                                                 IoLink::CKS_PD_VALID : IoLink::CKS_PD_INVALID));

            // enter next state
            if (++_i < odCount)
            {
                _frameState = FRAME_SND_ODX;
            }
            else
            {
                _i = 0;
                _frameState = FRAME_SND_PD;
            }
            break;
        }
        // FALLTHROUGH => FRAME_SND_ODX
        //lint -fallthrough

    case FRAME_SND_ODX:  // sending on-demand octet
        if (odCount > 1)
        {
            // send output on-demand data
            status = registerWrite(REG_FR0, 0);

            // enter next state
            if (++_i == odCount)
            {
                _i = 0;
                _frameState = FRAME_SND_PD;
            }
            break;
        }
        // FALLTHROUGH => FRAME_SND_PD
        //lint -fallthrough

    case FRAME_SND_PD:      // send process input data (if applicable to frame type)
frame_snd_pd:
        // with frame type 2, send the process data
        if (pdInSize > 0)
        {
            // get process data
            const uint8_t pd = _processDataIn.isValid ? _processDataIn.buffer[_i] : 0;

            // send process data
            status = registerWrite(REG_FR0, pd);

            // update checksum-8
            _checksum8 ^= pd;

            // pre-calculate CKS
            _cks = IoLink::calculateChecksum(_checksum8, ((pdInSize > 0 && _processDataIn.isValid) ? 
                                                 IoLink::CKS_PD_VALID : IoLink::CKS_PD_INVALID));

            if (++_i == pdInSize)
            {
                // enter next state
                _frameState = FRAME_SND_CKS;
            }
            else
            {
                _frameState = FRAME_SND_PD;
            };
            break;
        }

        // FALLTHROUGH => FRAME_SND_CKS
        //lint -fallthrough

    case FRAME_SND_CKS:     // CKS octet
        {
            // calculate and send CKS octet
            status = registerWrite(REG_FR0, _cks);

            // wait for completion
            _frameState = FRAME_SND_DONE;
        }
        break;

    case FRAME_SND_DONE:    // finish transmission
        {
            // signal ready to receive next byte
            status = registerWrite(REG_LINK, LINK_END);

            // reset state machine
            _frameState = FRAME_RCV_MC;

            // if we have successfully completed an exchange, ensure we are in startup
            if (DDL_MODE == IoLink::DDL_MODE_STARTUP) 
                _ddlMode = IoLink::DDL_MODE_STARTUP;
              
            // start user call-back timer
            startCallbackTimer();
        }
        break;

    case FRAME_RESET:
        return ResultPhyReset;

    default:
        return ResultIllegalMessageType;
    }

    if (!(status & STATUS_RST))
        return ResultPhyReset;
    else
        return ResultSuccess;
}

//------------------------------------------------------------------------------
void StackSingleByte::onPhyInterrupt()
{
    HandlerResult result;
    if (_frameState <= FRAME_RCV_CKT)
    {
        result = handleFirstOperatingModes();
    }
    
    if (_frameState > FRAME_RCV_CKT)
    {
        // call appropriate state handler
        switch (_receiveDdlMode)
        {
        case IoLink::DDL_MODE_OPERATE:    
            result = handleOperatingMode<IoLink::DDL_MODE_OPERATE>(); 
            break;
        case IoLink::DDL_MODE_PREOPERATE:
            result = handleOperatingMode<IoLink::DDL_MODE_PREOPERATE>();
            break;
        default:                  
            result = handleOperatingMode<IoLink::DDL_MODE_STARTUP>(); 
            break;
        }
    }

    // handle result codes
    switch (result)
    {
    case ResultChecksumError:
        {
            // abort current register access
            registerAbortAccess();

            // bad data has been received, so decline to respond
            uint8_t status = registerWrite(REG_LINK, LINK_END);
        
            if ((status & STATUS_SSC) && (_ddlMode == IoLink::DDL_MODE_IDLE))
            {
                // setIoLinkListen();
                
                // a possible WURQ has been received, go quiet
                startCallbackTimer(127);
            }

            // reset state
           _frameState = FRAME_RCV_MC;
           
           restartTimer();
        }
        
        break;

    case ResultIllegalMessageType:
        // bad data has been received, so decline to respond
        registerWrite(REG_LINK, LINK_END);

        // reset state
        _frameState = FRAME_RCV_MC;

        // revert to startup mode
        if (_ddlMode>IoLink::DDL_MODE_STARTUP)
            _ddlMode = IoLink::DDL_MODE_STARTUP;
        
        // restart the timer
        restartTimer();
        break;

    case ResultPhyReset:
        // reset the reset bit
        registerWrite(REG_STATUS, STATUS_RST);
        configurePhy();

        // reset state
        _frameState = FRAME_RCV_MC;
        
        // restart the timer
        restartTimer();
        break;

    default:
        break;
    }

    // When sending, the UART transmission delay t2 (see A.3.4) must be less
    // than 3*Tbit (= 13us @ 230'400baud). We can't meet this timing constraint
    // if we wait for the next interrupt, so we loop until the last octet has
    // been sent, and immediately re-enter the ISR.
    switch (_frameState)
    {
    case FRAME_SND_ODX:
    case FRAME_SND_PD:
    case FRAME_SND_CKS:
        // wait until byte sent (can be removed if MPU frequency 20Mhz)
        loop_until_bit_is_set(PINC, PINC0);
        break;
    default:
        break;
    }
}

//------------------------------------------------------------------------------
void StackSingleByte::startCallbackTimerAfterReply()
{
    startCallbackTimer();
}


/** \fn ISR(PCINT1_vect)

Interrupt service routine for the PHY interrupt.

We immediately forward the call to onPhyInterrupt().
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
        StackSingleByte::instance.onPhyInterrupt();

        // clear interrupt flag
        PCIFR = _BV(PCIF1);
    }
}

// Interrupt on Timer0 compare match B
ISR(TIMER0_COMPB_vect)
{
    StackSingleByte::instance.onTimer0CompBInterrupt();
}

#endif // STACK_MODE_SINGLE_BYTE
