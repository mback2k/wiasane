[Files]
Source: wiasane-pkg\Release\Win32\wiasane-pkg\winsane.dll; DestDir: {app}; Flags: overwritereadonly restartreplace 32bit; Check: Not Is64BitInstallMode
Source: wiasane-pkg\Release\Win32\wiasane-pkg\wiasane.dll; DestDir: {app}; Flags: overwritereadonly restartreplace 32bit; Check: Not Is64BitInstallMode
Source: wiasane-pkg\Release\Win32\wiasane-pkg\coisane.dll; DestDir: {app}; Flags: overwritereadonly restartreplace 32bit; Check: Not Is64BitInstallMode
Source: wiasane-pkg\Release\Win32\wiasane-pkg\wiasane.inf; DestDir: {app}; Flags: overwritereadonly restartreplace 32bit; Check: Not Is64BitInstallMode
Source: wiasane-pkg\Release\Win32\wiasane-pkg\wiasane.cat; DestDir: {app}; Flags: overwritereadonly restartreplace 32bit; Check: Not Is64BitInstallMode
Source: wiasane-pkg\Release\x64\wiasane-pkg\winsane.dll; DestDir: {app}; Flags: overwritereadonly restartreplace 64bit; Check: Is64BitInstallMode
Source: wiasane-pkg\Release\x64\wiasane-pkg\wiasane.dll; DestDir: {app}; Flags: overwritereadonly restartreplace 64bit; Check: Is64BitInstallMode
Source: wiasane-pkg\Release\x64\wiasane-pkg\coisane.dll; DestDir: {app}; Flags: overwritereadonly restartreplace 64bit; Check: Is64BitInstallMode
Source: wiasane-pkg\Release\x64\wiasane-pkg\wiasane.inf; DestDir: {app}; Flags: overwritereadonly restartreplace 64bit; Check: Is64BitInstallMode
Source: wiasane-pkg\Release\x64\wiasane-pkg\wiasane.cat; DestDir: {app}; Flags: overwritereadonly restartreplace 64bit; Check: Is64BitInstallMode
[Icons]
Name: {group}\{cm:StartInstallScanner}; Filename: {sys}\rundll32.exe; Parameters: .\coisane.dll,DeviceInstall {app}\wiasane.inf; WorkingDir: {app}; IconFilename: {uninstallexe}; AfterInstall: SetRunAsAdminFlag('{group}\{cm:StartInstallScanner}.lnk')
Name: {group}\{cm:StartUninstallDriver}; Filename: {uninstallexe}; IconFilename: {uninstallexe}
[Setup]
OutputDir=.
MinVersion=0,6.1.7600
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x86 x64
AppID={{B7D5E900-AF40-11DD-AD8B-0800200C9A65}
AppCopyright=Marc Hörsken
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
CloseApplications=no
SolidCompression=yes
SignTool=signtool
[Run]
Filename: {sys}\rundll32.exe; Parameters: .\coisane.dll,DriverInstall {app}\wiasane.inf; WorkingDir: {app}; Flags: waituntilterminated; StatusMsg: {cm:InstallDriver}
Filename: {sys}\rundll32.exe; Parameters: .\coisane.dll,DeviceInstall {app}\wiasane.inf; WorkingDir: {app}; Flags: waituntilterminated; StatusMsg: {cm:InstallDevice}
[UninstallRun]
Filename: {sys}\rundll32.exe; Parameters: .\coisane.dll,DriverUninstall {app}\wiasane.inf; WorkingDir: {app}; Flags: runhidden waituntilterminated; StatusMsg: {cm:UninstallDriver}
[UninstallDelete]
Name: {app}; Type: filesandordirs
[Languages]
Name: English; MessagesFile: compiler:Default.isl
Name: German; MessagesFile: compiler:Languages\German.isl
[CustomMessages]
StartInstallScanner=Install scanner
StartUninstallDriver=Uninstall driver
InstallDriver=Installing driver ...
InstallDevice=Installing device ...
UninstallDriver=Uninstalling driver ...
German.StartInstallScanner=Scanner installieren
German.StartUninstallDriver=Treiber deinstallieren
German.InstallDriver=Installiere Treiber ...
German.InstallDevice=Installiere Gerät ...
German.UninstallDriver=Deinstalliere Treiber ...
[Code]
const
  CLSID_ShellLink = '{00021401-0000-0000-C000-000000000046}';

type
  IShellLinkDataList = interface(IUnknown)
    '{45e2b4ae-b1c3-11d0-b92f-00a0c90312e1}'
    function AddDataBlock(pDataBlock: Cardinal): HResult;
    function CopyDataBlock(dwSig: DWORD; out ppDataBlock: Cardinal): HResult;
    function RemoveDataBlock(dwSig: DWORD): HResult;
    function GetFlags(out pdwFlags: DWORD): HResult;
    function SetFlags(dwFlags: DWORD): HResult;
  end;

  IPersist = interface(IUnknown)
    '{0000010C-0000-0000-C000-000000000046}'
    function GetClassID(var classID: TGUID): HResult;
  end;

  IPersistFile = interface(IPersist)
    '{0000010B-0000-0000-C000-000000000046}'
    function IsDirty: HResult;
    function Load(pszFileName: String; dwMode: Longint): HResult;
    function Save(pszFileName: String; fRemember: BOOL): HResult;
    function SaveCompleted(pszFileName: String): HResult;
    function GetCurFile(out pszFileName: String): HResult;
  end;

procedure SetRunAsAdminFlag(Filename: String);
var
  Obj: IUnknown;
  SL: IShellLinkDataList;
  PF: IPersistFile;
  FL: DWORD;
begin
  Filename := ExpandConstant(Filename);

  { Create the main ShellLink COM Automation object }
  Obj := CreateComObject(StringToGuid(CLSID_ShellLink));

  { Load the shortcut }
  PF := IPersistFile(Obj);
  OleCheck(PF.Load(Filename, 0));

  { Set the shortcut properties }
  SL := IShellLinkDataList(Obj);
  OleCheck(SL.GetFlags(FL));
  FL := FL or $00002000;
  OleCheck(SL.SetFlags(FL));

  { Save the shortcut }
  OleCheck(PF.Save(Filename, True));
end;
[_ISTool]
UseAbsolutePaths=false
