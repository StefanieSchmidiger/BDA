//******************************************************************************
//
//     Copyright © 2011-2013 by IQ² Development GmbH, Neckartenzlingen 
//     All rights reserved
//
//******************************************************************************

#ifndef DDL_CH_H_
#define DDL_CH_H_

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
 *  \defgroup DDL_Command_Handler
 *
 *  DL command handler
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.7, p. 96
 *  \{
 */

/**
 *  \defgroup DDL_Command_Handler_Commands
 *
 *  DL command handler commands
 *  See: IO-Link spec v1.1.1 Oct 2011, B.1.2, p. 215
 *  \{
 */

#define DDL_CH_MC_FALLBACK          0x5A
#define DDL_CH_MC_MASTER_IDENT      0x95
#define DDL_CH_MC_DEVICE_IDENT      0x96
#define DDL_CH_MC_DEVICE_STARTUP    0x97
#define DDL_CH_MC_PDOUT_OPERATE     0x98
#define DDL_CH_MC_DEVICE_OPERATE    0x99
#define DDL_CH_MC_DEVICE_PREOPERATE 0x9A

/** \} */ // end of DDL_Command_Handler_Commands

extern StateT DDL_MH_State;

extern BooleanT DDL_MEH_PDOutInvalid;

/**
 *  See description of DDL_Write() in message handler.
 *  Command handler uses this function to transfer
 *  master command to System manager.
 */
extern ResultT DDL_Write (UInteger8T address, UInteger8T value);

void DDL_MC_MasterCommand (EnumT master_command);

/** \} */ // end of DDL_Command_Handler

/**
 *  \fn void DDL_Control_ind(BooleanT controlCode)
 *  \brief DL PD Out validity indication
 *
 *  This function is used by Data link layer
 *  to indicate PD Out validity to Application layer.
 *  It should be defined in Application layer.
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.1.18, p.65
 *
 *  \param controlCode a code identifying the state of the process data,
 *             can be:
 *             #TRUE  -   valid
 *             #FALSE - invalid
 *
 */
extern void DDL_Control_ind (BooleanT controlCode);

void DDL_MEH_EndOfSend_ind (void);

/** \} */ // end of Data_Link_Layer

#ifdef __cplusplus
}
#endif

#endif
