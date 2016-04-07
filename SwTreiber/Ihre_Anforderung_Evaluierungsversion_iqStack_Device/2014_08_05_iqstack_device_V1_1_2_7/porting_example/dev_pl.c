//******************************************************************************
//
//     Copyright � 2011-2013 by IQ� Development GmbH, Neckartenzlingen 
//     All rights reserved
//
//******************************************************************************

#include "dev_iol.h"

/**
 *  \defgroup Physical_Layer
 *
 *  Physical layer of the specific hardware
 *  See: IO-Link spec v1.1.1 Oct 2011, 5, p. 39
 *  \{
 */

DPL_IOL_DATA_SEG_START

/**
 *  \brief Physical layer mode
 *
 *  Internal variable.
 *  Can be:
 *      #DPL_MODE_INACTIVE,
 *      #DPL_MODE_DO,
 *      #DPL_MODE_DI,
 *      #DPL_MODE_COM1,
 *      #DPL_MODE_COM2,
 *      #DPL_MODE_COM3
 *
 */
DPL_IOL_DATA_SEG_PREFIX
UInteger8T DPL_Mode;
DPL_IOL_DATA_SEG_POSTFIX

/**
 *  \brief PL TDSIO software timer
 *
 *  Internal variable.
 *
 */
DPL_IOL_DATA_SEG_PREFIX
UInteger8T DPL_TDSIO_Timer;
DPL_IOL_DATA_SEG_POSTFIX

/**
 *  \brief PL TFBD software timer
 *
 *  Internal variable.
 *
 */
DPL_IOL_DATA_SEG_PREFIX
UInteger8T DPL_TFBD_Timer;
DPL_IOL_DATA_SEG_POSTFIX

/**
 *  \brief PL TFBD software timer
 *
 *  Internal variable.
 *
 */
DPL_IOL_DATA_SEG_PREFIX
UInteger8T DPL_CycMonitor_Timer;
DPL_IOL_DATA_SEG_POSTFIX

DPL_IOL_DATA_SEG_END

/**
 *  \brief PL init
 *  
 *  Internal function.
 *  This function initializes Device Physical layer:
 *  IO-Link UART communication, timers and etc.
 *  It is called from DEV_IOL_Init().
 *
 */
void DPL_Init(void)
{
  
}

/**
 *  \brief PL mode switching request
 *
 *  This function switches IO-Link physical layer mode
 *  of the specific hardware of device or master.
 *  It should be called from the device or master system management.
 *  See: IO-Link spec v1.1.1 Oct 2011, 5.2.2.1, p.41
 *
 *  The Device has no inactive state. 
 *  By default at power on or cable reconnection, 
 *  the Device shall operate in the SIO mode, as a digital input. 
 *  The Device shall always be able to detect a wake-up current pulse (wake-up request). 
 *  The service DPL_WakeUp_ind reports successful detection of the wake-up request 
 *  (usually a microcontroller interrupt), 
 *  which is required for the Device to switch to the SDCI mode.
 *
 *  \param targetMode specifies the requested operation mode
 * 
 */
void DPL_Mode_req (DPL_TargetModeT targetMode)
{ 
  DPL_Mode = targetMode;
  
  DPL_TDSIO_Timer = 0;
  DPL_TFBD_Timer = 0;
  
  switch (targetMode)
  {
    case DPL_MODE_INACTIVE: 
      
      break;
      
    case DPL_MODE_DO:    
      
      break;
      
    case DPL_MODE_DI:
      
      break;
      
    case DPL_MODE_COM1:
      
      break;
      
    case DPL_MODE_COM2:
      
      break;
      
    case DPL_MODE_COM3:
      
      break;
      
    case DPL_MODE_PREOPERATE:
      
      break;
      
    case DPL_MODE_OPERATE:
      
      break;
  }
}

// When wake-up request is detected by the device hardware,
// DPL_Wakeup_ind() should be called.

/**
 *  \brief PL start the specific hardware timer
 *
 *  Internal function.
 *  This function starts the specific hardware timer
 *  in milliseconds or bit times, depending on communication baudrate.
 *  It should be defined for the specific platform.
 *  If the timer has been already started, it should be stopped at first.
 *  When the timer, started by this function, is elapsed,
 *  DPL_TimerCallback() should be called.
 *
 *      WARNING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *      TFBD Timer (DPL_TIME_TFBD_DELAY) should NOT DEPEND on
 *      DPL_MASTER_MESSAGE_BYTE_RECEIVE_TIMEOUT, 
 *      DPL_DEVICE_RESPONSE_DELAY,
 *      DPL_MAX_CYCLE_TIME timers !!!!!!!!!!!!!!!!!!!!!!!!!!!
 *      It can be, for example, another psysical or software timer. 
 *      
 *  \param delayType specifies delay to start corresponding timer
 *          
 */
