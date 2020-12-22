# This installs two files, app.exe and logo.ico, creates a start menu shortcut, builds an uninstaller, and
# adds uninstall information to the registry for Add/Remove Programs
 
# To get started, put this script into a folder with the two files (app.exe, logo.ico, and license.rtf -
# You'll have to create these yourself) and run makensis on it
 
Unicode True
ManifestDPIAware true

!include "UMUI.nsh"
!include "x64.nsh"


; First is default

# If you change the names "app.exe", "logo.ico", or "license.rtf" you should do a search and replace - they
# show up in a few places.
# All the other settings can be tweaked by editing the !defines at the top of this script
!define APPNAME "hörbert"
!define COMPANYNAME "WINZKI GmbH & Co. KG"
!define DESCRIPTION "Simply manage playlists of your hörbert"
# These three must be integers
!define VERSIONMAJOR 1
!define VERSIONMINOR 2
!define VERSIONBUILD 2
# These will be displayed by the "Click here for support information" link in "Add/Remove Programs"
# It is possible to use "mailto:" links in here to open the email client
!define HELPURL "https://www.hoerbert.com/service/" # "Support Information" link
!define UPDATEURL "https://www.hoerbert.com/software_update" # "Product Updates" link
!define ABOUTURL "http://www.hoerbert.com/" # "Publisher" link
# This is the size (in kB) of all the files copied into "Program Files"
!define INSTALLSIZE 153600
!define UMUI_SKIN SoftBrown
!define MUI_UNFINISHPAGE_NOAUTOCLOSE 1
!define UMUI_LEFTIMAGE_BMP "Left_hoerbert.bmp"
!define MUI_ICON "hoerbert\hoerbert.ico"
!define SIGNTOOLEXE "$\"C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.18362.0\x64\signtool$\""

RequestExecutionLevel admin ;Require admin rights on NT6+ (When UAC is turned on)
 
InstallDir "$PROGRAMFILES\hoerbert\${APPNAME}"
SetCompressor lzma
 
# rtf or txt file - remember if it is txt, it must be in the DOS text format (\r\n)
# This will be in the installer/uninstaller's title bar
Name "${APPNAME}"
 
!include LogicLib.nsh
 
# Just three pages - license agreement, install location, and installation
!insertmacro MUI_PAGE_LICENSE "License.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES 
!define MUI_FINISHPAGE_RUN "$INSTDIR\hoerbert.exe"
!insertmacro MUI_PAGE_FINISH

!ifdef INNER  
	!insertmacro MUI_UNPAGE_CONFIRM
	!insertmacro MUI_UNPAGE_INSTFILES
	!insertmacro MUI_UNPAGE_FINISH
!endif

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "French"

!macro VerifyUserIsAdmin
UserInfo::GetAccountType
pop $0
${If} $0 != "admin" ;Require admin rights on NT4+
        messageBox mb_iconstop "Administrator rights required!"
        setErrorLevel 740 ;ERROR_ELEVATION_REQUIRED
        quit
${EndIf}
!macroend

############################################## generate uninstaller and sign it #########################################
!ifdef INNER
  !echo "Inner invocation"                  ; just to see what's going on
  OutFile "$%TEMP%\tempinstaller.exe"       ; not really important where this is
  SetCompress off                           ; for speed
!else
  !echo "Outer invocation"
 
  ; Call makensis again against current file, defining INNER.  This writes an installer for us which, when it is invoked, 
  ; will just write the uninstaller to some location, and then exit.
  !makensis '/DINNER "${__FILE__}"' = 0
 
  ; So now run that installer we just created as %TEMP%\tempinstaller.exe. Since it calls quit the return value isn't zero.
  !system 'set __COMPAT_LAYER=RunAsInvoker&"$%TEMP%\tempinstaller.exe"' = 2
 
  ; That will have written an uninstaller binary for us.  Now we sign it with your favorite code signing tool.
  !system "${SIGNTOOLEXE} sign $%TEMP%\uninstaller.exe" = 0
  !system "${SIGNTOOLEXE} timestamp /t http://timestamp.comodoca.com $%TEMP%\uninstaller.exe" = 0
  !system "${SIGNTOOLEXE} verify /pa /d /v $%TEMP%\uninstaller.exe" = 0
 
  ; Good.  Now we can carry on writing the real installer.
  OutFile "..\Build32\hoerbert-installer.exe"
!endif
############################################## /generate uninstaller and sign it #########################################

 
function .onInit
	!ifdef INNER
	  ; If INNER is defined, then we aren't supposed to do anything except write out
	  ; the uninstaller.  This is better than processing a command line option as it means
	  ; this entire code path is not present in the final (real) installer.
	  SetSilent silent
	  WriteUninstaller "$%TEMP%\uninstaller.exe"
	  Quit  ; just bail out quickly when running the "inner" installer
	!endif

	setShellVarContext all
	!insertmacro VerifyUserIsAdmin
 
	${If} ${RunningX64}
		StrCpy $INSTDIR "$PROGRAMFILES64\hoerbert\${APPNAME}"	# Set "Program Files" as destination under X64
	${EndIf}

functionEnd
 
