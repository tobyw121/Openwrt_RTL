@echo off

:-----------------------------------------------------------
: Show Usage or build driver according to %DDK_TARGET_OS%
:-----------------------------------------------------------
if "%1%"=="" goto usage
if "%1%"=="/?" goto usage
if "%1%"=="-?" goto usage
if "%1%"=="/h" goto usage
if "%1%"=="-h" goto usage

if "%1%"=="cleanall" goto cleanall


::cls
:-----------------------------------------------------------
: WDK Environment Variable Checking
:-----------------------------------------------------------
if "%DDK_TARGET_OS%"=="" (
	echo Please set the environment variables.
	echo   - MSBUILD_PLATFORM_TOOLSET = WindowsKernelModeDriver10.0
	echo   - DDK_TARGET_OS = Win7 ^| Win10
	echo   - BUILD_DEFAULT_TARGETS = -i386 ^| -amd64 ^| -Arm
	echo   - DDKBUILDENV = chk ^| fre
	goto end
)

:-----------------------------------------------------------
: Development Host Current OS Version
:-----------------------------------------------------------
set HOST_OS_VERSION=""
for /F "tokens=4-5 delims=]. " %%i IN ('ver') DO (
	set HOST_OS_VERSION=%%i.%%j
)

:
: <TODO> overwrite this section for new IC
:
set PHL_SRC_ROOT=%CD%

:: PHL_PROPS_PLTFM: default value PHL_PLATFORM_NONE
::set PHL_PROPS_PLTFM=PHL_PLATFORM_WINDOWS

if "%1%"=="/e" (
	set SLN_PROJ=phle.sln
	set PHL_PROPS_HCI=CONFIG_PCI_HCI
	set BUILDCFG_H_FILE=Build_config.h.WlanE.Windows
) else if "%1%"=="/s" (
	set SLN_PROJ=phls.sln
	set PHL_PROPS_HCI=CONFIG_SDIO_HCI
	set BUILDCFG_H_FILE=Build_config.h.WlanS.Windows
) else if "%1%"=="/u" (
	set SLN_PROJ=phlu.sln
	set PHL_PROPS_HCI=CONFIG_USB_HCI
	set BUILDCFG_H_FILE=Build_config.h.WlanU.Windows
) else (
	goto usage
)



:-----------------------------------------------------------
: MSBuild System Starting Point
:-----------------------------------------------------------
if "%USE_MSBUILD_SYSTEM%"=="true" goto msb_common

:-----------------------------------------------------------
: Legacy Build System Starting Point
:-----------------------------------------------------------
if "%DDK_TARGET_OS%"=="" goto ndis62
if "%DDK_TARGET_OS%"=="Win7" goto ndis62


:-----------------------------------------------------------
: Set up PLATFORM dependent parameters
:-----------------------------------------------------------
:ndis62

IF "%BUILD_DEFAULT_TARGETS%"=="-amd64" (
	echo "ndis62 X64..........."
	set TMP_TARGET_DIR=amd64
) ELSE (
	echo "ndis620 X86..........."
	set TMP_TARGET_DIR=i386
)
set IMG_DIR=obj%BUILD_ALT_DIR%\%TMP_TARGET_DIR%
set SOURCES_CONVERTER_TARGET_OS=Win7
goto setup_by_ic

:: For MSBuild Common Settings ---------------------------------------------
:msb_common
if "%DDK_TARGET_OS%"=="Win7"	goto msb_ndis620
if "%DDK_TARGET_OS%"=="Win10"	goto msb_ndis650

:: For MSBuild Win7 -------------------------------------------------------
:msb_ndis620
if "%DDKBUILDENV%"=="fre" set MSBUILD_CONFIGURATION="Win7 Release"
if "%DDKBUILDENV%"=="chk" set MSBUILD_CONFIGURATION="Win7 Debug"

IF "%BUILD_DEFAULT_TARGETS%"=="-amd64" (
	echo "MSBuild ndis620 X64..........."
	set TMP_TARGET_DIR=amd64
	set MSBUILD_DEFAULT_OUTPUT=x64
	set MSBUILD_PLATFORM=x64
) else if "%BUILD_DEFAULT_TARGETS%"=="-i386" (
	echo "MSBuild ndis620 X86..........."
	set TMP_TARGET_DIR=i386
	set MSBUILD_DEFAULT_OUTPUT=x86
	set MSBUILD_PLATFORM=Win32
)
set DDK_INC_PATH=%WindowsSdkDir%Include\km;%WindowsSdkDir%Include\shared;%WindowsSdkDir%Include\um;%WindowsSdkDir%Include\wdf\kmdf\1.9;
set IMG_DIR=obj\%MSBUILD_DEFAULT_OUTPUT%
set SOURCES_CONVERTER_TARGET_OS=Win7
goto setup_by_ic

