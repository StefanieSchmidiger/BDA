//******************************************************************************
//
//     Copyright © 2011-2013 by IQ² Development GmbH, Neckartenzlingen 
//     All rights reserved
//
//******************************************************************************

#include "dev_iol.h"

/**
 *  \defgroup Application_layer
 *
 *  Application layer
 *  See: IO-Link spec v1.1.1 Oct 2011, 8, p. 102
 *  \{
 */

DPL_IOL_DATA_SEG_START

/**
 *  \brief Application layer state
 *
 *  Internal variable.
 *  Can be:
 *    #DAL_STATE_IDLE_0,
 *    #DAL_STATE_AWAIT_AL_WRITE_RSP_1,
 *    #DAL_STATE_AWAIT_AL_READ_RSP_2,
 *    #DAL_STATE_AWAIT_AL_RW_RSP_3
 *
 */
DPL_IOL_DATA_SEG_PREFIX 
StateT DAL_State; 
DPL_IOL_DATA_SEG_POSTFIX

DPL_IOL_DATA_SEG_END

/**
 *  \brief Application layer init
 *
 *  Internal function.
 *  It should be called from DEV_IOL_Init()
 */
void DAL_Init (void)
{
  DAL_State = DAL_STATE_IDLE_0;
}

/**
 *  \brief On-request data read response
 *
 *  This function should be called from device application
 *  to answer master read request.
 *  See: IO-Link spec v1.1.1 Oct 2011, 8.2.2.1, p.104
 *  
 *  \param length data length of response
 *
 */
void DAL_Read_rsp (UInteger8T length)
{
  if (DAL_State == DAL_STATE_AWAIT_AL_RW_RSP_3)
  {
    DDL_ISDUTransport_rsp(length, DDL_SH_ISDU_ERROR_INFO_NO_ERROR);
    DAL_State = DAL_STATE_IDLE_0;
  }
}

/**
 *  \brief On-request data read error response
 *
 *  This function should be called from device application,
 *  if an error has occured while processing master read request.
 *  See: IO-Link spec v1.1.1 Oct 2011, 8.2.2.1, p.104; Annex C, p.230
 *  
 *  \param errorCode               error code
 *  \param addErrorCode additional error code
 *  
 */
void DAL_Read_err (UInteger8T errorCode, UInteger8T addErrorCode)
{
  if (DAL_State == DAL_STATE_AWAIT_AL_RW_RSP_3)
  {
    DDL_SH_ISDU.errorCode = errorCode;
    DDL_SH_ISDU.addErrorCode = addErrorCode;
    DDL_ISDUTransport_rsp(0, DDL_SH_ISDU_ERROR_INFO_APPL_ERROR);
    DAL_State = DAL_STATE_IDLE_0;
  }
}

/**
 *  \brief On-request data write response
 *
 *  This function should be called from device application,
 *  when master service write request is complete.
 *  See: IO-Link spec v1.1.1 Oct 2011, 8.2.2.2, p.105
 *  
 */
void DAL_Write_rsp (void)
{
  if (DAL_State == DAL_STATE_AWAIT_AL_RW_RSP_3)
  {
    DDL_ISDUTransport_rsp(0, DDL_SH_ISDU_ERROR_INFO_NO_ERROR);
    DAL_State = DAL_STATE_IDLE_0;
  }
}

/**
 *  \brief On-request data write error response
 *
 *  This function should be called from device application,
 *  if an error has occured while processing master write request.
 *  See: IO-Link spec v1.1.1 Oct 2011, 8.2.2.2, p.105; Annex C, p.230
 *  
 *  \param errorCode               error code
 *  \param addErrorCode additional error code
 *  
 */
void DAL_Write_err (UInteger8T errorCode, UInteger8T addErrorCode)
{
  if (DAL_State == DAL_STATE_AWAIT_AL_RW_RSP_3)
  {
    DDL_SH_ISDU.errorCode = errorCode;
    DDL_SH_ISDU.addErrorCode = addErrorCode;
    DDL_ISDUTransport_rsp(0, DDL_SH_ISDU_ERROR_INFO_APPL_ERROR);
    DAL_State = DAL_STATE_IDLE_0;
  }
}

/**
 *  \brief Event request
 *
 *  This function should be called from device application,
 *  when several events have occured in device application
 *  and should be delivered to master.
 *  DAL_Event_cnf() will be called, when they all are delivered. 
 *  See: IO-Link spec v1.1.1 Oct 2011, 8.2.2.11, p.110
 *  
 *  \param eventCount size of events list
 *  \param events     events list
 *  
 */
