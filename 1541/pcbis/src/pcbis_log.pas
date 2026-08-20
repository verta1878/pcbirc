{ ===========================================================================
  pcbis_log.pas — Per-protocol logging for pcbis
  Supports: main, telnet, binkp, ftp, http, smtp, event, security logs.
  Apache common format for HTTP. Log rotation with date suffix.
  =========================================================================== }

unit pcbis_log;

{$mode objfpc}{$H+}

interface

type
  TLogLevel = (llDebug, llInfo, llWarn, llError);

  TLogProto = (
    lpMain,       { pcbis.log }
    lpTelnet,     { telnet.log }
    lpBinkp,      { binkp.log }
    lpFtp,        { ftp.log }
    lpHttp,       { http.log }
    lpSmtp,       { smtp.log }
    lpEvent,      { event.log }
    lpSecurity    { security.log }
  );

procedure LogInit(const LogDir : string; MinLevel : TLogLevel);
procedure LogFini;

{ Standard logging }
procedure LogMsg(Proto : TLogProto; Level : TLogLevel; const Msg : string);
procedure LogInfo(const Msg : string);
procedure LogWarn(const Msg : string);
procedure LogError(const Msg : string);
procedure LogDebug(const Msg : string);

{ Protocol-specific convenience }
procedure LogTelnet(Level : TLogLevel; const Remote, Msg : string);
procedure LogBinkp(Level : TLogLevel; const Remote, Msg : string);
procedure LogFtp(Level : TLogLevel; const Remote, Msg : string);
procedure LogHttpAccess(const Remote, Method, Path : string; Code : integer; Size : longint);
procedure LogSmtp(Level : TLogLevel; const Msg : string);
procedure LogEvent(Level : TLogLevel; const Msg : string);
procedure LogSecurity(Level : TLogLevel; const Remote, Msg : string);

{ Shorthand for connection log lines }
procedure LogConn(const Proto, Remote, Msg : string);

{ Log rotation }
procedure LogRotate;

implementation

uses
  SysUtils;

const
  ProtoNames : array[TLogProto] of string = (
    'MAIN', 'TELNET', 'BINKP', 'FTP', 'HTTP', 'SMTP', 'EVENT', 'SECURITY'
  );
  ProtoFiles : array[TLogProto] of string = (
    'pcbis.log', 'telnet.log', 'binkp.log', 'ftp.log',
    'http.log', 'smtp.log', 'event.log', 'security.log'
  );
  LevelNames : array[TLogLevel] of string = (
    'DEBUG', 'INFO', 'WARN', 'ERROR'
  );

var
  LogHandles : array[TLogProto] of TextFile;
  LogOpen    : array[TLogProto] of boolean;
  LogDir     : string;
  MinLogLevel: TLogLevel;
  UseStdout  : boolean;

function Timestamp : string;
begin
  Result := FormatDateTime('yyyy-mm-dd hh:nn:ss', Now);
end;

function HttpTimestamp : string;
begin
  Result := FormatDateTime('[dd/mmm/yyyy:hh:nn:ss +0000]', Now);
end;

procedure OpenLog(Proto : TLogProto);
var
  Fn : string;
begin
  if LogOpen[Proto] then Exit;
  if UseStdout then Exit;

  Fn := LogDir + DirectorySeparator + ProtoFiles[Proto];
  try
    AssignFile(LogHandles[Proto], Fn);
    if FileExists(Fn) then
      Append(LogHandles[Proto])
    else
      Rewrite(LogHandles[Proto]);
    LogOpen[Proto] := True;
  except
    on E: Exception do
    begin
      WriteLn('Warning: cannot open ', Fn, ': ', E.Message);
      LogOpen[Proto] := False;
    end;
  end;
end;

procedure LogInit(const ALogDir : string; MinLevel : TLogLevel);
var
  P : TLogProto;
begin
  LogDir := ALogDir;
  MinLogLevel := MinLevel;
  UseStdout := (ALogDir = '');

  for P := Low(TLogProto) to High(TLogProto) do
    LogOpen[P] := False;

  if not UseStdout then
  begin
    if not DirectoryExists(LogDir) then
      ForceDirectories(LogDir);
    { Open main log immediately }
    OpenLog(lpMain);
  end;
end;

procedure LogFini;
var
  P : TLogProto;
