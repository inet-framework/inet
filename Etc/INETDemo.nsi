;NSIS Modern User Interface version 1.70
;INET Demo install script
;Andras Varga

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;Defines

  !define Version           "20041019"
  !define DefaultInstallDir "$PROGRAMFILES\INET Demo"
  !define StartMenuFolder   "$SMPROGRAMS\INET Framework for OMNeT++ Demo"
  !define SwRegistryKey     "SOFTWARE\INET"
  !define SrcDir            "D:\home\INET-Demo"

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
  FileWrite $0 "INET Demo ${Version}"
  FileClose $0

  CreateDirectory "${StartMenuFolder}"
  CreateShortCut "${StartMenuFolder}\INET Framework Demo.lnk" "$INSTDIR\rundemo.bat" "" ""
  CreateShortCut "${StartMenuFolder}\Uninstall.lnk" "$INSTDIR\Uninstall.exe" "" ""

  ;Store installation folder
  WriteRegStr HKCU "${SwRegistryKey}" "" $INSTDIR

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  Delete "$INSTDIR\Uninstall.exe"

  RMDir /r "$INSTDIR"
  RMDir /r "${StartMenuFolder}"

  DeleteRegKey /ifempty HKCU "${SwRegistryKey}"

SectionEnd
