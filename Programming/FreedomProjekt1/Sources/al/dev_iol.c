//******************************************************************************
//
//     Copyright © 2011-2013 by IQ² Development GmbH, Neckartenzlingen 
//     All rights reserved
//
//******************************************************************************

#include "dev_iol.h"

/**
 *  \defgroup IOL_Device
 *
 *  Common part of device io-link protocol implementation
 *  \{
 */

/**
 *  \brief IO-Link device protocol initialization
 *
 *  Internal function.
 *  This function initializes io-link device protocol.
 *  It should be called before starting using io-link protocol in device application.
 *
 */
void DEV_IOL_Init (void)
{
                                                                                // init protocol modules
  DPL_Init();
  DDL_MH_Init();
  DDL_MEH_Init();
  DDL_OH_Init();
  DAL_Init();
  DSM_Init();
}

/**
 *  \brief IO-Link device protocol main cycle
 *
 *  Internal function.
 *  This function process main io-link device protocol tasks
 *  outside all interrupts.
 *  It should be called in a main cycle
 *  of device application firmware.
 *
 */
void DEV_IOL_MainCycle (void)
{
  DSM_MainCycle();
  DDL_SH_MainCycle();
  DDL_EH_MainCycle();
}

/** \} */ // end of IOL_Device