section "install"
	# Files for the install directory - to build the installer, these should be in the same directory as the install script (this file)
	setOutPath $INSTDIR

	# Files added here should be removed by the uninstaller (see section "uninstall")
	${If} ${RunningX64}
		File /r "..\Build64\Build\bin\*.*"
	${Else}
		File /r "..\Build32\Build\bin\*.*"
	${EndIf}

	# Add any other files for the install directory (license files, app data, etc) here
	File "hoerbert\hoerbert.ico"
 
	# Uninstaller - See function un.onInit and section "uninstall" for configuration
	#writeUninstaller "$INSTDIR\uninstall.exe"
	!ifndef INNER
	  ; this packages the signed uninstaller	 
	  File $%TEMP%\uninstaller.exe
	!endif	
 
	# Start Menu
	createDirectory "$SMPROGRAMS\${APPNAME}"
	createShortCut "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk" "$INSTDIR\hoerbert.exe" "" "$INSTDIR\hoerbert.ico"
 
	# Registry information for add/remove programs
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName" "${APPNAME}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "InstallLocation" "$\"$INSTDIR$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayIcon" "$\"$INSTDIR\hoerbert.ico$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "Publisher" "$\"${COMPANYNAME}$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "HelpLink" "$\"${HELPURL}$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "URLUpdateInfo" "$\"${UPDATEURL}$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "URLInfoAbout" "$\"${ABOUTURL}$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayVersion" "$\"${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}$\""
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "VersionMajor" ${VERSIONMAJOR}
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "VersionMinor" ${VERSIONMINOR}
	# There is no option for modifying or repairing the install
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoRepair" 1
	# Set the INSTALLSIZE constant (!defined at the top of this script) so Add/Remove Programs can accurately report the size
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "EstimatedSize" ${INSTALLSIZE}

	# Now sign the installer, too.
	!ifndef INNER
		!finalize '${SIGNTOOLEXE} sign "%1"'
		!finalize '${SIGNTOOLEXE} timestamp /t http://timestamp.comodoca.com "%1"'
		!finalize '${SIGNTOOLEXE} verify /pa /d /v "%1"'
	!endif
sectionEnd


!ifdef INNER
	# Uninstaller
	function un.onInit
		SetShellVarContext all
	 
		#Verify the uninstaller - last chance to back out
	#	MessageBox MB_OKCANCEL "Permanently remove ${APPNAME}?" IDOK next
	#		Abort
	#	next:
		!insertmacro VerifyUserIsAdmin
	functionEnd 

	; your normal uninstaller section is only needed in the "inner" installer and not needed in the "outer"
  	; installer where it would just cause warnings because there is no WriteUninstaller command in the outer installer

	section "uninstall"
	 
		# Remove Start Menu launcher
		delete "$SMPROGRAMS\hörbert\${APPNAME}.lnk"
		# Try to remove the Start Menu folder - this will only happen if it is empty
		rmDir "$SMPROGRAMS\hörbert"
	 
		# Remove files
		delete "$INSTDIR\7z\*.*"
		rmDir "$INSTDIR\7z"
		delete "$INSTDIR\bearer\*.*"
		rmdir "$INSTDIR\bearer"
		delete "$INSTDIR\diagnostics\0\*.*"
		rmdir "$INSTDIR\diagnostics\0"
		delete "$INSTDIR\diagnostics\1\*.*"
		rmdir "$INSTDIR\diagnostics\1"
		delete "$INSTDIR\diagnostics\2\*.*"
		rmdir "$INSTDIR\diagnostics\2"
		delete "$INSTDIR\diagnostics\3\*.*"
		rmdir "$INSTDIR\diagnostics\3"
		delete "$INSTDIR\diagnostics\4\*.*"
		rmdir "$INSTDIR\diagnostics\4"
		delete "$INSTDIR\diagnostics\5\*.*"
		rmdir "$INSTDIR\diagnostics\5"
		delete "$INSTDIR\diagnostics\6\*.*"
		rmdir "$INSTDIR\diagnostics\6"
		delete "$INSTDIR\diagnostics\7\*.*"
		rmdir "$INSTDIR\diagnostics\7"
		delete "$INSTDIR\diagnostics\8\*.*"
		rmdir "$INSTDIR\diagnostics\8"
		delete "$INSTDIR\diagnostics\*.*"
		rmdir "$INSTDIR\diagnostics"
		delete "$INSTDIR\EjectMedia\*.*"
		rmdir "$INSTDIR\EjectMedia"
		delete "$INSTDIR\freac\boca\*.*"
		rmdir "$INSTDIR\freac\boca"
		delete "$INSTDIR\freac\codecs\*.*"
		rmdir "$INSTDIR\freac\codecs"
		delete "$INSTDIR\freac\*.*"
		rmdir "$INSTDIR\freac"
		delete "$INSTDIR\ffmpeg\*.*"
		rmdir "$INSTDIR\ffmpeg"
		delete "$INSTDIR\iconengines\*.*"
		rmdir "$INSTDIR\iconengines"
		delete "$INSTDIR\imageformats\*.*"
		rmdir "$INSTDIR\imageformats"
		delete "$INSTDIR\platforms\*.*"
		rmdir "$INSTDIR\platforms"
		delete "$INSTDIR\styles\*.*"
		rmdir "$INSTDIR\styles"
		delete "$INSTDIR\Sync\*.*"
		rmdir "$INSTDIR\Sync"
		delete "$INSTDIR\translations\*.*"
		rmdir "$INSTDIR\translations"
		delete "$INSTDIR\*.*"
	 
		# Always delete uninstaller as the last action
		delete $INSTDIR\uninstaller.exe
	 
		# Try to remove the install directory - this will only happen if it is empty
		rmDir $INSTDIR
	 
		# Remove uninstaller information from the registry
		DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
	sectionEnd
!endif 
