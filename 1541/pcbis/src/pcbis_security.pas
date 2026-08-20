{ ===========================================================================
  pcbis_security.pas — Security features for pcbis
  Failed login tracking, IP blocking, rate limiting.
  =========================================================================== }

unit pcbis_security;

{$mode objfpc}{$H+}

interface

const
  MAX_TRACKED_IPS = 256;
  DEFAULT_MAX_FAILS = 3;
  DEFAULT_BLOCK_SECS = 300;

type
  TIpRecord = record
    IP          : string;
    FailCount   : integer;
    LastFail    : TDateTime;
    Blocked     : boolean;
    BlockUntil  : TDateTime;
  end;

  TPcbisSecurity = class
  private
    FRecords    : array[0..MAX_TRACKED_IPS-1] of TIpRecord;
    FCount      : integer;
    FMaxFails   : integer;
    FBlockSecs  : integer;

    function FindIP(const IP : string) : integer;
    function AddIP(const IP : string) : integer;
  public
    constructor Create(MaxFails, BlockSeconds : integer);

    { Check if an IP is currently blocked }
    function  IsBlocked(const IP : string) : boolean;

    { Record a failed login attempt }
    procedure RecordFailure(const IP, Proto, Username : string);

    { Record a successful login (resets counter) }
    procedure RecordSuccess(const IP : string);

    { Periodic cleanup — unblock expired IPs }
    procedure Cleanup;

    { Stats for WFC }
    function  BlockedCount : integer;
  end;

implementation

uses
  SysUtils, pcbis_log;

constructor TPcbisSecurity.Create(MaxFails, BlockSeconds : integer);
begin
  inherited Create;
  FCount := 0;
  if MaxFails > 0 then FMaxFails := MaxFails
  else FMaxFails := DEFAULT_MAX_FAILS;
  if BlockSeconds > 0 then FBlockSecs := BlockSeconds
  else FBlockSecs := DEFAULT_BLOCK_SECS;
end;

function TPcbisSecurity.FindIP(const IP : string) : integer;
var
  I : integer;
begin
  for I := 0 to FCount - 1 do
    if FRecords[I].IP = IP then begin Result := I; Exit; end;
  Result := -1;
end;

function TPcbisSecurity.AddIP(const IP : string) : integer;
begin
  if FCount >= MAX_TRACKED_IPS then
  begin
    { Evict oldest }
    Move(FRecords[1], FRecords[0], (MAX_TRACKED_IPS - 1) * SizeOf(TIpRecord));
    Dec(FCount);
  end;
  Result := FCount;
  FillChar(FRecords[Result], SizeOf(TIpRecord), 0);
  FRecords[Result].IP := IP;
  Inc(FCount);
end;

function TPcbisSecurity.IsBlocked(const IP : string) : boolean;
var
  Idx : integer;
begin
  Result := False;
  Idx := FindIP(IP);
  if Idx < 0 then Exit;

  if FRecords[Idx].Blocked then
  begin
    if Now < FRecords[Idx].BlockUntil then
      Result := True
    else
    begin
      { Block expired }
      FRecords[Idx].Blocked := False;
      FRecords[Idx].FailCount := 0;
      LogSecurity(llInfo, IP, 'unblocked after ' + IntToStr(FBlockSecs) + 's cooldown');
    end;
  end;
end;

procedure TPcbisSecurity.RecordFailure(const IP, Proto, Username : string);
var
  Idx : integer;
begin
  Idx := FindIP(IP);
  if Idx < 0 then Idx := AddIP(IP);

  Inc(FRecords[Idx].FailCount);
  FRecords[Idx].LastFail := Now;

  LogSecurity(llWarn, IP, Proto + ' login failed: ' + Username +
              ' (attempt ' + IntToStr(FRecords[Idx].FailCount) + ')');

  if FRecords[Idx].FailCount >= FMaxFails then
  begin
    FRecords[Idx].Blocked := True;
    FRecords[Idx].BlockUntil := Now + (FBlockSecs / 86400);
    LogSecurity(llWarn, IP, 'BLOCKED — ' + IntToStr(FMaxFails) +
                ' failed logins, blocked for ' + IntToStr(FBlockSecs) + 's');
  end;
end;

procedure TPcbisSecurity.RecordSuccess(const IP : string);
var
  Idx : integer;
begin
  Idx := FindIP(IP);
  if Idx >= 0 then
  begin
    FRecords[Idx].FailCount := 0;
    FRecords[Idx].Blocked := False;
  end;
end;

procedure TPcbisSecurity.Cleanup;
var
  I : integer;
begin
  for I := 0 to FCount - 1 do
  begin
    if FRecords[I].Blocked and (Now >= FRecords[I].BlockUntil) then
    begin
      FRecords[I].Blocked := False;
      FRecords[I].FailCount := 0;
      LogSecurity(llInfo, FRecords[I].IP, 'unblocked (cooldown expired)');
    end;
  end;
end;

function TPcbisSecurity.BlockedCount : integer;
var
  I : integer;
begin
  Result := 0;
  for I := 0 to FCount - 1 do
    if FRecords[I].Blocked then Inc(Result);
end;

end.
