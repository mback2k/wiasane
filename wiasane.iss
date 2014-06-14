#ifndef Configuration
#define Configuration "Release"
#endif
#if Configuration == "Release"
#define RedistDir32bit "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\redist\x86\Microsoft.VC110.CRT"
#define RedistDir64bit "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\redist\x64\Microsoft.VC110.CRT"
#define SourceDir32bit "wiasane-pkg\Release\Win32\wiasane-pkg"
#define SourceDir64bit "wiasane-pkg\Release\x64\wiasane-pkg"
#endif
#if Configuration == "Debug"
#define RedistDir32bit "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\redist\Debug_NonRedist\x86\Microsoft.VC110.DebugCRT"
#define RedistDir64bit "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\redist\Debug_NonRedist\x64\Microsoft.VC110.DebugCRT"
#define SourceDir32bit "wiasane-pkg\Debug\Win32\wiasane-pkg"
#define SourceDir64bit "wiasane-pkg\Debug\x64\wiasane-pkg"
#endif
[Files]
Source: {#RedistDir32bit}\*.dll; DestDir: {sys}; Flags: sharedfile uninsnosharedfileprompt uninsrestartdelete 32bit; Check: Not Is64BitInstallMode
Source: {#SourceDir32bit}\winsane.dll; DestDir: {app}; Flags: overwritereadonly replacesameversion restartreplace uninsrestartdelete 32bit; Check: Not Is64BitInstallMode
Source: {#SourceDir32bit}\wiasane.dll; DestDir: {app}; Flags: overwritereadonly replacesameversion restartreplace uninsrestartdelete 32bit; Check: Not Is64BitInstallMode
Source: {#SourceDir32bit}\coisane.dll; DestDir: {app}; Flags: overwritereadonly replacesameversion restartreplace uninsrestartdelete 32bit; Check: Not Is64BitInstallMode
Source: {#SourceDir32bit}\devsane.exe; DestDir: {app}; Flags: overwritereadonly replacesameversion restartreplace uninsrestartdelete 32bit; Check: Not Is64BitInstallMode
Source: {#SourceDir32bit}\wiasane.inf; DestDir: {app}; Flags: overwritereadonly replacesameversion restartreplace uninsrestartdelete 32bit; Check: Not Is64BitInstallMode
Source: {#SourceDir32bit}\wiasane.cat; DestDir: {app}; Flags: overwritereadonly replacesameversion restartreplace uninsrestartdelete 32bit; Check: Not Is64BitInstallMode
Source: {#RedistDir64bit}\*.dll; DestDir: {sys}; Flags: sharedfile uninsnosharedfileprompt uninsrestartdelete 64bit; Check: Is64BitInstallMode
Source: {#SourceDir64bit}\winsane.dll; DestDir: {app}; Flags: overwritereadonly replacesameversion restartreplace uninsrestartdelete 64bit; Check: Is64BitInstallMode
Source: {#SourceDir64bit}\wiasane.dll; DestDir: {app}; Flags: overwritereadonly replacesameversion restartreplace uninsrestartdelete 64bit; Check: Is64BitInstallMode
Source: {#SourceDir64bit}\coisane.dll; DestDir: {app}; Flags: overwritereadonly replacesameversion restartreplace uninsrestartdelete 64bit; Check: Is64BitInstallMode
Source: {#SourceDir64bit}\devsane.exe; DestDir: {app}; Flags: overwritereadonly replacesameversion restartreplace uninsrestartdelete 64bit; Check: Is64BitInstallMode
Source: {#SourceDir64bit}\wiasane.inf; DestDir: {app}; Flags: overwritereadonly replacesameversion restartreplace uninsrestartdelete 64bit; Check: Is64BitInstallMode
Source: {#SourceDir64bit}\wiasane.cat; DestDir: {app}; Flags: overwritereadonly replacesameversion restartreplace uninsrestartdelete 64bit; Check: Is64BitInstallMode
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
AppVersion=0.0.0.5
VersionInfoVersion=0.0.0.5
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
FailedInstallDriver=Installation of driver failed!%n%nError = %1%n%nError Code = %2
FailedInstallDevice=Installation of device failed!%n%nError = %1%n%nError Code = %2
FailedUninstallDevice=Uninstallation of devices failed!%n%nError = %1%n%nError Code = %2
FailedUninstallDriver=Uninstallation of driver failed!%n%nError = %1%n%nError Code = %2
FinishInstallAction=Please execute the "Finish installing device software" action within the info area resp. system tray.
German.StartInstallDevice=Scanner installieren
German.StartUninstallDevice=Scanner deinstallieren
German.StartUninstallDriver=Treiber deinstallieren
German.InstallDriver=Installiere Treiber ...
German.InstallDevice=Installiere Gerät ...
German.UninstallDevice=Deinstalliere Geräte ...
German.UninstallDriver=Deinstalliere Treiber ...
German.FailedInstallDriver=Installation des Treibers fehlgeschlagen!%n%nFehler = %1%n%nFehler Code = %2
German.FailedInstallDevice=Installation des Geräts fehlgeschlagen!%n%nFehler = %1%n%nFehler Code = %2
German.FailedUninstallDevice=Deinstallation der Geräte fehlgeschlagen!%n%nFehler = %1%n%nFehler Code = %2
German.FailedUninstallDriver=Deinstallation des Treibers fehlgeschlagen!%n%nFehler = %1%n%nFehler Code = %2
German.FinishInstallAction=Bitte die Aktion "Installation der Gerätesoftware abschließen" im Infobereich bzw. System Tray ausführen.
[Code]
const
  ERROR_NO_SUCH_DEVINST             = $E000020B;
  ERROR_DRIVER_PACKAGE_NOT_IN_STORE = $E0000302;
procedure CurStepChanged(CurStep: TSetupStep);
var
  ResultCode: integer;
  StatusLabel: string;
begin
  if CurStep = ssInstall then
  begin
    StatusLabel := WizardForm.StatusLabel.Caption;
    if FileExists(ExpandConstant('{app}\devsane.exe')) and FileExists(ExpandConstant('{app}\wiasane.inf')) then
    begin
      WizardForm.StatusLabel.Caption := CustomMessage('UninstallDriver');
      if not Exec(ExpandConstant('{app}\devsane.exe'), ExpandConstant('driver uninstall wiasane.inf'), ExpandConstant('{app}'), SW_HIDE, ewWaitUntilTerminated, ResultCode)
          or ((ResultCode <> 0) and (ResultCode <> ERROR_DRIVER_PACKAGE_NOT_IN_STORE)) then
        RaiseException(FmtMessage(CustomMessage('FailedUninstallDriver'), [SysErrorMessage(ResultCode), Format('%.8x', [ResultCode])]))
      else
        Sleep(1000);
    end;
    WizardForm.StatusLabel.Caption := StatusLabel;
  end
  else if CurStep = ssPostInstall then
  begin
    StatusLabel := WizardForm.StatusLabel.Caption;
    WizardForm.StatusLabel.Caption := CustomMessage('InstallDriver');
    if not Exec(ExpandConstant('{app}\devsane.exe'), ExpandConstant('driver install wiasane.inf {hwnd}'), ExpandConstant('{app}'), SW_SHOW, ewWaitUntilTerminated, ResultCode)
        or ((ResultCode <> 0) and (ResultCode <> ERROR_NO_SUCH_DEVINST)) then
      RaiseException(FmtMessage(CustomMessage('FailedInstallDriver'), [SysErrorMessage(ResultCode), Format('%.8x', [ResultCode])]))
    else
      Sleep(500);
    if ResultCode = ERROR_NO_SUCH_DEVINST then
    begin
      WizardForm.StatusLabel.Caption := CustomMessage('InstallDevice');
      if not Exec(ExpandConstant('{app}\devsane.exe'), ExpandConstant('device install wiasane.inf {hwnd}'), ExpandConstant('{app}'), SW_SHOW, ewWaitUntilTerminated, ResultCode)
          or (ResultCode <> 0) then
        RaiseException(FmtMessage(CustomMessage('FailedInstallDevice'), [SysErrorMessage(ResultCode), Format('%.8x', [ResultCode])]))
      else
      begin
        Sleep(500);
        if GetWindowsVersion >= $06020000 then
          MsgBox(CustomMessage('FinishInstallAction'), mbInformation, MB_OK);
      end;
    end;
    WizardForm.StatusLabel.Caption := StatusLabel;
  end;
end;
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  ResultCode: integer;
  StatusLabel: string;
begin
  if CurUninstallStep = usUninstall then
  begin
    StatusLabel := UninstallProgressForm.StatusLabel.Caption;
    UninstallProgressForm.StatusLabel.Caption := CustomMessage('UninstallDevice');
    if not Exec(ExpandConstant('{app}\devsane.exe'), ExpandConstant('device uninstall wiasane.inf'), ExpandConstant('{app}'), SW_HIDE, ewWaitUntilTerminated, ResultCode)
        or ((ResultCode <> 0) and (ResultCode <> ERROR_DRIVER_PACKAGE_NOT_IN_STORE)) then
      RaiseException(FmtMessage(CustomMessage('FailedUninstallDevice'), [SysErrorMessage(ResultCode), Format('%.8x', [ResultCode])]))
    else
      Sleep(500);
    UninstallProgressForm.StatusLabel.Caption := CustomMessage('UninstallDriver');
    if not Exec(ExpandConstant('{app}\devsane.exe'), ExpandConstant('driver uninstall wiasane.inf'), ExpandConstant('{app}'), SW_HIDE, ewWaitUntilTerminated, ResultCode)
        or ((ResultCode <> 0) and (ResultCode <> ERROR_DRIVER_PACKAGE_NOT_IN_STORE)) then
      RaiseException(FmtMessage(CustomMessage('FailedUninstallDriver'), [SysErrorMessage(ResultCode), Format('%.8x', [ResultCode])]))
    else
      Sleep(500);
    UninstallProgressForm.StatusLabel.Caption := StatusLabel;
  end;
end;
[_ISTool]
UseAbsolutePaths=false
