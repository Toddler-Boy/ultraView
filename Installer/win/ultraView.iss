; ultraView installer (Inno Setup)

#define MyAppName "ultraView"
#define MyAppCompany "Michael Hartmann"
#define MyAppPublisher "Michael Hartmann"
#define MyAppCopyright "2026 Michael Hartmann"
#define MyAppURL "https://ultrasid.com/"
#define MyAppVersion GetStringFileInfo("stage\ultraView.exe", "ProductVersion")
#define MyDefaultDirName "{autopf}\ultraView"

[Setup]
AppID={{E8B2F4A1-7C3D-4E5F-9A1B-2D3E4F5A6B7C}
AppName={#MyAppName} {#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppVersion={#MyAppVersion}
AppCopyright={#MyAppCopyright}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={#MyDefaultDirName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputDir=.\bin
OutputBaseFilename={#MyAppName}_{#MyAppVersion}
OutputManifestFile=manifest.txt
Compression=lzma/ultra
SolidCompression=true
ShowLanguageDialog=auto
InternalCompressLevel=ultra
MinVersion=10.0.14393
AlwaysShowDirOnReadyPage=yes
DisableWelcomePage=no
DisableReadyPage=no
DisableReadyMemo=no
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
VersionInfoVersion={#MyAppVersion}
VersionInfoCompany={#MyAppPublisher}
VersionInfoCopyright={#MyAppCopyright}
VersionInfoProductName={#MyAppName} {#MyAppVersion} (64-bit)
VersionInfoProductVersion={#MyAppVersion}
VersionInfoProductTextVersion={#MyAppVersion}
UsePreviousGroup=False
PrivilegesRequired=admin
WizardSmallImageFile=..\..\icons\windows_big.png

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Files]
; Data.pak sits next to the exe, where the installed-location probe looks
Source: "stage\ultraView.exe"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly
Source: "stage\Data.pak";      DestDir: "{app}"; Flags: ignoreversion overwritereadonly

[InstallDelete]
; 1.0.x shipped a naked Data tree into ProgramData; nothing reads it anymore
Type: filesandordirs; Name: "{commonappdata}\ultraView"
Type: filesandordirs; Name: "{app}\Data"

[Icons]
Name: "{group}\{#MyAppName}";           Filename: "{app}\ultraView.exe"
Name: "{commondesktop}\{#MyAppName}";   Filename: "{app}\ultraView.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional icons:"

[Run]
Filename: "{app}\ultraView.exe"; WorkingDir: "{app}"; Description: "Launch ultraView"; Flags: runasoriginaluser postinstall nowait skipifsilent
