#ifndef Configuration
#define Configuration "Release"
#endif
#if Configuration == "Release"
#define SourceDir32bit "wiasane-pkg\Release\Win32\wiasane-pkg"
#define SourceDir64bit "wiasane-pkg\Release\x64\wiasane-pkg"
#endif
#if Configuration == "Debug"
#define SourceDir32bit "wiasane-pkg\Debug\Win32\wiasane-pkg"
#define SourceDir64bit "wiasane-pkg\Debug\x64\wiasane-pkg"
#endif
[Files]
Source: {#SourceDir32bit}\winsane.dll; DestDir: {app}; Flags: overwritereadonly restartreplace 32bit; Check: Not Is64BitInstallMode
Source: {#SourceDir32bit}\wiasane.dll; DestDir: {app}; Flags: overwritereadonly restartreplace 32bit; Check: Not Is64BitInstallMode
Source: {#SourceDir32bit}\coisane.dll; DestDir: {app}; Flags: overwritereadonly restartreplace 32bit; Check: Not Is64BitInstallMode
Source: {#SourceDir32bit}\devsane.exe; DestDir: {app}; Flags: overwritereadonly restartreplace 32bit; Check: Not Is64BitInstallMode
Source: {#SourceDir32bit}\wiasane.inf; DestDir: {app}; Flags: overwritereadonly restartreplace 32bit; Check: Not Is64BitInstallMode
Source: {#SourceDir32bit}\wiasane.cat; DestDir: {app}; Flags: overwritereadonly restartreplace 32bit; Check: Not Is64BitInstallMode
Source: {#SourceDir64bit}\winsane.dll; DestDir: {app}; Flags: overwritereadonly restartreplace 64bit; Check: Is64BitInstallMode
Source: {#SourceDir64bit}\wiasane.dll; DestDir: {app}; Flags: overwritereadonly restartreplace 64bit; Check: Is64BitInstallMode
Source: {#SourceDir64bit}\coisane.dll; DestDir: {app}; Flags: overwritereadonly restartreplace 64bit; Check: Is64BitInstallMode
Source: {#SourceDir64bit}\devsane.exe; DestDir: {app}; Flags: overwritereadonly restartreplace 64bit; Check: Is64BitInstallMode
Source: {#SourceDir64bit}\wiasane.inf; DestDir: {app}; Flags: overwritereadonly restartreplace 64bit; Check: Is64BitInstallMode
Source: {#SourceDir64bit}\wiasane.cat; DestDir: {app}; Flags: overwritereadonly restartreplace 64bit; Check: Is64BitInstallMode
[Icons]
Name: {group}\{cm:StartInstallDevice}; Filename: {app}\devsane.exe; Parameters: device install wiasane.inf; WorkingDir: {app}; IconFilename: sti.dll
Name: {group}\{cm:StartUninstallDriver}; Filename: {uninstallexe}; IconFilename: devmgr.dll; IconIndex: 4
[Setup]
OutputDir=.
MinVersion=0,6.1.7600
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x86 x64
AppID={{B7D5E900-AF40-11DD-AD8B-0800200C9A65}
AppCopyright=Copyright (C) 2013 Marc Hörsken. All rights reserved.
AppPublisher=Marc Hörsken
AppPublisherURL=http://wiasane.marc-hoersken.de/
AppName=Scanner Access Now Easy - WIA Driver
AppVersion=0.0.0.1
VersionInfoVersion=0.0.0.1
DefaultDirName={pf}\SANE WIA Driver
DefaultGroupName=SANE WIA Driver
OutputBaseFilename=wiasane
ShowLanguageDialog=no
WizardImageFile=compiler:wizmodernimage.bmp
WizardSmallImageFile=compiler:wizmodernsmallimage.bmp
LicenseFile=COPYING
CloseApplications=no
SolidCompression=yes
SignTool=signtool
[Run]
Filename: {app}\devsane.exe; Parameters: driver install wiasane.inf {hwnd}; WorkingDir: {app}; Flags: waituntilterminated; StatusMsg: {cm:InstallDriver}
Filename: {app}\devsane.exe; Parameters: device install wiasane.inf {hwnd}; WorkingDir: {app}; Flags: waituntilterminated; StatusMsg: {cm:InstallDevice}
[UninstallRun]
Filename: {app}\devsane.exe; Parameters: device uninstall wiasane.inf {hwnd}; WorkingDir: {app}; Flags: runhidden waituntilterminated; StatusMsg: {cm:UninstallDevice}
Filename: {app}\devsane.exe; Parameters: driver uninstall wiasane.inf {hwnd}; WorkingDir: {app}; Flags: runhidden waituntilterminated; StatusMsg: {cm:UninstallDriver}
[UninstallDelete]
Name: {app}; Type: filesandordirs
[Languages]
Name: English; MessagesFile: compiler:Default.isl
Name: German; MessagesFile: compiler:Languages\German.isl
[CustomMessages]
StartInstallDevice=Install scanner
StartUninstallDevice=Uninstall scanners
StartUninstallDriver=Uninstall driver
InstallDriver=Installing driver ...
InstallDevice=Installing device ...
UninstallDevice=Uninstalling devices ...
UninstallDriver=Uninstalling driver ...
German.StartInstallDevice=Scanner installieren
German.StartUninstallDevice=Scanner deinstallieren
German.StartUninstallDriver=Treiber deinstallieren
German.InstallDriver=Installiere Treiber ...
German.InstallDevice=Installiere Gerät ...
German.UninstallDevice=Deinstalliere Geräte ...
German.UninstallDriver=Deinstalliere Treiber ...
[_ISTool]
UseAbsolutePaths=false
