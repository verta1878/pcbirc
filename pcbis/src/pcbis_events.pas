{ ===========================================================================
  pcbis_events.pas — Timed event engine with batch file execution
  6 configurable event slots + built-in internet events.
  =========================================================================== }

unit pcbis_events;

{$mode objfpc}{$H+}

interface

uses
  pcbis_config;

const
  MAX_EVENT_SLOTS = 6;

type
  TDayMask = set of 0..6;  { 0=Sun, 1=Mon ... 6=Sat }

  TEventSlot = record
    Enabled     : boolean;
    Hour        : integer;    { 0-23 }
    Minute      : integer;    { 0-59 }
    Days        : TDayMask;   { which days to run }
    BatchFile   : string;     { path to batch/script file }
    Description : string;
    LastRun     : TDateTime;
    Running     : boolean;
  end;

  TBuiltinEvent = (
    beSmtpQueue,        { scan SMTP outbound queue }
    beBinkpPoll,        { poll BinkP uplinks }
    beLogRotate,        { rotate log files }
    beQwkExchange       { QWK network exchange }
  );

  TBuiltinEntry = record
    EventType   : TBuiltinEvent;
    IntervalSec : integer;
    LastRun     : TDateTime;
    Enabled     : boolean;
    Description : string;
  end;

  TPcbisEvents = class
  private
    FSlots    : array[1..MAX_EVENT_SLOTS] of TEventSlot;
    FBuiltins : array of TBuiltinEntry;
    FCfg      : TPcbisConfig;
    FPaused   : boolean;   { true during event execution }

    procedure LoadSlotsFromConfig;
    procedure RunBatchFile(SlotNum : integer);
    function  ShouldRunSlot(SlotNum : integer) : boolean;
    function  DayMatch(const Days : TDayMask) : boolean;
  public
    constructor Create(ACfg : TPcbisConfig);
    destructor Destroy; override;

    procedure CheckAndRun;
    function  IsPaused : boolean;

    { Status for WFC display }
    function  GetSlotDesc(SlotNum : integer) : string;
    function  GetSlotNextRun(SlotNum : integer) : string;
  end;

implementation

uses
  SysUtils, BaseUnix, pcbis_log, pcbis_smtp;

constructor TPcbisEvents.Create(ACfg : TPcbisConfig);
var
  I : integer;
begin
  inherited Create;
  FCfg := ACfg;
  FPaused := False;

  for I := 1 to MAX_EVENT_SLOTS do
  begin
    FSlots[I].Enabled := False;
    FSlots[I].LastRun := 0;
    FSlots[I].Running := False;
  end;

  LoadSlotsFromConfig;

  { Built-in internet events }
  SetLength(FBuiltins, 0);

  if ACfg.SmtpEnabled then
  begin
    SetLength(FBuiltins, Length(FBuiltins) + 1);
    with FBuiltins[High(FBuiltins)] do
    begin
      EventType := beSmtpQueue;
      IntervalSec := 60;
      LastRun := Now;
      Enabled := True;
      Description := 'SMTP queue scan';
    end;
  end;

  if ACfg.BinkpEnabled then
  begin
    SetLength(FBuiltins, Length(FBuiltins) + 1);
    with FBuiltins[High(FBuiltins)] do
    begin
      EventType := beBinkpPoll;
      IntervalSec := 3600;
      LastRun := Now;
      Enabled := True;
      Description := 'BinkP uplink poll';
    end;
  end;

  { Log rotation — daily }
  SetLength(FBuiltins, Length(FBuiltins) + 1);
  with FBuiltins[High(FBuiltins)] do
  begin
    EventType := beLogRotate;
    IntervalSec := 86400;
    LastRun := Now;
    Enabled := True;
    Description := 'Log rotation';
  end;
end;

destructor TPcbisEvents.Destroy;
begin
  SetLength(FBuiltins, 0);
  inherited Destroy;
end;

function ParseDayMask(const S : string) : TDayMask;
var
  I : integer;
begin
  Result := [];
  { Format: MTWTFSS or M-W-F-- etc. Any non-dash = enabled }
  for I := 1 to Length(S) do
  begin
    if (I <= 7) and (S[I] <> '-') and (S[I] <> ' ') then
    begin
      case I of
        1: Include(Result, 1);  { Monday }
        2: Include(Result, 2);  { Tuesday }
        3: Include(Result, 3);  { Wednesday }
        4: Include(Result, 4);  { Thursday }
        5: Include(Result, 5);  { Friday }
        6: Include(Result, 6);  { Saturday }
        7: Include(Result, 0);  { Sunday }
      end;
    end;
  end;
  if Result = [] then
    Result := [0..6]; { default: every day }
end;

procedure TPcbisEvents.LoadSlotsFromConfig;
var
  I       : integer;
  Prefix  : string;
  TimeStr : string;
  P       : integer;
begin
  { TODO: read from pcbis.cfg [events] section
    For now, parse from config object if extended.
    Format: event1_time=02:00, event1_batch=..., event1_days=MTWTFSS }

  { Placeholder — events loaded from config when config parser is extended }
  for I := 1 to MAX_EVENT_SLOTS do
    FSlots[I].Enabled := False;
