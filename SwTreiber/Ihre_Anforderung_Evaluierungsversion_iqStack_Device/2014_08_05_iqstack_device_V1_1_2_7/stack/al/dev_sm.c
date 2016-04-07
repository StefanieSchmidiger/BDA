//******************************************************************************
//
//     Copyright © 2011-2013 by IQ² Development GmbH, Neckartenzlingen 
//     All rights reserved
//
//******************************************************************************

#include "dev_iol.h"

/**
 *  \defgroup System_manager
 *
 *  System manager
 *  See: IO-Link spec v1.1.1 Oct 2011, 9, p. 122
 *  \{
 */

DPL_IOL_DATA_SEG_START

/**
 *  \brief System manager state
 *
 *  Internal variable.
 *
 */
DPL_IOL_DATA_SEG_PREFIX
DSM_StateT DSM_State;
DPL_IOL_DATA_SEG_POSTFIX

/**
 *  \brief System manager device communication configuration
 *
 *  Internal variable.
 *
 */
DPL_IOL_DATA_SEG_PREFIX
DSM_DeviceComT DSM_DeviceCom;
DPL_IOL_DATA_SEG_POSTFIX

/**
 * \brief System manager device identification data
 *
 *  Internal variable.
 *
 */
DPL_IOL_DATA_SEG_PREFIX
DSM_DeviceIdentT DSM_DeviceIdent;
DPL_IOL_DATA_SEG_POSTFIX

DPL_IOL_DATA_SEG_END

/**
 *  \brief System manager init
 *
 *  Internal function.
 *  It should be called from DEV_IOL_Init()
 */
void DSM_Init (void)
{
  DSM_State = DSM_STATE_IDLE_0;
  DSM_DeviceCom.isDeviceModeChanged = FALSE;
}

/**
 *  \brief System manager main cycle handler
 *
 *  Internal function.
 *  Inform device application
 *  about new io-link mode is reached
 *  and should be processed
 *  in main device application cycle.
 *
 */
void DSM_MainCycle (void)
{
  if (DSM_DeviceCom.isDeviceModeChanged == TRUE)
  {
    DSM_DeviceCom.isDeviceModeChanged = FALSE;

    DSM_DeviceMode(DSM_DeviceCom.deviceMode);
  }
}

/**
 *  \brief System manager pd length bits to bytes converter
 *
 *  Internal function.
 *  Process data length bits to bytes converter
 *
 */
UInteger8T DSM_PDLenBits2Bytes (UInteger8T bitNum)
{
  UInteger8T byteNum = 0;  
  byteNum = bitNum >> 3;
  if ((bitNum - (byteNum << 3)) != 0) byteNum++;  
  return byteNum;
}

/**
 *  \brief System manager set PD configuration
 *
 *  Internal function.
 *  Configuration and packing to direct page of process data
 *  See: IO-Link spec v1.1.1 Oct 2011, B.1.7-8, p. 217-218
 *
 *  \return result of operation, can be:
 *      #SERVICE_RESULT_NO_ERROR
 *      #SERVICE_RESULT_STATE_CONFLICT - some error
 *
 */
