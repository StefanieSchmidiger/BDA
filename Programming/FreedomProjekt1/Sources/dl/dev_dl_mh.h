//******************************************************************************
//
//     Copyright © 2011-2013 by IQ² Development GmbH, Neckartenzlingen 
//     All rights reserved
//
//******************************************************************************

#ifndef DDL_MH_H_
#define DDL_MH_H_

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
 *  \defgroup DDL_Mode_Handler
 *
 *  DL-mode handler
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.2, p. 71
 *  \{
 */

/**
 *  \defgroup DDL_Mode_Handler_States
 *
 *  DL-mode handler states
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.2.5, Table 43, p. 78
 *  \{
 */

/** \brief Waiting on wakeup current pulse DPL_WakeUp_ind(). */
#define DDL_MH_STATE_IDLE_0              0
/** \brief Message handler activated and waiting for the next transmission */
#define DDL_MH_STATE_ESTABLISH_COMM_1    1
/** \brief Compatibility check (see 9.2.3.3) */
#define DDL_MH_STATE_STARTUP_2           2
/** \brief On-request data (parameter, commands, events) exchange without process data */
#define DDL_MH_STATE_PREOPERATE_3        3
/** \brief Process data exchange and on-request data (parameter, commands, events) exchange */
#define DDL_MH_STATE_OPERATE_4           4

/** \} */ // end of DDL_Mode_Handler_States

/**
 *  \defgroup DDL_Modes
 *
 *  DL modes
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.1.14, p.64
 *  \{
 */

/** \brief inactive */
#define DDL_MODE_INACTIVE   3
/** \brief sdci communication with baudrate COM1 */
#define DDL_MODE_COM1       DPL_MODE_COM1
/** \brief sdci communication with baudrate COM2 */
#define DDL_MODE_COM2       DPL_MODE_COM2
/** \brief sdci communication with baudrate COM3 */
#define DDL_MODE_COM3       DPL_MODE_COM3
/** \brief sdci communication is lost            */
#define DDL_MODE_COMMLOSS   4
/** \brief sdci startup */
#define DDL_MODE_ESTABCOM   5
/** \brief sdci startup */
#define DDL_MODE_STARTUP    6
/** \brief sdci preoperate */
#define DDL_MODE_PREOPERATE 7
/** \brief sdci operate */
#define DDL_MODE_OPERATE    8

/** \} */ // end of DDL_Modes

void DDL_MH_Init (void);

/**
 *  \fn void DDL_Mode_ind(void)
 *  \brief DL mode indentification
 *
 *  The DL uses the DDL_Mode local service to report
 *  to system management that an operating status has been reached.
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.1.14, p.63
 *
 *  \param targetMode specifies the requested operation mode, can be
 *      #DDL_MODE_INACTIVE,
 *      #DDL_MODE_COM1,
 *      #DDL_MODE_COM2,
 *      #DDL_MODE_COM3,
 *      #DDL_MODE_COMMLOSS,
 *      #DDL_MODE_ESTABCOM,
 *      #DDL_MODE_STARTUP,
 *      #DDL_MODE_OPERATE,
 *      #DDL_MODE_PREOPERATE
 */
extern void DDL_Mode_ind (UInteger8T realMode);

/**
 *  \fn void DDL_Read_ind(void)
 *  \brief DL read indentification
 *
 *  The DDL_Read service is used to read a parameter value 
 *  from the Device via the page communication channel.
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.1.4, p.57
 *
 *  \param address this parameter specifies the address of the requested parameter, 
 *                 i.e. the parameter addresses within the page communication channel.
 *                 Permitted values: 0 to 15, in accordance with parameter access rights 
 *      
 */
extern void DDL_Read_ind (UInteger8T address);

void DDL_MH_TimerCallback (DPL_DelayTypeT delayType);

void DDL_MH_FirstByteReceived_ind (void);

/** \} */ // end of DDL_Mode_Handler

void DDL_MEH_MHInfo_IllegalMessageType (void);

/** \} */ // end of Data_Link_Layer

void DPL_Wakeup_ind (void);

#ifdef __cplusplus
}
#endif

#endif