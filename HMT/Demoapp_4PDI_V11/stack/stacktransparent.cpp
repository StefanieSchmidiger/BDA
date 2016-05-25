//------------------------------------------------------------------------------
//! \file
//! Defines the StackTransparent class
//------------------------------------------------------------------------------
#ifdef STACK_MODE_TRANSPARENT

#include "stacktransparent.h"
#include <avr/power.h>
#include <util/parity.h>
#include <util/delay.h>
#include <string.h>

// Declare the direct parameter and event pages as global fixed variables
uint8_t   directParameter[32];   //!< direct parameter page
uint8_t   eventPage[8];          //!< event buffer

StackTransparent StackTransparent::instance;
bool TransparentModeSsHandler::_reading = true;

//------------------------------------------------------------------------------
void StackTransparent::configureStack()
{
    // ensure TX line is in idle state (high) initially
    PORTD |= _BV(PORTD1); // PB3/MOSI

    // configure TXD (PD1) as an output
    DDRD |= _BV(DDD1);
    
    // configure RXD (PD0) as an input
    DDRD &= ~_BV(DDD0);

    // disable UART RX/TX for now
    UCSR0B &= ~(_BV(RXCIE0) | _BV(RXEN0));

    // enable clock to USART
    power_usart0_enable();

    // set baud rate
    UBRR0 = (F_CPU / 16 / BAUD_RATE) - 1;

    // set frame format (8-even parity-1 stop bit)
    UCSR0C = _BV(UPM01) | _BV(UCSZ01) | _BV(UCSZ00);

    // switch to SIO mode
    uint8_t status = setSioListen();
    handleStatus(status);
}

uint8_t StackTransparent::setSioActive()
{
    static bool wasDriven = false;
    
    bool    txd, driven;
    uint8_t hslsBits;
    switch(SIO_DRIVE_MODE)
    {
      case DRIVE_MODE_PUSH_PULL:
        hslsBits = Phy::CTL_HS | Phy::CTL_LS;
        txd      = !_sioLevel;
        driven   = true;
        break;
        
      case DRIVE_MODE_PNP:
        hslsBits = Phy::CTL_HS;
        txd      = !_sioLevel;
        driven   = _sioLevel;
        break;   
        
      case DRIVE_MODE_NPN:
        hslsBits = Phy::CTL_LS;
        txd      = _sioLevel;
        driven   = _sioLevel;
        break; 
        
      case DRIVE_MODE_INACTIVE:
        hslsBits = 0;
        txd      = true;
        driven   = false;
        break;         
    }

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
 
     // set the data level
    if (txd)
    {
      PORTD |= _BV(PORTD1);
    }
    else
    {
      PORTD &= ~_BV(PORTD1);
    }

    // enable the receiver appropriately
    if (_hiZCounter == -1)
    {
      if (!(UCSR0B & _BV(RXEN0))) uartRxEnable();
    }
    else
    {
      UCSR0B &= ~(_BV(RXCIE0) | _BV(RXEN0));
    }

    return registerWrite(Phy::REG_CTL, 
        StackT::PHY_CTL_SCT | StackT::PHY_CTL_MODE | Phy::CTL_SIO_MODE | hslsBits);
}

//------------------------------------------------------------------------------
uint8_t StackTransparent::setIoLinkListen()
{
  // disable the line driver
  uint8_t status = registerWrite(Phy::REG_CTL, StackT::PHY_CTL_SCT | StackT::PHY_CTL_MODE | Phy::CTL_IOLINK_MODE);

  // set the idle mode of the TX line (uartTx disabled, PHY drivers enabled)
  PORTD |= _BV(PORTD1);

  // enable the receiver
  uartRxEnable();

  return status;
}

//------------------------------------------------------------------------------
uint8_t StackTransparent::setSioListen()
{
  // disable the line driver
  uint8_t status = registerWrite(Phy::REG_CTL, StackT::PHY_CTL_SCT | StackT::PHY_CTL_MODE | Phy::CTL_SIO_MODE);

  // set the idle mode of the TX line (uartTx disabled, PHY drivers enabled)
  PORTD |= _BV(PORTD1);

  // enable the receiver
  uartRxEnable();

  return status;
}

//------------------------------------------------------------------------------
void StackTransparent::onWakeUp()
{
    // read status register
    uint8_t status = registerReadStatus();
    handleStatus(status);
}

//------------------------------------------------------------------------------
void StackTransparent::uartRxEnable()
{
    // wait until byte transferred
    loop_until_bit_is_set(UCSR0A, UDRE0);

    // clear any outstanding interrupt requests
    TIFR0 |= _BV(OCF0A) | _BV(TOV0);

    // enable UART RX
    UCSR0B |= _BV(RXCIE0) | _BV(RXEN0);

    // clear error flags
    UCSR0A;
}