ResultT DSM_SetDevicePDConfig (void)
{
  if (DSM_DeviceCom.supportedSIOMode != DPL_MODE_INACTIVE)                      // pack PD In
    DSM_DeviceCom.processDataIn = 0x40;                                         // SIO mode supported
  else
    DSM_DeviceCom.processDataIn = 0;                                            // SIO mode not supported

  if (DSM_DeviceCom.lengthPDIn != 0)
  {
    if (DSM_DeviceCom.isBitLengthPDIn == TRUE)
    {
      if (DSM_DeviceCom.lengthPDIn > 16)
        return SERVICE_RESULT_STATE_CONFLICT;

      DSM_DeviceCom.processDataIn |= DSM_DeviceCom.lengthPDIn;
      
      DSM_DeviceCom.lengthPDInBytes =                                             
                                  DSM_PDLenBits2Bytes(DSM_DeviceCom.lengthPDIn);// define PD In length for MEH
    }
    else
    {
      if (DSM_DeviceCom.lengthPDIn < 3 || DSM_DeviceCom.lengthPDIn > 32)
        return SERVICE_RESULT_STATE_CONFLICT;

      DSM_DeviceCom.processDataIn |= 0x80;
      DSM_DeviceCom.processDataIn |= DSM_DeviceCom.lengthPDIn - 1;
      
      DSM_DeviceCom.lengthPDInBytes  = DSM_DeviceCom.lengthPDIn;                // define PD In length for MEH
    }
  }
  else
    DSM_DeviceCom.lengthPDInBytes = 0;

  DSM_DeviceCom.processDataOut = 0;                                             // pack PD Out

  if (DSM_DeviceCom.lengthPDOut != 0)
  {
    if (DSM_DeviceCom.isBitLengthPDOut == TRUE)
    {
      if (DSM_DeviceCom.lengthPDOut > 16)
        return SERVICE_RESULT_STATE_CONFLICT;

      DSM_DeviceCom.processDataOut |= DSM_DeviceCom.lengthPDOut;
      
      DSM_DeviceCom.lengthPDOutBytes =                                           
                                 DSM_PDLenBits2Bytes(DSM_DeviceCom.lengthPDOut);// define PD Out length for MEH
    }
    else
    {
      if (DSM_DeviceCom.lengthPDOut < 3 || DSM_DeviceCom.lengthPDOut > 32)
        return SERVICE_RESULT_STATE_CONFLICT;

      DSM_DeviceCom.processDataOut |= 0x80;
      DSM_DeviceCom.processDataOut |= DSM_DeviceCom.lengthPDOut - 1;
      
      DSM_DeviceCom.lengthPDOutBytes  = DSM_DeviceCom.lengthPDOut;               // define PD Out length for MEH
    }
  }
  else
    DSM_DeviceCom.lengthPDOutBytes = 0;
  
  return SERVICE_RESULT_NO_ERROR;
}

/**
 *  \brief System manager set M-sequence configuration
 *
 *  Internal function.
 *  Configuration and packing to direct page of M-sequence capability,
 *  M-sequence type and On-request data size.
 *  See: IO-Link spec v1.1.1 Oct 2011, A.2.6, p. 200-201, Table A.8-10
 *
 *  \return result of operation, can be:
 *      #SERVICE_RESULT_NO_ERROR
 *      #SERVICE_RESULT_STATE_CONFLICT - some error
 *
 */
