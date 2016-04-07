//******************************************************************************
//
//     Copyright © 2011-2013 by IQ² Development GmbH, Neckartenzlingen 
//     All rights reserved
//
//******************************************************************************

#ifndef DSM_H_
#define DSM_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  \defgroup System_manager
 *
 *  System manager
 *  See: IO-Link spec v1.1.1 Oct 2011, 9, p. 122
 *  \{
 */

/**
 *  \brief System manager states
 *
 *  System manager states
 *  See: IO-Link spec v1.1.1 Oct 2011, 9.3.3.2, p. 145-146
 *  
 */
typedef enum
{  
  /** \brief System manager idle state
   *
   *  In DSM_Idle the SM is waiting for configuration by the Device application and to be set
   *  to SIO mode. The state is left on receiving a DSM_SetDeviceMode(SIO) request from
   *  the Device application. The following sequence of services shall be executed
   *  between Device application and SM.
   *  - DSM_SetDeviceCom(initial parameter list)
   *  - DSM_SetDeviceIdent(VID, initial DID, FID)
   *
   */
  DSM_STATE_IDLE_0          = 0,
  
  /** \brief System manager SIO state
   *
   *  In DSM_SIO the SM Line Handler is remaining in the default SIO mode. The Physical
   *  Layer is set to the SIO mode characteristics defined by the Device application via the
   *  SetDeviceMode service. The state is left on receiving a DDL_Mode(STARTUP)
   *  indication.
   *
   */
  DSM_STATE_SIO_1           = 1,
  
  /** \brief System manager communication establishment state
   *
   *  In DSM_ComEstablish the SM is waiting for the communication to be established in the
   *  Data Link Layer. The state is left on receiving a DDL_Mode(INACTIVE) or a
   *  DDL_Mode(COMx) indication, where COMx may be any of COM1, COM2 or COM3.
   *
   */
  DSM_STATE_COM_ESTABLISH_2 = 2,
  
  /** \brief System manager communication startup state
   *
   *  In DSM_ComStartup the communication parameter (DP 02h to 06h) are read by the
   *  Master SM via DDL_Read requests. The state is left on receiving a
   *  DDL_Mode(INACTIVE), a DDL_Mode(OPERATE) indication (Master V1.0 only) or a
   *  DDL_Write(MASTERIDENT) request (Master >V1.0).
   *
   */
  DSM_STATE_COM_STARTUP_3   = 3,
  
  /** \brief System manager identification startup state
   *
   *  In DSM_IdentStartup the identification data (VID, DID, FID) are read and verified by the
   *  Master. In case of incompatibilities the Master SM writes the supported SDCI Revision
   *  (RID) and configured DeviceID (DID) to the Device. The state is left on receiving a
   *  DDL_Mode(INACTIVE), a DDL_Mode(PREOPERATE) indication (compatibility check
   *  passed) or a DDL_Write(DEVICEIDENT) request (new compatibility requested).
   *
   */
  DSM_STATE_IDENT_STARTUP_4 = 4,
  
  /** \brief System manager identification check state
   *
   *  In DSM_IdentCheck the SM waits for new initialization of communication and
   *  identification parameters. The state is left on receiving a DDL_Mode(INACTIVE)
   *  indication or a DDL_Read(DP 02h) request.
   *  Within this state the Device application shall check the RID and DID parameters from
   *  the SM and set these data to the supported values. Therefore the following sequence
   *  of services shall be executed between Device application and SM.
   *  - DSM_GetDeviceCom(configured RID, parameter list)
   *  - DSM_GetDeviceIdent(configured DID, parameter list)
   *  - Device application checks and provides compatibility function and parameters
   *  - DSM_SetDeviceCom(new supported RID, new parameter list)
   *  - DSM_SetDeviceIdent(new supported DID, parameter list)
   *
   */
  DSM_STATE_IDENT_CHECK_5   = 5,
  
  /** \brief System manager compatibility startup state
   *
   *  In DSM_CompatStartup the communication and identification data are reread and
   *  verified by the Master SM. The state is left on receiving a DDL_Mode(INACTIVE) or a
   *  DDL_Mode(PREOPERATE) indication.
   *
   */
  DSM_STATE_COMP_STARTUP_6  = 6,
  
  /** \brief System manager preoperate state
   *
   *  During DSM_Preoperate the the SerialNumber can be read and verified by the Master
   *  SM, as well as data storage and Device parameterization may be executed. The state
   *  is left on receiving a DDL_Mode(INACTIVE), a DDL_Mode(STARTUP) or a
   *  DDL_Mode(OPERATE) indication.
   *
   */
  DSM_STATE_PREOPERATE_7   = 7,
  
  /** \brief System manager operate state
   *
   *  During DSM_Operate the cyclic process data exchange and acyclic On-Request data
   *  transfer are active. The state is left on receiving a DDL_Mode(INACTIVE) or a
   *  DDL_Mode(STARTUP) indication.
   *
   */
  DSM_STATE_OPERATE_8      = 8
  
} DSM_StateT;

/**
 *  \brief System manager cycle time bases
 *
 *  the time base for the calculation of the MinCycleTime and MasterCycleTime
 *  See: IO-Link spec v1.1.1 Oct 2011, B.1.4, p. 216
 *  
 */
typedef enum
{
  /** \brief 0,1 ms */
  DSM_TIME_BASE_01         = 0x00,
  /** \brief 0,4 ms */
  DSM_TIME_BASE_04         = 0x40,
  /** \brief 1,6 ms */
  DSM_TIME_BASE_16         = 0x80,
  /** \brief Reserved */
  DSM_TIME_BASE_RESERVED   = 0xC0
  
} DSM_CycTimeBaseT;

/**
 *  \brief System manager modes
 *
 *  System manager modes
 *  See: IO-Link spec v1.1.1 Oct 2011, 9.3.2.7, p. 144
 *  
 */
typedef enum
{ 
  DSM_MODE_IDLE          = 0,
  DSM_MODE_SIO           = 1,
  DSM_MODE_ESTABCOM      = 2,     
  DSM_MODE_COM1          = 3,
  DSM_MODE_COM2          = 4,
  DSM_MODE_COM3          = 5,
  DSM_MODE_STARTUP       = 6,
  DSM_MODE_IDENT_STARTUP = 7,
  DSM_MODE_IDENT_CHANGE  = 8,
  DSM_MODE_PREOPERATE    = 9,
  DSM_MODE_OPERATE       = 10
  
} DSM_ModeT;

/**
 *  \brief System manager OD lengths
 *
 *  System manager possible OD lengths of M-sequence
 *  See: IO-Link spec v1.1.1 Oct 2011, A.2.6, Table A.7-10 p. 200-201
 *  
 */
typedef enum
{ 
  DSM_MSEQ_OD_LENGTH_1  = 1,
  DSM_MSEQ_OD_LENGTH_2  = 2,
  DSM_MSEQ_OD_LENGTH_8  = 8,
  DSM_MSEQ_OD_LENGTH_32 = 32
  
} DSM_MseqODLengT;

/**
 * \brief System manager device communication configuration type
 *
 * See: IO-Link spec v1.1.1 Oct 2011, 9.3.2.2, p. 139, ParameterList
 *
 */
typedef struct
{
  /**
   * \brief the SIO mode supported by the Device
   *
   * can be:
   *  DPL_MODE_INACTIVE,
   *  DPL_MODE_DO,
   *  DPL_MODE_DI
   *
   */
  DPL_TargetModeT supportedSIOMode;

  /**
   * \brief the transmission rates supported by the Device
   *
   * can be:
   *  #DPL_MODE_COM1
   *  #DPL_MODE_COM2
   *  #DPL_MODE_COM3
   *
   */
  DPL_TargetModeT supportedBaudrate;

  /**
   * \brief the minimum cycle time supported by the Device
   *
   *  Direct page 1 parameter address 1
   *  cycle time = multiplier * timeBase + const
   *  See: IO-Link spec v1.1.1 Oct 2011, B.1.3, p. 205
   *
   */
  UInteger8T masterCycleTime;

  /**
   * \brief the minimum cycle time supported by the Device
   *
   *  Direct page 1 parameter address 2
   *  min cycle time = multiplier * timeBase + const
   *  See: IO-Link spec v1.1.1 Oct 2011, B.1.4, p. 205
   *
   */
  UInteger8T minCycleTime;

  /** \brief the time base for the calculation of the MinCycleTime */
  DSM_CycTimeBaseT      minCycleTimeBase;

  /** \brief a 6-bit multiplier for the calculation of the MinCycleTime
   *
   *  can be from 0 to 63
   *
   */
  UInteger8T minCycleTimeMultiplier;

  /**
   * \brief the M-sequence capabilities supported by the Device
   *
   *  Direct page 1 parameter address 3
   *  - ISDU support
   *  - OPERATE M-sequence types
   *  - PREOPERATE M-sequence types
   *
   *  See: IO-Link spec v1.1.1 Oct 2011, B.1.5, p. 206
   *
   */
  UInteger8T msequenceCapability;

  /**
   *  \brief the M-sequence capability preoperate
   *
   *  See: IO-Link spec v1.1.1 Oct 2011, A.2.6, p. 200, Table A.8
   */
  UInteger8T msequenceCapabilityPreoperate;

  /**
   *  \brief the M-sequence capability operate
   *
   *  See: IO-Link spec v1.1.1 Oct 2011, A.2.6, p. 201, Table A.9-10
   */
  UInteger8T msequenceCapabilityOperate;

  BooleanT   isISDUSupported;
  
  /**
   *  \brief the M-sequence On-request data length preoperate
   *  
   *  Used only for IO-Link RevID > 1.1. Can be only 1, 2, 8, 32 (see DSM_MseqODLengT).
   *  See: IO-Link spec v1.1.1 Oct 2011, A.2.6, Table A.7-10 p. 200-201
   */
  DSM_MseqODLengT msequenceODLengthPreoper;

  /**
   *  \brief the M-sequence On-request data length operate
   *  
   *  Can be only 1, 2 for IO-Link RevID 1.0, 
   *  and also 8, 32 IO-Link RevID > 1.1 (see DSM_MseqODLengT).
   *  See: IO-Link spec v1.1.1 Oct 2011, A.2.6, Table A.7-10 p. 200-201
   */
  DSM_MseqODLengT msequenceODLengthOper;

  /**
   * \brief the M-sequence type preoperate
   *
   *  can be:
   *  #DDL_MEH_MESSAGE_TYPE_0
   *  #DDL_MEH_MESSAGE_TYPE_1
   *
   */
  EnumT      msequenceTypePreoper;

  /**
   * \brief the M-sequence type operate
   *
   *  can be:
   *  #DDL_MEH_MESSAGE_TYPE_0
   *  #DDL_MEH_MESSAGE_TYPE_1
   *  #DDL_MEH_MESSAGE_TYPE_2
   *
   */
  EnumT      msequenceTypeOper;

  /**
   * \brief the protocol revision RID supported by the Device
   *
   *  Direct page 1 parameter address 4.
   *  It can be only 0x10 or 0x11 for current IO-Link implementation.
   *  See: IO-Link spec v1.1.1 Oct 2011, B.1.6, p. 206
   *
   */
  UInteger8T revisionID;

  /**
   * \brief packed length of process data input to be sent to the Master
   *
   *  Direct page 1 parameter address 5
   *  See: IO-Link spec v1.1.1 Oct 2011, B.1.7, p. 217-218
   *
   */
  UInteger8T processDataIn;

  BooleanT   isBitLengthPDIn;
  /**
   * \brief the length of process data input to be sent to the Master
   *
   *  Set by Device Application. 
   *  Can be from 0 to 16 bits or from 3 to 32 bytes (depending on isBitLengthPDIn).
   *  See: IO-Link spec v1.1.1 Oct 2011, B.1.7, p. 217-218
   */
  UInteger8T lengthPDIn;

  /** \brief DO NOT SET FROM DEVICE APPLICATION !!! */
  UInteger8T lengthPDInBytes;

  /**
   * \brief packed length of process data output to be sent to the Master
   *
   *  Direct page 1 parameter address 6
   *  See: IO-Link spec v1.1.1 Oct 2011, B.1.7-8, p. 217-218
   *
   */
  UInteger8T processDataOut;

  BooleanT   isBitLengthPDOut;
  /**
   * \brief the length of process data output to be sent to the Master
   *
   *  Set by Device Application. 
   *  Can be from 0 to 16 bits or from 3 to 32 bytes (depending on isBitLengthPDOut).
   *  See: IO-Link spec v1.1.1 Oct 2011, B.1.7-8, p. 217-218
   */
  UInteger8T lengthPDOut;

  /** \brief DO NOT SET FROM DEVICE APPLICATION !!! */
  UInteger8T lengthPDOutBytes;

  #ifdef DPL_BACK_COMPATIBLE_RevID_10
  BooleanT   isInterleave;
  #endif
  
  BooleanT   isOK;

  /** \brief System manager device mode changed flag */
  BooleanT   isDeviceModeChanged;

  /** \brief Device mode */
  DSM_ModeT  deviceMode;

} DSM_DeviceComT;

/**
 * \brief System manager device identification data type
 *
 * See: IO-Link spec v1.1.1 Oct 2011, 9.3.2.4, p. 141, ParameterList; 
 *      B.1.1, Table B.1, p 214; B.1.9-11, p. 218
 *
 */
typedef struct
{
  /** \brief Direct page 1 parameter address 7 */
  UInteger8T vendorID_HI;
  /** \brief Direct page 1 parameter address 8 */
  UInteger8T vendorID_LO;

  /** \brief Direct page 1 parameter address 9 */
  UInteger8T deviceID_HI;
  /** \brief Direct page 1 parameter address 10 */
  UInteger8T deviceID_MID;
  /** \brief Direct page 1 parameter address 11 */
  UInteger8T deviceID_LO;

  /** \brief Direct page 1 parameter address 12 */
  UInteger8T functionID_HI;
  /** \brief Direct page 1 parameter address 13 */
  UInteger8T functionID_LO;

  BooleanT isOK;

} DSM_DeviceIdentT;

/**
 *  \fn void DSM_DeviceMode(DSM_ModeT mode)
 *  \brief SM mode indication
 *
 *  This function is used by System manager
 *  to indicate changes of communication states to the
 *  Device application.
 *  It should be defined in Device application.
 *  See: IO-Link spec v1.1.1 Oct 2011, 9.3.2.7, p.144
 *
 *  \param mode system manager mode
 *
 */
extern void DSM_DeviceMode (DSM_ModeT mode);

/**
 *  \fn void DSM_SwitchToRevID10_ind(void)
 *  \brief SM switching to RevID 1.0 indication
 *
 *  This function is used by System manager
 *  to indicate switching to RevID 1.0 to the
 *  Device application, because of connection with old IO-Link master 1.0.
 *  Device application can use this function to tune up itself 
 *  for old version 1.0 of IO-Link communication.
 *  It should be defined in Device application.
 *
 */
extern void DSM_SwitchToRevID10_ind(void);

void DSM_Init (void);

ResultT DSM_SetDeviceCom (void);

ResultT DSM_SetDeviceIdent (void);

DSM_DeviceComT * DSM_GetDeviceCom (void);

DSM_DeviceIdentT * DSM_GetDeviceIdent (void);

void DSM_MainCycle (void);

ResultT DSM_SetDeviceMode (EnumT mode);

/** \} */ // end of System_manager

ResultT DDL_Read (UInteger8T address, UInteger8T * value);

ResultT DDL_Write (UInteger8T address, UInteger8T value);

void DDL_Mode_ind (UInteger8T realMode);

#ifdef __cplusplus
}
#endif

#endif
