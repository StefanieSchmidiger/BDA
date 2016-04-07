//******************************************************************************
//
//     Copyright © 2011-2013 by IQ² Development GmbH, Neckartenzlingen 
//     All rights reserved
//
//******************************************************************************

#ifndef DDL_MEH_H_
#define DDL_MEH_H_

#ifdef __cplusplus
extern "C" {
#endif
  
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

/**
 *  \defgroup DDL_Message_Handler_States
 *
 *  DL Message Handler states
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.3.5, p. 86
 *  \{
 */

/** \brief Waiting for activation */
#define DDL_MEH_STATE_INACTIVE_0       0
/** \brief Waiting on first octet of Master message */
#define DDL_MEH_STATE_IDLE_1           1
/** \brief Receive Master message octet by octet */
#define DDL_MEH_STATE_GET_MESSAGE_2    2
/** \brief Check M-sequence type, checksum, length, consistency */
#define DDL_MEH_STATE_CHECK_MESSAGE_3  3
/** \brief Create message from OD.rsp, PD.rsp, EventFlag State and PD Valid State */
#define DDL_MEH_STATE_CREATE_MESSAGE_4 4
/** \brief Message is created and ready to be sent */
#define DDL_MEH_STATE_MESSAGE_READY_5  5
/** \brief Message should be sent (after response delay) */
#define DDL_MEH_STATE_SEND_MESSAGE_6   6

/** \} */ // end of DDL_Message_Handler_States

/**
 *  \defgroup DDL_Message_Handler_OD_Args
 *
 *  DL Message Handler OD service possible arguments values
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.2.2, p. 67
 *  \{
 */

/**
 *  \brief Message Handler OD RW Directions
 *
 *  OD read/write directions
 *  
 */
typedef enum
{
  /** \brief OD write direction */
  DDL_MEH_OD_RWDIR_WRITE = 0,
  /** \brief OD read direction */
  DDL_MEH_OD_RWDIR_READ  = 1

} DDL_MEH_OD_RWDirT;

/**
 *  \defgroup DDL_Message_Handler_OD_Com_Channels
 *
 *  OD communication channels
 *  \{
 */

/** \brief OD page communication channel */
#define DDL_MEH_OD_COM_CH_PAGE      1
/** \brief OD diagnosis communication channel */
#define DDL_MEH_OD_COM_CH_DIAGNOSIS 2
/** \brief OD ISDU communication channel */
#define DDL_MEH_OD_COM_CH_ISDU      3

/** \} */ // end of DDL_Message_Handler_OD_Com_Channels

/**
 *  \defgroup DDL_Message_Handler_Errors
 *
 *  OD errors
 *  \{
 */

#define DDL_MEH_OD_ERROR_INFO_NO_COMM        0
#define DDL_MEH_OD_ERROR_INFO_STATE_CONFLICT 1

/** \} */ // end of DDL_Message_Handler_Errors

/** \} */ // end of DDL_Message_Handler_OD_Args

/**
 *  \defgroup DDL_Message_Handler_Message_Types
 *
 *  Message types
 *  See: IO-Link spec v1.1.1 Oct 2011, A.1.3, p.193 and A.2, p. 195
 *
 *  \{
 */

#define DDL_MEH_MESSAGE_TYPE_0 0
#define DDL_MEH_MESSAGE_TYPE_1 1
#define DDL_MEH_MESSAGE_TYPE_2 2
#define DDL_MEH_MESSAGE_TYPE_RESERVED 3

/** \} */ // end of DDL_Message_Handler_Message_Types

/** \brief Checksum calculation seed value
 *  See: IO-Link spec v1.1.1 Oct 2011, A.1.6, p. 194
 */
#define DDL_MEH_CHECKSUM_SEED_VALUE 0x52

/**
 *  \defgroup DDL_Message_Handler_Com_Channel_types
 *
 *  Message communication channel types
 *  See: IO-Link spec v1.1.1 Oct 2011, A.1.2, p. 192
 *
 *  \{
 */

#define DDL_MEH_MESSAGE_COM_CH_PROCESS   0
/** \brief PARAMETER */
#define DDL_MEH_MESSAGE_COM_CH_PAGE      1
/** \brief EVENT */
#define DDL_MEH_MESSAGE_COM_CH_DIAGNOSIS 2
#define DDL_MEH_MESSAGE_COM_CH_ISDU      3

/** \} */ // end of DDL_Message_Handler_Com_Channel_types

/**
 *  \defgroup DDL_Message_Handler_Info_types
 *
 *  Message handler info types, used in DDL_MEH_MHInfo()
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.2.6, p. 70
 *
 *  \{
 */
#define DDL_MEH_INFO_COMMLOST             0
#define DDL_MEH_INFO_ILLEGAL_MESSAGE_TYPE 1
#define DDL_MEH_INFO_CHECKSUM_MISMATCH    2

/** \} */ // end of DDL_Message_Handler_Info_types

/** \brief Calculate device message buffer checksum */
#define DDL_MEH_CALC_DEV_MES_BUF_CHECKSUM(message_buffer, buffer_length)\
  message_buffer[buffer_length - 1] |=\
    DDL_MEH_CalcChecksum6(DDL_MEH_GetDataBytesXOR(message_buffer,\
                                                  buffer_length))

/** \brief Message type
 *
 *  See: IO-Link spec v1.1.1 Oct 2011, A.1, p. 192
 *
 */
typedef struct
{
  // MC
  /** \brief R/W direction bit */
  DDL_MEH_OD_RWDirT rwDirection;
  /** \brief Communication channel 
   *
   * can be:
   *     #DDL_MEH_OD_COM_CH_PAGE
   *     #DDL_MEH_OD_COM_CH_DIAGNOSIS
   *     #DDL_MEH_OD_COM_CH_ISDU
   *
   */
  EnumT comChannel;
  /** \brief Address or flow control of the ISDU data */
  UInteger8T addressISDUFC;

  // CKT
  /** \brief M-sequence type */
  EnumT mseqType;
  /** \brief Master checksum */
  UInteger8T masterChecksum;

  // PD out
  /** \brief PD Out data */
  UInteger8T * pdOut;

  // OD
  /** \brief OD data */
  UInteger8T * od;
  /** \brief OD data length */
  UInteger8T odLength;

  // PD in
  /** \brief PD In data */
  UInteger8T * pdIn;
  
  #ifdef DPL_BACK_COMPATIBLE_RevID_10
  /** \brief Previous PD R/W direction to define end of PD in/out cyle */
  DDL_MEH_OD_RWDirT rwDirPrevPD;
  /** \brief PD Out buffer for interleave */          
  UInteger8T pdOutBuf[32];
  /** \brief PD In buffer for interleave */
  UInteger8T pdInBuf[32];  
  #endif

  // CKS
  /** \brief Event flag bit */
  BooleanT isEvent;
  /** \brief PD Invalid flag bit */
  BooleanT isPDInvalid;
  /** \brief Device checksum */
  UInteger8T deviceChecksum;

  // send/receive buffer
  UInteger8T recvBuffer[DPL_MESSAGE_BUFFER_MAX_LENGTH];
  UInteger8T sendBuffer[DPL_MESSAGE_BUFFER_MAX_LENGTH];
  UInteger8T * pRecvBuf;
  UInteger8T * pSendBuf;
  UInteger8T * pTransferBuf;
  UInteger8T bufLength;
  UInteger8T recvLength;

#ifndef DPL_SKIP_CHECKSUM_CALC_MST_MESSAGE
  UInteger8T recvBytesXOR;
#endif
//#ifndef DPL_SKIP_CHECKSUM_CALC_DEV_MESSAGE
//  UInteger8T sendBytesXOR;
//#endif
  
  UInteger8T cksIndex;

} DDL_MEH_MessageT;

extern DSM_DeviceComT DSM_DeviceCom;

extern EnumT DDL_MH_State;

/**
 *  \fn void DDL_MEH_MHInfo_IlligalMessageType(void)
 *  \brief MHInfo service
 *
 *  The service MHInfo signals an exceptional
 *  operation within the message handler (ILLEGAL_MESSAGE_TYPE).
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.2.6, p. 70
 *
 */
extern void DDL_MEH_MHInfo_IllegalMessageType(void);

/**
 *  \fn void DDL_MEH_MHInfo_Commloss(void)
 *  \brief MHInfo service
 *
 *  The service MHInfo signals an exceptional
 *  operation within the message handler (COMMLOST).
 *  Actuators shall observe this information
 *  and take corresponding actions.
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.2.6,  p. 70, 7.3.3.5, p. 86
 *
 */
extern void DDL_MEH_MHInfo_Commloss(void);

/**
 *  \fn void DDL_MEH_OD_ind(void)
 *  \brief DL Message Handler OD indication
 *
 *  This function indicates, that a new master message
 *  with on-request data has been received.
 *  It should be called from DL on-request data handler
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.2.2, p.67
 *
 *  \param rwDirection read or write direction
 *
 *  \param comChannel  selected communication channel for the transmission
 *             can be:
 *             #DDL_MEH_MESSAGE_COM_CH_PAGE
 *             #DDL_MEH_MESSAGE_COM_CH_DIAGNOSIS
 *             #DDL_MEH_MESSAGE_COM_CH_ISDU
 *
 *  \param addressCtrl address or flow control value, can be 0 to 31
 *  \param length      data length to transmit, can be 0 to 32
 *  \param data        data to transmit
 *
 */
extern void DDL_MEH_OD_ind (EnumT rwDirection, EnumT comChannel, 
                           UInteger8T addressCtrl, UInteger8T length, 
                           UInteger8T * data);

void DDL_MEH_PD_ind (void);

void DDL_MEH_Init (void);

void DDL_MEH_SetStateIdle (void);

void DDL_MEH_TimerCallback (DPL_DelayTypeT delayType);

ResultT DDL_MEH_Config (BooleanT isActive);

UInteger8T DDL_MEH_GetDataBytesXOR (UInteger8T * data, UInteger8T length);

UInteger8T DDL_MEH_CalcChecksum6 (UInteger8T bytesXOR);

//ResultT DDL_MEH_ParseAndCheckMasterMessage (void);

void DDL_MEH_PackDeviceMessage (void);

void DDL_MEH_OD_ind (EnumT rwDirection, EnumT comChannel, UInteger8T addressCtrl,
                    UInteger8T length, UInteger8T * data);

void DDL_MEH_PD_rsp (UInteger8T * pdIn);

void DDL_MEH_OD_rsp (UInteger8T length, UInteger8T * data);

extern void DDL_MEH_EndOfSend_ind (void);

void DDL_MEH_SendResponse (void);

/** \} */ // end of DDL_Message_Handler

/**
 *  \fn void DDL_PDOutputTransport_ind(BooleanT pdOutInvalid)
 *  \brief DL Message Handler PD Out indication
 *
 *  This function indicates,
 *  that new PD out data has been received from master.
 *  It should be defined in Application Layer.
 *  Application layer can get PD Out from DDL_MEH_Message.pdOut
 *  and set PD In to DDL_MEH_Message.pdIn.
 *  PD lengths have to be set by SM initialization in Device Application.
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.1.9, p.61
 *
 *  \param pdOutValid PD Out invalid flag
 *            #TRUE  - PD In invalid
 *            #FALSE - PD In   valid
 *
 */
extern void DDL_PDOutputTransport_ind (BooleanT pdOutInvalid);

void DDL_Control_req (BooleanT pdInValid);

/** \} */ // end of Data_Link_Layer

void DPL_Transfer_ind (UInteger8T data, DPL_TransferStatusT status);
void DPL_TransferBuf_ind(UInteger8T * data, UInteger8T dataLength);

#ifdef __cplusplus
}
#endif

#endif
