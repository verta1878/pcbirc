{ ===========================================================================
  pcbis_nodedata.pas — PCBoard node status and caller log I/O
  
  Writes real PCBoard file formats from DEVELOP9.ZIP specifications:
  - PCBOARD.SYS — 128+ byte per-node status file (PCBSYS.DOC)
  - CALLERS — 64-byte random access caller log (CALLERS.DOC)
  - USERS — 400-byte user records (USERS.DOC)
  
  File format specs: Clark Development Company, Inc.
  Source: DEVELOP9.ZIP from files.mpoli.fi/software/DOS/BBS/
  =========================================================================== }

unit pcbis_nodedata;

{$mode objfpc}{$H+}

interface

type
  { PCBOARD.SYS record — first 128 bytes (v14.x compatible block)
    All strings are space-padded, NOT null-terminated.
    Offsets from PCBSYS.DOC (Clark Development, last update 10/03/94) }
  TPcbSysRecord = packed record
    DisplayOnOff    : array[0..1] of char;   {  0: "-1"=On, " 0"=Off }
    PrinterOnOff    : array[0..1] of char;   {  2: "-1"=On, " 0"=Off }
    PageBellOnOff   : array[0..1] of char;   {  4: "-1"=On, " 0"=Off }
    CallerAlarmOnOff: array[0..1] of char;   {  6: "-1"=On, " 0"=Off }
    Sysop           : char;                  {  8: ' ' }
    ErrorCorrected  : array[0..1] of char;   {  9: "-1"=On, " 0"=Off }
    Graphics        : char;                  { 11: 'Y','N','7' }
    NodeChat        : char;                  { 12: ' ' }
    DTESpeed        : array[0..4] of char;   { 13: DTE port speed }
    ConnectSpeed    : array[0..4] of char;   { 18: connect speed or "Local" }
    UserRecNum      : Word;                  { 23: user's record # in USERS }
    FirstName       : array[0..14] of char;  { 25: first name, 15 chars }
    Password        : array[0..11] of char;  { 40: password, 12 chars }
    TimeLoggedOn    : Word;                  { 52: minutes since midnight }
    TimeUsedToday   : SmallInt;              { 54: negative minutes }
    TimeLoggedOnStr : array[0..4] of char;   { 56: "HH:MM" }
    TimeAllowed     : Word;                  { 61: allowed minutes }
    AllowedKBytes   : Word;                  { 63: allowed KB download }
    Filler65        : array[65..75] of char; { 65: conference/status fields }
    ConfAddTime     : Word;                  { 76: add time minutes }
    UploadCredit    : Word;                  { 78: upload/chat credit mins }
    LangExtension   : array[0..3] of char;   { 80: language extension }
    FullName        : array[0..24] of char;  { 84: full name, 25 chars }
    MinsRemaining   : Word;                  { 109: calculated mins left }
    NodeNum8bit     : byte;                  { 111: node# (255=see extended) }
    EventTime       : array[0..4] of char;   { 112: "HH:MM" or "00:00" }
    EventActive     : array[0..1] of char;   { 117: "-1"=active }
    Reserved119     : array[0..1] of char;   { 119: reserved }
    Filler121       : array[121..127] of char;{ 121: fill to 128 }
  end;

  { CALLERS log record — exactly 64 bytes (CALLERS.DOC)
    Random access text file, can be TYPE'd to screen }
  TPcbCallerRecord = packed record
    Text : array[0..61] of char;   { 0: the log line text }
    CRLF : array[0..1] of char;    { 62: #13 #10 }
  end;

  { Status info for a telnet connection }
  TPcbNodeInfo = record
    NodeNum      : integer;
    UserName     : string;
    FirstName    : string;
    City         : string;
    ConnectSpeed : string;    { 'TELNET' for pcbis connections }
    ConnectTime  : TDateTime;
    SecurityLevel: integer;
    Activity     : string;
  end;

{ Write PCBOARD.SYS for a node — marks node as active with caller info }
procedure WriteNodeStatus(const PCBDir : string; const Info : TPcbNodeInfo);

{ Clear PCBOARD.SYS for a node — marks node as idle (no caller) }
procedure ClearNodeStatus(const PCBDir : string; NodeNum : integer);

{ Append to CALLERS log in correct 64-byte record format }
procedure AppendCaller(const PCBDir : string; NodeNum : integer;
                       const UserName, City, ConnectSpeed : string;
                       LoginTime : TDateTime);

{ Read who's online — scan PCBOARD.SYS files for active nodes }
function GetOnlineUsers(const PCBDir : string; MaxNodes : integer) : string;

{ Read last N callers from CALLERS log (reads from end, displays in reverse) }
function GetLastCallers(const PCBDir : string; Count : integer) : string;

implementation

uses
  SysUtils;

const
  PCBSYS_SIZE = 128;      { minimum PCBOARD.SYS size }
  CALLER_RECSIZE = 64;    { CALLERS log record size }

{ Pad a string to exactly Len chars with spaces }
function PadStr(const S : string; Len : integer) : string;
begin
  Result := S;
  while Length(Result) < Len do
    Result := Result + ' ';
  if Length(Result) > Len then
    Result := Copy(Result, 1, Len);
end;

{ Get PCBOARD.SYS filename for a node }
function SysFileName(const PCBDir : string; NodeNum : integer) : string;
begin
  if NodeNum <= 1 then
    Result := PCBDir + DirectorySeparator + 'PCBOARD.SYS'
  else
    Result := PCBDir + DirectorySeparator + 'PCBOARD.' + IntToStr(NodeNum);
end;

procedure WriteNodeStatus(const PCBDir : string; const Info : TPcbNodeInfo);
var
  F   : file;
  Rec : TPcbSysRecord;
  H, M : Word;
begin
  FillChar(Rec, SizeOf(Rec), ' ');  { pre-fill with spaces per spec }

  { Display/Printer/Bell/Alarm all On }
  Rec.DisplayOnOff := '-1';
  Rec.PrinterOnOff := ' 0';
  Rec.PageBellOnOff := '-1';
  Rec.CallerAlarmOnOff := '-1';

  { Error corrected = yes for TCP connections }
  Rec.ErrorCorrected := '-1';
  Rec.Graphics := 'Y';

  { Speeds }
  Move(PadStr(Info.ConnectSpeed, 5)[1], Rec.DTESpeed, 5);
  Move(PadStr(Info.ConnectSpeed, 5)[1], Rec.ConnectSpeed, 5);

  { User info }
  Rec.UserRecNum := 0; { TODO: lookup from USERS file }
  Move(PadStr(Info.FirstName, 15)[1], Rec.FirstName, 15);
  Move(PadStr(Info.UserName, 25)[1], Rec.FullName, 25);

  { Time }
  DecodeTime(Info.ConnectTime, H, M, Word(0), Word(0));
  Rec.TimeLoggedOn := H * 60 + M;
  Move(PadStr(FormatDateTime('hh:nn', Info.ConnectTime), 5)[1],
       Rec.TimeLoggedOnStr, 5);
  Rec.TimeAllowed := 120;   { default 2 hours }
  Rec.AllowedKBytes := 10240; { 10MB default }
  Rec.MinsRemaining := 120;

  { Node number }
  if Info.NodeNum < 255 then
    Rec.NodeNum8bit := Info.NodeNum
  else
    Rec.NodeNum8bit := 255;

  { Event }
  Move(PadStr('00:00', 5)[1], Rec.EventTime, 5);
  Rec.EventActive := ' 0';

  { Write the file }
  try
    AssignFile(F, SysFileName(PCBDir, Info.NodeNum));
    Rewrite(F, 1);
    BlockWrite(F, Rec, PCBSYS_SIZE);
    CloseFile(F);
  except
  end;
end;

procedure ClearNodeStatus(const PCBDir : string; NodeNum : integer);
var
  F   : file;
  Rec : TPcbSysRecord;
begin
  { Per spec: fill offsets 9-127 with spaces, leave 0-8 intact }
  FillChar(Rec, SizeOf(Rec), ' ');
  Rec.DisplayOnOff := '-1';
  Rec.PrinterOnOff := ' 0';
  Rec.PageBellOnOff := ' 0';
  Rec.CallerAlarmOnOff := ' 0';

  try
    AssignFile(F, SysFileName(PCBDir, NodeNum));
    Rewrite(F, 1);
    BlockWrite(F, Rec, PCBSYS_SIZE);
    CloseFile(F);
  except
  end;
end;

procedure AppendCaller(const PCBDir : string; NodeNum : integer;
                       const UserName, City, ConnectSpeed : string;
                       LoginTime : TDateTime);
var
  F    : file;
  Rec  : TPcbCallerRecord;
  Line : string;
begin
  { Format the 62-char log line — matches PCBoard's caller log format:
    "Node#  Name                     City                Speed  Date  Time" }
  Line := Format('%-4d %-25s %-13s %-5s %s %s', [
    NodeNum,
    Copy(UserName, 1, 25),
    Copy(City, 1, 13),
    Copy(ConnectSpeed, 1, 5),
    FormatDateTime('mm/dd/yy', LoginTime),
    FormatDateTime('hh:nn', LoginTime)
  ]);

  { Pad/truncate to exactly 62 chars }
  Line := PadStr(Line, 62);
  if Length(Line) > 62 then
    Line := Copy(Line, 1, 62);

  FillChar(Rec, SizeOf(Rec), ' ');
  Move(Line[1], Rec.Text, 62);
  Rec.CRLF[0] := #13;
  Rec.CRLF[1] := #10;

  try
    AssignFile(F, PCBDir + DirectorySeparator + 'CALLERS');
    if FileExists(PCBDir + DirectorySeparator + 'CALLERS') then
    begin
      Reset(F, 1);
      Seek(F, FileSize(F));
    end
    else
      Rewrite(F, 1);
    BlockWrite(F, Rec, CALLER_RECSIZE);
    CloseFile(F);
  except
  end;
end;

function GetOnlineUsers(const PCBDir : string; MaxNodes : integer) : string;
var
  I   : integer;
  F   : file;
  Rec : TPcbSysRecord;
  Fn  : string;
  Name : string;
begin
  Result := '';
  for I := 1 to MaxNodes do
  begin
    Fn := SysFileName(PCBDir, I);
    if not FileExists(Fn) then Continue;

    try
      AssignFile(F, Fn);
      Reset(F, 1);
      if FileSize(F) >= PCBSYS_SIZE then
      begin
        BlockRead(F, Rec, PCBSYS_SIZE);
        { Check if node has an active user (FirstName not all spaces) }
        Name := Trim(string(Rec.FullName));
        if Name <> '' then
          Result := Result + Format('Node %d: %s (connected at %s)'#13#10,
                    [I, Name, Trim(string(Rec.TimeLoggedOnStr))]);
      end;
      CloseFile(F);
    except
    end;
  end;

  if Result = '' then
    Result := 'No users online'#13#10;
end;

function GetLastCallers(const PCBDir : string; Count : integer) : string;
var
  F      : file;
  Rec    : TPcbCallerRecord;
  Fn     : string;
  Total  : integer;
  I, Start : integer;
begin
  Result := '';
  Fn := PCBDir + DirectorySeparator + 'CALLERS';
  if not FileExists(Fn) then Exit;

  try
    AssignFile(F, Fn);
    Reset(F, 1);
    Total := FileSize(F) div CALLER_RECSIZE;

    { Read last N records — PCBoard displays in reverse order }
    Start := Total - Count;
    if Start < 0 then Start := 0;

    for I := Total - 1 downto Start do
    begin
      Seek(F, I * CALLER_RECSIZE);
      BlockRead(F, Rec, CALLER_RECSIZE);
      Result := Result + Trim(string(Rec.Text)) + #13#10;
    end;

    CloseFile(F);
  except
  end;
end;

end.
