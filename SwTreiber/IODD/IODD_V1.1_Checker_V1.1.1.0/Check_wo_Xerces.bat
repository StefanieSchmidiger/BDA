@ECHO off
rem ==============================================================================
rem Sample Batch File
rem ==============================================================================
set CHKPATH="c:\program files\io-link consortium\IODD V1.1 Checker\IODDChecker.exe"
rem
%CHKPATH% -?
Pause
%CHKPATH% ".\Samples from Specification\IO-Link-2-ISDUDevice-20101112-IODD1.1.xml" -s -p -w
Pause
