@echo off
rem set ADB=C:\Program Files\Android\android-sdk\platform-tools\adb.exe
set ADB=adb
set ADBSHELL=%ADB% shell

set DRIVERENTRY=/proc/net/rtl8723bu
set DRIVERINTF=wlan0

set coexinfoentry=%DRIVERENTRY%/%DRIVERINTF%/btcoex
set readentry=%DRIVERENTRY%/%DRIVERINTF%/btreg_read
set writeentry=%DRIVERENTRY%/%DRIVERINTF%/btreg_write

:: Get Android permission
%ADB% root
%ADB% remount

SETLOCAL ENABLEDELAYEDEXPANSION
if NOT "%1"=="" (
	call :btcmd %*
	if NOT "!errmsg!"=="" echo !errmsg!
	exit /b
)

:loop
set input=
set /p input="bt>"

if "%input%"=="" goto loop
if /i "%input:~0,4%"=="exit" exit /b
if /i "%input:~0,4%"=="quit" exit /b

call :btcmd %input%
if NOT "%errmsg%"=="" echo %errmsg%
goto :loop

:: Function btcmd
:: ===============
:: Input parameters:
:: %1 - command
:: %2 - data1 for command; register type for read/write command
:: %3 - data2 for command; address for read/write command
:: %4 - data3 for command; value for write command
:: Return:
:: errmsg - store error messages
:btcmd
set errmsg=
setlocal

set cmd=%1
if "%cmd%"=="" exit /b
if "%cmd%"=="coexinfo" (
	%ADBSHELL% "cat %coexinfoentry%"
	exit /b
)

set type=%2
if "%type%"=="" exit /b
set addr=%3
if "%addr%"=="" exit /b
set val=%4
if "%cmd%%val%"=="write" exit /b

if "%cmd%"=="read" (
	%ADBSHELL% "echo %type% %addr% > %readentry%"
	%ADBSHELL% "cat %readentry%"
	exit /b
)
if "%cmd%"=="write" (
	%ADBSHELL% "echo %type% %addr% %val% > %writeentry%"
	%ADBSHELL% "cat %writeentry%"
	exit /b
)
endlocal & set errmsg=unknown command "%cmd%"!
exit /b
