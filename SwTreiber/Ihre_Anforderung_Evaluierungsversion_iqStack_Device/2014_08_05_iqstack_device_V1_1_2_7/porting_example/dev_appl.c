//******************************************************************************
//
//     Copyright © 2011-2013 by IQ² Development GmbH, Neckartenzlingen 
//     All rights reserved
//
//******************************************************************************

/**
 * \mainpage iqStack device integration manual
 *
 * \section desc_sec Introduction
 *
 * This project is dedicated to the implementation of IO-Link protocol (SDCI)
 * and based on IO-Link Interface and System specification v1.1.1 from Oct 2011:
 * http://io-link.com/en/Download/Download.php. 
 * Please get to know this specification before using this code.
 * 
 * The project consists of device IO-Link protocol 
 * and hardware porting interface. 
 * Integration of this code for device consists of
 * two parts: 
 * 1. porting this code to specific hardware, used to launch it
 * 2. creation of necessary application using the interface of this protocol relisation
 * 
 * The project is cross-platform and created using ANSI C programming language. 
 * All code is commented with references to IO-Link specification. 
 * To integrate the stack, please, read these comments to get more information about porting.
 *
 * This code can be used free only for getting to know. 
 * For any commercial purposes, please, contact the developer 
 * http://www.iq2-development.de/kontakt.
 * 
 * \section porting_sec Porting device physical layer
 *
 * Some specific features should be defined 
 * for correct and optimized functioning of this code. 
 * They depend on hardware platform,
 * which will be used to launch this code.
 * 
 * dev_pl.h:
 * 1. data type definitions
 * 2. Support of connection with IO-Link master 1.0,
 *    using define DPL_BACK_COMPATIBLE_RevID_10
 * 3. SDCI message maximum byte length
 * 4. ISDU maximum byte length
 * 5. if DPL_USE_DEVICE_RESPONSE_DELAY is switched off,
 *    device responses to the master immediately after processing its request
 *    else with delay, which can be defined in DPL_StartTimer() 
 *    for case DPL_DEVICE_RESPONSE_DELAY (dev_porting.c).
 * 6. supported by device SDCI baudrate: COM1, COM2 or COM3
 * 7. data segment definitions, if it is necessary to place all variables
 *    of this project in some special device data segment.
 * 8. DPL_Wakeup_ind() should be called 
 *    if master wake-up impulse request has been received.
 * 9. DPL_Transfer_ind() should be called if a byte has been received from master.

 * dev_pl.c:
 * 1. hardware and software timer definition (DPL_StartTimer, DPL_SWTimerCallback).
 *    Software timers may be not used (replaced by hardware). 
 *    They are used for long delays and implemented based on some hardware tick.
 *    - TDSIO and TFBD delay
 *    - master message interbyte timeout
 *    - device response delay
 *    - master cycle timeout can be used 
 *      to observe keeping by master some cycle time (communication integrity)
 * 2. this io-link implemention responses to master with full byte message (not bytewise),
 *    using DPL_TransferBuffer_req()
 * 3. DPL_Mode_req() is used for setting SDCI physical line mode,
 *    actions should defined for switching to:
 *    - supported SIO modes (inactive, digital input/output)
 *    - supported communication modes (COM1, COM2 or COM3)
 *    - preoperate and operate (optional, if necessary)
 * 
 *
 * \section appl_integration_sec Specific device application integration manual
 *
 * Device application consists of:
 * 1. io-link initialisation should be done before using io-link protocol (call (DA_Init())):
 *    - setting io-link communication properties
 *    - setting device identification parameters
 *    - start SIO communication
 * 2. definition of actions within device main cycle iteration (forever loop DA_MainCycle()),
 *    this actions can be interrupted by hardware
 *    - process data handling: 
 *      setting input by DAL_SetInput_req() and its validity DAL_Control_req(), 
 *      getting output by DAL_GetOutput_req() and its validity by callback DAL_Control_ind()
 *    - sending events by DAL_Event_req() 
 *      and waiting conformation by callback DAL_Event_cnf()
 * 3. definition of io-link protocol callback actions:
 *    - DSM_DeviceMode() indicates protocol mode change (startup, preoperate, operate)
 *    - DSM_SwitchToRevID10_ind() should be used for some actions in application,
 *      related to switching to legacy version of io-link
 *    - DAL_Read_ind() indicates master ISDU read request, 
 *      should be confirm after processing (may be later) 
 *      by DAL_Read_rsp() if result is successful or
 *      by DAL_Read_err() with an error.
 *    - DAL_Write_ind() indicates master ISDU write request, 
 *      should be confirm after processing (may be later)
 *      by DAL_Write_rsp() if result is successful or
 *      by DAL_Write_err() with an error.
 *    - DAL_Abort_ind() indicates that master has aborted its current ISDU request
 *    - DAL_PDCycle_ind() indicates a new cycle of io-link communication
 *    - DAL_NewOutput_ind() indicates a new output data from master (similar to DAL_PDCycle_ind())
 *    - DDL_MEH_MHInfo_Commloss() indicates a communication loss, 
 *      can be used by actuators for some standard actions
 *
 *
 */