void DPL_StartTimer (DPL_DelayTypeT delayType)
{
  switch (delayType)
  {
    // Standard IO delay in ms
    // After TDSIO the Device falls back to SIO mode (if supported)
    // See: IO-Link spec v1.1.1 Oct 2011, 7.3.2.2, Table 40, p. 73
    case DPL_TIME_TDSIO_DELAY:
      // software timer tick 10ms
      // DPL_TDSIO_Timer = 6 + 1; // 60 ms
      break;
    
    // Device specific master message received bytes timeout in bits
    // Timeout between received bytes of master message.
    // Used for observation of the message transmission time.
    case DPL_MASTER_MESSAGE_BYTE_RECEIVE_TIMEOUT:
      // stop timer
      // start timer 11 bit times: COM3 - 48탎, COM2 - 286탎, COM1 - 2292탎      
      break;
    
#ifdef DPL_USE_DEVICE_RESPONSE_DELAY
    // Device specific response delay in bits
    // Delay between master message request received moment
    // and moment of device response message sending start.
    // #DPL_USE_DEVICE_RESPONSE_DELAY switches on/off this timer
    // If it is off, device responds immediately, after answer is ready,
    // else it responds after end of master message plus this delay
    // if response is ready or later when the response will be ready.
    case DPL_DEVICE_RESPONSE_DELAY:
      // can be not started
      // stop timer
      // start timer 3 - 11 bit times
      break;
#endif
      
    // Device specific maximum cycle time
    // Can be not used.
    // See: IO-Link spec v1.1.1 Oct 2011, 7.3.3.5, p. 87 and 10.2, p. 152
    case DPL_MAX_CYCLE_TIME:
      // can be not started
      // stop timer
      // start timer
      // DPL_CycMonitor_Timer = 5 + 1; // 50 ms
      break;
      
    // Fallback delay in ms
    // After a time TFBD the Device shall be switched to SIO mode.
    // See: IO-Link spec v1.1.1 Oct 2011, 7.3.2.3, Table 41, p. 74
    case DPL_TIME_TFBD_DELAY:
      // software timer tick 10ms
      // DPL_TFBD_Timer = 35 + 1; // 350 ms
      break;
      
    default:
      break;
  }
}

/**
 *  \brief PL stop the specific hardware timer
 *
 *  Internal function.
 *  This function stops the specific hardware timer,
 *  started by DPL_StartTimer();
 *
 *  \param delayType specifies delay to stop corresponding timer
 *          
 */
void DPL_StopTimer (DPL_DelayTypeT delayType)
{
  switch (delayType)
  {
    case DPL_TIME_TDSIO_DELAY:
      DPL_TDSIO_Timer = 0;
      break;
    
    case DPL_MASTER_MESSAGE_BYTE_RECEIVE_TIMEOUT:
      break;
    
#ifdef DPL_USE_DEVICE_RESPONSE_DELAY
    case DPL_DEVICE_RESPONSE_DELAY:
      break;
#endif
      
    case DPL_MAX_CYCLE_TIME:
      DPL_CycMonitor_Timer = 0;
      break;
      
    case DPL_TIME_TFBD_DELAY:
      DPL_TFBD_Timer = 0;
      break;
      
    default:
      break;
  }
}

/**
 *  \brief PL the specific hardware timer callback
 *
 *  Internal function.
 *  This function should be called by the specific hardware,
 *  when the timer, started by the DPL_StartTimer() function, is elapsed.
 *
 *  \param delayType specifies delay of corresponding timer callback
 *
 */
void DPL_TimerCallback (DPL_DelayTypeT delayType)
{       
  DDL_MH_TimerCallback(delayType);
  DDL_MEH_TimerCallback(delayType);
}

// should be called by software timer 
// after each defined tick, for example, 10 ms
void DPL_SWTimerCallback (void)
{
  // TDSIO
  if (DPL_TDSIO_Timer > 1) 
    DPL_TDSIO_Timer--;
  else if ((DPL_TDSIO_Timer == 1))
  {
    DPL_TDSIO_Timer--;
    DPL_TimerCallback(DPL_TIME_TDSIO_DELAY);
  }
  
  // TFBD
  if (DPL_TFBD_Timer > 1) 
    DPL_TFBD_Timer--;
  else if ((DPL_TFBD_Timer == 1))
  {
    DPL_TFBD_Timer--;
    DPL_TimerCallback(DPL_TIME_TFBD_DELAY);
  }
      
  // cycle monitor
  if (DPL_CycMonitor_Timer > 1) 
    DPL_CycMonitor_Timer--;
  else if ((DPL_CycMonitor_Timer == 1))
  {
    DPL_CycMonitor_Timer--;
    DPL_TimerCallback(DPL_MAX_CYCLE_TIME);
  }
}

/**
 *  \brief PL byte buffer transfer request
 *
 *  This function sends data bytes from buffer by IO-Link physical layer.
 *  It is called by data link layer.
 *  It should be defined for the specific hardware.
 *  See: IO-Link spec v1.1.1 Oct 2011, 5.2.2.3, p.42
 *
 *  \param data   is byte buffer to send
 *  \param length is a size of data buffer
 *
 */
void DPL_TransferBuffer_req (UInteger8T length, UInteger8T * buffer)
{
  
}

/** \} */ // end of Physical_Layer
