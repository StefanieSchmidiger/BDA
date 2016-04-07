//******************************************************************************
//
//     Copyright © 2011-2013 by IQ² Development GmbH, Neckartenzlingen 
//     All rights reserved
//
//******************************************************************************

#ifndef DEVICE_APPL_H_
#define DEVICE_APPL_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  \defgroup Device_application
 *
 *  Device application
 *  See: IO-Link spec v1.1.1 Oct 2011, 10, p. 151
 *  \{
 */

void DA_Init (void);

void DA_InitDSM (void);

void DA_MainCycle (void);

void DSM_DeviceMode (DSM_ModeT mode);

void DSM_SwitchToRevID10_ind(void);

void DAL_Control_ind (BooleanT controlCode);

void DAL_Read_ind (UInteger16T index, UInteger8T subindex, 
                   UInteger8T * data);

void DAL_Write_ind (UInteger16T index, UInteger8T subindex,
                    UInteger8T  dataLength, UInteger8T * data);

void DAL_Abort_ind (void);

void DAL_Event_cnf (void);

void DAL_PDCycle_ind (void);

void DAL_NewOutput_ind (void);

void DDL_MEH_MHInfo_Commloss (void);

/** \} */ // end of Device_application

#ifdef __cplusplus
}
#endif

#endif
