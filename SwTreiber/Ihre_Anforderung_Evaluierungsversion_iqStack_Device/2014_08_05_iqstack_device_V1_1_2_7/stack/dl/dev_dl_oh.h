//******************************************************************************
//
//     Copyright © 2011-2013 by IQ² Development GmbH, Neckartenzlingen 
//     All rights reserved
//
//******************************************************************************

#ifndef DDL_OH_H_
#define DDL_OH_H_

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
 *  \defgroup DDL_Onrequest_data_Handler
 *
 *  DL On-request data handler
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.5, p. 90
 *  \{
 */

/**
 *  \defgroup DDL_Onrequest_data_Handler_States
 *
 *  DL On-request data states
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.5.3, p. 92
 *  \{
 */

/** \brief Waiting for activation */
#define DDL_OH_STATE_INACTIVE_0 0
/** \brief Waiting on messages with on-request data. Decomposition and analysis */
#define DDL_OH_STATE_IDLE_1     1

/** \} */ // end of DDL_Onrequest_data_Handler_States
  
extern BooleanT DDL_CH_Go2Operate;

extern StateT DDL_SH_State;

extern StateT DDL_EH_State;

extern DDL_MEH_MessageT DDL_MEH_Message;

extern DDL_SH_ISDU_T DDL_SH_ISDU;

void DDL_OH_Init (void);

ResultT DDL_OH_Config (BooleanT isActive);

void DDL_OH_OD_rsp_noservice (void);

void DDL_OH_DirectParameterHandler (EnumT rwDirection, UInteger8T address,
                                   UInteger8T length, UInteger8T * data);

/** \} */ // end of DDL_Onrequest_data_Handle

/**
 *  \fn DDL_Read(UInteger8T address, UInteger8T * value)
 *  \brief DL read direct parameter from SM
 *
 *  Reads a parameter value from the Device
 *  via the page communication channel.
 *  It should defined in System manager.
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.1.4, p. 57
 *
 *  \param address address of the requested parameter
 *  \param value   read parameter value
 *
 *  \return result of operation, can be:
 *      #SERVICE_RESULT_NO_ERROR
 *      #SERVICE_RESULT_STATE_CONFLICT
 *      #SERVICE_RESULT_NO_COMM
 *
 */
extern ResultT DDL_Read (UInteger8T address, UInteger8T * value);

/**
 *  \fn DDL_Write(UInteger8T address, UInteger8T value)
 *  \brief DL write direct parameter to SM
 *
 *  Writes a parameter value to the Device
 *  via the page communication channel.
 *  It should defined in System manager.
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.1.5, p. 58
 *
 *  \param address address of the requested parameter
 *  \param value   write parameter value
 *
 *  \return result of operation, can be:
 *      #SERVICE_RESULT_NO_ERROR
 *      #SERVICE_RESULT_STATE_CONFLICT
 *      #SERVICE_RESULT_NO_COMM
 *
 */
extern ResultT DDL_Write (UInteger8T address, UInteger8T value);

/**
 *  \fn DDL_ReadParam(UInteger8T address, UInteger8T * value)
 *  \brief DL read direct parameter from AL
 *
 *  Reads a parameter value from the Device
 *  via the page communication channel.
 *  It should defined in Application Layer.
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.1.2, p. 55
 *
 *  \param address address of the requested parameter
 *  \param value   read parameter value
 *
 *  \return result of operation, can be:
 *      #SERVICE_RESULT_NO_ERROR
 *      #SERVICE_RESULT_STATE_CONFLICT
 *      #SERVICE_RESULT_NO_COMM
 *
 */
extern ResultT DDL_ReadParam (UInteger8T address, UInteger8T * value);

/**
 *  \fn DDL_WriteParam(UInteger8T address, UInteger8T value)
 *  \brief DL write direct parameter to AL
 *
 *  Writes a parameter value to the Device
 *  via the page communication channel.
 *  It should defined in Application Layer.
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.1.3, p. 56
 *
 *  \param address address of the requested parameter
 *  \param value   write parameter value
 *
 *  \return result of operation, can be:
 *      #SERVICE_RESULT_NO_ERROR
 *      #SERVICE_RESULT_STATE_CONFLICT
 *      #SERVICE_RESULT_NO_COMM
 *
 */
extern ResultT DDL_WriteParam (UInteger8T address, UInteger8T value);

void DDL_MEH_OD_ind (EnumT rwDirection, EnumT comChannel, UInteger8T addressCtrl,
                    UInteger8T length, UInteger8T * data);

/** \} */ // end of Data_Link_Layer

#ifdef __cplusplus
}
#endif

#endif
