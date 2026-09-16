; Inno Setup script for WinTangle.
;
; Deliberately a per-user installation (PrivilegesRequired=lowest): the program
; needs no elevated rights, and without a UAC prompt the WinSparkle updater can
; run the setup unattended.
;
; The version arrives as /DAppVersion=... from the release workflow.

; AppVersion is numeric (0.1.0) and goes into the file properties; Windows
; rejects anything else there. AppVersionFull may carry a pre-release suffix
; (0.1.0-rc1) and is what the user sees.
#ifndef AppVersion
  #define AppVersion "0.0.0"
#endif
#ifndef AppVersionFull
  #define AppVersionFull AppVersion
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
AppVersion={#AppVersionFull}
AppVerName={#AppName} {#AppVersionFull}
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
OutputBaseFilename=wintangle-{#AppVersionFull}-setup
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
english.StillRunning=WinTangle is still running and did not close. Quit it from the notification area icon and run the setup again.
german.StillRunning=WinTangle läuft noch und hat sich nicht beendet. Bitte über das Symbol im Infobereich beenden und das Setup erneut starten.

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

; A silent update has no "run it now" checkbox, so the program would stay shut
; after updating itself. /UPDATED=1 is passed by the appcast for exactly this.
Filename: "{app}\{#AppExe}"; Flags: nowait runhidden; Check: StartedByUpdate

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
const
  // Parent handle that FindWindowEx needs to see message-only windows, which
  // is the only kind WinTangle has.
  HWND_MESSAGE_PARENT = -3;
  WM_CLOSE_MESSAGE = $0010;

function FindWindowExW(Parent, ChildAfter: Longint; ClassName: String;
  WindowName: Longint): Longint;
  external 'FindWindowExW@user32.dll stdcall';
function PostMessageW(Wnd: Longint; Msg: Cardinal; WParam, LParam: Longint): Boolean;
  external 'PostMessageW@user32.dll stdcall';
function IsWindowVisibleHandle(Wnd: Longint): Boolean;
  external 'IsWindow@user32.dll stdcall';

var
  RemoveSettings: Boolean;

// Closes a running WinTangle and waits for it to be gone.
//
// This talks to the window directly instead of running the installed
// executable with a flag: the copy on disk is the OLD version, which need not
// understand any flag we invent. An older build handed "--quit" would simply
// start the program -- locking the very file the installer is about to
// replace, which is exactly the failure this is meant to prevent.
function QuitRunningWinTangle(): Boolean;
var
  Wnd: Longint;
  Waited: Integer;
begin
  Wnd := FindWindowExW(HWND_MESSAGE_PARENT, 0, 'WinTangleMessageWindow', 0);
  if Wnd = 0 then
  begin
    Result := True;
    exit;
  end;

  PostMessageW(Wnd, WM_CLOSE_MESSAGE, 0, 0);

  Waited := 0;
  while (Waited < 5000) and IsWindowVisibleHandle(Wnd) do
  begin
    Sleep(100);
    Waited := Waited + 100;
  end;
  Result := not IsWindowVisibleHandle(Wnd);
end;

// Ask during uninstall whether the personal data should go as well. The
// default is "No": somebody merely moving to a new version should not lose
// their key bindings. A silent uninstall takes that default too.
function InitializeUninstall(): Boolean;
begin
  QuitRunningWinTangle();
  RemoveSettings := SuppressibleMsgBox(
    ExpandConstant('{cm:RemoveSettingsPrompt}'),
    mbConfirmation, MB_YESNO, IDNO) = IDYES;
  Result := True;
end;

// WinTangle has no ordinary window, only a tray icon, so the Restart Manager
// behind CloseApplications cannot find it. A running copy would keep the
// executable locked and the install would fail with "MoveFile failed; code 5".
// Ask it to quit itself instead.
function PrepareToInstall(var NeedsRestart: Boolean): String;
begin
  NeedsRestart := False;
  if QuitRunningWinTangle() then
    Result := ''
  else
    // A named cause beats the installer's own "error renaming a file in the
    // destination directory" further down the line.
    Result := ExpandConstant('{cm:StillRunning}');
end;

// True when the setup was started by WinSparkle rather than by a person.
function StartedByUpdate(): Boolean;
begin
  Result := ExpandConstant('{param:UPDATED|0}') = '1';
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
