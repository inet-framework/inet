;NSIS Modern User Interface version 1.70
;INET Demo install script
;Author: Andras Varga

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;Defines

  !define Product            "INET Framework Demo"
  !define Version            "20061020"
  !define ProductWithVersion "${Product} ${Version}"
  !define DefaultInstallDir  "$PROGRAMFILES\${Product}"
  !define StartMenuFolder    "$SMPROGRAMS\INET Framework for OMNeT++ Demo"
  !define SwRegistryKey      "SOFTWARE\${ProductWithVersion}"
  !define SrcDir             "E:\INET Demo"

;--------------------------------
;General

  ;Name and file
  Name "INET Framework for OMNeT++ Demo"
  OutFile "INETDemo-${Version}-win32.exe"

  ;Default installation folder
  InstallDir "${DefaultInstallDir}"

  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "${SwRegistryKey}" ""

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "${SrcDir}\License.txt"
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES

  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section ""

  SetOutPath "$INSTDIR"

  File /r "${SrcDir}\*"

  FileOpen $0 "Version.txt" "w"
  FileWrite $0 "${ProductWithVersion}"
  FileClose $0

  CreateDirectory "${StartMenuFolder}"
  CreateShortCut "${StartMenuFolder}\${Product}.lnk" "$INSTDIR\rundemo.bat" "" ""
  CreateShortCut "${StartMenuFolder}\Uninstall.lnk" "$INSTDIR\Uninstall.exe" "" ""

  ;Store installation folder
  WriteRegStr HKCU "${SwRegistryKey}" "" $INSTDIR

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"

  ;Register uninstaller
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductWithVersion}" "DisplayName" "${ProductWithVersion}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductWithVersion}" "UninstallString" '"$INSTDIR\uninstall.exe"'

SectionEnd

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  Delete "$INSTDIR\Uninstall.exe"

  RMDir /r "$INSTDIR"
  RMDir /r "${StartMenuFolder}"

  DeleteRegKey /ifempty HKCU "${SwRegistryKey}"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductWithVersion}"

SectionEnd
