@echo off
::-----------------------------------------------------------
:: Generate git related header
::-----------------------------------------------------------

IF EXIST %CD%\make_drv_win.bat (
	SET TARGET=%CD%
	call :GenGitInfo_Empty
) ELSE (
	IF "%1"=="" (
		SET TARGET=%CD%\phl
		call :GenGitInfo_Empty
	) ELSE (
		SET TARGET=%1
		IF EXIST %1 ( call :GenGitInfo )
	)
)
exit /b

:GenGitInfo_Empty
	IF EXIST %TARGET%\phl_git_info.h (
		del %TARGET%\phl_git_info.h
	)
	type %TARGET%\phl_git_info_header.txt >> %TARGET%\phl_git_info.h

	echo #define RTK_CORE_SHA1      "0" >> %TARGET%\phl_git_info.h
	echo #define RTK_PHL_SHA1       "0" >> %TARGET%\phl_git_info.h
	echo #define RTK_HALMAC_SHA1    "0" >> %TARGET%\phl_git_info.h
	echo #define RTK_HALBB_SHA1     "0" >> %TARGET%\phl_git_info.h
	echo #define RTK_HALRF_SHA1     "0" >> %TARGET%\phl_git_info.h
	echo #define RTK_BTC_SHA1       "0" >> %TARGET%\phl_git_info.h
	echo.>> %TARGET%\phl_git_info.h
	echo #define RTK_CORE_TAGINFO   "0" >> %TARGET%\phl_git_info.h
	echo #define RTK_PHL_TAGINFO    "0" >> %TARGET%\phl_git_info.h
	echo #define RTK_HALMAC_TAGINFO "0" >> %TARGET%\phl_git_info.h
	echo #define RTK_HALBB_TAGINFO  "0" >> %TARGET%\phl_git_info.h
	echo #define RTK_HALRF_TAGINFO  "0" >> %TARGET%\phl_git_info.h
	echo #define RTK_BTC_TAGINFO    "0" >> %TARGET%\phl_git_info.h
	echo.>> %TARGET%\phl_git_info.h
	echo #endif /* __PHL_GIT_INFO_H__ */ >> %TARGET%\phl_git_info.h

	goto :eof

