; install.nsi
;
; NSI install scripts of AlphaDict for windows.
;
; Copyright @ team AlphaDict
; 
;----------------------------------------------
;Include Modern UI

  !include "MUI2.nsh"
  !include "nsDialogs.nsh"
  !include "LogicLib.nsh"
  !define MUI_ICON   "app.ico"
  !define MUI_UNICON "app.ico"

;--------------------------------
;General

  ;Name and file
  Name "$(LNG_Name)"
  OutFile "install_alphadict_win32_Rev${_revision}.exe"

  XPStyle on
  
  ;Default installation folder
  InstallDir "$PROGRAMFILES\AlphaDict"

  ;Get installation folder from registry if available
  InstallDirRegKey HKLM "Software\AlphaDict"  "InstallDir"

  ;Request application privileges for Windows Vista
  RequestExecutionLevel admin ;highest

  ;AutoCloseWindow true

;--------------------------------
  Var StartMenuFolder

;--------------------------------
;Interface Settings
 
 !define MUI_FONT "Tahoma"
 !define MUI_FONTSIZE "9"
 !define MUI_FONT_HEADER "Tahoma"
 !define MUI_FONTSIZE_HEADER "9"

 !define MUI_HEADERIMAGE
 !define MUI_HEADERIMAGE_BITMAP "app.png"
 !define MUI_FINISHPAGE_LINK_LOCATION "https://github.com/kartorz/AlphaDict/wiki"

 ;--------------------------------
;Pages

  !insertmacro MUI_PAGE_WELCOME
  ;!insertmacro MUI_PAGE_LICENSE "$(LNG_License)"
  !insertmacro MUI_PAGE_DIRECTORY
  ;!insertmacro MUI_PAGE_COMPONENTS

  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\AlphaDict" 
  ;!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  ;!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder  

  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  UninstPage custom un.UnPageProfile un.UnPageProfileLeave
  UninstPage custom un.UnPageDict    un.UnPageDictLeave
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

 ;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"
  !insertmacro MUI_LANGUAGE "SimpChinese"

  ;LicenseLangString LNG_License ${LANG_ENGLISH} "eula.rtf" 
  ;LicenseLangString LNG_License ${LANG_SIMPCHINESE} "Chinese.rtf"  

  LangString LNG_Name ${LANG_ENGLISH} "AlphaDict" 
  LangString LNG_Name ${LANG_SIMPCHINESE} "Alpha词典"
  LangString LNG_CreateShortcut ${LANG_ENGLISH} "Create Shortcut"
  LangString LNG_CreateShortcut ${LANG_SIMPCHINESE} "创建快捷方式"
  LangString LNG_DesktopShortcut ${LANG_ENGLISH} "Desktop Shortcut"
  LangString LNG_DesktopShortcut ${LANG_SIMPCHINESE} "桌面快捷方式"
  LangString LNG_QuickLaunchShortcut ${LANG_ENGLISH} "Quick Launch Bar Shortcut"
  LangString LNG_QuickLaunchShortcut ${LANG_SIMPCHINESE} "快速启动栏快捷方式"

  LangString LNG_ReInstallWarnning ${LANG_ENGLISH} "A previous version of AlphaDict is already installed.$\n\
  Setup will uninstall the previous version before proceeding with this installation.$\n\
  Click 'OK' to uninstall the previous version or 'Cancel' to cancel this install. "

  ;LangString LNG_ReInstallWarnning ${LANG_SIMPCHINESE} "  "
  
  LangString LNG_ReInstallWarnning ${LANG_SIMPCHINESE} "已经安装了一个老版本,继续安装之前,需要卸载老版本. $\n\
  按 '确定' 卸载老版本, '取消' 停止本次安装"

  LangString LNG_UnInstallAlphaDict  ${LANG_ENGLISH} "Uninstall AlphaDict"
  LangString LNG_UnInstallAlphaDict  ${LANG_SIMPCHINESE} "卸载 Alpha词典"

;--------------------------------
;Uninstall Previous Version

Function .onInit
 ;!insertmacro MUI_UNGETLANGUAGE

 !insertmacro MUI_LANGDLL_DISPLAY
  ReadRegStr $R0 HKLM \
  "Software\AlphaDict" \
  ""
  StrCmp $R0 "" done
 
  ;MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION $(LNG_ReInstallWarnning) \
  ;IDOK uninst

  ;Abort