:: For MSBuild Win10 -------------------------------------------------------
:msb_ndis650
if "%DDKBUILDENV%"=="fre" set MSBUILD_CONFIGURATION="Win10 Release"
if "%DDKBUILDENV%"=="chk" set MSBUILD_CONFIGURATION="Win10 Debug"

IF "%BUILD_DEFAULT_TARGETS%"=="-amd64" (
	echo "MSBuild ndis650 X64..........."
	set TMP_TARGET_DIR=amd64
	set MSBUILD_DEFAULT_OUTPUT=x64
	set MSBUILD_PLATFORM=x64
) else if "%BUILD_DEFAULT_TARGETS%"=="-i386" (
	echo "MSBuild ndis650 X86..........."
	set TMP_TARGET_DIR=i386
	set MSBUILD_DEFAULT_OUTPUT=x86
	set MSBUILD_PLATFORM=Win32
) else if "%BUILD_DEFAULT_TARGETS%"=="-Arm" (
	echo "MSBuild ndis650 Arm..........."
	set TMP_TARGET_DIR=Arm
	set MSBUILD_DEFAULT_OUTPUT=Arm
	set MSBUILD_PLATFORM=Arm
)
set DDK_INC_PATH=%KM_IncludePath%;%KIT_SHARED_IncludePath%;%UM_IncludePath%;%KMDF_INC_PATH%%KMDF_VER_PATH%;
set IMG_DIR=obj\%MSBUILD_DEFAULT_OUTPUT%
set SOURCES_CONVERTER_TARGET_OS=Win10
goto setup_by_ic

:-----------------------------------------------------------
: Set up IC dependent parameters
: <TODO> To set the folder path of New IC or New Common
:**************************
:New Common about HAL: Add the path in HAL_INC_PATH below
:New Common about MAC: Add the path in MAC_INC_PATH below
:New Common about PHYDM: Add the path in PHYDM_INC_PATH below
:**************************
:New IC about PCI: Add the number of IC in \PLATFORM\WinInf\PCI\InfSupportIC.ini
:				   Add the path in HEADER\IncludeICHeader_PCI.ini
:New IC about SDIO: Add the number of IC in \PLATFORM\WinInf\SDIO\InfSupportIC.ini
:				   Add the path in HEADER\IncludeICHeader_SDIO.ini
:New IC about USB: Add the number of IC in \PLATFORM\WinInf\USB\InfSupportIC.ini
:				   Add the path in HEADER\IncludeICHeader_USB.ini
:-----------------------------------------------------------
:setup_by_ic

:: if PHL_PROPS_PLTFM=PHL_PLATFORM_WINSOWS, must set PHL_INC_PATH

IF NOT EXIST "%PHL_SRC_ROOT%\..\HEADER" (
	echo "HEADER not exist: %PHL_SRC_ROOT%\..\HEADER"
	set PHL_INC_PATH=
) ELSE (
	if "%PHL_PROPS_PLTFM%"=="PHL_PLATFORM_WINDOWS" (
		:: Windows Driver folder exist.
		set PHL_INC_PATH=%PHL_SRC_ROOT%\..\COMMON;%PHL_SRC_ROOT%\..\HEADER;%PHL_SRC_ROOT%\..\PLATFORM\NDIS6;

		copy %PHL_SRC_ROOT%\..\HEADER\%BUILDCFG_H_FILE% %PHL_SRC_ROOT%\..\HEADER\Build_config.h
		copy HEADER\%PRECOMP_H_FILE% HEADER\Precomp.h

		if exist %PHL_SRC_ROOT%\..\HEADER\platform-os-version.h 	del %PHL_SRC_ROOT%\..\HEADER\platform-os-version.h
		echo #define OS_VERSION OS_WIN_10  >	%PHL_SRC_ROOT%\..\HEADER\platform-os-version.h

		if "%1%"=="/u" (
			echo #define FLASH_SIZE_2M_SUPPORT     0	> 	%PHL_SRC_ROOT%\..\HEADER\FlashSizeDef.h
		)

	) ELSE (
		set PHL_INC_PATH=
	)
)


if "%DDK_TARGET_OS%"=="Win7" (
	set RT_PLATFORM=PLATFORM_WINDOWS
	set OS_VERSION=OS_WIN_7
	set PRECOMP_H_FILE=Precomp.h.WlanU.Windows
) else if "%DDK_TARGET_OS%"=="Win10" (
	set RT_PLATFORM=PLATFORM_WINDOWS
	set OS_VERSION=OS_WIN_10
	set PRECOMP_H_FILE=Precomp.h.WlanU.Windows
)

