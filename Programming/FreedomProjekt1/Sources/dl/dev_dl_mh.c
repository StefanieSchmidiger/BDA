//******************************************************************************
//
//     Copyright © 2011-2013 by IQ² Development GmbH, Neckartenzlingen 
//     All rights reserved
//
//******************************************************************************

#include "dev_iol.h"

/**
 *  \defgroup Data_Link_Layer
 *
 *  Data link layer protocol
 *  See: IO-Link spec v1.1.1 Oct 2011, 7, p. 53
 *  \{
 */

/**
 *  \defgroup DDL_Mode_Handler
 *
 *  DL-mode handler
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.2, p. 71
 *  \{
 */

DPL_IOL_DATA_SEG_START
  
/**
 *  \brief DL Mode Handler state
 *
 *  Internal variable.
 *  Can be:
 *    #DDL_MH_STATE_IDLE_0,
 *    #DDL_MH_STABLISH_COMM_1,
 *    #DDL_MH_STARTUP_2,
 *    #DDL_MH_PREOPERATE_3,
 *    #DDL_MH_OPERATE_4
 *
 */
DPL_IOL_DATA_SEG_PREFIX
StateT DDL_MH_State;
DPL_IOL_DATA_SEG_POSTFIX

DPL_IOL_DATA_SEG_END

/**
 *  \brief DL Mode Handler init
 *
 *  Internal function.
 *  It should be called from DEV_IOL_Init()
 */
void DDL_MH_Init (void)
{
  DDL_MH_State = DDL_MH_STATE_IDLE_0;
}

/**
 *  \brief DL Mode Handler Timer callback
 *
 *  Internal function.
 *  This function checks mode handler timeouts.
 */
void DDL_MH_TimerCallback (DPL_DelayTypeT delayType)
{
  
  if (delayType == DPL_TIME_TDSIO_DELAY &&
      DDL_MH_State == DDL_MH_STATE_ESTABLISH_COMM_1)
  {                                                                             // TDSIO elapsed
    DDL_MH_State = DDL_MH_STATE_IDLE_0;
    DDL_MEH_Config(FALSE);                                                       // deactivate message handler
    DDL_Mode_ind(DDL_MODE_INACTIVE);
  }

  if ((delayType == DPL_TIME_TFBD_DELAY) && 
      (DDL_MH_State == DDL_MH_STATE_PREOPERATE_3 || 
       DDL_MH_State == DDL_MH_STATE_OPERATE_4))
  {                                                                             // TFBD elapsed, go to SIO (fallback)
    DDL_MEH_Config(FALSE);                                                       // deactivate all handlers
    DDL_OH_Config(FALSE);
    DDL_MH_State = DDL_MH_STATE_IDLE_0;
    DDL_Mode_ind(DDL_MODE_INACTIVE);
  }
}

/**
 *  \brief DL Mode Handler first byte received indication
 *
 *  Internal function.
 *  By this function Message handler and On-request data handler
 *  indicate, that first byte received from master.
 *
 */
void DDL_MH_FirstByteReceived_ind (void)
{
  DPL_StopTimer(DPL_TIME_TDSIO_DELAY);                                          // correct message received, stop TDSIO timer
  DDL_Mode_ind(DSM_DeviceCom.supportedBaudrate);
  DDL_OH_Config(TRUE);
  DDL_MH_State = DDL_MH_STATE_STARTUP_2;
}

/** \} */ // end of DDL_Mode_Handler

/** \} */ // end of Data_Link_Layer

void DPL_Wakeup_ind (void)
{
  if (DDL_MH_State == DDL_MH_STATE_IDLE_0)
  {
    DDL_MH_State = DDL_MH_STATE_ESTABLISH_COMM_1;
    DDL_MEH_Config(TRUE);                                                        // activate message handler
    DDL_Mode_ind(DDL_MODE_ESTABCOM);                                              // startup ind. to SM
    DPL_StartTimer(DPL_TIME_TDSIO_DELAY);                                         // wait TDSIO for the first master message
  }
}

void DDL_MEH_MHInfo_IllegalMessageType(void)
{
  if (DDL_MH_State == DDL_MH_STATE_PREOPERATE_3 || 
      DDL_MH_State == DDL_MH_STATE_OPERATE_4)
  {   
    //DL_MEH_Config(FALSE);                                                     // deactivate on-request data, service, and event handler
    //DL_OH_Config(FALSE);
    DPL_StopTimer(DPL_TIME_TFBD_DELAY);
    DDL_Mode_ind(DDL_MODE_STARTUP);
    DDL_MH_State = DDL_MH_STATE_STARTUP_2;
  }
}