#include "dev_iol.h"
#include "dev_appl.h"

/**
 *  \defgroup Device_application
 *
 *  Device application
 *  See: IO-Link spec v1.1.1 Oct 2011, 10, p. 151
 *  \{
 */

/**
 *  \brief Device application main function example
 *
 *  Internal function.
 *  This function is an example of
 *  io-link device project main function.
 *  It consists of io-link protocol initialization
 *  and forever loop, which is responsible
 *  for all communication actions,
 *  while device is functioning.
 *
 */ 
void dev_main (void)
{
  DA_Init();
  
  while(1)                                                                      // do forever
  {  
    DA_MainCycle(); 
  }
  
}

/**
 *  \brief Device application initialization
 *
 *  Internal function.
 *  This function should be called only once
 *  while device initializing
 *  before start using io-link and
 *  before device main forever loop !!!
 *
 */
void DA_Init (void)
{    
  DEV_IOL_Init();
  
  DSM_SetDeviceMode(DSM_MODE_IDLE);

  DA_InitDSM();

  DSM_SetDeviceMode(DSM_MODE_SIO);
}

/**
 *  \brief Init device SM comm and ident
 *
 *  Internal function.
 *  Init of stack system manager
 *  communication and identification parameters
 *  before start of IO-Link.
 *
 */
void DA_InitDSM (void)
{
  DSM_DeviceComT   * DeviceCom;
  DSM_DeviceIdentT * DeviceIdent;
  
  DeviceCom = DSM_GetDeviceCom();
  
  // sample of device communication configuration
  DeviceCom->supportedSIOMode = DPL_MODE_INACTIVE;
  DeviceCom->supportedBaudrate = DPL_SUPPORTED_BAUDRATE;
  DeviceCom->minCycleTimeBase = DSM_TIME_BASE_01;
  DeviceCom->minCycleTimeMultiplier = 10;
  DeviceCom->msequenceODLengthPreoper = DSM_MSEQ_OD_LENGTH_8;
  DeviceCom->msequenceODLengthOper = DSM_MSEQ_OD_LENGTH_2;
  DeviceCom->isISDUSupported = TRUE;
  DeviceCom->revisionID = 0x11;
  DeviceCom->isBitLengthPDIn = TRUE;
  DeviceCom->lengthPDIn = 16;
  DeviceCom->isBitLengthPDOut = TRUE;
  DeviceCom->lengthPDOut = 0;
    
  DeviceIdent = DSM_GetDeviceIdent();
  
  // sample of device identification parameters  
  DeviceIdent->vendorID_HI  = 0x12;
  DeviceIdent->vendorID_LO  = 0x34;

  DeviceIdent->deviceID_HI  = 0x12;
  DeviceIdent->deviceID_MID = 0x34;
  DeviceIdent->deviceID_LO  = 0x56;

  DeviceIdent->functionID_HI  = 0;
  DeviceIdent->functionID_LO  = 0;
  
  if (DSM_SetDeviceCom() != SERVICE_RESULT_NO_ERROR)                            
  {
    // some error in init communication parameters, see dev_sm.c
  }
  
  if (DSM_SetDeviceIdent() != SERVICE_RESULT_NO_ERROR)                            
  {
    // some error in init communication parameters, see dev_sm.c
  }  
}

/**
 *  \brief Device application main cycle
 *
 *  Internal function.
 *  This function should be called
 *  in device main forever loop
 *  while using io-link and after DA_Init()
 *
 */
