; ============================================================
;  WinFOSSIL v2.0.0 Installer (Inno Setup / InnoIRC561)
; ============================================================
;  Build with the Win98-capable ISCC.exe from InnoIRC561:
;    ISCC.exe wnfossil_modern.iss
;
;  MinVersion 4.0,4.0 = Windows 95 / NT4 and up (requires the
;  InnoIRC561 Win98 fork; stock Inno 5.6 floors at Win2000).
;  Place the correct-arch FOSSIL.DLL next to this script before
;  compiling (i386 or x64 build).
; ============================================================

[Setup]
AppName=WinFOSSIL
AppVersion=2.0.0
AppPublisher=FPC264IRC Contributors
AppPublisherURL=https://github.com/verta1878/netmodem2irc
DefaultDirName={pf}\WinFOSSIL
DefaultGroupName=WinFOSSIL
OutputBaseFilename=wnfossil_setup
Compression=lzma
SolidCompression=yes
LicenseFile=LICENSE.TXT
; Win95/NT4 and up (InnoIRC561 fork required for <Win2000)
MinVersion=4.0,4.0
; Both 32-bit and 64-bit hosts
ArchitecturesAllowed=x86 x64
SetupIconFile=wnfossil.ico

[Files]
Source: "FOSSIL.DLL";    DestDir: "{sys}";       Flags: ignoreversion restartreplace
Source: "WNFOSSIL.EXE";  DestDir: "{app}";       Flags: ignoreversion
Source: "WNFOSCTL.EXE";  DestDir: "{win}";       Flags: ignoreversion
Source: "WNFOSSIL.CPL";  DestDir: "{sys}";       Flags: ignoreversion restartreplace
Source: "FOSSIL.INF";    DestDir: "{win}\Inf";   Flags: ignoreversion
Source: "LICENSE.TXT";   DestDir: "{app}";       Flags: ignoreversion
Source: "README.TXT";    DestDir: "{app}";       Flags: ignoreversion
Source: "WHATSNEW.TXT";  DestDir: "{app}";       Flags: ignoreversion
Source: "VMODEM.TXT";    DestDir: "{app}";       Flags: ignoreversion
Source: "WNFOSSIL.html"; DestDir: "{app}";       Flags: ignoreversion
Source: "FILE_ID.DIZ";   DestDir: "{app}";       Flags: ignoreversion

[Icons]
Name: "{group}\WinFOSSIL";         Filename: "{app}\WNFOSSIL.EXE"
Name: "{group}\WinFOSSIL Help";    Filename: "{app}\WNFOSSIL.html"
Name: "{group}\Configure (CPL)";   Filename: "control.exe"; Parameters: "WNFOSSIL.CPL"
Name: "{group}\Uninstall";         Filename: "{uninstallexe}"

[Registry]
Root: HKLM; Subkey: "SOFTWARE\WinFOSSIL"; ValueType: dword; ValueName: "Installed"; ValueData: "1"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\WinFOSSIL"; ValueType: string; ValueName: "Version"; ValueData: "2.0.0"; Flags: uninsdeletekey

[Run]
Filename: "{app}\WNFOSSIL.EXE"; Description: "Launch WinFOSSIL"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: files; Name: "{sys}\FOSSIL.DLL"
Type: files; Name: "{sys}\WNFOSSIL.CPL"
