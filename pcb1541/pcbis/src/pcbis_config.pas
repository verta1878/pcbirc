{ ===========================================================================
  pcbis_config.pas — Full configuration parser for pcbis
  Reads pcbis.cfg with sections: general, telnet, binkp, ftp, http,
  smtp, events, logging, security, qwk, uucp2
  =========================================================================== }

unit pcbis_config;

{$mode objfpc}{$H+}

interface

uses
  SysUtils, Classes;

const
  MAX_EVENT_SLOTS = 6;
  MAX_UPLINKS = 16;

type
  TBinkpUplink = record
    Address  : string;
    Host     : string;
    Password : string;
  end;

  TEventSlotCfg = record
    Enabled  : boolean;
    Time     : string;    { "HH:MM" }
    Batch    : string;    { path to batch/script file }
    Days     : string;    { "MTWTFSS" or "M-W-F--" }
    Desc     : string;
  end;

  TPcbisConfig = class
  private
    FLines : TStringList;
    function GetValue(const Section, Key, Default : string) : string;
    function GetInt(const Section, Key : string; Default : integer) : integer;
    function GetBool(const Section, Key : string; Default : boolean) : boolean;
  public
    { General }
    PCBDir         : string;    { PCBoard installation directory }
    LogDir         : string;
    PidFile        : string;
    MaxConnections : integer;

    { Telnet }
    TelnetEnabled  : boolean;
    TelnetListen   : string;
    TelnetPort     : integer;
    TelnetMaxNodes : integer;
    TelnetNodeStart: integer;
    TelnetFossilMode : string;
    TelnetIdleTimeout: integer;

    { BinkP }
    BinkpEnabled   : boolean;
    BinkpListen    : string;
    BinkpPort      : integer;
    BinkpAddress   : string;
    BinkpPassword  : string;
    BinkpInbound   : string;
    BinkpOutbound  : string;
    BinkpQFrontDir : string;
    BinkpUplinks   : array[0..MAX_UPLINKS-1] of TBinkpUplink;
    BinkpNumUplinks: integer;

    { FTP }
    FtpEnabled     : boolean;
    FtpListen      : string;
    FtpPort        : integer;
    FtpPassvMin    : integer;
    FtpPassvMax    : integer;
    FtpAnonAccess  : boolean;
    FtpAnonSecLevel: integer;

    { HTTP }
    HttpEnabled    : boolean;
    HttpListen     : string;
    HttpPort       : integer;
    HttpDocRoot    : string;

    { SMTP }
    SmtpEnabled    : boolean;
    SmtpRelay      : string;
    SmtpRelayPort  : integer;
    SmtpFrom       : string;
    SmtpUseTLS     : boolean;
    SmtpTriggerDir : string;

    { Events }
    EventSlots     : array[1..MAX_EVENT_SLOTS] of TEventSlotCfg;

    { Logging }
    LogLevel       : string;   { DEBUG, INFO, WARN, ERROR }
    LogRotateDays  : integer;

    { Security }
    SecMaxFails    : integer;
    SecBlockSecs   : integer;

    { QWK }
    QwkEnabled     : boolean;
    QwkBoardID     : string;
    QwkHubUrl      : string;
    QwkHubUser     : string;
    QwkHubPass     : string;
    QwkSchedule    : string;   { cron-style or interval }

    { UUCP2 }
    Uucp2Enabled   : boolean;
    Uucp2Host      : string;
    Uucp2Port      : integer;
    Uucp2Login     : string;
    Uucp2Password  : string;
    Uucp2SpoolDir  : string;

    { NNTP }
    NntpEnabled    : boolean;
    NntpServer     : string;
    NntpPort       : integer;
    NntpUser       : string;
    NntpPass       : string;

    constructor Create;
    destructor Destroy; override;
    function LoadFromFile(const Filename : string) : boolean;
    procedure Reload(const Filename : string);
    procedure Dump;

    { Convenience }
    property LogFile : string read LogDir;
  end;

implementation

constructor TPcbisConfig.Create;
var
  I : integer;
begin
  inherited Create;
  FLines := TStringList.Create;

  { Defaults }
  PCBDir := '.';
  LogDir := 'logs';
  PidFile := 'pcbis.pid';
  MaxConnections := 32;

  TelnetEnabled := True;
  TelnetListen := '0.0.0.0';
  TelnetPort := 2323;
  TelnetMaxNodes := 4;
  TelnetNodeStart := 1;
  TelnetFossilMode := 'pty';
  TelnetIdleTimeout := 300;

  BinkpEnabled := False;
  BinkpListen := '0.0.0.0';
  BinkpPort := 24554;
  BinkpNumUplinks := 0;

  FtpEnabled := True;
  FtpListen := '0.0.0.0';
  FtpPort := 21;
  FtpPassvMin := 10000;
  FtpPassvMax := 10100;
  FtpAnonAccess := True;
  FtpAnonSecLevel := 10;

  HttpEnabled := True;
  HttpListen := '0.0.0.0';
  HttpPort := 8080;
  HttpDocRoot := '.';

  SmtpEnabled := False;
  SmtpRelayPort := 587;

  for I := 1 to MAX_EVENT_SLOTS do
    EventSlots[I].Enabled := False;

  LogLevel := 'INFO';
  LogRotateDays := 30;

  SecMaxFails := 3;
  SecBlockSecs := 300;

  QwkEnabled := False;
  QwkBoardID := 'PCBREV';

  Uucp2Enabled := False;
  Uucp2Port := 540;

  NntpEnabled := False;
  NntpPort := 119;
