//******************************************************************************
//
//     Copyright © 2011-2013 by IQ² Development GmbH, Neckartenzlingen 
//     All rights reserved
//
//******************************************************************************

#ifndef DDL_SH_H_
#define DDL_SH_H_

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
 *  \defgroup DDL_Service_Handler
 *
 *  DL service handler
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.6, p. 92
 *  \{
 */

/**
 *  \defgroup DDL_Service_Handler_States
 *
 *  DL service handler states
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.6.4, p. 96
 *  \{
 */

/** \brief Waiting on next ISDU transmission */
#define DDL_SH_ISDU_IDLE_1     1
/** \brief Reception of ISDU request */
#define DDL_SH_ISDU_REQUEST_2  2
/** \brief Waiting on data from application layer to transmit (see DDL_ISDUTransport) */
#define DDL_SH_ISDU_WAIT_3     3
/** \brief Transmission of ISDU response data */
#define DDL_SH_ISDU_RESPONSE_4 4

/** \} */ // end of DDL_Service_Handler_States

/**
 *  \defgroup DDL_Service_Handler_FCTRL
 *
 *  DL service handler flow control values
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.6.2, p. 93
 *  \{
 */

/** \brief Start of an ISDU, i.e., start of a request or a response. */
#define DDL_SH_FCTRL_START  0x10
/** \brief No request for ISDU transmission. */
#define DDL_SH_FCTRL_IDLE_1 0x11
/** \brief No request for ISDU transmission. */
#define DDL_SH_FCTRL_IDLE_2 0x12
/** \brief Abort entire service. */
#define DDL_SH_FCTRL_ABORT  0x1F

/** \} */ // end of DDL_Service_Handler_FCTRL

/**
 *  \defgroup DDL_Service_Handler_ISDU_Service_bytes
 *
 *  DL service handler ISDU service bytes
 *  See: IO-Link spec v1.1.1 Oct 2011, A.5.3, p. 206
 *  \{
 */

#define DDL_SH_ISDU_SERVICE_BYTE_NO_SERVICE        0x00
#define DDL_SH_ISDU_SERVICE_BYTE_DEVICE_BUSY       0x01

/** \} */ // end of DDL_Service_Handler_ISDU_Service_bytes

/**
 *  \defgroup DDL_Service_Handler_ISDU_Services
 *
 *  DL service handler ISDU services
 *  See: IO-Link spec v1.1.1 Oct 2011, A.5.2, p. 205
 *  \{
 */

#define DDL_SH_ISDU_SERVICE_NO_SERVICE             0x00
#define DDL_SH_ISDU_SERVICE_MASTER_WRITE_REQ_8BI   0x10
#define DDL_SH_ISDU_SERVICE_MASTER_WRITE_REQ_8BIS  0x20
#define DDL_SH_ISDU_SERVICE_MASTER_WRITE_REQ_16BIS 0x30
#define DDL_SH_ISDU_SERVICE_DEVICE_WRITE_RSP_ERR   0x40
#define DDL_SH_ISDU_SERVICE_DEVICE_WRITE_RSP_OK    0x50
#define DDL_SH_ISDU_SERVICE_MASTER_READ_REQ_8BI    0x90
#define DDL_SH_ISDU_SERVICE_MASTER_READ_REQ_8BIS   0xA0
#define DDL_SH_ISDU_SERVICE_MASTER_READ_REQ_16BIS  0xB0
#define DDL_SH_ISDU_SERVICE_DEVICE_READ_RSP_ERR    0xC0
#define DDL_SH_ISDU_SERVICE_DEVICE_READ_RSP_OK     0xD0

/** \} */ // end of DDL_Service_Handler_ISDU_Services

/**
 *  \defgroup DDL_Service_Handler_ISDU_byte_numbers
 *
 *  DL service handler byte numbers in ISDU buffer
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.6.1, p. 92
 *  \{
 */

#define DDL_SH_ISDU_SERVICE_BYTE      0
#define DDL_SH_ISDU_EXTLENGTH_BYTE    1
#define DDL_SH_ISDU_INDEX_HI_8B_BYTE  2
#define DDL_SH_ISDU_INDEX_LO_16B_BYTE 3
#define DDL_SH_ISDU_SUBINDEX_8B_BYTE  3
#define DDL_SH_ISDU_SUBINDEX_16B_BYTE 4

/** \} */ // end of DDL_Service_Handler_ISDU_byte_numbers

/**
 *  \defgroup DDL_Service_Handler_ISDU_Service_directions
 *
 *  DL service handler ISDU service directions
 *  See: IO-Link spec v1.1.1 Oct 2011, A.5.2, p. 205
 *  \{
 */

#define DDL_SH_ISDU_DIRECTION_NO_SERVICE 0
#define DDL_SH_ISDU_DIRECTION_READ       1
#define DDL_SH_ISDU_DIRECTION_WRITE      2

/** \} */ // end of DDL_Service_Handler_ISDU_Service_directions

/**
 *  \defgroup DDL_Service_Handler_ISDU_ErrorInfo
 *
 *  DL service handler ISDU error info types for DDL_ISDUTransport_rsp()
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.1.6, p. 59
 *  \{
 */

#define DDL_SH_ISDU_ERROR_INFO_NO_ERROR           0
#define DDL_SH_ISDU_ERROR_INFO_NO_COMM            1
#define DDL_SH_ISDU_ERROR_INFO_STATE_CONFLICT     2
#define DDL_SH_ISDU_ERROR_INFO_TIMEOUT            3
#define DDL_SH_ISDU_ERROR_INFO_NO_ISDU_SUPPORTED  4
#define DDL_SH_ISDU_ERROR_INFO_VALUE_OUT_OF_RANGE 5
#define DDL_SH_ISDU_ERROR_INFO_APPL_ERROR         6

/** \} */ // end of DDL_Service_Handler_ISDU_ErrorInfo

/** \brief ISDU type
 *
 *  See: IO-Link spec v1.1.1 Oct 2011, A.5, p. 205
 *
 */
typedef struct
{
  /** \brief Service byte of master
   *
   *  can be:
   *  #DDL_SH_ISDU_SERVICE_MASTER_WRITE_REQ_8BI
   *  #DDL_SH_ISDU_SERVICE_MASTER_WRITE_REQ_8BIS
   *  #DDL_SH_ISDU_SERVICE_MASTER_WRITE_REQ_16BIS
   *  #DDL_SH_ISDU_SERVICE_MASTER_READ_REQ_8BI
   *  #DDL_SH_ISDU_SERVICE_MASTER_READ_REQ_8BIS
   *  #DDL_SH_ISDU_SERVICE_MASTER_READ_REQ_16BIS  *
   *
   */
  EnumT        serviceMaster;
  /** \brief Service byte of device
   *
   *  can be:
   *  #DDL_SH_ISDU_SERVICE_DEVICE_WRITE_RSP_ERR
   *  #DDL_SH_ISDU_SERVICE_DEVICE_WRITE_RSP_OK
   *  #DDL_SH_ISDU_SERVICE_DEVICE_READ_RSP_ERR
   *  #DDL_SH_ISDU_SERVICE_DEVICE_READ_RSP_OK
   *
   */
  EnumT        serviceDevice;

  /** \brief Current flow control in device ISDU handler */
  UInteger8T   flowControl;

  /** \brief Service byte length */
  UInteger8T   length;
  /** \brief Service byte length in ext length */
  BooleanT     isExtLength;

  /** \brief Service index */
  UInteger16T  index;
  /** \brief Service subindex */
  UInteger8T   subIndex;

  /** \brief Service data */
  UInteger8T * data;
  /** \brief Service data length */
  UInteger8T   dataLength;

  /** \brief Service error flag */
  BooleanT     isError;
  /** \brief Service error code */
  UInteger8T   errorCode;
  /** \brief Service additional error code */
  UInteger8T   addErrorCode;

  /** \brief Service checkSum */
  UInteger8T   checkSum;

  /** \brief Service direction
   *
   *  can be:
   *  #DDL_SH_ISDU_DIRECTION_NO_SERVICE
   *  #DDL_SH_ISDU_DIRECTION_READ
   *  #DDL_SH_ISDU_DIRECTION_WRITE
   *
   */
  EnumT        direction;

  /** \brief Flag of reception ISDU request for processing in main cycle */
  BooleanT     isduRequestReceivedToProcess;
  
  // ISDU buffer, length, index
  UInteger8T Buffer[DPL_ISDU_BUFFER_MAX_LENGTH];
  UInteger8T * BufStartRef;
  UInteger8T BufferLength;
  UInteger16T BufferIndex;

} DDL_SH_ISDU_T;

void DDL_SH_MainCycle (void);

void DDL_SH_OD_ind (EnumT rwDirection, UInteger8T addressCtrl,
                   UInteger8T length, UInteger8T * data);

UInteger8T DDL_SH_CalcISDUChecksum (UInteger8T dataLength, UInteger8T * data);

void DDL_SH_ParseISDUReq (void);

void DDL_SH_PackISDURsp (void);

void DDL_ISDUTransport_rsp (UInteger16T dataLength, EnumT errorInfo);

void DDL_SH_OD_rsp (void);

void DDL_SH_OD_rsp_busy (void);

/** \} */ // end of DDL_Service_Handler

/**
 *  \fn DDL_ISDUTransport_ind()
 *  \brief DL ISDU transport indication
 *
 *  DL Service handler indicates to Application layer,
 *  that new ISDU service request has been received
 *  from master.
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.1.6, p. 58
 *
 *  \param index      service index
 *  \param subindex   service subindex
 *
 *  \param direction  service direction, can be
 *                  #DDL_SH_ISDU_DIRECTION_READ
 *                  #DDL_SH_ISDU_DIRECTION_WRITE
 *
 *  \param dataLength service data length
 *  \param data     service data bytes
 *
 */
extern void DDL_ISDUTransport_ind (UInteger16T  index, UInteger8T subIndex, 
                                  EnumT direction, UInteger8T dataLength, 
                                  UInteger8T * data);

/**
 *  \fn DDL_ISDUAbort()
 *  \brief DL ISDU abort indication
 *
 *  The DDL_ISDUAbort service aborts the actual ISDU transmission,
 *  indicates it to Application layer.
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.1.7, p. 59
 *
 */
extern void DDL_ISDUAbort (void);

/** \} */ // end of Data_Link_Layer

#ifdef __cplusplus
}
#endif

#endif