begin
  for P := Low(TLogProto) to High(TLogProto) do
  begin
    if LogOpen[P] then
    begin
      CloseFile(LogHandles[P]);
      LogOpen[P] := False;
    end;
  end;
end;

procedure LogMsg(Proto : TLogProto; Level : TLogLevel; const Msg : string);
var
  Line : string;
begin
  if Level < MinLogLevel then Exit;

  Line := Timestamp + ' [' + LevelNames[Level] + '] [' + ProtoNames[Proto] + '] ' + Msg;

  if UseStdout then
    WriteLn(Line)
  else
  begin
    { Write to protocol-specific log }
    OpenLog(Proto);
    if LogOpen[Proto] then
    begin
      WriteLn(LogHandles[Proto], Line);
      Flush(LogHandles[Proto]);
    end;
    { Also write WARN and ERROR to main log }
    if (Proto <> lpMain) and (Level >= llWarn) then
    begin
      OpenLog(lpMain);
      if LogOpen[lpMain] then
      begin
        WriteLn(LogHandles[lpMain], Line);
        Flush(LogHandles[lpMain]);
      end;
    end;
  end;
end;

{ Standard logging — goes to main log }
procedure LogInfo(const Msg : string);
begin LogMsg(lpMain, llInfo, Msg); end;

procedure LogWarn(const Msg : string);
begin LogMsg(lpMain, llWarn, Msg); end;

procedure LogError(const Msg : string);
begin LogMsg(lpMain, llError, Msg); end;

procedure LogDebug(const Msg : string);
begin LogMsg(lpMain, llDebug, Msg); end;

{ Protocol-specific }
procedure LogTelnet(Level : TLogLevel; const Remote, Msg : string);
begin LogMsg(lpTelnet, Level, Remote + ' ' + Msg); end;

procedure LogBinkp(Level : TLogLevel; const Remote, Msg : string);
begin LogMsg(lpBinkp, Level, Remote + ' ' + Msg); end;

procedure LogFtp(Level : TLogLevel; const Remote, Msg : string);
begin LogMsg(lpFtp, Level, Remote + ' ' + Msg); end;

procedure LogHttpAccess(const Remote, Method, Path : string; Code : integer; Size : longint);
var
  Line : string;
begin
  { Apache common log format }
  Line := Remote + ' - - ' + HttpTimestamp + ' "' + Method + ' ' + Path +
          ' HTTP/1.0" ' + IntToStr(Code) + ' ' + IntToStr(Size);
  if UseStdout then
    WriteLn(Line)
  else
  begin
    OpenLog(lpHttp);
    if LogOpen[lpHttp] then
    begin
      WriteLn(LogHandles[lpHttp], Line);
      Flush(LogHandles[lpHttp]);
    end;
  end;
end;

procedure LogSmtp(Level : TLogLevel; const Msg : string);
begin LogMsg(lpSmtp, Level, Msg); end;

procedure LogEvent(Level : TLogLevel; const Msg : string);
begin LogMsg(lpEvent, Level, Msg); end;

procedure LogSecurity(Level : TLogLevel; const Remote, Msg : string);
begin LogMsg(lpSecurity, Level, Remote + ' ' + Msg); end;

{ Shorthand }
procedure LogConn(const Proto, Remote, Msg : string);
begin
  LogMsg(lpMain, llInfo, '[' + Proto + '] ' + Remote + ' ' + Msg);
end;

{ Log rotation — close all, rename with date, reopen }
procedure LogRotate;
var
  P     : TLogProto;
  OldFn : string;
  NewFn : string;
  DateStr : string;
begin
  DateStr := FormatDateTime('yyyy-mm-dd', Now - 1); { yesterday's date }

  for P := Low(TLogProto) to High(TLogProto) do
  begin
    if LogOpen[P] then
    begin
      CloseFile(LogHandles[P]);
      LogOpen[P] := False;
    end;

    OldFn := LogDir + DirectorySeparator + ProtoFiles[P];
    NewFn := LogDir + DirectorySeparator +
             ChangeFileExt(ProtoFiles[P], '') + '-' + DateStr + '.log';

    if FileExists(OldFn) then
      RenameFile(OldFn, NewFn);
  end;

  { Reopen main log }
  OpenLog(lpMain);
  LogMsg(lpMain, llInfo, 'Logs rotated');
end;

end.