void DA_MainCycle (void)
{ 
    DEV_IOL_MainCycle();
      
    // Process data input valid/invalid handling
    // by DAL_Control_req()
    // ...
    
    // Event handling
    // by DAL_Event_req() and DAL_Event_cnf()
    // ...        
    // sample of sending event to master
    // event.instance = DDL_EH_EVENT_INSTANCE_APPL;
    // event.type     = DDL_EH_EVENT_TYPE_NOTIFICATION;
    // event.mode     = DDL_EH_EVENT_MODE_SINGLE_SHOT;
    // event.source   = DDL_EH_EVENT_SOURCE_REMOTE;
    // event.code     = 0x4210;        
    // DAL_Event_req(1, &event);
    // wait DAL_Event_cnf() callback (see bellow), 
    // next events can be send only after it.
  
    // other actions
}

/** \} */ // end of Device_application

// This function is used by System manager
// to indicate changes of communication states to the
// Device application.
// It should be defined in Device application.
// See: IO-Link spec v1.1.1 Oct 2011, 9.3.2.7, p.144
// see comments in dev_sm.h
void DSM_DeviceMode (DSM_ModeT mode)
{
  switch (mode)
  {
    case DSM_MODE_IDLE:
      
      DA_InitDSM();
      DSM_SetDeviceMode(DSM_MODE_SIO);

      break;

    case DSM_MODE_SIO:

      break;

    case DSM_MODE_ESTABCOM:

      break;

    case DSM_MODE_COM1:

      break;

    case DSM_MODE_COM2:

      break;

    case DSM_MODE_COM3:

      break;

    case DSM_MODE_IDENT_STARTUP:

      break;

    case DSM_MODE_IDENT_CHANGE:

      // DeviceCom = DSM_GetDeviceCom();  
      // DeviceIdent = DSM_GetDeviceIdent();
           
      // This mode indication means, that master has not accepted
      // init device parameters, set in DA_InitDSM().
      // Master could change DeviceCom->revisionID and DeviceIdent->deviceID_XX.
      // They can be checked here and set to the compatible ones.
      // In this case delete call DA_InitDSM() at the end of current case.
      // See: IO-Link spec v1.1.1 Oct 2011, 9.3.3.2, Table 93, p.146,
      //      SM_IdentStartup_4 and SM_IdentCheck_5 states of SM
      // 
      // if change of its configuration is not supported by device,
      // it can be just reinitialised once more by DA_InitDSM(),
      // as made by default in this example at the end of current case
      
      // if (DSM_SetDeviceCom() != SERVICE_RESULT_NO_ERROR)                            
      //{
      // some error in init communication parameters, see dev_sm.c
      //}
      
      // if (DSM_SetDeviceIdent() != SERVICE_RESULT_NO_ERROR)                            
      //{
      // some error in init communication parameters, see dev_sm.c
      //}  
      
      DA_InitDSM();

      break;

    case DSM_MODE_PREOPERATE:

      break;

    case DSM_MODE_OPERATE:

      break;

    default:

      break;

  }
}

#ifdef DPL_BACK_COMPATIBLE_RevID_10
// This function is used by System manager
// to indicate switching to RevID 1.0 to the
// Device application, because of connection with old IO-Link master 1.0
// see comments in dev_sm.h
// It should be processed quickly !!!
// because it is called from DPL_Transfer_ind()
// (mainly by interrupt)
void DSM_SwitchToRevID10_ind(void)
{
  
}
#endif

