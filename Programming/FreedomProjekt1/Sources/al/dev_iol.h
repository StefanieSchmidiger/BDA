//******************************************************************************
//
//     Copyright © 2011-2013 by IQ² Development GmbH, Neckartenzlingen 
//     All rights reserved
//
//******************************************************************************

#ifndef IOL_DEVICE_H_
#define IOL_DEVICE_H_

#ifdef __cplusplus
extern "C" {
#endif
  
#include "dev_pl.h"
#include "dev_cmn.h"
#include "dev_dl_mh.h"
#include "dev_sm.h"
#include "dev_dl_meh.h"
#include "dev_dl_sh.h"
#include "dev_dl_oh.h"
#include "dev_dl_ch.h"
#include "dev_dl_eh.h"
#include "dev_al.h"
  
/**
 *  \defgroup IOL_Device
 *
 *  Common part of device io-link protocol implementation
 *  \{
 */
  
/** \brief Maximum IO-Link Revision ID supported by this protocol implementation. */
#define DEV_IOLINK_REVISION_ID 0x11

/** \brief Current version of this IO-Link protocol implementation. */
#define DEV_STACK_VERSION      0x27

void DEV_IOL_Init(void);

void DEV_IOL_MainCycle(void);

/** \} */ // end of IOL_Device

#ifdef __cplusplus
}
#endif

#endif

