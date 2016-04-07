@ECHO off
rem ==============================================================================
rem Sample Batch File
rem ==============================================================================
set CHKPATH="c:\program files\io-link consortium\IODD V1.1 Checker\IODDChecker.exe"
rem
%CHKPATH% -?
Pause
%CHKPATH% "C:\Users\Stefanie\Documents\Studium\6.Semester\BDA\HMT\ministack-1.0.1\IODD\HMT-Mini_stack_v1_1_230kBd-20130208-IODD1.1.xml" -s -p -w
Pause