// This function is used by Application layer
// to indicate OD read to Device application.
// It should be defined in Device application.
// See: IO-Link spec v1.1.1 Oct 2011, 8.2.2.1, p. 104
// see comments in dev_al.h
void DAL_Read_ind (UInteger16T index, UInteger8T subindex, UInteger8T * data)
{
  // sample of positive read response
  // copy response data to parameter "UInteger8T * data"
  // and call DAL_Read_rsp() with response byte length (max 232 bytes):
  // data[0] = data_byte0;
  // data[1] = data_byte1;
  // ...
  // DAL_Read_rsp(response_length);
  //
  // sample of negative read response
  // in case of application error call DAL_Read_err()
  // with IO-Link error code and additional error code:
  // DAL_Read_err(error_code, additional_error_code);
  
  switch(index)
  {
                                                                                // Index assignment of data objects (predefined device parameters)
                                                                                // See: IO-Link spec v1.1.1 Oct 2011, B.2, Table B.8, p.220
    case 0x10:
                                                                                // VendorName
      break;

    case 0x11:
                                                                                // VendorText
      break;

    case 0x12:
                                                                                // ProductName
      break;

    case 0x13:
                                                                                // ProductID
      break;

    case 0x14:
                                                                                // ProductText
      break;

    case 0x15:
                                                                                // SerialNumber
      break;

    case 0x16:      
                                                                                // HardwareRevision
      break;
      
    default:
      // sample of negative read response
      DAL_Read_err(0x80, 0x11);
      return;
  }
}

// This function is used by Application layer
// to indicate OD write to Device application.
// It should be defined in Device application.
// See: IO-Link spec v1.1.1 Oct 2011, 8.2.2.2, p. 105
// see comments in dev_al.h
void DAL_Write_ind (UInteger16T index, UInteger8T subindex, 
                    UInteger8T  dataLength, UInteger8T * data)
{
  // sample of positive write response
  // DAL_Write_rsp();
  //
  // sample of negative read response
  // in case of application error call DAL_Write_err()
  // with IO-Link error code and additional error code:
  // DAL_Write_err(error_code, additional_error_code);
}

// This function is used by Application layer
// to indicate the aborting of current
// DAL_Read or DAL_Write services by Master.
// It abandons the response to DAL_Read or DAL_Write services.
// It should be defined in Device application.
// See: IO-Link spec v1.1.1 Oct 2011, 8.2.2.3, p. 106
// see comments in dev_al.h
// It should be processed quickly !!!
// because it is called from DPL_Transfer_ind()
// (mainly by interrupt)
void DAL_Abort_ind (void)
{

}

// This function is used by Application layer
// to indicate PD Out validity to Device application.
// It should be defined in Device application.
// See: IO-Link spec v1.1.1 Oct 2011, 8.2.2.12, p. 111
// see comments in dev_al.h
// It should be processed quickly !!!
// because it is called from DPL_Transfer_ind()
// (mainly by interrupt)
// controlCode:
// #TRUE  - pd out valid
// #FALSE - pd out invalid
void DAL_Control_ind (BooleanT controlCode)
{

}

// This function is used by Application layer
// to indicate the end of a process data cycle. 
// The Device application can use this service 
// to transmit new input data to the application layer via DAL_SetInput_req().
// It should be defined in Device application.
// See: IO-Link spec v1.1.1 Oct 2011, 8.2.2.7, p.108
// see comments in dev_al.h
// It should be processed quickly !!!
// because it is called from DPL_Transfer_ind()
// (mainly by interrupt)
void DAL_PDCycle_ind (void)
{
  // Process data handling, for example, copy to buffer
  // by DAL_SetInput_req(), DAL_GetOutput_req()
  // ...
}

// This function is used by Application layer
// to indicate the receiption of updated valid output data from master.
// It should be defined in Device application.
// See: IO-Link spec v1.1.1 Oct 2011, 8.2.2.9, p.109
// see comments in dev_al.h
// It should be processed quickly !!!
// because it is called from DPL_Transfer_ind()
// (mainly by interrupt)
void DAL_NewOutput_ind (void)
{
  // Process data handling, for example, copy to buffer
  // by DAL_GetOutput_req()
  // ...
}

// This function is used by Application layer
// to confirm the receiption of sent events by master.
// If events have been sent by DAL_Event_req(),
// next events can be sent only after receiption of this confirmation.
// It should be defined in Device application.
// See: IO-Link spec v1.1.1 Oct 2011, 8.2.2.11, p.110
// see comments in dev_al.h
void DAL_Event_cnf (void)
{
  
}

// The service MHInfo signals an exceptional
// operation within the message handler (COMMLOST).
// Actuators shall observe this information
// and take corresponding actions.
// See: IO-Link spec v1.1.1 Oct 2011, 7.2.2.6,  p. 69, 7.3.3.5, p. 86
// see comments in dev_dl_meh.h
void DDL_MEH_MHInfo_Commloss (void)
{
  
}
