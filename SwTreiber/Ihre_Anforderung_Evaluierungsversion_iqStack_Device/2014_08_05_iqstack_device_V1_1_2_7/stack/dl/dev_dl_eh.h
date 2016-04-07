//******************************************************************************
//
//     Copyright © 2011-2013 by IQ² Development GmbH, Neckartenzlingen 
//     All rights reserved
//
//******************************************************************************

#ifndef DDL_EH_H_
#define DDL_EH_H_

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
 *  \defgroup DDL_Event_Handler
 *
 *  DL event handler
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.8, p. 99
 *  \{
 */

/**
 *  \defgroup DDL_Event_Handler_States
 *
 *  DL event handler states
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.8.4, p. 101
 *  \{
 */

/** \brief Waiting on firing event flag */
#define DDL_EH_IDLE_1                1
/** \brief Reading event buffer and waiting confirmation
  *  of event buffer reading from master
  */
#define DDL_EH_FREEZE_EVENT_TABLE_2  2
/** \brief Waiting one communication cycle with no event flag 
  *  after confirmation to signal master end of event reading message flow.
  *  New events can be sent by device application only after this message
  *  (set event flag once more).
  */
#define DDL_EH_WAIT_CONFIRM_CYCLE_3  3
/** \brief Confirm end of event sending to AL in DDL_EH_MainCycle() */
#define DDL_EH_CONFIRM_TO_AL_4       4

/** \} */ // end of DDL_Event_Handler_States

/**
 *  \defgroup DDL_Event_Handler_Event_instances
 *
 *  DL event handler event instances
 *  of DDL_EH_EventT and DDL_Event_req()
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.1.15, p. 64, A.6.4, Table A.17, p. 211
 *  \{
 */

#define DDL_EH_EVENT_INSTANCE_UNKNOWN 0
//#define DDL_EH_EVENT_INSTANCE_PL      1
//#define DDL_EH_EVENT_INSTANCE_DL      2
//#define DDL_EH_EVENT_INSTANCE_AL      3
#define DDL_EH_EVENT_INSTANCE_APPL    4

/** \} */ // end of DDL_Event_Handler_Event_instances

/**
 *  \defgroup DDL_Event_Handler_Event_sources
 *
 *  DL event handler event sources
 *  of DDL_EH_EventT and DDL_Event_req()
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.1.15, p. 64, A.6.4, Table A.18, p. 212
 *  \{
 */

#define DDL_EH_EVENT_SOURCE_REMOTE 0
#define DDL_EH_EVENT_SOURCE_LOCAL  8

/** \} */ // end of DDL_Event_Handler_Event_sources

/**
 *  \defgroup DDL_Event_Handler_Event_types
 *
 *  DL event handler event types
 *  of DDL_EH_EventT and DDL_Event_req()
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.1.15, p. 64, A.6.4, Table A.19, p. 212
 *  \{
 */

#define DDL_EH_EVENT_TYPE_RESERVED        0
#define DDL_EH_EVENT_TYPE_NOTIFICATION 0x10
#define DDL_EH_EVENT_TYPE_WARNING      0x20
#define DDL_EH_EVENT_TYPE_ERROR        0x30

/** \} */ // end of DDL_Event_Handler_Event_types

/**
 *  \defgroup DDL_Event_Handler_Event_modes
 *
 *  DL event handler event modes
 *  of DDL_EH_EventT and DDL_Event_req()
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.1.15, p. 64, A.6.4, Table A.20, p. 212
 *  \{
 */

#define DDL_EH_EVENT_MODE_RESERVED        0
#define DDL_EH_EVENT_MODE_SINGLE_SHOT  0x40
#define DDL_EH_EVENT_MODE_DISAPPEARS   0x80
#define DDL_EH_EVENT_MODE_APPEARS      0xC0

/** \} */ // end of DDL_Event_Handler_Event_modes

/**
 *  \brief DL Event handler event buffer size
 *
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.8.1, p. 99, A.6.3, p. 210
 *
 */
#define DDL_EH_EVENT_BUFFER_SIZE 6

/**
 *  \brief DL Event handler event status no events
 *
 *  StatusCode type 2 (with details), no events
 *  See: IO-Link spec v1.1.1 Oct 2011, A.6.3, p. 210
 *
 */
#define DDL_EH_EVENT_STATUS_EMPTY 0x80

#define DDL_EH_SET_EVENT_IN_STATUS(num)   (DDL_EH_StatusCode = \
                                                  DDL_EH_StatusCode | (1 << num))
#define DDL_EH_CLEAR_EVENT_IN_STATUS(num) (DDL_EH_StatusCode = \
                                               DDL_EH_StatusCode & (~(1 << num)))

/**
 *  \brief DL Event handler event type
 *
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.1.15, p. 64, A.6.4-5, p. 211
 *
 */
typedef struct
{
  /**
   * \brief Event instance
   *
   * can be:
   * #DDL_EH_EVENT_INSTANCE_UNKNOWN
   * #DDL_EH_EVENT_INSTANCE_APPL
   *
   */
  EnumT     instance;
  /**
   * \brief Event type
   *
   * can be:
   * #DDL_EH_EVENT_TYPE_RESERVED
   * #DDL_EH_EVENT_TYPE_NOTIFICATION
   * #DDL_EH_EVENT_TYPE_WARNING
   * #DDL_EH_EVENT_TYPE_ERROR
   *
   */
  EnumT     type;
  /**
   * \brief Event mode
   *
   * can be:
   * #DDL_EH_EVENT_MODE_RESERVED
   * #DDL_EH_EVENT_MODE_SINGLE_SHOT
   * #DDL_EH_EVENT_MODE_DISAPPEARS
   * #DDL_EH_EVENT_MODE_APPEARS
   *
   */
  EnumT     mode;
  /**
   * \brief Event source
   *
   * can be:
   * #DDL_EH_EVENT_SOURCE_REMOTE
   * #DDL_EH_EVENT_SOURCE_LOCAL
   *
   */
  EnumT     source;
  /** \brief Event code
   * See: IO-Link spec v1.1.1 Oct 2011, A.6.5, p. 212, Annex D, p. 235
   */
  UInteger16T code;
} DDL_EH_EventT;

void DDL_EH_Init (void);

void DDL_EH_MainCycle (void);

void DDL_EH_OD_ind (EnumT rwDirection, UInteger8T address);

void DDL_EH_ReadConf (void);

void DDL_EH_ReadEvent (UInteger8T address);

/** \} */ // end of DDL_Event_Handler

void DDL_Event_req (DDL_EH_EventT * event);

void DDL_EventTriger_req (void);

/**
 *  \fn void DDL_EventTriger_cnf(void)
 *  \brief DL Event handler event trigger confirmation
 *
 *  This function indicates that the event buffer
 *  is realized and events transfer to master is finished.
 *  It should be defined in Application Layer.
 *  DDL_EventTriger_req() is used by Application layer to
 *  start transfer of device events to master
 *  and freeze the event buffer.
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.1.17, p. 65
 *
 */
extern void DDL_EventTriger_cnf (void);

/** \} */ // end of Data_Link_Layer

#ifdef __cplusplus
}
#endif

#endif