end;

destructor TPcbisConfig.Destroy;
begin
  FLines.Free;
  inherited Destroy;
end;

function TPcbisConfig.GetValue(const Section, Key, Default : string) : string;
var
  I          : integer;
  InSection  : boolean;
  Line, S, K : string;
  P          : integer;
begin
  Result := Default;
  InSection := False;
  for I := 0 to FLines.Count - 1 do
  begin
    Line := Trim(FLines[I]);
    if (Length(Line) = 0) or (Line[1] = '#') or (Line[1] = ';') then Continue;
    if (Line[1] = '[') then
    begin
      P := Pos(']', Line);
      if P > 0 then S := LowerCase(Copy(Line, 2, P - 2)) else S := '';
      InSection := (S = LowerCase(Section));
      Continue;
    end;
    if InSection then
    begin
      P := Pos('=', Line);
      if P > 0 then
      begin
        K := Trim(Copy(Line, 1, P - 1));
        if LowerCase(K) = LowerCase(Key) then
        begin
          Result := Trim(Copy(Line, P + 1, Length(Line)));
          Exit;
        end;
      end;
    end;
  end;
end;

function TPcbisConfig.GetInt(const Section, Key : string; Default : integer) : integer;
begin
  Result := StrToIntDef(GetValue(Section, Key, IntToStr(Default)), Default);
end;

function TPcbisConfig.GetBool(const Section, Key : string; Default : boolean) : boolean;
var S : string;
begin
  if Default then S := 'yes' else S := 'no';
  S := LowerCase(GetValue(Section, Key, S));
  Result := (S = 'yes') or (S = 'true') or (S = '1');
end;

function TPcbisConfig.LoadFromFile(const Filename : string) : boolean;
var
  I : integer;
