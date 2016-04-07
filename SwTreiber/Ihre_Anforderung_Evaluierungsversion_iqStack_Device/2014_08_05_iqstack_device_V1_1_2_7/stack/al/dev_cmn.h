//******************************************************************************
//
//     Copyright © 2011-2013 by IQ² Development GmbH, Neckartenzlingen 
//     All rights reserved
//
//******************************************************************************

#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  \defgroup Device Common
 *
 *  Common part of Device io-link protocol implementation
 *  \{
 */

/**
 *  \brief Common Services Results
 *  
 *  Result type for services result: ok or error.
 *  
 */
typedef enum
{
  /** \brief ok result */
  SERVICE_RESULT_NO_ERROR           = 0,
  /** \brief some error */
  SERVICE_RESULT_ERROR              = 1,
  /** \brief state conflict */
  SERVICE_RESULT_STATE_CONFLICT     = 2,
  /** \brief no communication */
  SERVICE_RESULT_NO_COMM            = 3,
  /** \brief wrong parameter */
  SERVICE_RESULT_PARAMETER_CONFLICT = 4,
  /** \brief timing error */
  SERVICE_RESULT_TIMING_CONFLICT    = 5,
  /** \brief service busy error */
  SERVICE_RESULT_BUSY               = 6

} ResultT;

/** \brief State type for state machines. */
typedef  UInteger8T StateT;

/** \brief Enum type for services arguments values. */
typedef  UInteger8T EnumT;

/** \} */ // end of Common

#ifdef __cplusplus
}
#endif

#endif
