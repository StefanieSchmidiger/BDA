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
 *  \defgroup DDL_Onrequest_data_Handler
 *
 *  DL On-request data handler
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.5, p. 90
 *  \{
 */

DPL_IOL_DATA_SEG_START

/**
 *  \brief DL On-request data state
 *
 *  Internal variable.
 *  Can be:
 *    #DDL_OH_STATE_INACTIVE_0
 *    #DDL_OH_STATE_IDLE_1
 *
 */
DPL_IOL_DATA_SEG_PREFIX
StateT DDL_OH_State;
DPL_IOL_DATA_SEG_POSTFIX

DPL_IOL_DATA_SEG_END

/**
 *  \brief DL On-request data init
 *
 *  Internal function.
 *  It should be called from DEV_IOL_Init()
 *
 */
void DDL_OH_Init (void)
{
  DDL_OH_State = DDL_OH_STATE_INACTIVE_0;

  DDL_SH_State = DDL_SH_ISDU_IDLE_1;
  DDL_SH_ISDU.isduRequestReceivedToProcess = FALSE;
  DDL_SH_ISDU.BufStartRef = DDL_SH_ISDU.Buffer;

  DDL_EH_Init();
  
  DDL_CH_Go2Operate = FALSE;
}

/**
 *  \brief DL On-request data activator
 *
 *  Internal function.
 *  Activate or deactivate On-request data
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.5.3, p. 91
 *
 *  \param  isActive manage On-request data activation
 *             #TRUE - activate, #FALSE - deactivate
 *
 *  \return result of operation, can be:
 *      #SERVICE_RESULT_NO_ERROR
 *      #SERVICE_RESULT_STATE_CONFLICT
 *
 */
ResultT DDL_OH_Config (BooleanT isActive)
{
  if (DDL_OH_State == DDL_OH_STATE_INACTIVE_0 && isActive == TRUE)
  {
    DDL_OH_State = DDL_OH_STATE_IDLE_1;
    DDL_SH_State = DDL_SH_ISDU_IDLE_1;
    return SERVICE_RESULT_NO_ERROR;
  }
  else if (isActive == FALSE)
  {
    DDL_OH_Init();
    return SERVICE_RESULT_NO_ERROR;
  }
  return SERVICE_RESULT_STATE_CONFLICT;
}

/**
 *  \brief DL Message On-request data no service / empty OD_rsp
 *
 *  Internal function.
 *  Fills OD of Message in device message by zero.
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.5.1, A.5, p. 90, 205, 206
 *
 */
void DDL_OH_OD_rsp_noservice (void)
{
  UInteger8T odIndex;

  for (odIndex = 0; odIndex < DDL_MEH_Message.odLength; odIndex++)
  {
    DDL_MEH_Message.od[odIndex] = 0;
  }
}

/**
 *  \brief DL Message On-request data direct parameter handler
 *
 *  Internal function.
 *  Process direct parameter access
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.5.3, p. 91
 *
 */
void DDL_OH_DirectParameterHandler (EnumT rwDirection, UInteger8T address,
                                   UInteger8T length, UInteger8T * data)
{
  UInteger8T value = DDL_MEH_Message.od[0];

  if (address < 32)
  {
    if (DDL_MH_State == DDL_MH_STATE_STARTUP_2)
    {
      if (address < 16)
      {
        if (rwDirection == DDL_MEH_OD_RWDIR_READ)
        {
           if (DDL_Read(address, &value) == SERVICE_RESULT_NO_ERROR) 
                                                   DDL_MEH_Message.od[0] = value;
        }
        else
        {
          DDL_Write(address, value);
        }
      }
    }
    else
    {
      if (rwDirection == DDL_MEH_OD_RWDIR_READ)
      {
         if (DDL_ReadParam(address, &value) == SERVICE_RESULT_NO_ERROR) 
                                                   DDL_MEH_Message.od[0] = value;
      }
      else
      {
        DDL_WriteParam(address, value);
      }
    }
  }
}

/** \} */ // end of DDL_Onrequest_data_Handler

void DDL_MEH_OD_ind (EnumT rwDirection, EnumT comChannel, UInteger8T addressCtrl,
                    UInteger8T length, UInteger8T * data)
{
  
  if (DDL_OH_State == DDL_OH_STATE_INACTIVE_0) return;                          // OH is inactive

  if (comChannel != DDL_MEH_MESSAGE_COM_CH_ISDU &&
      rwDirection == DDL_MEH_OD_RWDIR_READ) 
    DDL_OH_OD_rsp_noservice();                                                  // init OD of Message in device 
                                                                                // message by zero
  if (DDL_OH_State == DDL_OH_STATE_IDLE_1)
  {
    DDL_EH_ReadConf();
    switch (comChannel)
    {
      case DDL_MEH_MESSAGE_COM_CH_PAGE:
        if (addressCtrl == 0)
        {                                                                       // check master command request
          if (rwDirection == DDL_MEH_OD_RWDIR_WRITE) 
                                                   DDL_MC_MasterCommand(data[0]);
          else return;
        }
        else
        {                                                                       // check parameter request
          DDL_OH_DirectParameterHandler(rwDirection, addressCtrl, length, data);
        }
        break;
        
      case DDL_MEH_MESSAGE_COM_CH_ISDU:                                          // check ISDU request
        if (DSM_DeviceCom.isISDUSupported != TRUE)                               // ISDU not supported
          return;
        else
          DDL_SH_OD_ind(rwDirection, addressCtrl, length, data);
        break;
        
      case DDL_MEH_MESSAGE_COM_CH_DIAGNOSIS:                                     // check Event request
        DDL_EH_OD_ind(rwDirection, addressCtrl);
        break;

      default:
        break;
    }
  }
}

/** \} */ // end of Data_Link_Layer