ResultT DSM_SetMseqConfig (void)
{ 
#ifdef DPL_BACK_COMPATIBLE_RevID_10 
  DSM_DeviceCom.isInterleave = FALSE;

  if (DSM_DeviceCom.revisionID == 0x10)                                         // RevID 1.0 
  { 
    DSM_DeviceCom.msequenceCapabilityPreoperate = 0;

    if (DSM_DeviceCom.lengthPDIn == 0 && DSM_DeviceCom.lengthPDOut == 0)          
    {
      if (DSM_DeviceCom.msequenceODLengthOper == DSM_MSEQ_OD_LENGTH_1)
      {        
        DSM_DeviceCom.msequenceTypeOper = DDL_MEH_MESSAGE_TYPE_0;
      }
      else
      {
        DSM_DeviceCom.msequenceODLengthOper = DSM_MSEQ_OD_LENGTH_2;      
        DSM_DeviceCom.msequenceCapabilityOperate = 1;        
        DSM_DeviceCom.msequenceTypeOper = DDL_MEH_MESSAGE_TYPE_1;
      }
    }
    else
    {
      if (DSM_DeviceCom.lengthPDInBytes + DSM_DeviceCom.lengthPDOutBytes > 2)
      {
        // interleave mode
        DSM_DeviceCom.isInterleave = TRUE;
        DSM_DeviceCom.msequenceODLengthOper = DSM_MSEQ_OD_LENGTH_2;      
        DSM_DeviceCom.msequenceTypeOper = DDL_MEH_MESSAGE_TYPE_1;
      }
      else
      {
        DSM_DeviceCom.msequenceODLengthOper = DSM_MSEQ_OD_LENGTH_1;      
        DSM_DeviceCom.msequenceTypeOper = DDL_MEH_MESSAGE_TYPE_2;
      }
    }
  }
  else                                                                          // RevID 1.1
#endif
  {  
    DSM_DeviceCom.msequenceTypePreoper = DDL_MEH_MESSAGE_TYPE_1;
      
    switch (DSM_DeviceCom.msequenceODLengthPreoper)                             // define Mseq preoperate capability RevID 1.1
    {                                                                             
      case DSM_MSEQ_OD_LENGTH_1:
        DSM_DeviceCom.msequenceTypePreoper = DDL_MEH_MESSAGE_TYPE_0;
        DSM_DeviceCom.msequenceCapabilityPreoperate = 0;
        break;
      case DSM_MSEQ_OD_LENGTH_2:
        DSM_DeviceCom.msequenceCapabilityPreoperate = 1;
        break;
      case DSM_MSEQ_OD_LENGTH_8:
        DSM_DeviceCom.msequenceCapabilityPreoperate = 2;
        break;
      case DSM_MSEQ_OD_LENGTH_32:
        DSM_DeviceCom.msequenceCapabilityPreoperate = 3;
        break;
      default:
        return SERVICE_RESULT_STATE_CONFLICT;
    }
    
    switch (DSM_DeviceCom.msequenceODLengthOper)                                // define Mseq operate capability RevID 1.1
    {                                                                             
      case DSM_MSEQ_OD_LENGTH_1:        
        if ((DSM_DeviceCom.lengthPDIn  != 0 && DSM_DeviceCom.isBitLengthPDIn  == FALSE) ||
            (DSM_DeviceCom.lengthPDOut != 0 && DSM_DeviceCom.isBitLengthPDOut == FALSE))
          DSM_DeviceCom.msequenceCapabilityOperate = 4;
        else
          DSM_DeviceCom.msequenceCapabilityOperate = 0;          
        break;
  
      case DSM_MSEQ_OD_LENGTH_2:        
        if (DSM_DeviceCom.lengthPDIn == 0 && DSM_DeviceCom.lengthPDOut == 0)
          DSM_DeviceCom.msequenceCapabilityOperate = 1;
        else
          DSM_DeviceCom.msequenceCapabilityOperate = 5;          
        break;
        
      case DSM_MSEQ_OD_LENGTH_8:        
        DSM_DeviceCom.msequenceCapabilityOperate = 6;
        break;
        
      case DSM_MSEQ_OD_LENGTH_32:        
        DSM_DeviceCom.msequenceCapabilityOperate = 7;
        break;
        
      default:
        return SERVICE_RESULT_STATE_CONFLICT;
    }
    
    if (DSM_DeviceCom.lengthPDIn == 0 && DSM_DeviceCom.lengthPDOut == 0)        // define m seq type for operate RevID 1.1
      if (DSM_DeviceCom.msequenceODLengthOper == DSM_MSEQ_OD_LENGTH_1)          // no interleave mode 
        DSM_DeviceCom.msequenceTypeOper = DDL_MEH_MESSAGE_TYPE_0;
      else
        DSM_DeviceCom.msequenceTypeOper = DDL_MEH_MESSAGE_TYPE_1;
    else
      DSM_DeviceCom.msequenceTypeOper = DDL_MEH_MESSAGE_TYPE_2;

  }
  
  DSM_DeviceCom.msequenceCapability = 0 |                                       // pack Mseq capability
                    ((DSM_DeviceCom.msequenceCapabilityPreoperate << 4) & 0x30) |
                    ((DSM_DeviceCom.msequenceCapabilityOperate    << 1) & 0x0E) |
                    ((DSM_DeviceCom.isISDUSupported == TRUE) ? 1 : 0 );
  
  return SERVICE_RESULT_NO_ERROR;
}

