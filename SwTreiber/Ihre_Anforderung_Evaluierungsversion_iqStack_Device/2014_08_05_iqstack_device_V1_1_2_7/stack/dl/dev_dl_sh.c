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
 *  \defgroup DDL_Service_Handler
 *
 *  DL service handler
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.6, p. 92
 *  \{
 */

DPL_IOL_DATA_SEG_START

/**
 *  \brief DL Service handler state
 *
 *  Internal variable.
 *  Can be:
 *    #DDL_SH_ISDU_IDLE_1
 *    #DDL_SH_ISDU_REQUEST_2
 *    #DDL_SH_ISDU_WAIT_3
 *    #DDL_SH_ISDU_RESPONSE_4
 *
 */
DPL_IOL_DATA_SEG_PREFIX
StateT DDL_SH_State;
DPL_IOL_DATA_SEG_POSTFIX

/**
 *  \brief DL Service handler ISDU
 *
 *  Internal variable.
 *
 */
DPL_IOL_DATA_SEG_PREFIX
DDL_SH_ISDU_T DDL_SH_ISDU;
DPL_IOL_DATA_SEG_POSTFIX

DPL_IOL_DATA_SEG_END

/**
 *  \brief DL Service handler add OD to ISDU buffer
 *
 *  Internal function.
 *  Adds current master message message OD data to ISDU buffer
 *
 */
void DDL_SH_PutData2Buf (UInteger8T length, UInteger8T * data)
{
  UInteger8T odIndex;

  if ((DDL_MEH_Message.addressISDUFC & 0x0F) == DDL_SH_ISDU.flowControl)
  {
    for (odIndex = 0; odIndex < DDL_MEH_Message.odLength; odIndex++)
    {
      DDL_SH_ISDU.BufStartRef[DDL_SH_ISDU.BufferLength] = DDL_MEH_Message.od[odIndex];
      DDL_SH_ISDU.BufferLength++;
    }
            
    if (DDL_SH_ISDU.flowControl >= 15)
      DDL_SH_ISDU.flowControl = 0;
    else
      DDL_SH_ISDU.flowControl++;
  }
}

/**
 *  \brief DL Service handler fill OD from ISDU buffer
 *
 *  Internal function.
 *  Fills device message message OD from ISDU buffer
 *  and empty this part of ISDU buffer
 *
 */
void DDL_SH_OD_rsp (void)
{
  UInteger8T odIndex;

  if ((DDL_MEH_Message.addressISDUFC & 0x0F) == DDL_SH_ISDU.flowControl)
  {        
    if (DDL_SH_ISDU.flowControl >= 15)
      DDL_SH_ISDU.flowControl = 0;
    else
      DDL_SH_ISDU.flowControl++;
  }
  else
    DDL_SH_ISDU.BufferIndex -= DDL_MEH_Message.odLength;
  
  for (odIndex = 0; odIndex < DDL_MEH_Message.odLength; odIndex++)
  {
    if (DDL_SH_ISDU.BufferIndex < DDL_SH_ISDU.BufferLength)
      DDL_MEH_Message.od[odIndex] = DDL_SH_ISDU.BufStartRef[DDL_SH_ISDU.BufferIndex];      
    else
      DDL_MEH_Message.od[odIndex] = 0;
    DDL_SH_ISDU.BufferIndex++;
  }
}

/**
 *  \brief DL Service handler OD response busy
 *
 *  Internal function.
 *  Set OD response busy
 *  See: IO-Link spec v1.1.1 Oct 2011, A.5.3, p. 206
 *
 */
void DDL_SH_OD_rsp_busy (void)
{
  DDL_MEH_Message.od[0] = DDL_SH_ISDU_SERVICE_BYTE_DEVICE_BUSY;
}

/**
 *  \brief DL Service handler main cycle
 *
 *  Internal function.
 *  Process current ISDU request
 *  in main cycle of device application.
 *
 */
void DDL_SH_MainCycle(void)
{
  if (DDL_SH_ISDU.isduRequestReceivedToProcess == TRUE)
  {                                                                             // check that there is an ISDU request to process by device application
    DDL_SH_ISDU.isduRequestReceivedToProcess = FALSE;
    DDL_SH_ParseISDUReq();                                                       // process current ISDU request
    DDL_ISDUTransport_ind(DDL_SH_ISDU.index, DDL_SH_ISDU.subIndex,
                         DDL_SH_ISDU.direction, DDL_SH_ISDU.dataLength, 
                         DDL_SH_ISDU.data);
  }
}

/**
 *  \brief DL Service handler OD
 *
 *  Internal function.
 *  DL Service OD handler
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.6.4, p. 96
 *
 */
void DDL_SH_OD_ind (EnumT rwDirection, UInteger8T addressCtrl,
                   UInteger8T length, UInteger8T * data)
{
  if (DDL_SH_State != DDL_SH_ISDU_RESPONSE_4 &&
      rwDirection == DDL_MEH_OD_RWDIR_READ)
    DDL_OH_OD_rsp_noservice();                                                  // init OD of Message in device
  
  if (addressCtrl == DDL_SH_FCTRL_START)
  {
    if (rwDirection == DDL_MEH_OD_RWDIR_WRITE)
    {                                                                           // ISDU Start (request)
      DDL_SH_ISDU.BufferLength = 0;                                             // reset ISDU buffer
      DDL_SH_ISDU.BufStartRef = DDL_SH_ISDU.Buffer;
      DDL_SH_ISDU.flowControl = 0;
      DDL_SH_PutData2Buf(length, data);
      DDL_SH_ISDU.isduRequestReceivedToProcess = FALSE;

      // Some master can send start request during response 
      // without sending idle or abort message, as end of response
      // according to specification.
      // Stack restarts ISDU state machine because of it,
      // in spite of specification says to send "no service" in this case.
      
      DDL_SH_State = DDL_SH_ISDU_REQUEST_2;

      DDL_ISDUAbort();

      return;
      /*
      switch (DDL_SH_State)
      {
        case DDL_SH_ISDU_IDLE_1:
            DDL_SH_State = DDL_SH_ISDU_REQUEST_2;
            return;

        case DDL_SH_ISDU_REQUEST_2:
          return;

        case DDL_SH_ISDU_WAIT_3:
          return;

        case DDL_SH_ISDU_RESPONSE_4:
          return;

        default:                                                                // no service
          return;
      }
      */
    }
    
    else
    {                                                                           // ISDU Request Complete / 
                                                                                // Read (start response)
      switch (DDL_SH_State)
      {
        //case DDL_SH_ISDU_IDLE_1:
        //    return;

        case DDL_SH_ISDU_REQUEST_2:                                              // ISDU Request Complete (start response)
          DDL_SH_State = DDL_SH_ISDU_WAIT_3;
          DDL_SH_ISDU.flowControl = 0;
          DDL_SH_OD_rsp_busy();
          DDL_SH_ISDU.isduRequestReceivedToProcess = TRUE;
          return;

        case DDL_SH_ISDU_WAIT_3:
          DDL_SH_OD_rsp_busy();
          return;

        case DDL_SH_ISDU_RESPONSE_4:
          DDL_SH_OD_rsp();
          return;

        default:
          return;
      }
    }
  }

  if (addressCtrl < DDL_SH_FCTRL_START)
  {
    if (rwDirection == DDL_MEH_OD_RWDIR_WRITE)
    {                                                                           // ISDU Write (request)
      if (DDL_SH_State == DDL_SH_ISDU_REQUEST_2)
      {
        DDL_SH_PutData2Buf(length, data);
      }
      return;
      /*
      switch (DDL_SH_State)
      {
        case DDL_SH_ISDU_IDLE_1:
            return;

        case DDL_SH_ISDU_REQUEST_2:
          DDL_SH_PutData2Buf(length, data);
          return;

        case DDL_SH_ISDU_WAIT_3:
          return;

        case DDL_SH_ISDU_RESPONSE_4:
          return;

        default:
          return;
      }
      */
    }    
    else
    {                                                                           // ISDU Read (response)
      switch (DDL_SH_State)
      {
        /*
        case DDL_SH_ISDU_IDLE_1:
            return;

        case DDL_SH_ISDU_REQUEST_2:
          return;
        */

        case DDL_SH_ISDU_WAIT_3:
          DDL_SH_OD_rsp_busy();
          return;

        case DDL_SH_ISDU_RESPONSE_4:
          DDL_SH_OD_rsp();
          return;

        default:
          return;
      }
    }
  }

  if (addressCtrl == DDL_SH_FCTRL_IDLE_1 || addressCtrl == DDL_SH_FCTRL_IDLE_2)
  {
    if (rwDirection == DDL_MEH_OD_RWDIR_READ)
    {                                                                           // ISDU Send Complete (stop response)
      if (DDL_SH_State == DDL_SH_ISDU_RESPONSE_4)
      {
        DDL_SH_ISDU.BufferLength = 0;                                          // reset ISDU buffer
        DDL_SH_State = DDL_SH_ISDU_IDLE_1;
      }
      return;
      /*
      switch (DDL_SH_State)
      {
        case DDL_SH_ISDU_IDLE_1:
          return;

        case DDL_SH_ISDU_REQUEST_2:
          return;

        case DDL_SH_ISDU_WAIT_3:
          return;

        case DDL_SH_ISDU_RESPONSE_4:
          DDL_SH_ISDU.BufferLength = 0;                                          // reset ISDU buffer
          DDL_SH_State = DDL_SH_ISDU_IDLE_1;
          return;

        default:
          return;
      }
      */
    }
  }
  
  if (addressCtrl == DDL_SH_FCTRL_ABORT)
  {
    DDL_SH_State = DDL_SH_ISDU_IDLE_1;
    DDL_ISDUAbort();
    /*
    switch (DDL_SH_State)
    {
      case DDL_SH_ISDU_IDLE_1:
        return;

      case DDL_SH_ISDU_REQUEST_2:
        return;

      case DDL_SH_ISDU_WAIT_3:
        DDL_ISDUAbort();
        return;

      case DDL_SH_ISDU_RESPONSE_4:
        DDL_ISDUAbort();
        return;

      default:
        return;
    }
    */
  }
}

/**
 *  \brief DL Service handler ISDU checksum
 *
 *  Internal function.
 *  Calculate ISDU checksum
 *  See: IO-Link spec v1.1.1 Oct 2011, A.5.6, p. 207
 *
 */
UInteger8T DDL_SH_CalcISDUChecksum (UInteger8T dataLength, UInteger8T * data)
{
  UInteger8T dataIndex;
  UInteger8T checksum;

  if (dataLength <= 0) return 0;

  checksum = data[0];
  for (dataIndex = 1; dataIndex < dataLength; dataIndex++)
  {
    checksum ^= data[dataIndex];
  }

  return checksum;
}

/**
 *  \brief DL Service handler ISDU request parser
 *
 *  Internal function.
 *  Parse ISDU request from DDL_SH_ISDU.BufStartRef to DDL_SH_ISDU
 *  See: IO-Link spec v1.1.1 Oct 2011, A.5, p. 205
 *
 */
void DDL_SH_ParseISDUReq (void)
{
  UInteger8T serviceByte = DDL_SH_ISDU.BufStartRef[0];
  UInteger8T indexIndex = 1;
  UInteger8T subIndexIndex = 2;
  UInteger8T dataIndex = 2;
  UInteger8T isduLength = 3;
  UInteger8T cks = 0;

  DDL_SH_ISDU.serviceMaster = serviceByte & 0xF0;
  DDL_SH_ISDU.length        = serviceByte & 0x0F;

  DDL_SH_ISDU.isExtLength = DDL_SH_ISDU.length == 1 ? TRUE : FALSE;

  if (DDL_SH_ISDU.isExtLength == TRUE)
  {
    DDL_SH_ISDU.length = DDL_SH_ISDU.BufStartRef[1];
    indexIndex++;
    subIndexIndex++;
    dataIndex++;
    isduLength++;
  }
  
  switch (DDL_SH_ISDU.serviceMaster)
  {                                                                             // R/W direction
    case DDL_SH_ISDU_SERVICE_MASTER_WRITE_REQ_8BI:
    case DDL_SH_ISDU_SERVICE_MASTER_WRITE_REQ_8BIS:
    case DDL_SH_ISDU_SERVICE_MASTER_WRITE_REQ_16BIS:
      DDL_SH_ISDU.direction = DDL_SH_ISDU_DIRECTION_WRITE;
      break;
    case DDL_SH_ISDU_SERVICE_MASTER_READ_REQ_8BI:
    case DDL_SH_ISDU_SERVICE_MASTER_READ_REQ_8BIS:
    case DDL_SH_ISDU_SERVICE_MASTER_READ_REQ_16BIS:
      DDL_SH_ISDU.direction = DDL_SH_ISDU_DIRECTION_READ;
      break;
    default:                                                                    // no service
      DDL_SH_ISDU.direction = DDL_SH_ISDU_DIRECTION_NO_SERVICE;
      return;
  }
  
  switch (DDL_SH_ISDU.serviceMaster)
  {                                                                             // index / subindex
    case DDL_SH_ISDU_SERVICE_MASTER_WRITE_REQ_8BI:
    case DDL_SH_ISDU_SERVICE_MASTER_READ_REQ_8BI:
      DDL_SH_ISDU.index = DDL_SH_ISDU.BufStartRef[indexIndex];
      DDL_SH_ISDU.subIndex = 0;
      break;
    case DDL_SH_ISDU_SERVICE_MASTER_WRITE_REQ_8BIS:
    case DDL_SH_ISDU_SERVICE_MASTER_READ_REQ_8BIS:
      dataIndex++;
      isduLength++;
      DDL_SH_ISDU.index = DDL_SH_ISDU.BufStartRef[indexIndex];
      DDL_SH_ISDU.subIndex = DDL_SH_ISDU.BufStartRef[subIndexIndex];
      break;
    case DDL_SH_ISDU_SERVICE_MASTER_WRITE_REQ_16BIS:
    case DDL_SH_ISDU_SERVICE_MASTER_READ_REQ_16BIS:
      subIndexIndex++;
      dataIndex++;
      dataIndex++;
      isduLength++;
      isduLength++;
      DDL_SH_ISDU.index = ((((UInteger16T)DDL_SH_ISDU.BufStartRef[indexIndex ]) << 8)
                                                                    & 0xFF00) + // HI
                          (((UInteger16T)DDL_SH_ISDU.BufStartRef[indexIndex + 1])       
                                                                      & 0x00FF);// LO
      DDL_SH_ISDU.subIndex = DDL_SH_ISDU.BufStartRef[subIndexIndex];
      break;
    default:                                                                    // no service
      DDL_SH_ISDU.direction = DDL_SH_ISDU_DIRECTION_NO_SERVICE;
      return;
  }

  if (DDL_SH_ISDU.direction == DDL_SH_ISDU_DIRECTION_WRITE)                     // get ISDU data
    DDL_SH_ISDU.data = DDL_SH_ISDU.BufStartRef + dataIndex;                     
  else
    DDL_SH_ISDU.data = &(DDL_SH_ISDU.BufStartRef[2]);
  DDL_SH_ISDU.dataLength = DDL_SH_ISDU.length - 1 - dataIndex;

  isduLength += DDL_SH_ISDU.dataLength;

  if (isduLength != DDL_SH_ISDU.length)
  {                                                                             // check ISDU length
    DDL_SH_ISDU.direction = DDL_SH_ISDU_DIRECTION_NO_SERVICE;                   // wrong ISDU length, answer no service
    return;
  }

  cks = DDL_SH_CalcISDUChecksum(DDL_SH_ISDU.length, DDL_SH_ISDU.BufStartRef);   // checksum
  if (cks != 0)
  {                                                                             // wrong checksum, answer no service
    DDL_SH_ISDU.direction = DDL_SH_ISDU_DIRECTION_NO_SERVICE;
    return;
  }
}

/**
 *  \brief DL Service handler pack ISDU response
 *
 *  Internal function.
 *  Pack ISDU response from DDL_SH_ISDU to DDL_SH_ISDU.BufStartRef
 *  See: IO-Link spec v1.1.1 Oct 2011, A.5, p. 205
 *
 */
void DDL_SH_PackISDURsp (void)
{
  //UInteger8T bufIndex;
  UInteger8T dataIndex = 1;

  if (DDL_SH_ISDU.isError == FALSE)    
  {                                                                             // normal response
    //if (DDL_SH_ISDU.direction == DDL_SH_ISDU_DIRECTION_WRITE)
    //  DDL_SH_ISDU.dataLength = 0;
  
    DDL_SH_ISDU.length = DDL_SH_ISDU.dataLength + 2;
    if (DDL_SH_ISDU.length < 16)
    {
      DDL_SH_ISDU.BufStartRef = &(DDL_SH_ISDU.BufStartRef[1]);
      DDL_SH_ISDU.BufStartRef[0] = DDL_SH_ISDU.serviceDevice + DDL_SH_ISDU.length;
    }
    else
    {                                                                           // ext length    
      DDL_SH_ISDU.BufStartRef = &(DDL_SH_ISDU.BufStartRef[0]);
      DDL_SH_ISDU.length++;
      dataIndex++;
      DDL_SH_ISDU.BufStartRef[0] = DDL_SH_ISDU.serviceDevice + 1;
      DDL_SH_ISDU.BufStartRef[1] = DDL_SH_ISDU.length;
    }
    
    /* Must be copied by user !!! see DDL_SH_GetISDUReadRspBuf()
    for (bufIndex = 0; bufIndex < DDL_SH_ISDU.dataLength; bufIndex++)
    {                                                                           // copy ISDU response data to ISDU buffer
      DDL_SH_ISDU.BufStartRef[dataIndex + bufIndex] = DDL_SH_ISDU.data[bufIndex];
    }
    */

    DDL_SH_ISDU.BufferLength = dataIndex + DDL_SH_ISDU.dataLength + 1;
  }
  else
  {
    if (DDL_SH_ISDU.serviceDevice == DDL_SH_ISDU_SERVICE_NO_SERVICE)
    {
      DDL_SH_ISDU.length = 1;                                                   // no service
      DDL_SH_ISDU.BufferLength = 1;
    }
    else
    { 
      DDL_SH_ISDU.BufStartRef[0] = DDL_SH_ISDU.serviceDevice + 4;                    // error response
      DDL_SH_ISDU.BufStartRef[1] = DDL_SH_ISDU.errorCode;
      DDL_SH_ISDU.BufStartRef[2] = DDL_SH_ISDU.addErrorCode;
      DDL_SH_ISDU.BufferLength = 4;
    }
  }
  DDL_SH_ISDU.BufferIndex = 0;
  DDL_SH_ISDU.BufStartRef[DDL_SH_ISDU.BufferLength - 1] = 0;                         // cks
  DDL_SH_ISDU.BufStartRef[DDL_SH_ISDU.BufferLength - 1] = 
                                 DDL_SH_CalcISDUChecksum(DDL_SH_ISDU.BufferLength, 
                                                        DDL_SH_ISDU.BufStartRef);
}

/** \} */ // end of DDL_Service_Handler

/**
 *  \brief DL Service handler ISDU transport response
 *
 *  This function is used by Application layer to
 *  response result of ISDU request to Service handler.
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.1.6, p. 58
 *
 *  \param dataLength service response data length
 *  \param errorInfo  service error response info
 *            can be:
 *            #DDL_SH_ISDU_ERROR_INFO_NO_ERROR
 *            #DDL_SH_ISDU_ERROR_INFO_NO_COMM
 *            #DDL_SH_ISDU_ERROR_INFO_STATE_CONFLICT
 *            #DDL_SH_ISDU_ERROR_INFO_TIMEOUT
 *            #DDL_SH_ISDU_ERROR_INFO_NO_ISDU_SUPPORTED
 *            #DDL_SH_ISDU_ERROR_INFO_VALUE_OUT_OF_RANGE
 *            #DDL_SH_ISDU_ERROR_INFO_APPL_ERROR
 *
 */
void DDL_ISDUTransport_rsp (UInteger16T dataLength, EnumT errorInfo)
{
  DDL_SH_ISDU.isError = 
                     errorInfo == DDL_SH_ISDU_ERROR_INFO_NO_ERROR ? FALSE : TRUE;

  if (DDL_SH_ISDU.isError == FALSE)
  {  
    DDL_SH_ISDU.dataLength    = 
             DDL_SH_ISDU.direction == DDL_SH_ISDU_DIRECTION_READ ? dataLength : 0;
    DDL_SH_ISDU.serviceDevice = 
                             DDL_SH_ISDU.direction == DDL_SH_ISDU_DIRECTION_READ ?
                             DDL_SH_ISDU_SERVICE_DEVICE_READ_RSP_OK : 
                                         DDL_SH_ISDU_SERVICE_DEVICE_WRITE_RSP_OK;
  }
  else
  {
    if (DDL_SH_ISDU.direction == DDL_SH_ISDU_DIRECTION_NO_SERVICE)
    {
      DDL_SH_ISDU.serviceDevice = DDL_SH_ISDU_SERVICE_NO_SERVICE;               // no service
    }
    else
    {
      DDL_SH_ISDU.serviceDevice = 
                             DDL_SH_ISDU.direction == DDL_SH_ISDU_DIRECTION_READ ?
                             DDL_SH_ISDU_SERVICE_DEVICE_READ_RSP_ERR : 
                                        DDL_SH_ISDU_SERVICE_DEVICE_WRITE_RSP_ERR;
    }
  }
  DDL_SH_PackISDURsp();                                                         // pack device ISDU response
  DDL_SH_State = DDL_SH_ISDU_RESPONSE_4;                                        // start response
}

/** \} */ // end of Data_Link_Layer
