*******************************************************************************
***                                                                         ***
***                       IODD V1.1 Checker V1.1.1              						***
***                 Copyright (c) 2013 IO-Link Consortium                   ***
***                                                                         ***
*******************************************************************************

Important:		MSXML4 parser is no longer used
===============================================
MSXML4 is not supported with Windows 7 64 Bit. MSXML6 delivers same results
as .Net parser. The working group decided to cancel MSXML4 usage.
The commandline option -me is no longer available.

To run this tool, please install:
	Microsoft .NET Framework Version 2.0 Redistributable Package (x86)
		Please use the current Service Pack (currently SP2).
		It is recommended to use Microsoft Update to get the newest fixes.
		dotnetfx.exe	from http://www.microsoft.com

To get the full functionality of this tool, please install:

	Xerces-C++ Version 2.8.0 or newer
		http://xerces.apache.org/xerces-c/
		It's not the whole installation required. Only the following files are used:
			DOMCount.exe
			xerces-c_2_8.dll
		Please unpack archive to a folder of your choice.

In the archive you'll find:
	- ReadMe.txt
		this file
	- IODD_V1.1_Checker_V1.1.1_Setup.msi
		msi installation of the checker tool, 
		running with Windows XP SP3 32 Bit, Windows Vista 32 Bit and Windows 7 32/64 Bit
	- Check.bat
		sample batch file
		please change the installation path of checker and Xerces
			set CHKPATH="c:\program files\io-link consortium\IODD V1.1 Checker\IODDChecker.exe"
			set XERCES=".\Xerces"
				For Xerces only DOMcount.exe and xerces-c_2_8.dll are needed or use the
				official installation.
	- Check_wo_Xerces.bat
		sample batch file
		please change the installation path of the checker
			set CHKPATH="c:\program files\io-link consortium\IODD V1.1 Checker\IODDChecker.exe"

Notes:
	It's not possible to use the Checker reentrant. Please secure to run only one instance of the checker at the same time.
	If no schema location is defined XERCES brings up a lot of error messages without a hint to the reason.
	Length of VendorUrl is checked to 255 characters instead of 1024. A CR to the specification will be discussed in the working group.
	The schema allows white spaces at most of the element or attribute values. The checker doesn't consider this and gives error messages.


Comments to V1.1.1
	- bugfix: wrong line number with condition errors
	- bugfix: CR1: UInteger values are not accepted
	- bugfix: CR2: Integer values are not accepted
	- bugfix: CR3: Array of UInteger values are not accepted
	- bugfix: CR4: ProcessData of type Array of String or OctetString length calculation wrong
	- bugfix: CR5: SystemCommand is required for some features
	- bugfix: CR6: Allowed values of standard variables are not fully checked
	- bugfix: CR7: StdRecordItemRef/@defaultValue is not checked
	- bugfix: CR8: SupportedAccessLocks versus V_DeviceAccessLocks
	- bugfix: CR9: Special floating point values


Comments to V1.1.0.8 Beta
	- Schema included from August,2011
	- bugfix: if filename of IODD was given without path graphic files were not found
	- bugfix: fixLengthRestriction with ArrayT
	- cancel usage of MSXML4
	- bugfix: V_SytemCommand: if no StdSingleValueRefs or SingleValues defined, definition of StandardDefinitions is used
	- bugfix: Conditions with RecordItems: detection of default value and line info

Karlsruhe, February 12,2013
