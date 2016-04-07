//******************************************************************************
//
//     Copyright © 2011-2013 by IQ² Development GmbH, Neckartenzlingen 
//     All rights reserved
//
//******************************************************************************

#include "dev_iol.h"

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

DPL_IOL_DATA_SEG_START

/**
 *  \brief DL Event handler state
 *
 *  Internal variable.
 *  Can be:
 *    #DDL_EH_IDLE_1
 *    #DDL_EH_FREEZE_EVENT_TABLE_2
 *
 */
DPL_IOL_DATA_SEG_PREFIX
StateT DDL_EH_State;
DPL_IOL_DATA_SEG_POSTFIX

/**
 *  \brief DL Event handler Status Code
 *
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.8.1, p. 99, A.6.3, p. 210
 *
 */
DPL_IOL_DATA_SEG_PREFIX
UInteger8T DDL_EH_StatusCode;
DPL_IOL_DATA_SEG_POSTFIX

/**
 *  \brief DL Event handler event buffer
 *
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.8.1, p. 99, A.6.3, p. 210
 *
 */
DPL_IOL_DATA_SEG_PREFIX
DDL_EH_EventT DDL_EH_EventBuffer[DDL_EH_EVENT_BUFFER_SIZE];
DPL_IOL_DATA_SEG_POSTFIX

/**
 *  \brief DL Event handler event buffer length
 *
 *  Internal variable.
 *
 */
DPL_IOL_DATA_SEG_PREFIX
UInteger8T DDL_EH_EventBufferLength;
DPL_IOL_DATA_SEG_POSTFIX

DPL_IOL_DATA_SEG_END

/**
 *  \brief DL Event handler init
 *
 *  Internal function.
 *  It should be called from DDL_OH_Init()
 *
 */
void DDL_EH_Init (void)
{
  DDL_MEH_Message.isEvent = FALSE;                                              // clear event flag in message handler
  DDL_EH_StatusCode = DDL_EH_EVENT_STATUS_EMPTY;                                // clear event status
  DDL_EH_EventBufferLength = 0;                                                 // reset event buffer
  DDL_EH_State = DDL_EH_IDLE_1;                                                 // all events are changeable  
}

/**
 *  \brief DL Event handler main cycle
 *
 *  Internal function.
 *  confirm to Device AL end of event sending to Master
 *
 */
void DDL_EH_MainCycle (void)
{
  if (DDL_EH_State == DDL_EH_CONFIRM_TO_AL_4)
  {
    DDL_EH_Init();
    DDL_EventTriger_cnf();                                                      // confirm it to AL
  }
}

/**
 *  \brief DL Event handler OD
 *
 *  Internal function.
 *  DL Event OD handler
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.8.4, p. 101
 *
 */
void DDL_EH_OD_ind (EnumT rwDirection, UInteger8T address)
{
  switch (DDL_EH_State)
  {
    case DDL_EH_IDLE_1:
      if (rwDirection == DDL_MEH_OD_RWDIR_READ)
      {                                                                         // event read
        DDL_EH_ReadEvent(address);
      }
      break;
    case DDL_EH_FREEZE_EVENT_TABLE_2:
      if (rwDirection == DDL_MEH_OD_RWDIR_READ)
      {                                                                         // event read
        DDL_EH_ReadEvent(address);
      }
      else if (rwDirection == DDL_MEH_OD_RWDIR_WRITE && address == 0)
      {                                                                         // event conf
        DDL_MEH_Message.isEvent = FALSE;                                        // clear event flag in message handler
        DDL_EH_State = DDL_EH_WAIT_CONFIRM_CYCLE_3;        
      }
      break;
  }
}

/**
 *  \brief DL Event handler event read confirmation
 *
 *  Internal function.
 *  Finish event buffer transfer to master,
 *  reset it and confirm it to AL
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.8.4, p. 101
 *
 */
void DDL_EH_ReadConf (void)
{
  if (DDL_EH_State == DDL_EH_WAIT_CONFIRM_CYCLE_3)
  {
    DDL_EH_State = DDL_EH_CONFIRM_TO_AL_4;                
  }
}

/**
 *  \brief DL Event handler read event
 *
 *  Internal function.
 *  Transfer event buffer to message handler
 *  and than to master
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.3.8.4, p. 101
 *
 *  \param address address in event buffer
 *           See: IO-Link spec v1.1.1 Oct 2011, 7.3.8.1, p. 99
 *
 */