/**
 *  \brief SM set device communication configuration
 *
 *  This function is used to configure the communication properties supported
 *  by the Device in the System Management.
 *  It should be called from Device application
 *  after DSM_SetDeviceMode(DSM_MODE_IDLE)
 *  and before DSM_SetDeviceIdent(), DSM_SetDeviceMode(DSM_MODE_SIO).
 *
 *  The pointer to DSM_DeviceComT can be obtained by DSM_GetDeviceCom().
 *  The following fields of this DSM_DeviceComT should be defined before call:
 *
 *  - supportedSIOMode
 *  - supportedBaudrate
 *  - minCycleTimeBase   
 *  - minCycleTimeMultiplier 
 *  - msequenceODLengthPreoper
 *  - msequenceODLengthOper
 *  - isISDUSupported
 *  - revisionID
 *  - isBitLengthPDIn
 *  - lengthPDIn
 *  - isBitLengthPDOut
 *  - lengthPDOut
 *
 *  See these fields description in DSM_DeviceComT definition.
 *
 *  See: IO-Link spec v1.1.1 Oct 2011, 9.3.2.2, p.139
 *
 *  \return result of operation, can be:
 *      #SERVICE_RESULT_NO_ERROR
 *      #SERVICE_RESULT_STATE_CONFLICT - some error
 *
 */
ResultT DSM_SetDeviceCom (void)
{     
  DSM_DeviceCom.minCycleTime = DSM_DeviceCom.minCycleTimeBase | 
                               (DSM_DeviceCom.minCycleTimeMultiplier & 0x3F);

  if (DSM_SetDevicePDConfig() != SERVICE_RESULT_NO_ERROR)
    return SERVICE_RESULT_STATE_CONFLICT;
  
  if (DSM_SetMseqConfig() != SERVICE_RESULT_NO_ERROR)
    return SERVICE_RESULT_STATE_CONFLICT;

  DSM_DeviceCom.isOK = TRUE;

  return SERVICE_RESULT_NO_ERROR;
}

/**
 *  \brief SM get actual device communication configuration
 *
 *  This function returns pointer to DSM_DeviceComT.
 *
 *  See: IO-Link spec v1.1.1 Oct 2011, 9.3.2.3, p.140
 *
 */
DSM_DeviceComT * DSM_GetDeviceCom (void)
{
  return &DSM_DeviceCom;
}

/**
 *  \brief SM set device identification data
 *
 *  This function is used to configure the Device identification data in the
 *  System Management.
 *  It should be called from Device application
 *  after DSM_SetDeviceMode(DSM_MODE_IDLE), DSM_GetDeviceCom()
 *  and before DSM_SetDeviceMode(DSM_MODE_SIO).
 *
 *  The pointer to DSM_DeviceIdentT can be obtained by DSM_GetDeviceIdent().
 *  The following fields of this DSM_DeviceIdentT should be defined before call:
 *
 *  - vendorID
 *  - deviceID
 *  - functionID
 *
 *  See these fields description in DSM_DeviceIdentT definition.
 *
 *  See: IO-Link spec v1.1.1 Oct 2011, 9.3.2.4, p.141
 *
 *  \return result of operation, can be:
 *      #SERVICE_RESULT_NO_ERROR
 *      #SERVICE_RESULT_STATE_CONFLICT - some error
 *
 */
ResultT DSM_SetDeviceIdent (void)
{
  DSM_DeviceIdent.isOK = TRUE;
  return SERVICE_RESULT_NO_ERROR;
}

/**
 *  \brief SM get actual device identification data
 *
 *  This function returns pointer to DSM_DeviceIdentT.
 *
 *  See: IO-Link spec v1.1.1 Oct 2011, 9.3.2.5, p.142
 *
 */
DSM_DeviceIdentT * DSM_GetDeviceIdent (void)
{
  return &DSM_DeviceIdent;
}

/**
 *  \brief SM set mode
 *
 *  This function is used to set the Device
 *  into a defined operational state during initialization.
 *  It should be called from Device application.
 *  See: IO-Link spec v1.1.1 Oct 2011, 9.3.2.6, p.143
 *
 *  \param mode system manager mode, can be
 *        #DSM_MODE_IDLE
 *        #DSM_MODE_SIO
 *
 *  \return result of operation, can be:
 *      #SERVICE_RESULT_NO_ERROR
 *      #SERVICE_RESULT_STATE_CONFLICT - some error
 *
 */