:GenGitInfo
	IF EXIST %TARGET%\phl\phl_git_info.h (
		del %TARGET%\phl\phl_git_info.h
	)
	IF EXIST %TARGET%\.git (
		REM ( LOAD HEADER TXT )
		type %TARGET%\phl\phl_git_info_header.txt >> %TARGET%\phl\phl_git_info.h
		REM ( GET SHA1 VALUE )
		REM -- GET CORE SHA1 ---------------------------------------------
		setlocal enableDelayedExpansion
		cd !TARGET!
		set /A Msgtemp=0
		for /f %%i in ('git rev-parse HEAD') do set Msgtemp=%%i
		echo #define RTK_CORE_SHA1   "!Msgtemp!" >> !TARGET!\phl\phl_git_info.h
		endlocal
		REM -- GET PHL SHA1 ----------------------------------------------
		setlocal enableDelayedExpansion
		cd !TARGET!\phl
		set /A Msgtemp=0
		for /f %%i in ('git rev-parse HEAD') do set Msgtemp=%%i
		echo #define RTK_PHL_SHA1    "!Msgtemp!" >> !TARGET!\phl\phl_git_info.h
		endlocal
		REM -- GET HALMAC SHA1 -------------------------------------------
		setlocal enableDelayedExpansion
		cd !TARGET!\phl\hal_g6\mac
		set /A Msgtemp=0
		for /f %%i in ('git rev-parse HEAD') do set Msgtemp=%%i
		echo #define RTK_HALMAC_SHA1 "!Msgtemp!" >> !TARGET!\phl\phl_git_info.h
		endlocal
		REM -- GET HALBB SHA1 --------------------------------------------
		setlocal enableDelayedExpansion
		cd !TARGET!\phl\hal_g6\phy\bb
		set /A Msgtemp=0
		for /f %%i in ('git rev-parse HEAD') do set Msgtemp=%%i
		echo #define RTK_HALBB_SHA1  "!Msgtemp!" >> !TARGET!\phl\phl_git_info.h
		endlocal
		REM -- GET HALRF SHA1 --------------------------------------------
		setlocal enableDelayedExpansion
		cd !TARGET!\phl\hal_g6\phy\rf
		set /A Msgtemp=0
		for /f %%i in ('git rev-parse HEAD') do set Msgtemp=%%i
		echo #define RTK_HALRF_SHA1  "!Msgtemp!" >> !TARGET!\phl\phl_git_info.h
		endlocal
		REM -- GET BTC SHA1 ----------------------------------------------
		setlocal enableDelayedExpansion
		cd !TARGET!\phl\hal_g6\btc
		set /A Msgtemp=0
		for /f %%i in ('git rev-parse HEAD') do set Msgtemp=%%i
		echo #define RTK_BTC_SHA1    "!Msgtemp!" >> !TARGET!\phl\phl_git_info.h
		endlocal

		echo.>> %TARGET%\phl\phl_git_info.h
		REM ( GET BRANCH NAMES )
		REM -- GET CORE BRANCH -------------------------------------------
		setlocal enableDelayedExpansion
		cd !TARGET!
		set /A Msgtemp=0
		for /f %%i in ('git describe --tags --long --always') do set Msgtemp=%%i
		echo #define RTK_CORE_TAGINFO     "!Msgtemp!" >> !TARGET!\phl\phl_git_info.h
		endlocal
		REM -- GET PHL BRANCH --------------------------------------------
		setlocal enableDelayedExpansion
		cd !TARGET!\phl
		set /A Msgtemp=0
		for /f %%i in ('git describe --tags --long --always') do set Msgtemp=%%i
		echo #define RTK_PHL_TAGINFO      "!Msgtemp!" >> !TARGET!\phl\phl_git_info.h
		endlocal
		REM -- GET HALMAC BRANCH -----------------------------------------
		setlocal enableDelayedExpansion
		cd !TARGET!\phl\hal_g6\mac
		set /A Msgtemp=0
		for /f %%i in ('git describe --tags --long --always') do set Msgtemp=%%i
		echo #define RTK_HALMAC_TAGINFO   "!Msgtemp!" >> !TARGET!\phl\phl_git_info.h
		endlocal
		REM -- GET HALBB BRANCH ------------------------------------------
		setlocal enableDelayedExpansion
		cd !TARGET!\phl\hal_g6\phy\bb
		set /A Msgtemp=0
		for /f %%i in ('git describe --tags --long --always') do set Msgtemp=%%i
		echo #define RTK_HALBB_TAGINFO    "!Msgtemp!" >> !TARGET!\phl\phl_git_info.h
		endlocal
		REM -- GET HALRF BRANCH ------------------------------------------
		setlocal enableDelayedExpansion
		cd !TARGET!\phl\hal_g6\phy\rf
		set /A Msgtemp=0
		for /f %%i in ('git describe --tags --long --always') do set Msgtemp=%%i
		echo #define RTK_HALRF_TAGINFO    "!Msgtemp!" >> !TARGET!\phl\phl_git_info.h
		endlocal
		REM -- GET HALRF BRANCH ------------------------------------------
		setlocal enableDelayedExpansion
		cd !TARGET!\phl\hal_g6\btc
		set /A Msgtemp=0
		for /f %%i in ('git describe --tags --long --always') do set Msgtemp=%%i
		echo #define RTK_BTC_TAGINFO      "!Msgtemp!" >> !TARGET!\phl\phl_git_info.h
		endlocal
		REM ( LAST LINE OF THE FILE )
		echo.>> %TARGET%\phl\phl_git_info.h
		echo #endif /* __PHL_GIT_INFO_H__ */ >> %TARGET%\phl\phl_git_info.h
	)

	goto :eof