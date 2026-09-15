; Inno Setup script for WinTangle.
;
; Deliberately a per-user installation (PrivilegesRequired=lowest): the program
; needs no elevated rights, and without a UAC prompt the WinSparkle updater can
; run the setup unattended.
;
; The version arrives as /DAppVersion=... from the release workflow.

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

; GPL: the licence is shown during setup.
LicenseFile=..\LICENSE

; Prevents an update from being installed over a running instance.
CloseApplications=yes
RestartApplications=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"

; The wizard's own text comes from the language files above; only this script's
; own prompt needs translating by hand.
[CustomMessages]
english.RemoveSettingsPrompt=Remove WinTangle's personal data as well?%n%n• settings and key bindings%n• stored update state%n%nWindows' own snapping will be switched back on if WinTangle turned it off.%n%nChoosing No keeps your settings for a later installation.
german.RemoveSettingsPrompt=Sollen auch die persönlichen Daten von WinTangle entfernt werden?%n%n• Einstellungen und Tastenbelegung%n• gespeicherter Update-Zustand%n%nDas Windows-eigene Andocken wird dabei wieder eingeschaltet, falls WinTangle es abgeschaltet hat.%n%nBei Nein bleiben die Einstellungen für eine spätere Installation erhalten.

[Tasks]
Name: "autostart"; Description: "{cm:AutoStartProgram,{#AppName}}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#SourceDir}\{#AppExe}"; DestDir: "{app}"; Flags: ignoreversion
; WinSparkle only ships when the build included update checking.
Source: "{#SourceDir}\WinSparkle.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\CHANGELOG.md"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\{#AppExe}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExe}"; Tasks: desktopicon

[Registry]
; Autostart through the same key the program itself uses -- the tray menu
; switch and this checkbox mean the same setting.
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; \
    ValueType: string; ValueName: "WinTangle"; ValueData: """{app}\{#AppExe}"""; \
    Flags: uninsdeletevalue; Tasks: autostart

; URL protocol wintangle://, so it works before the program has ever run.
Root: HKCU; Subkey: "Software\Classes\wintangle"; ValueType: string; \
    ValueData: "URL:WinTangle Protocol"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\wintangle"; ValueType: string; \
    ValueName: "URL Protocol"; ValueData: ""
Root: HKCU; Subkey: "Software\Classes\wintangle\shell\open\command"; ValueType: string; \
    ValueData: """{app}\{#AppExe}"" ""%1"""

[Run]
Filename: "{app}\{#AppExe}"; Description: "{cm:LaunchProgram,{#AppName}}"; \
    Flags: nowait postinstall skipifsilent

[UninstallRun]
; If the user asks for it, the program cleans up after itself. The list of what
; to remove therefore lives in exactly one place (src/app/Cleanup.cpp) and is
; not duplicated in this script.
; Runs before the files are deleted, while the executable is still there.
Filename: "{app}\{#AppExe}"; Parameters: "--cleanup --silent"; \
    Flags: runhidden waituntilterminated; RunOnceId: "wintangle-cleanup"; \
    Check: ShouldRemoveSettings

[UninstallDelete]
Type: dirifempty; Name: "{userappdata}\WinTangle"

[Code]
var
  RemoveSettings: Boolean;

// Ask during uninstall whether the personal data should go as well. The
// default is "No": somebody merely moving to a new version should not lose
// their key bindings. A silent uninstall takes that default too.
function InitializeUninstall(): Boolean;
begin
  RemoveSettings := SuppressibleMsgBox(
    ExpandConstant('{cm:RemoveSettingsPrompt}'),
    mbConfirmation, MB_YESNO, IDNO) = IDYES;
  Result := True;
end;

function ShouldRemoveSettings(): Boolean;
begin
  Result := RemoveSettings;
end;

// Autostart and the URL protocol go in any case -- including when those
// entries did not come from the setup but were written by the program itself.
// That is exactly what happens when somebody enables autostart later from the
// tray menu.
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usPostUninstall then
  begin
    RegDeleteValue(HKCU, 'Software\Microsoft\Windows\CurrentVersion\Run', 'WinTangle');
    RegDeleteKeyIncludingSubkeys(HKCU, 'Software\Classes\wintangle');
  end;
end;
