; Inno-Setup-Skript fuer WinTangle.
;
; Bewusst eine Installation pro Benutzer (PrivilegesRequired=lowest): das
; Programm braucht keine erhoehten Rechte, und ohne UAC-Abfrage kann auch der
; WinSparkle-Updater das Setup unbeaufsichtigt ausfuehren.
;
; Die Version kommt ueber /DAppVersion=... aus dem Release-Workflow.

#ifndef AppVersion
  #define AppVersion "0.0.0"
#endif
#ifndef SourceDir
  #define SourceDir "..\build\src\platform\Release"
#endif

#define AppName "WinTangle"
#define AppPublisher "Andreas Hacker"
#define AppUrl "https://github.com/kirkanos/winTangle"
#define AppExe "wintangle.exe"

[Setup]
AppId={{7F4C1E92-3A6D-4B18-9C55-2E8D0A17B3F1}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppUrl}
AppSupportURL={#AppUrl}/issues
AppUpdatesURL={#AppUrl}/releases
VersionInfoVersion={#AppVersion}

DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
DisableDirPage=auto
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog

OutputDir=..\dist
OutputBaseFilename=wintangle-{#AppVersion}-setup
SetupIconFile=..\resources\wintangle.ico
UninstallDisplayIcon={app}\{#AppExe}
WizardStyle=modern
Compression=lzma2/max
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

; GPL: die Lizenz wird im Setup angezeigt.
LicenseFile=..\LICENSE

; Verhindert, dass ein Update ueber eine laufende Instanz installiert wird.
CloseApplications=yes
RestartApplications=no

[Languages]
Name: "deutsch"; MessagesFile: "compiler:Languages\German.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "autostart"; Description: "{cm:AutoStartProgram,{#AppName}}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#SourceDir}\{#AppExe}"; DestDir: "{app}"; Flags: ignoreversion
; WinSparkle wird nur mitgeliefert, wenn mit Update-Pruefung gebaut wurde.
Source: "{#SourceDir}\WinSparkle.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\CHANGELOG.md"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\{#AppExe}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExe}"; Tasks: desktopicon

[Registry]
; Autostart ueber denselben Schluessel, den das Programm selbst verwendet --
; der Schalter im Tray-Menue und dieser Haken meinen dieselbe Einstellung.
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; \
    ValueType: string; ValueName: "WinTangle"; ValueData: """{app}\{#AppExe}"""; \
    Flags: uninsdeletevalue; Tasks: autostart

; URL-Protokoll wintangle://, damit es auch ohne vorherigen Programmstart geht.
Root: HKCU; Subkey: "Software\Classes\wintangle"; ValueType: string; \
    ValueData: "URL:WinTangle Protocol"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\wintangle"; ValueType: string; \
    ValueName: "URL Protocol"; ValueData: ""
Root: HKCU; Subkey: "Software\Classes\wintangle\shell\open\command"; ValueType: string; \
    ValueData: """{app}\{#AppExe}"" ""%1"""

[Run]
Filename: "{app}\{#AppExe}"; Description: "{cm:LaunchProgram,{#AppName}}"; \
    Flags: nowait postinstall skipifsilent

[UninstallDelete]
; Die Konfiguration bleibt bewusst stehen: wer nur aktualisiert, will seine
; Tastenbelegung behalten. Vollstaendig entfernen kann man sie von Hand unter
; %APPDATA%\WinTangle.
Type: dirifempty; Name: "{userappdata}\WinTangle"