//------------------------------------------------------------------------------
inline void StackTransparent::onRxDone()
{
    // check error flags
    _rxError = (UCSR0A & (_BV(FE0) | _BV(DOR0) | _BV(UPE0)));
 
    if (!_rxError)
    {
        // we expect the next octet within less than 12 * Tbit, otherwise discard
        // the frame => start a timer with 13 * Tbit, which calls onRxTimeout()
        TCCR0B = 0;
        TCNT0 = 0;
        TIFR0 |= _BV(OCF0B) | _BV(OCF0A) | _BV(TOV0);  // clear any outstanding interrupt requests
        TIMSK0 = _BV(OCIE0A); // Timer/Counter0 Output Compare Match A Interrupt Enable    
        TCCR0A = _BV(WGM01); // clear Timer on Compare Match A
        TCCR0B = _BV(CS01) | _BV(CS00); // pre-scaler clkIO/64
        OCR0A = F_CPU / BAUD_RATE * 13 / 64;
    }

    // update state machine
    onUpdateDlStateMachine();
}

//------------------------------------------------------------------------------
void StackTransparent::onRxTimeout()
{
    // stop frame time-out timer
    TCCR0B = 0;

    // switch to SIO-Listen mode and wait for data
    _frameState = FRAME_RCV_MC;

    // await input data
    uartRxEnable();

    // start user call-back timer, unless we are being deliberately quiet
    if (_ddlMode != IoLink::DDL_MODE_ESTABLISH_COM)
    {
        startCallbackTimer();
    }
}

//------------------------------------------------------------------------------
void StackTransparent::uartTx(uint8_t ch)
{
    // wait until byte transferred
    loop_until_bit_is_set(UCSR0A, UDRE0);

    // switch to write mode
    if (_ssHndlr.isReading())
    {
        // disable frame octet timeout timer
        TCCR0B = 0;

        // switch SS handler to writing
        _ssHndlr.setReading(false);

        // switch PHY to transmit (push-pull) mode
        uint8_t status = registerWrite(REG_CTL, PHY_CTL_SCT | PHY_CTL_MODE | CTL_HS | CTL_LS);
        
        // if short-circuit bit set, clear bit and retry
        if (status & STATUS_SSC)
        {
            registerWrite(REG_CTL, PHY_CTL_SCT | PHY_CTL_MODE | CTL_IOLINK_MODE);
            registerWrite(REG_CTL, PHY_CTL_SCT | PHY_CTL_MODE | CTL_HS | CTL_LS);
        }
    }

    // disable USART receiver
    UCSR0B &= ~(_BV(RXCIE0) | _BV(RXEN0));

    // enable USART transmitter
    UCSR0B |= _BV(TXEN0) | _BV(TXCIE0);

    // put data into transmit buffer, sends the data
    UDR0 = ch;
}

//------------------------------------------------------------------------------
inline void StackTransparent::onTxDone()
{
    // disable USART transmitter
    UCSR0B &= ~(_BV(TXEN0) | _BV(TXCIE0));

    // re-enable USART receiver
    UCSR0B |= (_BV(RXEN0) | _BV(RXCIE0));

    // update state machine
    onUpdateDlStateMachine();
}