begin
  Result := False;
  if not FileExists(Filename) then Exit;
  try
    FLines.LoadFromFile(Filename);
  except
    Exit;
  end;

  { General }
  PCBDir := GetValue('general', 'pcbdir', PCBDir);
  LogDir := GetValue('general', 'logdir', LogDir);
  PidFile := GetValue('general', 'pidfile', PidFile);
  MaxConnections := GetInt('general', 'max_connections', MaxConnections);

  { Telnet }
  TelnetEnabled := GetBool('telnet', 'enabled', TelnetEnabled);
  TelnetListen := GetValue('telnet', 'listen', TelnetListen);
  TelnetPort := GetInt('telnet', 'port', TelnetPort);
  TelnetMaxNodes := GetInt('telnet', 'max_nodes', TelnetMaxNodes);
  TelnetNodeStart := GetInt('telnet', 'node_start', TelnetNodeStart);
  TelnetFossilMode := GetValue('telnet', 'fossil_mode', TelnetFossilMode);
  TelnetIdleTimeout := GetInt('telnet', 'idle_timeout', TelnetIdleTimeout);

  { BinkP }
  BinkpEnabled := GetBool('binkp', 'enabled', BinkpEnabled);
  BinkpListen := GetValue('binkp', 'listen', BinkpListen);
  BinkpPort := GetInt('binkp', 'port', BinkpPort);
  BinkpAddress := GetValue('binkp', 'address', BinkpAddress);
  BinkpPassword := GetValue('binkp', 'password', BinkpPassword);
  BinkpInbound := GetValue('binkp', 'inbound', BinkpInbound);
  BinkpOutbound := GetValue('binkp', 'outbound', BinkpOutbound);
  BinkpQFrontDir := GetValue('binkp', 'qfront_dir', BinkpQFrontDir);

  { FTP }
  FtpEnabled := GetBool('ftp', 'enabled', FtpEnabled);
  FtpListen := GetValue('ftp', 'listen', FtpListen);
  FtpPort := GetInt('ftp', 'port', FtpPort);
  FtpPassvMin := GetInt('ftp', 'pasv_min', FtpPassvMin);
  FtpPassvMax := GetInt('ftp', 'pasv_max', FtpPassvMax);
  FtpAnonAccess := GetBool('ftp', 'anonymous', FtpAnonAccess);
  FtpAnonSecLevel := GetInt('ftp', 'anon_security', FtpAnonSecLevel);

  { HTTP }
  HttpEnabled := GetBool('http', 'enabled', HttpEnabled);
  HttpListen := GetValue('http', 'listen', HttpListen);
  HttpPort := GetInt('http', 'port', HttpPort);
  HttpDocRoot := GetValue('http', 'docroot', HttpDocRoot);

  { SMTP }
  SmtpEnabled := GetBool('smtp', 'enabled', SmtpEnabled);
  SmtpRelay := GetValue('smtp', 'relay', SmtpRelay);
  SmtpRelayPort := GetInt('smtp', 'relay_port', SmtpRelayPort);
  SmtpFrom := GetValue('smtp', 'from', SmtpFrom);
  SmtpUseTLS := GetBool('smtp', 'use_tls', SmtpUseTLS);
  SmtpTriggerDir := GetValue('smtp', 'trigger_dir', SmtpTriggerDir);

  { Events }
  for I := 1 to MAX_EVENT_SLOTS do
  begin
    EventSlots[I].Time := GetValue('events', 'event' + IntToStr(I) + '_time', '');
    EventSlots[I].Batch := GetValue('events', 'event' + IntToStr(I) + '_batch', '');
    EventSlots[I].Days := GetValue('events', 'event' + IntToStr(I) + '_days', 'MTWTFSS');
    EventSlots[I].Desc := GetValue('events', 'event' + IntToStr(I) + '_desc', '');
    EventSlots[I].Enabled := (EventSlots[I].Time <> '') and (EventSlots[I].Batch <> '');
  end;

  { Logging }
  LogLevel := GetValue('logging', 'level', LogLevel);
  LogRotateDays := GetInt('logging', 'rotate_days', LogRotateDays);

  { Security }
  SecMaxFails := GetInt('security', 'max_failed_logins', SecMaxFails);
  SecBlockSecs := GetInt('security', 'block_duration', SecBlockSecs);

  { QWK }
  QwkEnabled := GetBool('qwk', 'enabled', QwkEnabled);
  QwkBoardID := GetValue('qwk', 'board_id', QwkBoardID);
  QwkHubUrl := GetValue('qwk', 'hub_url', QwkHubUrl);
  QwkHubUser := GetValue('qwk', 'hub_user', QwkHubUser);
  QwkHubPass := GetValue('qwk', 'hub_pass', QwkHubPass);
  QwkSchedule := GetValue('qwk', 'schedule', QwkSchedule);

  { UUCP2 }
  Uucp2Enabled := GetBool('uucp2', 'enabled', Uucp2Enabled);
  Uucp2Host := GetValue('uucp2', 'host', Uucp2Host);
  Uucp2Port := GetInt('uucp2', 'port', Uucp2Port);
  Uucp2Login := GetValue('uucp2', 'login', Uucp2Login);
  Uucp2Password := GetValue('uucp2', 'password', Uucp2Password);
  Uucp2SpoolDir := GetValue('uucp2', 'spool_dir', Uucp2SpoolDir);

  { NNTP }
  NntpEnabled := GetBool('nntp', 'enabled', NntpEnabled);
  NntpServer := GetValue('nntp', 'server', NntpServer);
  NntpPort := GetInt('nntp', 'port', NntpPort);
  NntpUser := GetValue('nntp', 'user', NntpUser);
  NntpPass := GetValue('nntp', 'pass', NntpPass);

  Result := True;
end;

procedure TPcbisConfig.Reload(const Filename : string);
begin
  LoadFromFile(Filename);
end;

procedure TPcbisConfig.Dump;
var I : integer;
begin
  WriteLn('[general]');
  WriteLn('  pcbdir = ', PCBDir);
  WriteLn('  logdir = ', LogDir);
  WriteLn('  max_connections = ', MaxConnections);
  WriteLn('[telnet] enabled=', TelnetEnabled, ' port=', TelnetPort, ' nodes=', TelnetMaxNodes);
  WriteLn('[binkp]  enabled=', BinkpEnabled, ' port=', BinkpPort, ' addr=', BinkpAddress);
  WriteLn('[ftp]    enabled=', FtpEnabled, ' port=', FtpPort, ' anon=', FtpAnonAccess);
  WriteLn('[http]   enabled=', HttpEnabled, ' port=', HttpPort, ' root=', HttpDocRoot);
  WriteLn('[smtp]   enabled=', SmtpEnabled, ' relay=', SmtpRelay);
  WriteLn('[qwk]    enabled=', QwkEnabled, ' id=', QwkBoardID);
  WriteLn('[uucp2]  enabled=', Uucp2Enabled, ' host=', Uucp2Host);
  WriteLn('[nntp]   enabled=', NntpEnabled, ' server=', NntpServer);
  for I := 1 to MAX_EVENT_SLOTS do
    if EventSlots[I].Enabled then
      WriteLn('[events] ', I, ': ', EventSlots[I].Time, ' ', EventSlots[I].Desc);
end;

end.