void DDL_EH_ReadEvent (UInteger8T address)
{
  UInteger8T odIndex;
  UInteger8T eventBufferByte;
  UInteger8T eventBufferNum;
  UInteger8T eventBufferNumByte;

  if (address == 0)
  {
    if (DDL_EH_StatusCode != 0x81)
      address++;
    
    eventBufferByte = DDL_EH_StatusCode;                                         // send event status code
  }
  else
  {                                                                             // send event buffer byte
    eventBufferNum     = (address - 1) / 3;
    eventBufferNumByte = (address - 1) % 3;

    switch (eventBufferNumByte)
    {
      case 0:                                                                   // event status code
        eventBufferByte = 0;
        eventBufferByte |= DDL_EH_EventBuffer[eventBufferNum].instance;
        eventBufferByte |= DDL_EH_EventBuffer[eventBufferNum].type;
        eventBufferByte |= DDL_EH_EventBuffer[eventBufferNum].mode;
        eventBufferByte |= DDL_EH_EventBuffer[eventBufferNum].source;
        break;
      
      case 1:                                                                   // event code MSB        
        eventBufferByte = (UInteger8T)
                         ((DDL_EH_EventBuffer[eventBufferNum].code >> 8) & 0xFF);
        if (eventBufferByte == 0)
          address++;
        break;
      case 2:                                                                   // event code LSB
        eventBufferByte = (UInteger8T)
                              ((DDL_EH_EventBuffer[eventBufferNum].code) & 0xFF);
        break;
    }
  }

  DDL_MEH_Message.od[0] = eventBufferByte;
  for (odIndex = 1; odIndex < DDL_MEH_Message.odLength; odIndex++)
    DDL_MEH_Message.od[odIndex] = 0;                                             // null other OD bytes
}

/**
 *  \brief DL Event handler event indication
 *
 *  This function is used by Application layer to
 *  indicate about new event to Event handler.
 *  It should be called only
 *  if there is no transfer of event buffer to master,
 *  before DDL_EventTriger_req() and after DDL_EventTriger_cnf().
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.1.15, p. 64
 *
 *  \param event    event has type DDL_EH_EventT
 *          with following fields:
 *
 *    instance event instance, can be
 *          #DDL_EH_EVENT_INSTANCE_UNKNOWN
 *          #DDL_EH_EVENT_INSTANCE_APPL
 *
 *    type     event type, can be
 *          #DDL_EH_EVENT_TYPE_RESERVED
 *          #DDL_EH_EVENT_TYPE_NOTIFICATION
 *          #DDL_EH_EVENT_TYPE_WARNING
 *          #DDL_EH_EVENT_TYPE_ERROR
 *
 *    mode     event mode, can be
 *          #DDL_EH_EVENT_MODE_RESERVED
 *          #DDL_EH_EVENT_MODE_SINGLE_SHOT
 *          #DDL_EH_EVENT_MODE_DISAPPEARS
 *          #DDL_EH_EVENT_MODE_APPEARS
 *
 *    code    event code
 *          See: IO-Link spec v1.1.1 Oct 2011, A.6.5, p. 212, Annex D, p. 235
 *
 */
void DDL_Event_req (DDL_EH_EventT * event)
{
  
  if (DDL_EH_State == DDL_EH_IDLE_1 && 
      DDL_EH_EventBufferLength < DDL_EH_EVENT_BUFFER_SIZE)
  {                                                                             // add event to buffer
    DDL_EH_EventBuffer[DDL_EH_EventBufferLength].instance = event->instance;
    DDL_EH_EventBuffer[DDL_EH_EventBufferLength].type     = event->type;
    DDL_EH_EventBuffer[DDL_EH_EventBufferLength].mode     = event->mode;
    DDL_EH_EventBuffer[DDL_EH_EventBufferLength].code     = event->code;
    DDL_EH_EventBuffer[DDL_EH_EventBufferLength].source   = 
                                                      DDL_EH_EVENT_SOURCE_REMOTE;
    
    DDL_EH_SET_EVENT_IN_STATUS(DDL_EH_EventBufferLength);
    DDL_EH_EventBufferLength++;
  }
}

/**
 *  \brief DL Event handler event trigger request
 *
 *  This function is used by Application layer to
 *  start transfer of device events to master
 *  and freeze the event buffer.
 *  DDL_EH_ReadConf() indicates that the event buffer
 *  is realized.
 *  See: IO-Link spec v1.1.1 Oct 2011, 7.2.1.17, p. 65
 *
 */
void DDL_EventTriger_req (void)
{
  if (DDL_EH_State == DDL_EH_IDLE_1)
  {    
    DDL_EH_State = DDL_EH_FREEZE_EVENT_TABLE_2;
    DDL_MEH_Message.isEvent = TRUE;                                             // set event flag in message handler
  }
}

/** \} */ // end of DDL_Event_Handler

/** \} */ // end of Data_Link_Layer

