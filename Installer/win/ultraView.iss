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
AppName={#MyAppCompany} {#MyAppName} {#MyAppVersion}
AppVerName={#MyAppCompany} {#MyAppName} {#MyAppVersion}
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
OutputBaseFilename=ultraView
Compression=lzma/ultra
SolidCompression=true
ShowLanguageDialog=auto
InternalCompressLevel=ultra
MinVersion=0,6.1.7600
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

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Files]
Source: "stage\ultraView.exe"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly
Source: "stage\Data\*";       DestDir: "{commonappdata}\ultraView"; Flags: ignoreversion overwritereadonly recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}";           Filename: "{app}\ultraView.exe"
Name: "{commondesktop}\{#MyAppName}";   Filename: "{app}\ultraView.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional icons:"

[Run]
Filename: "{app}\ultraView.exe"; Description: "Launch ultraView"; Flags: nowait postinstall skipifsilent
