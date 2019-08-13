; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "AMT Pangaea CP PA"
#define MyAppVersion "1.0.1673a"
#define MyAppPublisher "AMT Electronics"
#define MyAppURL "http://www.amtelectronics.com/"
#define MyAppExeName "PangaeaCPPA.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{473EA954-005A-42BA-AFBD-B00D2BE0FFC9}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputDir=C:\projects\pangaea\distrib
OutputBaseFilename=PangaeaCPPA_Setup
SetupIconFile=pangaea_amt.ico
Compression=lzma
SolidCompression=yes
DisableDirPage=no


[Languages]
Name: english; MessagesFile: compiler:Default.isl
Name: russian; MessagesFile: compiler:Languages\Russian.isl

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked

[Files]
;Source: release\PangaeaCPPA.exe; DestDir: {app}; Flags: ignoreversion
Source: release\*; DestDir: {app}; Flags: ignoreversion recursesubdirs createallsubdirs
;Source: release\*.qml; DestDir: {app}; Flags: ignoreversion recursesubdirs createallsubdirs
;Source: release\*.qmlc; DestDir: {app}; Flags: ignoreversion recursesubdirs createallsubdirs
;Source: release\*.qmlc; DestDir: {app}; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: {group}\{#MyAppName}; Filename: {app}\{#MyAppExeName}
Name: {group}\{cm:ProgramOnTheWeb,{#MyAppName}}; Filename: {#MyAppURL}
Name: {group}\{cm:UninstallProgram,{#MyAppName}}; Filename: {uninstallexe}
Name: {commondesktop}\{#MyAppName}; Filename: {app}\{#MyAppExeName}; Tasks: desktopicon

[Run]
Filename: {app}\{#MyAppExeName}; Description: {cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}; Flags: nowait postinstall skipifsilent