:-----------------------------------------------------------
: Build driver and wrap up related output files.
:-----------------------------------------------------------

echo MSBUILD_CONFIGURATION=%MSBUILD_CONFIGURATION%
echo MSBUILD_PLATFORM=%MSBUILD_PLATFORM%

: generate header file "phl_git_info.h"
call gen_git_info.bat
if not exist %CD%\phl_git_info.h (
	echo phl_git_info.h is not generated, skip build.
	goto clean_up
)

if "%USE_MSBUILD_SYSTEM%"=="true" (
	msbuild /p:Configuration=%MSBUILD_CONFIGURATION% /p:Platform="%MSBUILD_PLATFORM%" %SLN_PROJ%
) else (
	if "%1%"=="r" 	(
		build -Zgw -M2
	) else (
		build -cZgw -M2
	)
)

: Destination Output Directory (Without Symbol)
if "%HOST_OS_VERSION%" GEQ "6.1" (
	powershell -command "&{ write-host `n------ Preparing Desination Output Directory ---------------- -foregroundcolor Magenta}" < NUL
)


goto clean_up
:-----------------------------------------------------------
: Show Usage
: usage: %0 [/?|-?|/h|-h]
: or
: usage: %0 [DriverVersion]
:-----------------------------------------------------------
:usage
@echo.
@echo usage: %0 [interface]
@echo /e : PCIE
@echo /s : SDIO
@echo /u : USB
@echo.


goto end



:-----------------------------------------------------------
: Clean up environment varibles
:-----------------------------------------------------------
:clean_up

set IMG_DIR=

set PHL_SRC_ROOT=
set PHL_INC_PATH=
set PHL_PROPS_HCI=
set PHL_PROPS_PLTFM=
set RT_PLATFORM=
set PRECOMP_H_FILE=
set BUILDCFG_H_FILE=

goto end

:cleanall

:: remove git info
IF EXIST .\phl_git_info.h (
	echo    Delete : git info
	del .\phl_git_info.h
)

:: windows_phl removal
for /D %%d in (".\windows_phl") do (
	echo    Delete : %%d
	if exist  %%d				rmdir /Q /S  %%d
)

for /D %%s in (".\*") do (
	call :delObjFolder %%s

	for /D %%t in (%%s\*) do (
		call :delObjFolder %%t

		for /D %%u in (%%t\*) do (
			call :delObjFolder %%u

			for /D %%v in (%%u\*) do (
				call :delObjFolder %%v
			)

				for /D %%w in (%%v\*) do (
					call :delObjFolder %%w
				)
		)
	)
)

call :delObjFolder ".\"

goto end
:: ================ 20190916 ================ ::
:: =========== Delete Object Files ========== ::
:delObjFolder
set TARGET_OBJ_DIR=%1
echo     TARGET_OBJ_DIR=%TARGET_OBJ_DIR%
for %%a in (i386, x64, amd64, ia64, Arm, obj, objchk, objfre, Debug, Release, out) do (
	if exist %TARGET_OBJ_DIR%\%%a	rmdir /Q /S %TARGET_OBJ_DIR%\%%a
)

for %%a in (objchk, objfre) do (
	for %%b in (win7, win10) do (
		for %%c in (x86, x64, amd64, ia64, Arm) do (
			if exist %TARGET_OBJ_DIR%\%%a_%%b_%%c	rmdir /Q /S %TARGET_OBJ_DIR%\%%a_%%b_%%c
		)
	)
)

for %%a in (Win7, Win10) do (
	for %%b in (Debug, Release) do (
		if exist %TARGET_OBJ_DIR%\%%a%%b	rmdir /Q /S %TARGET_OBJ_DIR%\%%a%%b
	)
)

if exist %TARGET_OBJ_DIR%\sources	del %TARGET_OBJ_DIR%\sources
if exist %TARGET_OBJ_DIR%\sources.props	del %TARGET_OBJ_DIR%\sources.props

:: Directory Removal
for %%a in (buildchk, buildfre, objchk, objfre) do (
	for %%b in (win7, win10) do (
		for %%c in (x86, x64, amd64, ia64, Arm) do (
			if exist %%a_%%b_%%c	rmdir /Q /S %%a_%%b_%%c
			if exist %%a_%%b_%%c.*  del %%a_%%b_%%c.*
		)
	)
)

:: File Removal
for %%a in (buildchk, buildfre) do (
	for %%b in (win7, win10) do (
		for %%c in (x86, x64, amd64, ia64, Arm) do (
			for %%d in (log, err, prf, wrn) do (
				if exist %%a_%%b_%%c.%%d 	del %%a_%%b_%%c.%%d
			)
		)
	)
)
goto end

goto end

:end
