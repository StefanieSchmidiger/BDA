//******************************************************************************
//
//     Copyright © 2011-2013 by IQ² Development GmbH, Neckartenzlingen 
//     All rights reserved
//
//******************************************************************************

#ifndef DPL_H
#define DPL_H

#ifdef __cplusplus
extern "C" {
#endif

/** 
 *  \defgroup Physical_Layer
 *
 *  Physical layer of the specific hardware
 *  See: IO-Link spec v1.1.1 Oct 2011, 5, p. 39
 *  \{
 */

/**
 *  \defgroup porting_bool_values
 *
 *  BooleanT data type value
 *  See: IO-Link spec v1.1.1 Oct 2011, annex E.2.2, p. 238
 *  \{
 */

/** \brief bool value true  */
#define TRUE  0xFF
/** \brief bool value false */
#define FALSE 0

/** \} */ // end of porting_bool_values

/**
 *  \defgroup porting_basic_data_types
 *  
 *  Basic data types
 *  See: IO-Link spec v1.1.1 Oct 2011, annex E.2, p. 238
 *  \{
 */

/** \brief logical boolean type */
typedef unsigned char      BooleanT;
/** \brief unsigned number 8 bits */
typedef unsigned char      UInteger8T;
/** \brief unsigned number 16 bits */
typedef unsigned int       UInteger16T;
/** \brief unsigned number 32 bits */
typedef unsigned long      UInteger32T;

//typedef unsigned long      UInteger64T;

/** \brief signed number 8 bits */
typedef char               Integer8T;
/** \brief signed number 16 bits */
typedef int                Integer16T;
/** \brief signed number 32 bits */
typedef long               Integer32T;

//typedef long               Integer64T;

//typedef float              Float32T;

/** 
 *  \brief string contains maximum 232 symbols 
 *  encoded in US-ASCII (7 bit) or UTF-8 [7]. UTF-8 
 */
#define StringT(var)       Integer8T var[232]
/** \brief string contains maximum 232 bytes */
#define OctetStringT(var)  Integer8T var[232]

//typedef UInteger64T        TimeT;

/** \} */ // end of porting_basic_data_types

/** 
 *  \brief Support of IO-Link v1.0
 *  
 *  If it is defined and 
 *  device is connected to old master with the previous IO-Link RevisionID 1.0, 
 *  stack will automatically switch to RevisionID 1.0 and
 *  device application will receive callback from stack.
 *
 */
#define DPL_BACK_COMPATIBLE_RevID_10

/**
 *  \brief DPL_tranfer_status
 *  
 *  PL Transfer status values of DPL_Transfer_ind()
 *  See: IO-Link spec v1.1.1 Oct 2011, 5.2.2.3, p. 42
 *  
 */

typedef enum
{
  /** \brief no error */
  DPL_TRANSFER_STATUS_NO_ERROR = 0,
  /** \brief parity error */
  DPL_TRANSFER_STATUS_PARITY_ERROR = 1,
  /** \brief framing error */
  DPL_TRANSFER_STATUS_FRAMING_ERROR = 2,
  /** \brief status overrun */
  DPL_TRANSFER_STATUS_OVERRUN = 3
  
} DPL_TransferStatusT;

/**
 *  \brief DPL_TargetMode
 *
 *  Physical layer modes, switched by DPL_Mode_req()
 *  See: IO-Link spec v1.1.1 Oct 2011, 5.2.2.1, p. 41
 *  
 */

typedef enum
{
  /** \brief inactive mode */
  DPL_MODE_INACTIVE = 3,
  /** \brief sdci communication with baudrate COM1 4,8   kbit/s */
  DPL_MODE_COM1 = 0,
  /** \brief sdci communication with baudrate COM2 38,4  kbit/s */
  DPL_MODE_COM2 = 1,
  /** \brief sdci communication with baudrate COM3 230,4 kbit/s */
  DPL_MODE_COM3 = 2,
  /** \brief digital output */
  DPL_MODE_DO = 4,
  /** \brief digital input */
  DPL_MODE_DI = 5,
  
  /** \brief preoperate */
  DPL_MODE_PREOPERATE = 6,
  /** \brief operate */
  DPL_MODE_OPERATE = 7

} DPL_TargetModeT;

/** \brief Receive message buffer maximum length */
#define DPL_MESSAGE_BUFFER_MAX_LENGTH 70

/** \brief ISDU buffer maximum length */
#define DPL_ISDU_BUFFER_MAX_LENGTH 256

/** 
 *  \brief Calculate checksum of send/receive message using const array 
 *  It takes additional 256 byte of ROM, but speeds up checksum calculation.
 */
#define DPL_PACK_CKS_FAST

/** \brief if defined, master message checksum checking will be skipped */
//#define DPL_SKIP_CHECKSUM_CALC_MST_MESSAGE

/** \brief if defined, device message checksum calculation will be skipped 
 *
 *  #DDL_MEH_CALC_DEV_MES_BUF_CHECKSUM macro from dev_dl_meh.h 
 *  can be used to calc it manually.
 */
//#define DPL_SKIP_CHECKSUM_CALC_DEV_MESSAGE

/**
 *  \brief DPL_Delay_types
 *
 *  Delay types used in protocol
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.2.2, Table 40, p. 73
 *  
 */
typedef enum
{
  /** \brief Standard IO delay */
  DPL_TIME_TDSIO_DELAY,
  /** \brief Fallback delay */
  DPL_TIME_TFBD_DELAY,
  /** \brief Device specific master message received bytes timeout */
  DPL_MASTER_MESSAGE_BYTE_RECEIVE_TIMEOUT,
  /** \brief Device specific response delay */
  DPL_DEVICE_RESPONSE_DELAY,
  /** \brief Device specific maximum cycle time */
  DPL_MAX_CYCLE_TIME

} DPL_DelayTypeT;

/** \brief Switch on/off DPL_DEVICE_RESPONSE_DELAY */
//#define DPL_USE_DEVICE_RESPONSE_DELAY

/** 
* \brief hardware supported baudrate
* 
* can be:
*   #DPL_MODE_COM1
*   #DPL_MODE_COM2
*   #DPL_MODE_COM3
* 
*/
#define DPL_SUPPORTED_BAUDRATE DPL_MODE_COM3

/**
 *  \defgroup DPL_Memory_Arrange_Management
 *
 *  It can be necessary for some hardware to arrange
 *  data memory segment manually.
 *  These macros are for this puporse.
 *  They surround defined in io-link code variables  
 *  to place them in special data segment.
 *  \{
 */

/** \brief Start placing defined variables in special data segment */
#define DPL_IOL_DATA_SEG_START
/** \brief End placing defined variables in special data segment */
#define DPL_IOL_DATA_SEG_END
/** \brief Defined variable prefix for special data segment */
#define DPL_IOL_DATA_SEG_PREFIX
/** \brief Defined variable postfix for special data segment */
#define DPL_IOL_DATA_SEG_POSTFIX

/** \} */ // end of DPL_Memory_Arrange_Management
/**
 *  \fn void DPL_Transfer_ind(UInteger8T data, DPL_TransferStatusT status)
 *  \brief PL byte transfer identification
 *
 *  This function should be called 
 *  by the device specific hardware physical layer
 *  to report data link layer, 
 *  that a data byte is received by io-link physical layer.
 *  See: IO-Link spec v1.1.1 Oct 2011, 5.2.2.3, p.42
 *  
 *  \param data   is a data byte to be received
 *  \param status UART error
 *
 */
extern void DPL_Transfer_ind (UInteger8T data, DPL_TransferStatusT status);

/**
 *  \fn void DPL_TransferBuf_ind(UInteger8T * data, UInteger8T dataLength)
 *  \brief PL byte buffer transfer identification
 *
 *  This function should be called 
 *  by the device specific hardware physical layer
 *  to report data link layer, 
 *  that a data buffer is received by io-link physical layer.
 *  This function can be used instead of DPL_Transfer_ind(),
 *  but master message length should be checked by PL in this case.
 *  See: IO-Link spec v1.1.1 Oct 2011, 5.2.2.3, p.42
 *  
 *  \param data       is a data buffer to be received
 *  \param dataLength data buffer length
 *
 */
extern void DPL_TransferBuf_ind(UInteger8T * data, UInteger8T dataLength);

/**
 *  \fn void DPL_Wakeup_ind(void)
 *  \brief PL wake-up identification
 *
 *  This function should be called 
 *  by the device specific hardware physical layer
 *  to report data link layer of device,
 *  that a wake-up request is received from the master.
 *  See: IO-Link spec v1.1.1 Oct 2011, 5.2.2.2, p.41
 */
extern void DPL_Wakeup_ind (void);

void DPL_Init(void);

void DPL_Transfer_req (UInteger8T data);

void DPL_Mode_req (DPL_TargetModeT targetMode);

void DPL_StartTimer (DPL_DelayTypeT delayType);
void DPL_StopTimer (DPL_DelayTypeT delayType);
void DPL_TimerCallback (DPL_DelayTypeT delayType);

void DPL_SWTimerCallback (void);

void DPL_TransferBuffer_req (UInteger8T length, UInteger8T * buffer);

/** \} */ // end of Physical_Layer

#ifdef __cplusplus
}
#endif

#endif