ResultT DSM_SetDeviceMode (EnumT mode)
{
  switch (mode)
  {
    case DSM_MODE_IDLE:
      DSM_DeviceCom.isOK = FALSE;
      DSM_DeviceIdent.isOK = FALSE;
      DSM_DeviceCom.isDeviceModeChanged = TRUE;
      DSM_DeviceCom.deviceMode = DSM_MODE_IDLE;
      DSM_State = DSM_STATE_IDLE_0;
      break;

    case DSM_MODE_SIO:
      if (DSM_State != DSM_STATE_IDLE_0)
        return SERVICE_RESULT_STATE_CONFLICT;

      if (DSM_DeviceCom.isOK != TRUE || DSM_DeviceIdent.isOK != TRUE)
        return SERVICE_RESULT_STATE_CONFLICT;

      DPL_Mode_req(DSM_DeviceCom.supportedSIOMode);

      DSM_DeviceCom.isDeviceModeChanged = TRUE;
      DSM_DeviceCom.deviceMode = DSM_MODE_SIO;

      DSM_State = DSM_STATE_SIO_1;

      break;

    default:
      return SERVICE_RESULT_STATE_CONFLICT;
  }

  return SERVICE_RESULT_NO_ERROR;
}


/** \} */ // end of System_manager

ResultT DDL_Read (UInteger8T address, UInteger8T * value)
{
  switch (address)
  {
    case 0:                                                                     // read master command
      *value = 0;
      break;
    case 1:
      *value = DSM_DeviceCom.masterCycleTime;
      break;
    case 2:
      if (DSM_State == DSM_STATE_IDENT_CHECK_5 &&
          DSM_DeviceCom.isOK == TRUE && DSM_DeviceIdent.isOK == TRUE)
      {
        DSM_State = DSM_STATE_COMP_STARTUP_6;
      }
      *value = DSM_DeviceCom.minCycleTime;
      break;
    case 3:
      *value = DSM_DeviceCom.msequenceCapability;
      break;
    case 4:
      *value = DSM_DeviceCom.revisionID;
      break;
    case 5:
      *value = DSM_DeviceCom.processDataIn;
      break;
    case 6:
      *value = DSM_DeviceCom.processDataOut;
      break;
    case 7:
      *value = DSM_DeviceIdent.vendorID_HI;
      break;
    case 8:
      *value = DSM_DeviceIdent.vendorID_LO;
      break;
    case 9:
      *value = DSM_DeviceIdent.deviceID_HI;
      break;
    case 10:
      *value = DSM_DeviceIdent.deviceID_MID;
      break;
    case 11:
      *value = DSM_DeviceIdent.deviceID_LO;
      break;
    case 12:
      *value = DSM_DeviceIdent.functionID_HI;
      break;
    case 13:
      *value = DSM_DeviceIdent.functionID_LO;
      break;
    case 14:
      *value = 0;
      break;
    case 15:
      *value = 0;
      break;

    default:
      return SERVICE_RESULT_STATE_CONFLICT;
  }

  return SERVICE_RESULT_NO_ERROR;
}

ResultT DDL_Write (UInteger8T address, UInteger8T value)
{
  switch (address)
  {
    case 0:                                                                     // master command
      switch (value)
      {
        case DDL_CH_MC_MASTER_IDENT:
          if (DSM_State == DSM_STATE_COM_STARTUP_3 ||
              DSM_State == DSM_STATE_COMP_STARTUP_6)
          {
            DSM_DeviceCom.isDeviceModeChanged = TRUE;
            DSM_DeviceCom.deviceMode = DSM_MODE_IDENT_STARTUP;
            DSM_State = DSM_STATE_IDENT_STARTUP_4;
          }
          break;

        case DDL_CH_MC_DEVICE_IDENT:
          if (DSM_State == DSM_STATE_IDENT_STARTUP_4)
          {            
            DSM_DeviceCom.isOK = FALSE;                                         // reset comm and ident settings
            DSM_DeviceIdent.isOK = FALSE;                                       // they should be checked and 
            DSM_DeviceCom.isDeviceModeChanged = TRUE;                           // set once more by device application
            DSM_DeviceCom.deviceMode = DSM_MODE_IDENT_CHANGE;                   // in DSM_DeviceMode(DSM_MODE_IDENT_CHANGE)
            DSM_State = DSM_STATE_IDENT_CHECK_5;
          }
          break;
      }
      break;

    case 1:
      DSM_DeviceCom.masterCycleTime = value;
      break;
    case 4:
      DSM_DeviceCom.revisionID = value;
      break;

    case 9:
      DSM_DeviceIdent.deviceID_HI = value;
      break;
    case 10:
      DSM_DeviceIdent.deviceID_MID = value;
      break;
    case 11:
      DSM_DeviceIdent.deviceID_LO = value;
      break;

    default:
      return SERVICE_RESULT_STATE_CONFLICT;
  }

  return SERVICE_RESULT_NO_ERROR;
}