void DAL_Event_req (UInteger8T eventCount, DDL_EH_EventT * events)
{
  UInteger8T eventIndex;

  for (eventIndex = 0; eventIndex < eventCount; eventIndex++)
  {
    DDL_Event_req(&(events[eventIndex]));
  }

  DDL_EventTriger_req();
}

/**
 *  \brief Set process data input validity
 *
 *  This function should be called from device application
 *  to set current process data input validity flag.
 *  See: IO-Link spec v1.1.1 Oct 2011, 8.2.2.12, p.111
 *  
 *  \param pdInInvalid this flag defines 
 *                     whether process data input is invalid or not
 *  
 */
void DAL_Control_req (BooleanT pdInInvalid)
{
  DDL_Control_req(pdInInvalid);
}

/**
 *  \brief Set process data input request
 *
 *  This function should be called from device application
 *  to set current process data input values.
 *  See: IO-Link spec v1.1.1 Oct 2011, 8.2.2.6, p.108
 *  
 */
UInteger8T * DAL_SetInput_req (void)
{
#ifdef DPL_BACK_COMPATIBLE_RevID_10
  if (DSM_DeviceCom.isInterleave == TRUE)
    return DDL_MEH_Message.pdInBuf;    
  else
#endif
    return DDL_MEH_Message.pdIn;
}

/**
 *  \brief Get process data output request
 *
 *  This function should be called from device application
 *  to obtain current process data output values.
 *  See: IO-Link spec v1.1.1 Oct 2011, 8.2.2.8, p.109
 *  
 */
UInteger8T * DAL_GetOutput_req (void)
{
#ifdef DPL_BACK_COMPATIBLE_RevID_10
  if (DSM_DeviceCom.isInterleave == TRUE)
    return DDL_MEH_Message.pdOutBuf;    
  else
#endif
    return DDL_MEH_Message.pdOut;
}

/** \} */ // end of Application_layer

void DDL_ISDUTransport_ind (UInteger16T  index, UInteger8T subIndex, 
                           EnumT direction, UInteger8T dataLength, 
                           UInteger8T * data)
{
  switch (direction)
  {
    case DDL_SH_ISDU_DIRECTION_NO_SERVICE:
      DDL_ISDUTransport_rsp(0, DDL_SH_ISDU_ERROR_INFO_APPL_ERROR);
      break;
      
    case DDL_SH_ISDU_DIRECTION_READ:
      DAL_State = DAL_STATE_AWAIT_AL_RW_RSP_3;
      DAL_Read_ind(index, subIndex, data);     
      break;

    case DDL_SH_ISDU_DIRECTION_WRITE:
      DAL_State = DAL_STATE_AWAIT_AL_RW_RSP_3;
      DAL_Write_ind(index, subIndex, dataLength, data);      
      break;
  }
}

void DDL_EventTriger_cnf (void)
{
  DAL_Event_cnf();
}

void DDL_Control_ind (BooleanT controlCode)
{
  DAL_Control_ind(controlCode);
}

void DDL_ISDUAbort (void)
{  
  if (DAL_State == DAL_STATE_AWAIT_AL_RW_RSP_3)
  {
    DAL_State = DAL_STATE_IDLE_0;                                               // abort ISDU operations
    DAL_Abort_ind();                                                            // inform Device application
  }
}

ResultT DDL_ReadParam (UInteger8T address, UInteger8T * value)
{
  if (DAL_State != DAL_STATE_IDLE_0)
    return SERVICE_RESULT_STATE_CONFLICT;

  if (address < 0x0F)
    return DDL_Read(address, value);

  DAL_Read_ind(0, address + 1, value);

  return SERVICE_RESULT_NO_ERROR;
}

ResultT DDL_WriteParam (UInteger8T address, UInteger8T value)
{
  if (DAL_State != DAL_STATE_IDLE_0)
    return SERVICE_RESULT_STATE_CONFLICT;

  if (address < 14)
    return DDL_Write(address, value);

  DAL_Write_ind(0, address + 1, 1, &value);

  return SERVICE_RESULT_NO_ERROR;
}

void DDL_PDOutputTransport_ind (BooleanT pdOutInvalid)
{
  DAL_PDCycle_ind();
  if (pdOutInvalid == FALSE)
    DAL_NewOutput_ind();
}