;Run the uninstaller
uninst:
  ClearErrors
  ExecWait '$R0\Uninstall.exe /S _?=$INSTDIR' ;Do not copy the uninstaller to a temp file
 
  IfErrors no_remove_uninstaller done
    ;You can either use Delete /REBOOTOK in the uninstaller or add some code
    ;here to remove the uninstaller. Use a registry key to check
    ;whether the user has chosen to uninstall. If you are using an uninstaller
    ;components page, make sure all sections are uninstalled.
  no_remove_uninstaller:
 
done:
 
FunctionEnd

;--------------------------------
;Installer Sections 

SectionGroup "$(LNG_CreateShortcut)" SEC_Shortcut
    Section "$(LNG_DesktopShortcut)" SEC_DesktopShortcut
        SectionIn 1 2
		SetShellVarContext all
		${If} $LANGUAGE ==  ${LANG_SIMPCHINESE}
		    CreateShortCut "$DESKTOP\Alpha词典.lnk" "$INSTDIR\AlphaDict.exe" ""  "$INSTDIR\app.ico"
		${Else}
            CreateShortCut "$DESKTOP\AlphaDict.lnk" "$INSTDIR\AlphaDict.exe" ""  "$INSTDIR\app.ico"
		${Endif}
    SectionEnd
    ;Section "$(LNG_QuickLaunchShortcut)" SEC_QuickLaunchShortcut
    ;    SectionIn 2
	;    SetShellVarContext all
    ;    CreateShortCut "$QUICKLAUNCH\AlphaDict.lnk" "$INSTDIR\AlphaDict.exe"
    ;SectionEnd
SectionGroupEnd

Section "AlphaDict" SecAlphaDict
  StrCpy $0 $INSTDIR
  SectionIn RO
  SectionIn 1 2
  ;ADD YOUR OWN FILES HERE...
  SetOutPath "$INSTDIR"
  File "${_root}\app.ico"
  File "${_root}\Release\*.exe"
  File "${_root}\Release\*.dll"
  File "${_root}\Release\README-win32.txt"
  File "${_root}\Release\qt.conf"

  SetOutPath "$INSTDIR\plugins\platforms"
  File "${_root}\Release\plugins\platforms\*.*"

  SetOutPath "$INSTDIR\doc"
  File "${_root}\Release\doc\*.*"

  SetOutPath "$INSTDIR\system"
  File "${_root}\Release\system\*.*"  

  SetOutPath "$INSTDIR\system\dicts"
  ;File "${_root}\Release\system\dicts\cedict_1_0_ts_mdbg.aldict"
  ;File "${_root}\Release\system\dicts\langdao-ec-gb.aldict"

  SetOutPath "$INSTDIR\system\xmls"
  ;File "${_root}\Release\system\xmls\cedict_1_0_ts_mdbg.xml"

  SetOutPath "$INSTDIR\tools"
  File "${_root}\Release\tools\*.*"


  ;Store installation folder
  WriteRegStr HKLM "Software\AlphaDict" "" $INSTDIR

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  ;"$INSTDIR\app.ico"
  ;Create shortcuts
  SetOutPath "$INSTDIR"
  ;CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
  SetShellVarContext all
  ${If} $LANGUAGE ==  ${LANG_SIMPCHINESE}
      CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Alpha词典.lnk" "$INSTDIR\AlphaDict.exe"
  ${Else}
      CreateShortCut "$SMPROGRAMS\$StartMenuFolder\AlphaDict.lnk" "$INSTDIR\AlphaDict.exe"
  ${Endif}
   
  ;add entry to add/remove programs
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AlphaDict" \
                 "DisplayName" "AlphaDict"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AlphaDict" \
                 "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AlphaDict" \
                 "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AlphaDict" \
                 "NoRepair" 1
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AlphaDict" \
                 "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AlphaDict" \
                 "DisplayIcon" "$INSTDIR\AlphaDict.exe,0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AlphaDict" \
                 "Publisher" "Alpha Team"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AlphaDict" \
                 "URLInfoAbout" "https://github.com/kartorz/AlphaDict/wiki"

  ;Exec "$INSTDIR\AlphaDict.exe"
SectionEnd


;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecAlphaDict ${LANG_ENGLISH} "AlphaDict"

  ;Assign language strings to sections
  ;!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    ;!insertmacro MUI_DESCRIPTION_TEXT ${SecXBMC} $(DESC_SecXBMC)
 ; !insertmacro MUI_FUNCTION_DESCRIPTION_END

;---------------------
; Uninstaller Section
;---------------------

