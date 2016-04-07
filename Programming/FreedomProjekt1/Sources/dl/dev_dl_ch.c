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

DPL_IOL_DATA_SEG_START

/**
 *  \defgroup DDL_Command_Handler
 *
 *  DL command handler
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.7, p. 96
 *  \{
 */
DPL_IOL_DATA_SEG_PREFIX
BooleanT DDL_CH_Go2Operate;
DPL_IOL_DATA_SEG_POSTFIX

DPL_IOL_DATA_SEG_END

/**
 *  \brief DL Master command handler
 *
 *  Handle master command
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.2.5, p.77, 7.3.7, p. 96, B.1.2, p. 214
 *
 *  \return master_command master command, can be:
 *               DDL_CH_MC_FALLBACK
 *               DDL_CH_MC_MASTER_IDENT
 *               DDL_CH_MC_DEVICE_IDENT
 *               DDL_CH_MC_DEVICE_STARTUP
 *               DDL_CH_MC_PDOUT_OPERATE
 *               DDL_CH_MC_DEVICE_OPERATE
 *               DDL_CH_MC_DEVICE_PREOPERATE
 *
 */
void DDL_MC_MasterCommand (EnumT master_command)
{
  if (DDL_MH_State == DDL_MH_STATE_STARTUP_2 || 
      DDL_MH_State == DDL_MH_STATE_PREOPERATE_3)
  {                                                                                 
    if (master_command == DDL_CH_MC_DEVICE_PREOPERATE && 
        DDL_MH_State == DDL_MH_STATE_STARTUP_2)
    {                                                                           // preoperate
                                                                                // activate on-request data, service, 
                                                                                // and event handler      
      DDL_MEH_Config(TRUE);
      DDL_OH_Config(TRUE);
      DDL_Mode_ind(DDL_MODE_PREOPERATE);
      DDL_MH_State = DDL_MH_STATE_PREOPERATE_3;
    }
    
    else if (master_command == DDL_CH_MC_DEVICE_OPERATE)
    {                                                                           // operate
      
      DDL_MEH_PDOutInvalid = TRUE;                                               // set pd out invalid
      
      DDL_Control_ind(FALSE);                                                    // indicate to AL
                                                                                // activate process data handler. 
                                                                                // activate on-request data, service, 
                                                                                // and event handler.
      DDL_MEH_Config(TRUE);
      DDL_OH_Config(TRUE);
      DDL_Mode_ind(DDL_MODE_OPERATE);
      DDL_CH_Go2Operate = TRUE;
    }
    else
    {
      DDL_Write(0, master_command);                                              // transfer master command to SM
    }
  }

  if (DDL_MH_State == DDL_MH_STATE_PREOPERATE_3 || 
      DDL_MH_State == DDL_MH_STATE_OPERATE_4)
  { 
    if (master_command == DDL_CH_MC_FALLBACK)
    {                                                                           // fallback command
      DPL_StartTimer(DPL_TIME_TFBD_DELAY);                                        // wait TFBD
    }
    else if (master_command == DDL_CH_MC_DEVICE_STARTUP)
    {                                                                           // startup command
                                                                                // deactivate process data handler;                                                                                 
      //DDL_MEH_Config(FALSE);
      //DDL_OH_Config(FALSE);
      
      DDL_Mode_ind(DDL_MODE_STARTUP);

      DDL_MH_State = DDL_MH_STATE_STARTUP_2;
    }
  }
  if (DDL_MH_State == DDL_MH_STATE_OPERATE_4 && 
      master_command == DDL_CH_MC_PDOUT_OPERATE)
  {                                                                             // pd out valid command
    DDL_MEH_PDOutInvalid = FALSE;                                                // set pd out valid
    DDL_Control_ind(TRUE);                                                       // indicate to AL
  }
  if (DDL_MH_State == DDL_MH_STATE_OPERATE_4 && 
      master_command == DDL_CH_MC_DEVICE_OPERATE &&
      DDL_MEH_PDOutInvalid == FALSE)
  {                                                                             // pd out invalid command
    DDL_MEH_PDOutInvalid = TRUE;                                                 // set pd out invalid
    DDL_Control_ind(FALSE);                                                      // indicate to AL
  }
}

/** \} */ // end of DDL_Command_Handler

void DDL_MEH_EndOfSend_ind(void)
{
  if (DDL_CH_Go2Operate == TRUE)
  {
    DDL_CH_Go2Operate = FALSE;
    DDL_MH_State = DDL_MH_STATE_OPERATE_4;
  }
}

/** \} */ // end of Data_Link_Layer