end;

function TPcbisEvents.DayMatch(const Days : TDayMask) : boolean;
begin
  Result := DayOfWeek(Now) - 1 in Days;  { DayOfWeek: 1=Sun, so -1 = 0=Sun }
end;

function TPcbisEvents.ShouldRunSlot(SlotNum : integer) : boolean;
var
  NowH, NowM, NowS, NowMS : Word;
  LastH, LastM, LastS, LastMS : Word;
begin
  Result := False;
  if not FSlots[SlotNum].Enabled then Exit;
  if FSlots[SlotNum].Running then Exit;
  if not DayMatch(FSlots[SlotNum].Days) then Exit;

  DecodeTime(Time, NowH, NowM, NowS, NowMS);

  { Check if current time matches event time and hasn't run today }
  if (NowH = FSlots[SlotNum].Hour) and (NowM >= FSlots[SlotNum].Minute) then
  begin
    { Don't run if already ran today }
    if Trunc(FSlots[SlotNum].LastRun) < Trunc(Now) then
      Result := True;
  end;
end;

procedure TPcbisEvents.RunBatchFile(SlotNum : integer);
var
  ExitCode : integer;
  StartTime : TDateTime;
begin
  if FSlots[SlotNum].BatchFile = '' then Exit;
  if not FileExists(FSlots[SlotNum].BatchFile) then
  begin
    LogEvent(llError, 'Event ' + IntToStr(SlotNum) + ': batch file not found: ' +
             FSlots[SlotNum].BatchFile);
    Exit;
  end;

  FSlots[SlotNum].Running := True;
  FPaused := True;

  LogEvent(llInfo, 'Event ' + IntToStr(SlotNum) + ' triggered: ' +
           FSlots[SlotNum].Description);
  LogEvent(llInfo, 'Pausing new connections...');
  LogEvent(llInfo, 'Running: ' + FSlots[SlotNum].BatchFile);

  StartTime := Now;

  { Execute the batch file / shell script }
  {$IFDEF UNIX}
  ExitCode := fpSystem(PChar(FSlots[SlotNum].BatchFile));
  {$ELSE}
  ExitCode := -1; { TODO: DOS Exec() }
  {$ENDIF}

  LogEvent(llInfo, 'Event ' + IntToStr(SlotNum) + ': ' +
           ExtractFileName(FSlots[SlotNum].BatchFile) +
           ' completed (exit=' + IntToStr(ExitCode) +
           ', duration=' + FormatDateTime('nn:ss', Now - StartTime) + ')');

  FSlots[SlotNum].LastRun := Now;
  FSlots[SlotNum].Running := False;
  FPaused := False;

  LogEvent(llInfo, 'Resuming connections');
end;

procedure TPcbisEvents.CheckAndRun;
var
  I       : integer;
  Elapsed : integer;
begin
  { Check batch event slots }
  for I := 1 to MAX_EVENT_SLOTS do
  begin
    if ShouldRunSlot(I) then
      RunBatchFile(I);
  end;

  { Check built-in events }
  for I := 0 to High(FBuiltins) do
  begin
    if not FBuiltins[I].Enabled then Continue;

    Elapsed := Round((Now - FBuiltins[I].LastRun) * 86400);
    if Elapsed < FBuiltins[I].IntervalSec then Continue;

    FBuiltins[I].LastRun := Now;

    case FBuiltins[I].EventType of
      beSmtpQueue:
        SmtpProcessQueue(FCfg);

      beBinkpPoll:
        begin
          LogEvent(llInfo, 'BinkP poll: scanning uplinks');
          { TODO: connect to each uplink }
        end;

      beLogRotate:
        begin
          LogEvent(llInfo, 'Rotating logs');
          LogRotate;
        end;

      beQwkExchange:
        begin
          LogEvent(llInfo, 'QWK network exchange');
          { TODO: run QWK export/import cycle }
        end;
    end;
  end;
end;

function TPcbisEvents.IsPaused : boolean;
begin
  Result := FPaused;
end;

function TPcbisEvents.GetSlotDesc(SlotNum : integer) : string;
begin
  if (SlotNum >= 1) and (SlotNum <= MAX_EVENT_SLOTS) and FSlots[SlotNum].Enabled then
    Result := Format('Event %d: %s at %2.2d:%2.2d',
              [SlotNum, FSlots[SlotNum].Description,
               FSlots[SlotNum].Hour, FSlots[SlotNum].Minute])
  else
    Result := Format('Event %d: (not configured)', [SlotNum]);
end;

function TPcbisEvents.GetSlotNextRun(SlotNum : integer) : string;
begin
  if (SlotNum >= 1) and (SlotNum <= MAX_EVENT_SLOTS) and FSlots[SlotNum].Enabled then
  begin
    if FSlots[SlotNum].LastRun = 0 then
      Result := 'never'
    else
      Result := FormatDateTime('yyyy-mm-dd hh:nn:ss', FSlots[SlotNum].LastRun);
  end
  else
    Result := '-';
end;

end.
