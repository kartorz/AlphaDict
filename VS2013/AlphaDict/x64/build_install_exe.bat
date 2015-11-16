@ECHO OFF
CLS
COLOR 4A
TITLE AlphaDict for Windows Build Script

rem -------------------------------------------------------------

:NSIS_EXE
  SET ALPHADICT_REV=1.3
  SET ALPHADICT_SETUPFILE="install_alphadict_x64_Rev%ALPHADICT_REV%.exe"

  ECHO Creating installer %ALPHADICT_SETUPFILE% ...
  IF EXIST %ALPHADICT_SETUPFILE% del %ALPHADICT_SETUPFILE% > NUL

  rem get path to makensis.exe from registry, first try tab delim
  FOR /F "tokens=2* delims=	" %%A IN ('REG QUERY "HKLM\Software\NSIS" /ve') DO SET NSISExePath=%%B
  IF NOT EXIST "%NSISExePath%" (
    rem try with space delim instead of tab
	FOR /F "tokens=2* delims= " %%A IN ('REG QUERY "HKLM\Software\NSIS" /ve') DO SET NSISExePath=%%B
  )
  rem x64 - get path to makensis.exe from registry, first try tab delim
  FOR /F "tokens=2* delims=	" %%A IN ('REG QUERY "HKLM\SOFTWARE\Wow6432Node\NSIS" /ve') DO SET NSISExePath=%%B
  IF NOT EXIST "%NSISExePath%" (
    rem x64 - try with space delim instead of tab
	FOR /F "tokens=2* delims= " %%A IN ('REG QUERY "HKLM\SOFTWARE\Wow6432Node\NSIS" /ve') DO SET NSISExePath=%%B
  )
  SET NSISExe=%NSISExePath%\makensis.exe
  "%NSISExe%" /V1 /X"SetCompressor /FINAL lzma" /D_root="%CD%" /D_revision="%ALPHADICT_REV%" "install.nsi"
  IF NOT EXIST "%ALPHADICT_SETUPFILE%" (
	  ECHO Failed to create %ALPHADICT_SETUPFILE% .
	  goto END
  )

  ECHO ------------------------------------------------------------
  ECHO Done!
  ECHO Setup is located at %CD%\%ALPHADICT_SETUPFILE%
  ECHO ------------------------------------------------------------
  REM GOTO VIEWLOG_EXE

  :END
  ECHO Press any key to exit...
  pause > NUL
 