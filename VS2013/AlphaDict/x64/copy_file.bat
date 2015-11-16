SET ROOT_DIR=..\..\..

IF NOT EXIST "Release\tools" (
	MKDIR "Release\tools"
	MKDIR "Release\doc"
)

DEL /Q /F  "Release\tools\*"
DEL /Q /F  "Release\doc\*"

COPY /Y  %ROOT_DIR%\src\gui\qt\res\app.ico      app.ico
COPY /Y  %ROOT_DIR%\src\gui\qt\res\appicon.png  app.png
COPY /Y  %ROOT_DIR%\src\system\*				Release\system
COPY /Y  %ROOT_DIR%\src\gui\qt\uitr_cn.ts		Release\system
COPY /Y  %ROOT_DIR%\src\gui\qt\uitr_cn.qm		Release\system
COPY /Y  %ROOT_DIR%\src\tools\*					Release\tools       
COPY /Y  %ROOT_DIR%\README.cn					Release\doc
COPY /Y  %ROOT_DIR%\README.en					Release\doc

pause