Var UnPageProfileDialog
Var UnPageProfileCheckbox
Var UnPageProfileCheckbox_State
Var UnPageProfileEditBox

Function un.UnPageProfile
    !insertmacro MUI_HEADER_TEXT $(LNG_UnInstallAlphaDict) "Remove AlphaDict's profile folder from your computer."
	nsDialogs::Create /NOUNLOAD 1018
	Pop $UnPageProfileDialog

	${If} $UnPageProfileDialog == error
		Abort
	${EndIf}

	${NSD_CreateLabel} 0 0 100% 12u "Do you want to delete the profile folder?"
	Pop $0

	${NSD_CreateText} 0 13u 100% 12u "$APPDATA\AlphaDict\"
	Pop $UnPageProfileEditBox
    SendMessage $UnPageProfileEditBox ${EM_SETREADONLY} 1 0

	${NSD_CreateLabel} 0 46u 100% 24u "Leave unchecked to keep the profile folder for later use or check to delete the profile folder."
	Pop $0

	${NSD_CreateCheckbox} 0 71u 100% 8u "Yes, also delete the profile folder."
	Pop $UnPageProfileCheckbox

	nsDialogs::Show
FunctionEnd

Function un.UnPageProfileLeave
  ${NSD_GetState} $UnPageProfileCheckbox $UnPageProfileCheckbox_State
FunctionEnd

Var UnPageDictDialog
Var UnPageDictCheckbox
Var UnPageDictCheckbox_State
Var UnPageDictEditBox

Function un.UnPageDict
    !insertmacro MUI_HEADER_TEXT $(LNG_UnInstallAlphaDict) "Remove AlphaDict's dicts folder from your computer."
	nsDialogs::Create /NOUNLOAD 1018
	Pop $UnPageDictDialog

	${If} $UnPageDictDialog == error
		Abort
	${EndIf}

	${NSD_CreateLabel} 0 0 100% 12u "Do you want to delete the dicts folder?"
	Pop $0

	${NSD_CreateText} 0 13u 100% 12u "$INSTDIR\system\{dicts,xmls}"
	;${NSD_CreateText} 0 13u 100% 12u "$INSTDIR\system\xmls"
	Pop $UnPageDictEditBox
    SendMessage $UnPageDictEditBox ${EM_SETREADONLY} 1 0

	${NSD_CreateLabel} 0 46u 100% 24u "Leave unchecked to keep the dicts folder for later use or check to delete the dicts folder."
	Pop $0

	${NSD_CreateCheckbox} 0 71u 100% 8u "Yes, also delete the dicts folder."
	Pop $UnPageDictCheckbox
	
	nsDialogs::Show
FunctionEnd

Function un.UnPageDictLeave
  ${NSD_GetState} $UnPageDictCheckbox $UnPageDictCheckbox_State
FunctionEnd

Function .onInstSuccess

  ;StrCpy $0 $INSTDIR
  ;Exec "$INSTDIR\AlphaDict.exe"

FunctionEnd

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...
  Delete "$INSTDIR\*.*"
  Delete "$INSTDIR\system\*.*"

  RMDir /r "$INSTDIR\doc"
  RMDir /r "$INSTDIR\plugins"
  ;RMDir /r "$INSTDIR\system"
  RMDir /r "$INSTDIR\tools"

  Delete "$INSTDIR\Uninstall.exe"
  ;RMDir /r "$INSTDIR"
  
${If} $UnPageProfileCheckbox_State == ${BST_CHECKED}
  RMDir /r "$APPDATA\AlphaDict\"
${EndIf}

${If} $UnPageDictCheckbox_State == ${BST_CHECKED}
  RMDir /r "$INSTDIR\system\dicts"
  RMDir /r "$INSTDIR\system\xmls"
  RMDir /r "$INSTDIR\system\"
  RMDir /r "$INSTDIR"
${EndIf}

  ;!insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
  ;Delete "$SMPROGRAMS\$StartMenuFolder\AlphaDict.lnk"
  ;Delete "$SMPROGRAMS\$StartMenuFolder\Alpha词典.lnk"
  ;Delete "$SMPROGRAMS\$StartMenuFolder\AlphaDict (Windowed).lnk"
  ;Delete "$SMPROGRAMS\$StartMenuFolder\Visit AlphaDict Online.url"
  ;RMDir "$SMPROGRAMS\$StartMenuFolder"  
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AlphaDict"

  DeleteRegKey /ifempty HKLM "Software\AlphaDict"

SectionEnd