void DDL_Mode_ind (UInteger8T realMode)
{
  switch (realMode)
  {
    case DDL_MODE_ESTABCOM:
      if (DSM_State != DSM_STATE_SIO_1) break;
      DPL_Mode_req(DSM_DeviceCom.supportedBaudrate);
      DSM_DeviceCom.isDeviceModeChanged = TRUE;
      DSM_DeviceCom.deviceMode = DSM_MODE_ESTABCOM;
      DSM_State = DSM_STATE_COM_ESTABLISH_2;
      break;

    case DDL_MODE_STARTUP:
      switch (DSM_State)
      {
        case DSM_STATE_PREOPERATE_7:
        case DSM_STATE_OPERATE_8:
          DSM_DeviceCom.isDeviceModeChanged = TRUE;
          DSM_DeviceCom.deviceMode = DSM_MODE_STARTUP;
          DSM_State = DSM_STATE_COM_STARTUP_3;
          break;
      }
      break;

    case DDL_MODE_COM1:
      if (DSM_State != DSM_STATE_COM_ESTABLISH_2) break;
      DSM_DeviceCom.isDeviceModeChanged = TRUE;
      DSM_DeviceCom.deviceMode = DSM_MODE_COM1;
      DSM_State = DSM_STATE_COM_STARTUP_3;
      break;
      
    case DDL_MODE_COM2:
      if (DSM_State != DSM_STATE_COM_ESTABLISH_2) break;
      DSM_DeviceCom.isDeviceModeChanged = TRUE;
      DSM_DeviceCom.deviceMode = DSM_MODE_COM2;
      DSM_State = DSM_STATE_COM_STARTUP_3;
      break;

    case DDL_MODE_COM3:
      if (DSM_State != DSM_STATE_COM_ESTABLISH_2) break;
      DSM_DeviceCom.isDeviceModeChanged = TRUE;
      DSM_DeviceCom.deviceMode = DSM_MODE_COM3;
      DSM_State = DSM_STATE_COM_STARTUP_3;
      break;

    case DDL_MODE_PREOPERATE:
      if (DSM_State != DSM_STATE_IDENT_STARTUP_4 && 
          DSM_State != DSM_STATE_COMP_STARTUP_6)
        break;
      DSM_DeviceCom.isDeviceModeChanged = TRUE;
      DSM_DeviceCom.deviceMode = DSM_MODE_PREOPERATE;
      DSM_State = DSM_STATE_PREOPERATE_7;      
      DPL_Mode_req(DPL_MODE_PREOPERATE);
    break;

    case DDL_MODE_OPERATE:
      if (DSM_State == DSM_STATE_COM_STARTUP_3 || 
          DSM_State == DSM_STATE_PREOPERATE_7)
      {
        #ifdef DPL_BACK_COMPATIBLE_RevID_10
          if (DSM_State == DSM_STATE_COM_STARTUP_3 &&
              DSM_DeviceCom.revisionID != 0x10)
          {
            DSM_DeviceCom.revisionID = 0x10;
            DSM_SwitchToRevID10_ind();
            if (DSM_SetMseqConfig() != SERVICE_RESULT_NO_ERROR)
              break;
          }          
        #endif        
          
        DSM_DeviceCom.isDeviceModeChanged = TRUE;
        DSM_DeviceCom.deviceMode = DSM_MODE_OPERATE;
        DSM_State = DSM_STATE_OPERATE_8;
        DPL_Mode_req(DPL_MODE_OPERATE);
      }
      break;

    case DDL_MODE_INACTIVE:
      DSM_DeviceCom.isDeviceModeChanged = TRUE;
      DSM_DeviceCom.deviceMode = DSM_MODE_IDLE;
      DSM_State = DSM_STATE_IDLE_0;
      break;
  }
}