//------------------------------------------------------------------------------
StackTransparent::HandlerResult StackTransparent::handleFirstOperatingModes()
{
    // always read rx data
    uint8_t rxData = UDR0;

    switch (_frameState)
    {
    case FRAME_RCV_MC:      // expecting MC octet
        {
            // request MC octet
            if (_rxError)
                return ResultChecksumError;

            // get master sequence control (MC) octet
            _mc = rxData;

            // expecting data
            _ssHndlr.setReading();

            // don't change state if receiving non-data MC octet
            if (!validateControlOctet(rxData))
                return ResultNoData;

            // enter next state
            _frameState = FRAME_RCV_CKT;
            _ssHndlr.setReading(true);
        }
        break;

    case FRAME_RCV_CKT: // expecting CKT octet in read access
        {
            // request CKT octet
            if (_rxError)
                return ResultChecksumError;

            // get CKT octet
            _ckt = rxData;

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

    return ResultSuccess;
}

//------------------------------------------------------------------------------
template <IoLink::DeviceDLMode DDL_MODE>
StackTransparent::HandlerResult StackTransparent::handleOperatingMode()
{
    const int8_t odCount = getOdOctetCount<DDL_MODE>();
    const uint8_t pdInSize = DDL_MODE == IoLink::DDL_MODE_OPERATE ? PD_IN_SIZE : 0;
    const uint8_t pdOutSize = DDL_MODE == IoLink::DDL_MODE_OPERATE ? PD_OUT_SIZE : 0;

    // always read rx data
    uint8_t rxData = UDR0;

    //  state machine
    switch (_frameState)
    {
    case FRAME_RCV_CKT_CONT: // expecting CKT octet in read access
        {            
            // determine next state
            if (pdOutSize > 0)
            {
                // expecting PDout data
                _frameState = FRAME_RCV_PD;               
            }
            else if (odCount > 0 && (_mc & IoLink::MC_RW_MASK) == IoLink::MC_WRITE)
            {
                // expecting OD data
                _frameState = FRAME_RCV_OD0;
            }
            else
            {
                // verify message checksum, priority over illegal message type
                if (IoLink::calculateChecksum(_checksum8, _ckt) != _ckt)
                    return ResultChecksumError;

                // immediately enter next state
                // _frameState = FRAME_SND_OD;
                goto frame_snd_od;
            }
        }
        break;

    case FRAME_RCV_PD:
        {
            // request PD octet
            if (_rxError)
                return ResultChecksumError;

            // get PD octet
            _checksum8 ^= _pdOut[_i] = rxData;            

            if (++_i >= pdOutSize)
            {
                if (odCount > 0 // check for OD data
                    && (_mc & IoLink::MC_RW_MASK) == IoLink::MC_WRITE)
                {
                    // expecting OD data
                    _frameState = FRAME_RCV_OD0;
                    _i = 0;
                }
                else // done receiving
                {
                    // verify message checksum
                    if (IoLink::calculateChecksum(_checksum8, _ckt) != _ckt)
                        return ResultChecksumError;

                    // commit process output data (use double-buffer and swap instead?)
                    memcpy(_processDataOut.buffer, _pdOut, PD_OUT_SIZE);

                    // immediately enter next state
                    // _frameState = FRAME_SND_OD;
                    _i = 0;
                    goto frame_snd_od;
                }
            }
        }
        break;

    case FRAME_RCV_OD0:  // expecting on-demand data in write access
        {
            // request OD octet
            if (_rxError)
                return ResultChecksumError;

            // read OD octet
            _checksum8 ^= _odOut = rxData;

            if (++_i < odCount) // get next OD octet
            {
                _frameState = FRAME_RCV_ODX;
            }
            else
            {
                // verify message checksum
                if (IoLink::calculateChecksum(_checksum8, _ckt) != _ckt)
                    return ResultChecksumError;

                // commit on-demand data
                const uint8_t channel = _mc & IoLink::MC_CHANNEL_MASK;
                const uint8_t address = _mc & IoLink::MC_ADDRESS_MASK;
                odWrite(channel, address, _odOut);

                // commit process output data (use double-buffer and swap instead?)
                memcpy(_processDataOut.buffer, _pdOut, PD_OUT_SIZE);

                // immediately enter next state
                _frameState = FRAME_SND_PD;
                _i = 0;
                _checksum8 = 0;
                goto frame_snd_pd;
            }
        }
        break;

    case FRAME_RCV_ODX:  // expecting on-demand data in write access
        {
            // request OD octet
            if (_rxError)
                return ResultChecksumError;

            // read OD octet
            _checksum8 ^= rxData;

            if (++_i >= odCount)
            {
                // verify message checksum
                if (IoLink::calculateChecksum(_checksum8, _ckt) != _ckt)
                    return ResultChecksumError;

                // commit on-demand data
                const uint8_t channel = _mc & IoLink::MC_CHANNEL_MASK;
                const uint8_t address = _mc & IoLink::MC_ADDRESS_MASK;
                odWrite(channel, address, _odOut);

                // commit process output data (use double-buffer and swap instead?)
                memcpy(_processDataOut.buffer, _pdOut, PD_OUT_SIZE);

                // immediately enter next state
                _frameState = FRAME_SND_PD;
                _i = 0;
                _checksum8 = 0;
                goto frame_snd_pd;
            }
        }
        break;

    case FRAME_SND_OD0:  // sending on-demand octet
frame_snd_od:
        // check the sequence type conforms to the mode
        if (!_frameTypeOk)
            return ResultIllegalMessageType; 
            
        // good data received, switch to IO-Link mode
        if (DDL_MODE <= IoLink::DDL_MODE_STARTUP && _ddlMode != IoLink::DDL_MODE_STARTUP)
        {
            // if IDLE, enter STARTUP state
            _ddlMode = IoLink::DDL_MODE_STARTUP;
        }
        
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
            uartTx(odIn);

            // update checksum-8
            _checksum8 ^= odIn;

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
            uartTx(0);

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
        // check the sequence type conforms to the mode
        if (!_frameTypeOk)
            return ResultIllegalMessageType;
            
        // good data received, switch to IO-Link mode
        if (DDL_MODE <= IoLink::DDL_MODE_STARTUP && _ddlMode != IoLink::DDL_MODE_STARTUP)
        {
            // if IDLE, enter STARTUP state
            _ddlMode = IoLink::DDL_MODE_STARTUP;
        }

        // with frame type 2, send the process data
        if (pdInSize > 0)
        {
            // get process data
            const uint8_t pd = _processDataIn.isValid ? _processDataIn.buffer[_i] : 0;

            // send process data
            uartTx(pd);

            // update checksum-8
            _checksum8 ^= pd;

            if (++_i == pdInSize)
            {
                // enter next state
                _frameState = FRAME_SND_CKS;
            }
            break;
        }

        // FALLTHROUGH => FRAME_SND_CKS
        //lint -fallthrough

    case FRAME_SND_CKS:     // CKS octet
        {
            // save current CKS status flags
            uint8_t cks = _processDataIn.isValid ? IoLink::CKS_PD_VALID : IoLink::CKS_PD_INVALID;

            // calculate and send CKS octet
            uartTx(IoLink::calculateChecksum(_checksum8, cks));

            // wait for completion
            _frameState = FRAME_SND_DONE;
        }
        break;

    case FRAME_SND_DONE:    // finish transmission
        {
            // reset state machine
            _frameState = FRAME_RCV_MC;

            // start user call-back timer
            startCallbackTimer();

            // synchronize stack mode
            handleStatus(setIoLinkListen());
        }
        break;

    default:
        return ResultIllegalMessageType;
    }

    return ResultSuccess;
}

//------------------------------------------------------------------------------
inline void StackTransparent::onUpdateDlStateMachine()
{
    HandlerResult result;
    if (_frameState <= FRAME_RCV_CKT)
    {
        result = handleFirstOperatingModes();
    }
        
    // call appropriate state handler, _frameState is not set to FRAME_RCV_CONT
    // with a bad result from handleFirstOperatingModes
    if (_frameState >= FRAME_RCV_CKT_CONT)
    {
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
    if (result == ResultChecksumError || result == ResultIllegalMessageType)
    {
        // bad data has been received, so decline to respond
        uartRxEnable();

        // reset state
        _frameState = FRAME_RCV_MC;

    if ((result == ResultIllegalMessageType) && (_ddlMode > IoLink::DDL_MODE_STARTUP))
        {
            // revert to startup mode
            _ddlMode = IoLink::DDL_MODE_STARTUP;
        }
    }
}

//------------------------------------------------------------------------------
void StackTransparent::handleStatus(uint8_t status)
{
    // check the reset bit and configure PHY if necessary
    if (!(status & STATUS_RST))
    {
        // reset the reset bit
        registerWrite(REG_STATUS, STATUS_RST);
        configurePhy();
    }

    if (status & STATUS_DAT)
    {
        // PHY in wait mode
        registerWrite(REG_LINK, LINK_END);
    }

    // handle the short-circuit bit
    if ((status & STATUS_SSC) && (_ddlMode == IoLink::DDL_MODE_IDLE))
    {   
        // short-circuit detected => switch to SioListen mode and wait for data
        // leaving the PHY short-circuit timer running
        setSioListen();
        
        // disable frame octet timeout timer
        TCCR0B = 0;

        _ddlMode = IoLink::DDL_MODE_ESTABLISH_COM;
        _frameState = FRAME_RCV_MC;
    }
    else if ((status & STATUS_SSC) && (_ddlMode != IoLink::DDL_MODE_IDLE))
    {
        // disable the transmitter
        UCSR0B &= ~(_BV(TXEN0) | _BV(TXCIE0));
        
        // short-circuit detected during transmission, break-off transmission
        setIoLinkListen();

        startCallbackTimer();
        _frameState = FRAME_RCV_MC;
    }
    else if (!(status & STATUS_SSC) && (_ddlMode == IoLink::DDL_MODE_ESTABLISH_COM))
    {
        // drop back to SIO operation
        _ddlMode = IoLink::DDL_MODE_IDLE;
        _frameState = FRAME_RCV_MC;
        startCallbackTimer();
    }
}

/** \fn ISR(PCINT1_vect)
 * Interrupt service routine for the PHY interrupt.
 *
 *We immediately forward the call to onPhyInterrupt().
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
        StackTransparent::instance.onWakeUp();

        // clear interrupt flag
        PCIFR = _BV(PCIF1);
    }
}

// Interrupt on Timer0 compare match A
// => Used by frame octet receive time-out
ISR(TIMER0_COMPA_vect)
{
    StackTransparent::instance.onRxTimeout();
}

// Interrupt on Timer0 compare match B
// => Used to trigger user-code callback
ISR(TIMER0_COMPB_vect)
{
    StackTransparent::instance.onTimer0CompBInterrupt();
}

// USART Tx Complete
ISR(USART_TX_vect)
{
    StackTransparent::instance.onTxDone();
}

// USART Rx Complete 
ISR(USART_RX_vect)
{
    StackTransparent::instance.onRxDone();
}

#endif // STACK_MODE_TRANSPARENT

