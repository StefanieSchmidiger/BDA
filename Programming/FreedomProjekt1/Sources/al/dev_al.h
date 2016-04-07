//******************************************************************************
//
//     Copyright © 2011-2013 by IQ² Development GmbH, Neckartenzlingen 
//     All rights reserved
//
//******************************************************************************

#ifndef DAL_H_
#define DAL_H_

#ifdef __cplusplus
extern "C" {
#endif
  
/**
 *  \defgroup Application_layer
 *
 *  Application layer
 *  See: IO-Link spec v1.1.1 Oct 2011, 8, p. 102
 *  \{
 */

/**
 *  \defgroup Application_layer_States
 *
 *  Application layer states
 *  See: IO-Link spec v1.1.1 Oct 2011, 8.3.2.2, p. 115
 *  \{
 */

/** \brief Idle state
 *
 * The Device AL is waiting on subordinated DL service calls
 * triggered by Master messages.
 *
 */
#define DAL_STATE_IDLE_0               0

/** \brief Wait write response
 *
 * The Device AL is waiting on a response from the technology specific application
 * (write access to Direct Parameter page).
 *
 */
#define DAL_STATE_AWAIT_AL_WRITE_RSP_1 1

/** \brief Wait read response
 *
 * The Device AL is waiting on a response from the technology specific application
 * (read access to Direct Parameter page).
 *
 */
#define DAL_STATE_AWAIT_AL_READ_RSP_2  2

/** \brief Wait R/W response
 *
 * The Device AL is waiting on a response from the technology specific application
 * (read or write access via ISDU).
 *
 */
#define DAL_STATE_AWAIT_AL_RW_RSP_3    3

/** \} */ // end of Application_layer_States

extern DDL_MEH_MessageT DDL_MEH_Message;
extern DDL_SH_ISDU_T DDL_SH_ISDU;


void DAL_Init (void);

/**
 *  \fn void DAL_Control_ind(BooleanT controlCode)
 *  \brief AL PD Out validity indication
 *
 *  This function is used by Application layer
 *  to indicate PD Out validity to Device application.
 *  It should be defined in Device application.
 *  See: IO-Link spec v1.1.1 Oct 2011, 8.2.2.12, p. 111
 *
 *  \param controlCode a code identifying the state of the process data,
 *             can be:
 *             #TRUE  -   valid
 *             #FALSE - invalid
 *
 */
extern void DAL_Control_ind (BooleanT controlCode);

/**
 *  \fn void DAL_Read_ind(UInteger16T index, UInteger8T subindex)
 *  \brief AL read indication
 *
 *  This function is used by Application layer
 *  to indicate OD read to Device application.
 *  It should be defined in Device application.
 *  See: IO-Link spec v1.1.1 Oct 2011, 8.2.2.1, p. 104
 *
 *  \param index    On-request data index
 *
 *  \param subindex On-request data subindex,
 *          index 0, subindex 1-16: parameters of direct page 1
 *          index 1, subindex 1-16: parameters of direct page 2
 *          index > 1: ISDU
 *
 *  \param data    On-request data to response
 *
 */
extern void DAL_Read_ind (UInteger16T index, UInteger8T subindex, 
                          UInteger8T * data);

/**
 *  \fn void DAL_Write_ind(UInteger16T index, UInteger8T subindex,
                           UInteger8T  dataLength, UInteger8T * data)
 *  \brief AL write indication
 *
 *  This function is used by Application layer
 *  to indicate OD write to Device application.
 *  It should be defined in Device application.
 *  See: IO-Link spec v1.1.1 Oct 2011, 8.2.2.1, p. 105
 *
 *  \param index    On-request data index
 *
 *  \param subindex On-request data subindex,
 *          index 0, subindex 1-16: parameters of direct page 1
 *          index 1, subindex 1-16: parameters of direct page 2
 *          index > 1: ISDU
 *
 *  \param dataLength OD data length
 *  \param data       OD data bytes
 *
 */
extern void DAL_Write_ind (UInteger16T index, UInteger8T subindex,
                          UInteger8T  dataLength, UInteger8T * data);
/**
 *  \fn void DAL_Abort_ind(void)
 *  \brief AL abort indication
 *
 *  This function is used by Application layer
 *  to indicate the aborting of current
 *  DAL_Read or DAL_Write services by Master.
 *  It abandons the response to DAL_Read or DAL_Write services.
 *  It should be defined in Device application.
 *  See: IO-Link spec v1.1.1 Oct 2011, 8.2.2.3, p. 106
 *
 */
extern void DAL_Abort_ind (void);

/**
 *  \fn void DAL_Event_cnf(void)
 *  \brief AL event confirmation
 *
 *  This function is used by Application layer
 *  to indicate that all events are delivered to master
 *  after calling DAL_Event_req() by device application.
 *  It should be defined in Device application.
 *  See: IO-Link spec v1.1.1 Oct 2011, 8.2.2.11, p.110
 *
 */
extern void DAL_Event_cnf (void);

/**
 *  \fn void DAL_PDCycle_ind(void)
 *  \brief AL event confirmation
 *
 *  This function is used by Application layer
 *  to indicate the end of a process data cycle. 
 *  The Device application can use this service 
 *  to transmit new input data to the application layer via DAL_SetInput_req().
 *  It should be defined in Device application.
 *  See: IO-Link spec v1.1.1 Oct 2011, 8.2.2.7, p.108
 *
 */
extern void DAL_PDCycle_ind (void);

/**
 *  \fn void DAL_NewOutput_ind(void)
 *  \brief new output data indication
 *
 *  This function is used by Application layer
 *  to indicate the receipt of updated output data from master.
 *  It should be defined in Device application.
 *  See: IO-Link spec v1.1.1 Oct 2011, 8.2.2.9, p.109
 *
 */
extern void DAL_NewOutput_ind (void);

void DAL_Control_req (BooleanT controlCode);

void DAL_Read_rsp (UInteger8T length);

void DAL_Read_err (UInteger8T errorCode, UInteger8T addErrorCode);

void DAL_Write_rsp (void);

void DAL_Write_err (UInteger8T errorCode, UInteger8T addErrorCode);

void DAL_Event_req (UInteger8T eventCount, DDL_EH_EventT * events);

UInteger8T * DAL_SetInput_req (void);

UInteger8T * DAL_GetOutput_req (void);

/** \} */ // end of Application_layer

extern void DDL_Event_req (DDL_EH_EventT * event);

extern void DDL_EventTriger_req (void);

extern void DDL_Control_req (BooleanT pdInInvalid);

void    DDL_Control_ind (BooleanT controlCode);

void    DDL_ISDUAbort  (void);

ResultT DDL_ReadParam  (UInteger8T address, UInteger8T * value);

ResultT DDL_WriteParam (UInteger8T address, UInteger8T value);

void DDL_ISDUTransport_ind (UInteger16T  index, UInteger8T subIndex, 
                           EnumT direction, UInteger8T dataLength, 
                           UInteger8T * data);

void DDL_EventTriger_cnf (void);

void DDL_PDOutputTransport_ind (BooleanT pdOutInvalid);

#ifdef __cplusplus
}
#endif

#endif
