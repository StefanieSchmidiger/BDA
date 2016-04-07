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
 *  \defgroup DDL_Message_Handler
 *
 *  DL message handler
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.3, p. 79
 *  \{
 */

DPL_IOL_DATA_SEG_START

/**
 *  \brief DL Message Handler state
 *
 *  Internal variable.
 *  Can be:
 *    #DDL_MEH_STATE_INACTIVE_0
 *    #DDL_MEH_STATE_IDLE_1
 *    #DDL_MEH_STATE_GET_MESSAGE_2
 *    #DDL_MEH_STATE_CREATE_MESSAGE_3
 *    #DDL_MEH_STATE_CHECK_MESSAGE_4
 *
 */
DPL_IOL_DATA_SEG_PREFIX
volatile StateT DDL_MEH_State;
DPL_IOL_DATA_SEG_POSTFIX

/**
 *  \brief DL Message Handler message
 *
 *  Internal variable.
 *  Message
 *
 */
DPL_IOL_DATA_SEG_PREFIX
DDL_MEH_MessageT DDL_MEH_Message;
DPL_IOL_DATA_SEG_POSTFIX

/**
 *  \brief DL Message Handler PD Out invalid flag
 *
 *  Internal variable.
 *  PD Out invalid flag:
 *  #TRUE  - PD In invalid
 *  #FALSE - PD In   valid
 *
 */
DPL_IOL_DATA_SEG_PREFIX
BooleanT DDL_MEH_PDOutInvalid;
DPL_IOL_DATA_SEG_POSTFIX

DPL_IOL_DATA_SEG_END

#ifdef DPL_PACK_CKS_FAST
const UInteger8T DDL_MEH_PackCKS8To6[] = 
{
  0x00, 0x11, 0x21, 0x30, 0x12, 0x03, 0x33, 0x22, 0x22, 0x33, 0x03, 0x12, 0x30, 0x21, 0x11, 0x00,
  0x14, 0x05, 0x35, 0x24, 0x06, 0x17, 0x27, 0x36, 0x36, 0x27, 0x17, 0x06, 0x24, 0x35, 0x05, 0x14,
  0x24, 0x35, 0x05, 0x14, 0x36, 0x27, 0x17, 0x06, 0x06, 0x17, 0x27, 0x36, 0x14, 0x05, 0x35, 0x24,
  0x30, 0x21, 0x11, 0x00, 0x22, 0x33, 0x03, 0x12, 0x12, 0x03, 0x33, 0x22, 0x00, 0x11, 0x21, 0x30,
  0x18, 0x09, 0x39, 0x28, 0x0A, 0x1B, 0x2B, 0x3A, 0x3A, 0x2B, 0x1B, 0x0A, 0x28, 0x39, 0x09, 0x18,
  0x0C, 0x1D, 0x2D, 0x3C, 0x1E, 0x0F, 0x3F, 0x2E, 0x2E, 0x3F, 0x0F, 0x1E, 0x3C, 0x2D, 0x1D, 0x0C,
  0x3C, 0x2D, 0x1D, 0x0C, 0x2E, 0x3F, 0x0F, 0x1E, 0x1E, 0x0F, 0x3F, 0x2E, 0x0C, 0x1D, 0x2D, 0x3C,
  0x28, 0x39, 0x09, 0x18, 0x3A, 0x2B, 0x1B, 0x0A, 0x0A, 0x1B, 0x2B, 0x3A, 0x18, 0x09, 0x39, 0x28,
  0x28, 0x39, 0x09, 0x18, 0x3A, 0x2B, 0x1B, 0x0A, 0x0A, 0x1B, 0x2B, 0x3A, 0x18, 0x09, 0x39, 0x28,
  0x3C, 0x2D, 0x1D, 0x0C, 0x2E, 0x3F, 0x0F, 0x1E, 0x1E, 0x0F, 0x3F, 0x2E, 0x0C, 0x1D, 0x2D, 0x3C,
  0x0C, 0x1D, 0x2D, 0x3C, 0x1E, 0x0F, 0x3F, 0x2E, 0x2E, 0x3F, 0x0F, 0x1E, 0x3C, 0x2D, 0x1D, 0x0C,
  0x18, 0x09, 0x39, 0x28, 0x0A, 0x1B, 0x2B, 0x3A, 0x3A, 0x2B, 0x1B, 0x0A, 0x28, 0x39, 0x09, 0x18,
  0x30, 0x21, 0x11, 0x00, 0x22, 0x33, 0x03, 0x12, 0x12, 0x03, 0x33, 0x22, 0x00, 0x11, 0x21, 0x30,
  0x24, 0x35, 0x05, 0x14, 0x36, 0x27, 0x17, 0x06, 0x06, 0x17, 0x27, 0x36, 0x14, 0x05, 0x35, 0x24,
  0x14, 0x05, 0x35, 0x24, 0x06, 0x17, 0x27, 0x36, 0x36, 0x27, 0x17, 0x06, 0x24, 0x35, 0x05, 0x14,
  0x00, 0x11, 0x21, 0x30, 0x12, 0x03, 0x33, 0x22, 0x22, 0x33, 0x03, 0x12, 0x30, 0x21, 0x11, 0x00
};
#endif

/**
 *  \brief DL Message Handler init
 *
 *  Internal function.
 *  It should be called from DEV_IOL_Init()
 */
void DDL_MEH_Init (void)
{
  UInteger8T index;
  DDL_MEH_State = DDL_MEH_STATE_INACTIVE_0;
  DDL_MEH_Message.bufLength = 0;
  DDL_MEH_Message.recvLength = 0;
  DDL_MEH_PDOutInvalid = TRUE;
  DDL_MEH_Message.pRecvBuf = DDL_MEH_Message.recvBuffer;
  DDL_MEH_Message.pSendBuf = DDL_MEH_Message.sendBuffer;
  for (index = 0; index < sizeof(DDL_MEH_Message.sendBuffer); index++)
  {  
    DDL_MEH_Message.sendBuffer[index] = 0;
  }
}

/**
 *  \brief DL Message Handler set idle state
 *
 *  Internal function.
 *  This function set message handler to idle state
 *  and start ComTimout timer for specific device maximum cycle time,
 *  if this time is not zero:
 *
 *  It can be actual for actuators.
 *
 */
void DDL_MEH_SetStateIdle (void)
{  
  #ifdef DPL_USE_DEVICE_RESPONSE_DELAY
        DPL_StopTimer(DPL_DEVICE_RESPONSE_DELAY);
  #endif
  
  DPL_StartTimer(DPL_MAX_CYCLE_TIME);
  DDL_MEH_State = DDL_MEH_STATE_IDLE_1;
}

/**
 *  \brief DL Message Handler Timer callback
 *
 *  Internal function.
 *  This function checks message handler timeouts.
 */
void DDL_MEH_TimerCallback (DPL_DelayTypeT delayType)
{
  
  if (delayType == DPL_MAX_CYCLE_TIME &&
      DDL_MEH_State == DDL_MEH_STATE_IDLE_1)
  {                                                                             // communication timeout DPL_MAX_CYCLE_TIME
    DDL_MEH_SetStateIdle();
    DDL_MEH_MHInfo_Commloss();
  }

  if (delayType == DPL_MASTER_MESSAGE_BYTE_RECEIVE_TIMEOUT &&
      DDL_MEH_State == DDL_MEH_STATE_GET_MESSAGE_2)
  {                                                                             // timeout between master message 
                                                                                // received bytes
    DDL_MEH_SetStateIdle();
  }

#ifdef DPL_USE_DEVICE_RESPONSE_DELAY
  if (delayType == DPL_DEVICE_RESPONSE_DELAY)                                   // send response timer      
  {            
    if (DDL_MEH_State == DDL_MEH_STATE_MESSAGE_READY_5)
      DDL_MEH_SendResponse();
    else if (DDL_MEH_State == DDL_MEH_STATE_CREATE_MESSAGE_4)
      DDL_MEH_State = DDL_MEH_STATE_SEND_MESSAGE_6;
  }
#endif
}

/**
 *  \brief DL Message Handler activator
 *
 *  Internal function.
 *  Activate or deactivate message handler
 *
 *  \param  isActive manage message handler activation
 *             #TRUE - activate, #FALSE - deactivate
 *
 *  \return result of operation, can be:
 *      #SERVICE_RESULT_NO_ERROR
 *      #SERVICE_RESULT_STATE_CONFLICT
 *
 */
ResultT DDL_MEH_Config (BooleanT isActive)
{
  if (DDL_MEH_State == DDL_MEH_STATE_INACTIVE_0 && isActive == TRUE)
  {
    DDL_MEH_SetStateIdle();
    return SERVICE_RESULT_NO_ERROR;
  }
  else if (isActive == FALSE)
  {
    DDL_MEH_Init();
    return SERVICE_RESULT_NO_ERROR;
  }
  return SERVICE_RESULT_STATE_CONFLICT;
}

/**
 *  \brief DL Message Handler get XOR of message bytes
 *
 *  Internal function.
 *
 *  \param  data   bytes of message
 *  \param  length bytes number of message
 *
 *  \return XOR of message bytes
 *
 */
UInteger8T DDL_MEH_GetDataBytesXOR (UInteger8T * data, UInteger8T length)
{
  UInteger8T bytesXOR = DDL_MEH_CHECKSUM_SEED_VALUE;
  UInteger8T byteNum;
  for (byteNum = 0; byteNum < length; byteNum++) bytesXOR ^= data[byteNum];     // xor all message bytes
  return bytesXOR;
}

/**
 *  \brief DL Message Handler checksum calculator
 *
 *  Internal function.
 *  Calculate device/master checksum
 *  See: IO-Link spec v1.1.1 Oct 2011, A.1.6, p. 195
 *
 *  \param  bytesXOR  XOR of message bytes
 *
 *  \return compressed checksum 6
 *
 */
UInteger8T DDL_MEH_CalcChecksum6 (UInteger8T bytesXOR)
{    
#ifndef DPL_PACK_CKS_FAST
    UInteger8T evenOddXOR;
    UInteger8T neighbourXOR;
#endif    
#ifdef DPL_PACK_CKS_FAST
    bytesXOR = DDL_MEH_PackCKS8To6[bytesXOR];                                   // Pack 8 bit cks to 6 bit
#else    
    evenOddXOR   = (bytesXOR   << 4) ^ bytesXOR;                                // calculate 2 bits xor of even/odd bits of message xor
    evenOddXOR  ^= (evenOddXOR >> 2);
    evenOddXOR   =  evenOddXOR & 0x30;                                          // mask 00110000
    
    neighbourXOR = (bytesXOR   >> 1) ^ bytesXOR;                                // calculate 4 bits xor of neighbour bits of message xor
    neighbourXOR = (neighbourXOR       & 1) | ((neighbourXOR >> 1) & 2) |
                  ((neighbourXOR >> 2) & 4) | ((neighbourXOR >> 3) & 8);
    bytesXOR = evenOddXOR | neighbourXOR;
#endif        
    return bytesXOR;  
}

/**
 *  \brief DL Message Handler check master message M-seq type
 *
 *  Internal function.
 *  Check M-sequence type of master message
 *  See: IO-Link spec v1.1.1 Oct 2011, A.1.3, 193
 *
 *  \return TRUE - M-sequence type is ok,
 *          FALSE - M-sequence type is illegal
 *
 */
BooleanT DDL_MEH_CheckMasterMessageMseq (void)
{
  UInteger8T ckt = DDL_MEH_Message.pRecvBuf[1];
  DDL_MEH_Message.mseqType = (ckt >> 6) & 3;                                    // M-sequence type  
  DDL_MEH_Message.masterChecksum = ckt & 0x3F;                                  // CKT
                                                                                // master checksum
  if ((DDL_MH_State == DDL_MH_STATE_PREOPERATE_3 && 
       DSM_DeviceCom.msequenceTypePreoper != DDL_MEH_Message.mseqType) ||       // check M-sequence type
      (DDL_MH_State == DDL_MH_STATE_OPERATE_4 &&
           DSM_DeviceCom.msequenceTypeOper != DDL_MEH_Message.mseqType) ||
      DDL_MEH_Message.mseqType == DDL_MEH_MESSAGE_TYPE_RESERVED)
  {
    DDL_MEH_MHInfo_IllegalMessageType();
    return FALSE;
  }    
  return TRUE;
}

/**
 *  \brief DL Message Handler parse/pack OD write/read
 *  Internal function.
 */
void DDL_MEH_ParsePackOD (UInteger8T pdOutLen)
{
  // parse OD write data
  if (DDL_MEH_Message.rwDirection == DDL_MEH_OD_RWDIR_WRITE)
  {
    DDL_MEH_Message.recvLength += DDL_MEH_Message.odLength;
    DDL_MEH_Message.od = &(DDL_MEH_Message.pRecvBuf[pdOutLen + 2]);
#ifdef DPL_BACK_COMPATIBLE_RevID_10
    if (DSM_DeviceCom.isInterleave != TRUE)
#endif
      DDL_MEH_Message.cksIndex += DDL_MEH_Message.odLength;
  }
  else // if (DDL_MEH_Message.rwDirection == DDL_MEH_OD_RWDIR_READ)    
  // pack OD read data to buffer to send
  {                                                                             
    DDL_MEH_Message.od = DDL_MEH_Message.pSendBuf;
    DDL_MEH_Message.cksIndex += DDL_MEH_Message.odLength;
    if (DDL_MEH_Message.comChannel == DDL_MEH_MESSAGE_COM_CH_ISDU)
      DDL_MEH_OD_ind(DDL_MEH_Message.rwDirection, DDL_MEH_Message.comChannel,
                     DDL_MEH_Message.addressISDUFC, 
                     DDL_MEH_Message.odLength, DDL_MEH_Message.od);             // od.ind
  }
}

/**
 *  \brief DL Message Handler master M-sequence control byte parser
 *
 *  Internal function.
 *  Parse M-sequence control byte (MC).
 *  See: IO-Link spec v1.1.1 Oct 2011, A.1.2, p. 192
 *
 */
void DDL_MEH_ParseMseqControl (UInteger8T mc)
{
  UInteger8T pdOutLen = 0;
 
  DDL_MEH_Message.rwDirection = (DDL_MEH_OD_RWDirT)
    (((mc >> 7) & 1) == 1 ? DDL_MEH_OD_RWDIR_READ : DDL_MEH_OD_RWDIR_WRITE);    // R/W direction bit
  
  DDL_MEH_Message.comChannel = (mc >> 5) & 3;                                   // MC
                                                                                // communication channel
  DDL_MEH_Message.addressISDUFC = mc & 0x1F;                                    // address or flow control of the ISDU data
      
  if (DDL_MH_State == DDL_MH_STATE_PREOPERATE_3)                                // define current OD length
      DDL_MEH_Message.odLength = DSM_DeviceCom.msequenceODLengthPreoper;
  else if (DDL_MH_State == DDL_MH_STATE_OPERATE_4)
    DDL_MEH_Message.odLength = DSM_DeviceCom.msequenceODLengthOper;      
  else
    DDL_MEH_Message.odLength = 1;
  
  DDL_MEH_Message.cksIndex = 0;
  
#ifdef DPL_BACK_COMPATIBLE_RevID_10
  if (DSM_DeviceCom.isInterleave == TRUE)
  {
    // interleave mode only for communication with IO-Link Master RevID 1.0    
    if (DDL_MH_State == DDL_MH_STATE_OPERATE_4 &&
        DDL_MEH_Message.comChannel == DDL_MEH_MESSAGE_COM_CH_PROCESS)
    {
      if (DDL_MEH_Message.rwDirection == DDL_MEH_OD_RWDIR_WRITE)
        // PD out
        DDL_MEH_Message.recvLength += 2;
      else      
        // PD in
        DDL_MEH_Message.cksIndex += 2;
    }
    else
      DDL_MEH_ParsePackOD(pdOutLen);
  }
  else
#endif
  {
    // parse PD out data
    if (DDL_MH_State == DDL_MH_STATE_OPERATE_4)
    {                                         
      DDL_MEH_Message.pdOut = &(DDL_MEH_Message.pRecvBuf[2]);
      DDL_MEH_Message.recvLength += DSM_DeviceCom.lengthPDOutBytes;
      pdOutLen = DSM_DeviceCom.lengthPDOutBytes;
    }
    
    DDL_MEH_ParsePackOD(pdOutLen);
      
    // pack PD in to buffer to send  
    if (DDL_MH_State == DDL_MH_STATE_OPERATE_4)
    {                                                                             
      DDL_MEH_Message.pdIn = &(DDL_MEH_Message.pSendBuf[DDL_MEH_Message.cksIndex]);
      DDL_MEH_Message.cksIndex += DSM_DeviceCom.lengthPDInBytes;
    }
  }
}

void DDL_MEH_PD_rsp (UInteger8T * pdIn)
{                                                                               // not used

}

// not used
void DDL_MEH_OD_rsp  (UInteger8T length, UInteger8T * data)
{

}

/**
 *  \brief DL Message Handler device message transfer
 *
 *  Internal function.
 *  Pack device message to the message buffer.
 *  See: IO-Link spec v1.1.1 Oct 2011, A.1.5, p. 194
 *
 */
void DDL_MEH_PackDeviceMessage (void)
{
  UInteger8T cks = DDL_MEH_Message.isEvent == TRUE ? 0x80 : 0;                             // pack CKS to buffer to send
  if (DDL_MEH_Message.isPDInvalid == TRUE) cks = cks | 0x40;
  DDL_MEH_Message.pSendBuf[DDL_MEH_Message.cksIndex] = cks;
  
  DDL_MEH_Message.bufLength = DDL_MEH_Message.cksIndex + 1;  
  DDL_MEH_Message.pTransferBuf = DDL_MEH_Message.pSendBuf;
#ifdef DPL_BACK_COMPATIBLE_RevID_10
  if (DSM_DeviceCom.isInterleave != TRUE)
#endif
    if (DDL_MEH_Message.rwDirection == DDL_MEH_OD_RWDIR_WRITE)
    {      
      DDL_MEH_Message.bufLength -= DDL_MEH_Message.odLength;
      DDL_MEH_Message.pTransferBuf = 
        &(DDL_MEH_Message.pTransferBuf[DDL_MEH_Message.odLength]);
    }
  
#ifndef DPL_SKIP_CHECKSUM_CALC_DEV_MESSAGE
  DDL_MEH_CALC_DEV_MES_BUF_CHECKSUM(DDL_MEH_Message.pTransferBuf, 
                                    DDL_MEH_Message.bufLength);
#endif
}

void DDL_MEH_SendResponse (void)
{     
  DPL_TransferBuffer_req(DDL_MEH_Message.bufLength, 
                         DDL_MEH_Message.pTransferBuf);                         // send device response message
  
  if (DDL_MEH_Message.rwDirection == DDL_MEH_OD_RWDIR_WRITE &&
      DDL_MEH_Message.comChannel == DDL_MEH_MESSAGE_COM_CH_ISDU)
    DDL_MEH_OD_ind(DDL_MEH_Message.rwDirection, DDL_MEH_Message.comChannel,
                   DDL_MEH_Message.addressISDUFC, 
                   DDL_MEH_Message.odLength, DDL_MEH_Message.od);               // od.ind
  
  DDL_MEH_SetStateIdle();                                                       // ready to receive next master message
  DDL_MEH_EndOfSend_ind();
  
  #ifdef DPL_BACK_COMPATIBLE_RevID_10
  if (DSM_DeviceCom.isInterleave == TRUE &&
      DDL_MEH_Message.comChannel == DDL_MEH_MESSAGE_COM_CH_PROCESS)
  {
    if (DDL_MEH_Message.rwDirection == DDL_MEH_OD_RWDIR_WRITE &&
        DDL_MEH_Message.addressISDUFC + 2 >= DSM_DeviceCom.lengthPDOutBytes)
      DDL_PDOutputTransport_ind(DDL_MEH_PDOutInvalid);
    //DDL_MEH_Message.rwDirPrevPD = DDL_MEH_Message.rwDirection;
  }
  #endif
}

/**
 *  \brief DL Message Handler PD indication
 *
 *  This function indicates to Application layer,
 *  that new PD out has been received from master,
 *  and there is a new PD cycle.
 *  It also reports PD Out invalid flag.
 *  Application layer can get PD Out from DDL_MEH_Message.pdOut
 *  and set PD In to DDL_MEH_Message.pdIn.
 *  PD lengths have to be set by SM initialization in Device Application.
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.1.9, p.61, 7.2.2.3, p. 68
 *
 */
void DDL_MEH_PD_ind (void)
{
#ifdef DPL_BACK_COMPATIBLE_RevID_10
  if (DSM_DeviceCom.isInterleave == TRUE)
  {
    if (DDL_MEH_Message.rwDirection == DDL_MEH_OD_RWDIR_WRITE)
    {
      // parse PD out data
      DDL_MEH_Message.pdOutBuf[DDL_MEH_Message.addressISDUFC]     = DDL_MEH_Message.pRecvBuf[2];
      DDL_MEH_Message.pdOutBuf[DDL_MEH_Message.addressISDUFC + 1] = DDL_MEH_Message.pRecvBuf[3];
    }
    else
    {
      // pack PD in to buffer to send
      DDL_MEH_Message.pSendBuf[0] = DDL_MEH_Message.pdInBuf[DDL_MEH_Message.addressISDUFC];
      DDL_MEH_Message.pSendBuf[1] = DDL_MEH_Message.pdInBuf[DDL_MEH_Message.addressISDUFC + 1];
    }
    // PD cycle and out ind occurs after sending in DDL_MEH_SendResponse()     
  }
  else
#endif
    DDL_PDOutputTransport_ind(DDL_MEH_PDOutInvalid);
}

/**
 *  \brief DL Message Handler PD In update
 *
 *  This function updates PD In invalid flag.
 *  It should be called from Application layer
 *  Application layer can set PD In to DDL_MEH_Message.pdIn.
 *  PD lengths have to be set by SM initialization in Device Application.
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.1.18, p.65
 *
 *  \param pdInInvalid this flag defines 
 *                     whether process data input is invalid or not
 * 
 */
void DDL_Control_req (BooleanT pdInInvalid)
{
  DDL_MEH_Message.isPDInvalid = pdInInvalid;
}

/**
 *  \brief DL Message Handler master message parser
 *
 *  Internal function.
 *  Parse master message to Message and check it
 *  See: IO-Link spec v1.1.1 Oct 2011, A.1.2, A.1.3, p. 192, 193 
 *
*/
void DDL_TransferBuf_ind(void)
{
  #ifndef DPL_SKIP_CHECKSUM_CALC_MST_MESSAGE
  if (DDL_MEH_Message.masterChecksum != 
      DDL_MEH_CalcChecksum6(DDL_MEH_Message.recvBytesXOR))                      // check master checksum
  {
    DDL_MEH_SetStateIdle();                                                     // error checksum in master message
    return;
  }
  #endif
  
  DDL_MEH_State = DDL_MEH_STATE_CREATE_MESSAGE_4;

#ifdef DPL_USE_DEVICE_RESPONSE_DELAY    
  DPL_StartTimer(DPL_DEVICE_RESPONSE_DELAY);                                    // start device message response 
                                                                                // after DPL_DEVICE_RESPONSE_DELAY  
#endif
                                                                                // proccess master message and make a response
  if (DDL_MH_State == DDL_MH_STATE_ESTABLISH_COMM_1) 
  {                                                                             // first byte of first message is 
                                                                                // received from master
    #ifdef DPL_BACK_COMPATIBLE_RevID_10
    DDL_MEH_Message.rwDirPrevPD = DDL_MEH_OD_RWDIR_READ;
    #endif          
    DDL_MH_FirstByteReceived_ind();
  }
          
  if (DDL_MEH_Message.comChannel != DDL_MEH_MESSAGE_COM_CH_PROCESS &&
      DDL_MEH_Message.comChannel != DDL_MEH_MESSAGE_COM_CH_ISDU)
    DDL_MEH_OD_ind(DDL_MEH_Message.rwDirection, DDL_MEH_Message.comChannel,
                   DDL_MEH_Message.addressISDUFC, 
                   DDL_MEH_Message.odLength, DDL_MEH_Message.od);               // od.ind

  
  if (DDL_MH_State == DDL_MH_STATE_OPERATE_4)
  {              
    #ifdef DPL_BACK_COMPATIBLE_RevID_10
    if ((DSM_DeviceCom.isInterleave == TRUE &&
         DDL_MEH_Message.comChannel == DDL_MEH_MESSAGE_COM_CH_PROCESS)                        
        || DSM_DeviceCom.isInterleave == FALSE)
    #endif
      DDL_MEH_PD_ind();                                                         // pd.ind
  }

  DDL_MEH_PackDeviceMessage();
  
#ifdef DPL_USE_DEVICE_RESPONSE_DELAY    
  if (DDL_MEH_State == DDL_MEH_STATE_SEND_MESSAGE_6)                            // response delay
    DDL_MEH_SendResponse(); 
  else
    DDL_MEH_State = DDL_MEH_STATE_MESSAGE_READY_5;
#else
  DDL_MEH_SendResponse();                                                       // send a response immediately after 
                                                                                // proccessing master message
#endif    
}

/** \} */ // end of DDL_Message_Handler

/** \} */ // end of Data_Link_Layer

void DPL_Transfer_ind (UInteger8T data, DPL_TransferStatusT status)
{   
  if (DDL_MEH_State == DDL_MEH_STATE_INACTIVE_0) 
  {
      return;
  }  
  if (status != DPL_TRANSFER_STATUS_NO_ERROR)
  {                                                                             // UART communicaton error, ignore octet
    DDL_MEH_SetStateIdle();          
    return;
  }  
  DPL_StopTimer(DPL_MASTER_MESSAGE_BYTE_RECEIVE_TIMEOUT);
      
  if (DDL_MEH_State == DDL_MEH_STATE_IDLE_1)
  {                                                                             // first octet        
    DDL_MEH_Message.bufLength = 0;                                              // reset buffer
    DDL_MEH_Message.recvLength = 2;
    DDL_MEH_Message.odLength = 0;
#ifndef DPL_SKIP_CHECKSUM_CALC_MST_MESSAGE
    DDL_MEH_Message.recvBytesXOR = DDL_MEH_CHECKSUM_SEED_VALUE;
#endif
    DDL_MEH_State = DDL_MEH_STATE_GET_MESSAGE_2;
    DDL_MEH_ParseMseqControl(data);
  }
  
  if (DDL_MEH_State == DDL_MEH_STATE_GET_MESSAGE_2)
  {                                                                             // get message    
    DDL_MEH_Message.pRecvBuf[DDL_MEH_Message.bufLength] = data;                 // add octet to buffer
    DDL_MEH_Message.bufLength++;

    if (DDL_MEH_Message.bufLength == 2)                                         // CKT byte received
    {
      if (DDL_MEH_CheckMasterMessageMseq() != TRUE)                             // Check header, M-sequence type.
      { 
        DDL_MEH_SetStateIdle();                                                 // Wrong header (M-sequence type)
        return;                                                
      }
#ifndef DPL_SKIP_CHECKSUM_CALC_MST_MESSAGE
      data = data & 0xC0;                                                       // reset cks to 0, to calc it again for checking below in DDL_TransferBuf_ind()
#endif
    }
    
#ifndef DPL_SKIP_CHECKSUM_CALC_MST_MESSAGE
    DDL_MEH_Message.recvBytesXOR ^= data;
#endif

    if (DDL_MEH_Message.bufLength == DDL_MEH_Message.recvLength)
    {
      DDL_TransferBuf_ind();
    }
    else
    {
      DPL_StartTimer(DPL_MASTER_MESSAGE_BYTE_RECEIVE_TIMEOUT);                  // start timer to observe timeout between 
                                                                                // master message received bytes
    }
  }
}

void DPL_TransferBuf_ind(UInteger8T * data, UInteger8T dataLength)
{
  DDL_MEH_State = DDL_MEH_STATE_GET_MESSAGE_2;
  DDL_MEH_Message.bufLength = dataLength;
  DDL_MEH_Message.pRecvBuf = data;
  DDL_MEH_ParseMseqControl(DDL_MEH_Message.pRecvBuf[0]);
#ifndef DPL_SKIP_CHECKSUM_CALC_MST_MESSAGE
  DDL_MEH_Message.pRecvBuf[1] = DDL_MEH_Message.pRecvBuf[1] & 0xC0;           // reset cks to 0, to calc it again for checking below in DDL_TransferBuf_ind()
  DDL_MEH_Message.recvBytesXOR = DDL_MEH_GetDataBytesXOR(data, dataLength);
#endif
  DDL_TransferBuf_ind();
}
