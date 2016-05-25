//------------------------------------------------------------------------------
//! \file
//! Declares the DemoApp class
//
//------------------------------------------------------------------------------
#ifndef demoapp_h__
#define demoapp_h__

// Select the stack type
#if defined(STACK_MODE_TRANSPARENT)
# include "stacktransparent.h"
#elif defined (STACK_MODE_SINGLE_BYTE)
# include "stacksinglebyte.h"
#elif defined (STACK_MODE_MULTI_BYTE)
# include "stackmultibyte.h"
#else
# error No stack type selected
#endif

/**
 * The DemoApp is a demonstration application which uses the Mini-stack software.
 * The demonstration application and its related IODD file is supplied with
 * the Mini-stack software to facilitate a rapid development start in assosiation
 * with one of the development boards (TM96.1 var. A or B, TM141.0 or TM142.0)
 *
 * The related IODD file for this application can be found in directory IODD,
 * files HMT-Mini_stack_38kB-20120727-IODD1.0.xml for 38.4kBd variants, and
 * HMT-Mini_stack_230kB-20120727-IODD1.0.xml for 230.4kBd variants.
 *
 * The demonstration application may be compiled with any stack mode, multi-octet,
 * single-octet or transparent.
 *
 * The board LEDs indicate the status:
 * \li in SIO-mode the red LED alternately brightens and dims
 * \li in IO-Link operation the green LED alternately brightens and dims
 * \li BUT, the red LED lights permanently if the push-button is pressed.
 *
 * <table width=100% margin=0 border=1>
 * <tr><th>address</th><th>direct parameter</th><th>comment</th></tr>
 * <tr><td>0x10</td><td>VendorParamMirrorOutput</td><td>value read from device</td></tr>
 * <tr><td>0x11</td><td>VendorParamMirrorInput</td><td>value sent to device</td></tr>
 * <tr><td>0x12</td><td>VendorParamPidMode</td><td>selects value for process data</td></tr>
 * </table>
 * <center><b>Direct Parameter Memory map</b></center>
 *
 * Basic data exchange on the direct parameter page is demonstrated. A value written
 * in direct parameter VendorParamMirrorOutput will subsequently be read in
 * VendorParamMirrorInput.
 *
 * The process data can have different contents depending on the value set
 * in direct parameter VendorParamPidMode.
 *
 * <table width=100% margin=0 border=1>
 * <tr><th>setting in VendorParamPidMode</th><th>comment</th><th>interpretation</th></tr>
 * <tr><td>0x00</td><td>1 bit representing the push-button state</td><td>'1' => pressed</td></tr>
 * <tr><td>0x01</td><td>8 bit octet for the potentiometer setting</td><td></td></tr>
 * <tr><td>0x02</td><td>8 bit internally generated saw-tooth value</td><td></td></tr>
 * </table>
 * <center><b>Process data selection</b></center>
 *
 */
class DemoApp
{
public:
    /**
     * The one and only instance of this class
     */
    static DemoApp      instance;
    
    /**
     * Configure the DemoApp class
     */
    static void         configure();

    /**
     * This is the main loop, which is called by the stack after a frame has been
     * received.
     * 
     * \param param         Pointer to Parameter structure if the most recent message
     *                      completed a write access to the direct parameter page. 
     *                      The data is *not* automatically written to the direct 
     *                      parameter page, but needs to be manually committed by
     *                      calling parameterWrite(). This parameter may be NULL
     *                      if no write access occurred.
     */
    void                run(const Stack::Parameter* param);

private:
    // Private constructors and copy assignment operator to prevent multiple instances
    DemoApp() {}
    DemoApp(const DemoApp&);
    DemoApp& operator=(const DemoApp&);

    /**
     * Handle write access to direct parameter page
     *
     * \param param         Pointer to Parameter structure. 
     *                      The data is *not* automatically written to the direct 
     *                      parameter page, but needs to be manually committed by
     *                      calling parameterWrite().
     */
    void                handleParameterWrite(const Stack::Parameter* param);

    /**
     * Update input process data
     */
    void                updateProcessInputData();

    /**
     * Test if timer period has elapsed10ms
     * 
     * \return true if timer period elapsed10ms, false otherwise.
     */
    static bool         elapsed10ms();

    /**
     * Test state of digital input (push button)
     */
    static bool         isDigitalButtonPressed();

    /**
     * Variables in direct parameter page
     */
    enum DirectParamPage
    {
        VendorParamMirrorOutput = 0x10,
        VendorParamMirrorInput  = 0x11,
        VendorParamPidMode      = 0x12
    };

    /**
     * Valid values for VendorParamPidMode direct parameter
     */
    enum InputDataMode
    {
        PidDigitalInput,    //!< Update process input data from digital input (push button)
        PidAnalogInput,     //!< Update process input data from analog input (poti)
        PidSawtooth,        //!< Update process input data from a counter
		PidSensors,
    };

private:
    uint8_t             _ctr;
    Stack::LedLevel     _ledLevel1;
    Stack::LedLevel     _ledLevel2;
    uint8_t             _writeOperationCtr;
};

#endif // demoapp_h__
