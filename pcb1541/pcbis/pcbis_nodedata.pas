{
  pcbis_nodedata.pas — PCBoard Interprocess System: Node Data File Access
  ========================================================================
  Free Pascal unit for reading/writing PCBoard 15.x binary data files:
    PCBOARD.SYS  — Per-node caller/status information (systype)
    USERS.SYS    — Current caller's user record (syshdrtype + userrectype)
    USERNET.DAT  — Multi-node status/chat (usernethdrtype + nodetype[])

  Structure layouts taken from Clark Development Company source:
    SYS.C        — systype (PCBOARD.SYS)
    USERSYS.H    — syshdrtype, userrectype (USERS.SYS)
    PCBOARD.H    — usernethdrtype, nodetype, updttype (USERNET.DAT)

  Part of the PCBoard 15.41 Revival Project
  License: GPLv3 (our additions)
  August 2026
}
unit pcbis_nodedata;

{$MODE OBJFPC}
{$PACKRECORDS 1}    { Byte-aligned, matches C #pragma pack(1) }
{$H+}

interface

uses
  SysUtils, Classes;

const
  { Node status characters (nodetype.Status) }
  NS_AVAILABLE    = 'A';   { Available for chat }
  NS_UNAVAILABLE  = 'U';   { Unavailable }
  NS_GROUP_CHAT   = 'G';   { In group chat }
  NS_XFER_DNLD    = 'D';   { Downloading }
  NS_XFER_UPLD    = 'L';   { Uploading }
  NS_LOGOFF       = 'O';   { Logging off }
  NS_ENTERING_MSG = 'E';   { Entering a message }
  NS_READING_MSG  = 'R';   { Reading messages }
  NS_DOOR         = 'X';   { In a door/external }
  NS_WAITING      = 'W';   { Waiting for caller }
  NS_EVENT        = 'N';   { Running event }
  NS_SYSOP_CHAT   = 'C';   { Chatting with sysop }

  { Graphics mode (systype.GraphicsMode) }
  GM_YES       = 'Y';      { ANSI graphics }
  GM_NO        = 'N';      { No graphics, 8-bit clean }
  GM_7BIT      = '7';      { No graphics, 7-bit }

  { UserNet status (systype.UserNetStatus) }
  AVAILABLE    = 1;
  UNAVAILABLE  = 0;

  { Maximum nodes for USERNET flag arrays }
  PCB_MAXNODES = 250;
  USERNETFLAGSIZE = (PCB_MAXNODES + 7) div 8;  { 32 bytes }

type
  { -------------------------------------------------------------------
    PCBOARD.SYS — systype
    Written/read by makepcboardsys()/readpcboardsys() in SYS.C
    One file per node, found in the node's working directory.
    All char fields are space-padded (memset ' '), NOT null-terminated.
    ------------------------------------------------------------------- }

  { Bitfield byte reserved in PCBOARD.SYS }
  TResByte = bitpacked record
    TerseMode  : Boolean;   { Terse display mode }
    RipMode    : Boolean;   { RIPscrip graphics mode }
    UseAlias   : Boolean;   { Alias support enabled }
    AliasInUse : Boolean;   { Caller is currently using alias }
    HstMode    : Boolean;   { HST modem mode }
    Telnet     : Boolean;   { Telnet connection }
    Reserved6  : Boolean;
    Reserved7  : Boolean;
  end;

  TPCBoardSys = packed record
    Screen          : SmallInt;     { abool: display on/off }
    PrintLog        : SmallInt;     { abool: printer logging }
    PageBell        : SmallInt;     { abool: page bell on/off }
    Alarm           : SmallInt;     { abool: alarm on/off }
    SysopFlag       : Char;         { sysop available flag }
    ErrorCorrected  : SmallInt;     { abool: error-corrected connection }
    GraphicsMode    : Char;         { 'Y'=ANSI, 'N'=none, '7'=7-bit }
    UserNetStatus   : Char;         { node availability status }
    ModemSpeed      : array[0..4] of Char;    { port open rate }
    CarrierSpeed    : array[0..4] of Char;    { actual caller speed }
    UserRecNo       : Word;         { user record number }
    FirstName       : array[0..14] of Char;   { caller's first name }
    Password        : array[0..11] of Char;   { caller's password }
    LogonMinute     : Word;         { minute of day logged on }
    TimeUsed        : SmallInt;     { minutes used this session }
    LogonTime       : array[0..4] of Char;    { HH:MM logon time }
    PwrdTimeAllowed : SmallInt;     { time allowed by password }
    MaxKBytesAllowed: SmallInt;     { max KB download allowed }
    Conference      : Char;         { current conference (byte, 0-254) }
    ConfJoined      : array[0..4] of Char;    { bitmap: conferences joined }
    ConfScanned     : array[0..4] of Char;    { bitmap: conferences scanned }
    ConfAddTime     : SmallInt;     { added time for conference }
    CreditMinutes   : SmallInt;     { credit minutes }
    MultiLangExt    : array[0..3] of Char;    { language extension }
    Name            : array[0..24] of Char;   { full caller name }
    MinutesLeft     : SmallInt;     { minutes remaining }
    NodeNum         : Char;         { node number (byte, 0=non-network) }
    EventTime       : array[0..4] of Char;    { next event time HH:MM }
    EventActive     : SmallInt;     { abool: event active }
    EventSlide      : SmallInt;     { abool: event can slide }
    MemorizeNum     : array[0..3] of Char;    { bassngl: memorized msg number }
    ComPortNumber   : Char;         { COM port }
    PackFlag        : Char;         { message pack flag }
    Reserve         : TResByte;     { bitfield byte }
    UseAnsi         : Boolean;      { ANSI in use }
    Country         : SmallInt;     { country code }
    CodePage        : SmallInt;     { code page }
    YesChar         : Char;         { localized Yes character }
    NoChar          : Char;         { localized No character }
    Language        : Char;         { language number }
    Reserve2        : array[0..2] of Char;    { reserved (old date/time) }
    RemoteDOS       : Boolean;      { in remote DOS/door }
    RunningEvent    : Boolean;      { event in progress }
    StopUploads     : Boolean;      { uploads stopped (pre-event) }
    Conference2     : Word;         { current conference (word, 0-65534) }
  end;

  { -------------------------------------------------------------------
    USERS.SYS — syshdrtype + userrectype
    Written/read by USERSYS.C writeusersys()/readusersys()
    Per-node file containing the current caller's complete user record.

    File layout:
      Offset 0: syshdrtype header
      After header: userrectype fixed record
      After fixed: conference-specific bit flags and records
    ------------------------------------------------------------------- }

  TUserSysHdr = packed record
    Version         : Word;     { PCBoard version (145, 150, 152) }
    RecNo           : LongInt;  { Record number from USERS file }
    SizeOfRec       : Word;     { Size of fixed user record }
    NumOfAreas      : Word;     { Number of conference areas }
    NumOfBitFields  : Word;     { Number of bitmap fields }
    SizeOfBitFields : Word;     { Size of each bitmap field }
    AppName         : array[0..14] of Char;   { Third-party app name }
    AppVersion      : Word;     { App version number }
    AppSizeOfRec    : Word;     { App fixed record size }
    AppSizeOfConfRec: Word;     { App per-conference record size }
    AppRecOffset    : LongInt;  { Offset of app record in USERS.INF }
    Updated         : Boolean;  { TRUE if file has been updated }
  end;

  { Bit-packed flags byte 1 }
  TPackedByte = bitpacked record
    Dirty          : Boolean;   { Record has been modified }
    MsgClear       : Boolean;   { Clear screen after messages }
    HasMail        : Boolean;   { New mail waiting }
    DontAskFSE    : Boolean;   { Don't prompt for FSE }
    FSEDefault     : Boolean;   { Use Full Screen Editor }
    ScrollMsgBody  : Boolean;   { Scroll message body }
    ShortHeader    : Boolean;   { Short message headers }
    WideEditor     : Boolean;   { Wide editor mode }
  end;

  { Bit-packed flags byte 2 }
  TPackedByte2 = bitpacked record
    UnAvailable    : Boolean;   { Unavailable for chat }
    SingleLines    : Boolean;   { Single-line file listings }
    Reserved       : 0..63;     { 6 bits reserved }
  end;

  { DOS packed date }
  TDOSDate = packed record
    DateWord : Word;  { Bits 0-4: Day, 5-8: Month, 9-15: Year-1980 }
  end;

  { Extended user info sub-records }
  TAddressType = packed record
    Street  : array[0..1] of array[0..50] of Char;   { 2 x 51 }
    City    : array[0..25] of Char;
    State   : array[0..10] of Char;
    Zip     : array[0..10] of Char;
    Country : array[0..15] of Char;
  end;

  TPasswordType = packed record
    Previous    : array[0..2] of array[0..12] of Char; { 3 x 13 }
    LastChange  : Word;
    TimesChanged: Word;
    ExpireDate  : Word;
  end;

  TNotesType = packed record
    Line : array[0..4] of array[0..60] of Char;   { 5 x 61 }
  end;

  TCallerStatType = packed record
    FirstDateOn     : Word;
    NumSysopPages   : Word;
    NumGroupChats   : Word;
    NumComments     : Word;
    Num300          : Word;
    Num1200         : Word;
    Num2400         : Word;
    Num9600         : Word;
    Num14400        : Word;
    NumSecViol      : Word;
    NumNotReg       : Word;
    NumReachDnldLim : Word;
    NumFileNotFound : Word;
    NumPwrdErrors   : Word;
    NumVerifyErrors : Word;
  end;

  TAccountType = packed record
    StartingBalance       : Double;
    StartThisSession      : Double;
    DebitCall             : Double;
    DebitTime             : Double;
    DebitMsgRead          : Double;
    DebitMsgReadCapture   : Double;
    DebitMsgWrite         : Double;
    DebitMsgWriteEchoed   : Double;
    DebitMsgWritePrivate  : Double;
    DebitDownloadFile     : Double;
    DebitDownloadBytes    : Double;
    DebitGroupChat        : Double;
    DebitTPU              : Double;
    DebitSpecial          : Double;
    CreditUploadFile      : Double;
    CreditUploadBytes     : Double;
    CreditSpecial         : Double;
    DropSecLevel          : Char;
  end;

  TQwkConfigType = packed record
    MaxMsgs            : Word;
    MaxMsgsPerConf     : Word;
    PersonalAttachLimit: LongInt;
    PublicAttachLimit   : LongInt;
    NewBltLimit        : LongInt;
    NewFiles           : Boolean;
    Reserved           : array[0..12] of Char;
  end;

  TUserRec = packed record
    Name             : array[0..25] of Char;    { NULL terminated }
    City             : array[0..24] of Char;    { NULL terminated }
    Password         : array[0..12] of Char;    { NULL terminated }
    BusDataPhone     : array[0..13] of Char;    { NULL terminated }
    HomeVoicePhone   : array[0..13] of Char;    { NULL terminated }
    LastDateOn       : Word;                    { Julian date }
    LastTimeOn       : array[0..5] of Char;     { NULL terminated }
    ExpertMode       : Boolean;
    Protocol         : Char;                    { A-Z }
    PackedFlags      : TPackedByte;
    DateLastDirRead  : TDOSDate;
    SecurityLevel    : SmallInt;
    NumTimesOn       : Word;
    PageLen          : Byte;
    NumUploads       : Word;
    NumDownloads     : Word;
    DailyDnldBytes   : LongInt;
    UserComment      : array[0..30] of Char;    { NULL terminated }
    SysopComment     : array[0..30] of Char;    { NULL terminated }
    ElapsedTimeOn    : SmallInt;
    RegExpDate       : Word;                    { Julian date }
    ExpSecurityLevel : SmallInt;
    LastConference   : Word;
    TotDnldBytesLo   : LongWord;               { ulTotDnldBytes }
    TotUpldBytesLo   : LongWord;               { ulTotUpldBytes }
    DeleteFlag       : Boolean;
    RecNum           : LongInt;                 { USERS.INF record }
    Flags            : TPackedByte2;
    Reserved         : array[0..7] of Char;
    MsgsRead         : LongWord;
    MsgsLeft         : LongWord;
    AliasSupport     : Boolean;
    Alias            : array[0..25] of Char;
    AddressSupport   : Boolean;
    Address          : TAddressType;
    PasswordSupport  : Boolean;
    PwrdHistory      : TPasswordType;
    VerifySupport    : Boolean;
    Verify           : array[0..25] of Char;
    StatsSupport     : Boolean;
    Stats            : TCallerStatType;
    NotesSupport     : Boolean;
    Notes            : TNotesType;
    AccountSupport   : Boolean;
    Account          : TAccountType;
    QwkSupport       : Boolean;
    QwkConfig        : TQwkConfigType;
    TotDnldBytes     : Double;                  { IEEE double }
    TotUpldBytes     : Double;                  { IEEE double }
  end;

  { -------------------------------------------------------------------
    USERNET.DAT — usernethdrtype + flag arrays + nodetype[]
    Multi-node interprocess communication file.

    File layout:
      Offset 0: usernethdrtype header (6 bytes)
      Offset 6: Busy flags   [USERNETFLAGSIZE bytes] — 1 bit per node
      Offset 6+USERNETFLAGSIZE: Chat flags [USERNETFLAGSIZE bytes]
      Offset USERNETSTART: nodetype[0..NumOfNodes-1]
    ------------------------------------------------------------------- }

  TUserNetHdr = packed record
    Version    : Word;     { PCBoard version number }
    NumOfNodes : Word;     { Number of nodes in USERNET.DAT }
    SizeOfRec  : Word;     { Size of each nodetype record }
  end;

  { Time/date stamp union for node updates }
  TUpdtType = packed record
    Time : Word;           { Seconds past midnight div 2 }
    Date : Word;           { Julian date }
  end;

  TNodeType = packed record
    Status      : Char;                       { Node status char (NS_* consts) }
    MailWaiting : Boolean;                    { Message posted for this node }
    Pager       : Word;                       { Node number of caller paging }
    Name        : array[0..25] of Char;       { Caller's name }
    City        : array[0..24] of Char;       { Caller's city }
    Operation   : array[0..48] of Char;       { Current operation text }
    Msg         : array[0..79] of Char;       { Broadcast message text }
    Channel     : Char;                       { Chat channel number }
    LastUpdate  : TUpdtType;                  { Timestamp of last update }
  end;

  { -------------------------------------------------------------------
    High-level access class
    ------------------------------------------------------------------- }

  TPCBNodeData = class
  private
    FNodePath : string;        { Path to node's working directory }
    FPCBDir   : string;        { Path to main PCBoard directory }
    FMaxNodes : Word;
  public
    Sys       : TPCBoardSys;   { PCBOARD.SYS data }
    UserHdr   : TUserSysHdr;   { USERS.SYS header }
    UserRec   : TUserRec;      { USERS.SYS user record }
    NetHdr    : TUserNetHdr;   { USERNET.DAT header }
    Nodes     : array of TNodeType;  { USERNET.DAT node records }
    BusyFlags : array of Byte;       { Node busy flags }
    ChatFlags : array of Byte;       { Node chat flags }

    constructor Create(const ANodePath, APCBDir: string);

    { Read PCBOARD.SYS from node directory }
    function ReadPCBoardSys: Boolean;
    { Write PCBOARD.SYS back }
    function WritePCBoardSys: Boolean;

    { Read USERS.SYS from node directory }
    function ReadUsersSys: Boolean;
    { Write USERS.SYS back }
    function WriteUsersSys: Boolean;

    { Read USERNET.DAT from PCBoard directory }
    function ReadUserNet: Boolean;
    { Write a single node record back to USERNET.DAT }
    function WriteNodeRecord(NodeNum: Word): Boolean;

    { Helpers }
    function GetCallerName: string;
    function GetCallerCity: string;
    function GetNodeStatus(NodeNum: Word): Char;
    function GetNodeOperation(NodeNum: Word): string;
    function IsNodeBusy(NodeNum: Word): Boolean;
    function IsNodeAvailable(NodeNum: Word): Boolean;

    { DOS date conversion }
    class function DOSDateToDateTime(D: TDOSDate): TDateTime;
    class function DateTimeToDOSDate(DT: TDateTime): TDOSDate;
    class function JulianToDate(J: Word): TDateTime;

    { Extract null/space-terminated string from char array }
    class function ExtractStr(const Buf; Len: Integer): string;

    property NodePath: string read FNodePath write FNodePath;
    property PCBDir: string read FPCBDir write FPCBDir;
    property MaxNodes: Word read FMaxNodes;
  end;

implementation

{ ---------- Helpers ---------- }

class function TPCBNodeData.ExtractStr(const Buf; Len: Integer): string;
var
  P: PChar;
  I: Integer;
begin
  P := @Buf;
  Result := '';
  for I := 0 to Len - 1 do
  begin
    if (P[I] = #0) then Break;
    Result := Result + P[I];
  end;
  Result := TrimRight(Result);
end;

class function TPCBNodeData.DOSDateToDateTime(D: TDOSDate): TDateTime;
var
  Day, Month, Year: Word;
begin
  Day   := D.DateWord and $1F;
  Month := (D.DateWord shr 5) and $0F;
  Year  := ((D.DateWord shr 9) and $7F) + 1980;
  if (Day = 0) or (Month = 0) then
    Result := 0
  else
    Result := EncodeDate(Year, Month, Day);
end;

class function TPCBNodeData.DateTimeToDOSDate(DT: TDateTime): TDOSDate;
var
  Y, M, D: Word;
begin
  DecodeDate(DT, Y, M, D);
  Result.DateWord := D or (M shl 5) or ((Y - 1980) shl 9);
end;

class function TPCBNodeData.JulianToDate(J: Word): TDateTime;
begin
  { PCBoard Julian = days since Jan 1, 1900 }
  if J = 0 then
    Result := 0
  else
    Result := EncodeDate(1900, 1, 1) + J - 1;
end;

{ ---------- Constructor ---------- }

constructor TPCBNodeData.Create(const ANodePath, APCBDir: string);
begin
  inherited Create;
  FNodePath := IncludeTrailingPathDelimiter(ANodePath);
  FPCBDir   := IncludeTrailingPathDelimiter(APCBDir);
  FMaxNodes := 0;
  FillChar(Sys, SizeOf(Sys), 0);
  FillChar(UserHdr, SizeOf(UserHdr), 0);
  FillChar(UserRec, SizeOf(UserRec), 0);
  FillChar(NetHdr, SizeOf(NetHdr), 0);
  SetLength(Nodes, 0);
  SetLength(BusyFlags, 0);
  SetLength(ChatFlags, 0);
end;

{ ---------- PCBOARD.SYS ---------- }

function TPCBNodeData.ReadPCBoardSys: Boolean;
var
  F: file;
  BytesRead: Integer;
begin
  Result := False;
  AssignFile(F, FNodePath + 'PCBOARD.SYS');
  {$I-}
  Reset(F, 1);
  {$I+}
  if IOResult <> 0 then Exit;

  try
    FillChar(Sys, SizeOf(Sys), 0);
    BlockRead(F, Sys, SizeOf(Sys), BytesRead);
    Result := (BytesRead >= 128);  { PCBoard accepts 128-byte legacy format }
  finally
    CloseFile(F);
  end;
end;

function TPCBNodeData.WritePCBoardSys: Boolean;
var
  F: file;
begin
  Result := False;
  AssignFile(F, FNodePath + 'PCBOARD.SYS');
  {$I-}
  Rewrite(F, 1);
  {$I+}
  if IOResult <> 0 then Exit;

  try
    BlockWrite(F, Sys, SizeOf(Sys));
    Result := True;
  finally
    CloseFile(F);
  end;
end;

{ ---------- USERS.SYS ---------- }

function TPCBNodeData.ReadUsersSys: Boolean;
var
  F: file;
  BytesRead: Integer;
begin
  Result := False;
  AssignFile(F, FNodePath + 'USERS.SYS');
  {$I-}
  Reset(F, 1);
  {$I+}
  if IOResult <> 0 then Exit;

  try
    { Read header }
    FillChar(UserHdr, SizeOf(UserHdr), 0);
    BlockRead(F, UserHdr, SizeOf(UserHdr), BytesRead);
    if BytesRead < SizeOf(UserHdr) then Exit;

    { Read fixed user record }
    FillChar(UserRec, SizeOf(UserRec), 0);
    BlockRead(F, UserRec, SizeOf(UserRec), BytesRead);
    Result := (BytesRead > 0);
  finally
    CloseFile(F);
  end;
end;

function TPCBNodeData.WriteUsersSys: Boolean;
var
  F: file;
begin
  Result := False;
  AssignFile(F, FNodePath + 'USERS.SYS');
  {$I-}
  Rewrite(F, 1);
  {$I+}
  if IOResult <> 0 then Exit;

  try
    UserHdr.Updated := True;
    BlockWrite(F, UserHdr, SizeOf(UserHdr));
    BlockWrite(F, UserRec, SizeOf(UserRec));
    Result := True;
  finally
    CloseFile(F);
  end;
end;

{ ---------- USERNET.DAT ---------- }

function TPCBNodeData.ReadUserNet: Boolean;
var
  F: file;
  BytesRead: Integer;
  FlagSize: Integer;
  I: Integer;
begin
  Result := False;
  AssignFile(F, FPCBDir + 'USERNET.DAT');
  {$I-}
  Reset(F, 1);
  {$I+}
  if IOResult <> 0 then Exit;

  try
    { Read header }
    FillChar(NetHdr, SizeOf(NetHdr), 0);
    BlockRead(F, NetHdr, SizeOf(NetHdr), BytesRead);
    if BytesRead < SizeOf(NetHdr) then Exit;

    FMaxNodes := NetHdr.NumOfNodes;
    FlagSize := (FMaxNodes + 7) div 8;

    { Read busy flags }
    SetLength(BusyFlags, FlagSize);
    FillChar(BusyFlags[0], FlagSize, 0);
    BlockRead(F, BusyFlags[0], FlagSize, BytesRead);

    { Read chat flags }
    SetLength(ChatFlags, FlagSize);
    FillChar(ChatFlags[0], FlagSize, 0);
    BlockRead(F, ChatFlags[0], FlagSize, BytesRead);

    { Read node records }
    SetLength(Nodes, FMaxNodes);
    for I := 0 to FMaxNodes - 1 do
    begin
      FillChar(Nodes[I], SizeOf(TNodeType), 0);
      if NetHdr.SizeOfRec <= SizeOf(TNodeType) then
        BlockRead(F, Nodes[I], NetHdr.SizeOfRec, BytesRead)
      else
      begin
        { Record is larger than our struct — read what we know, skip rest }
        BlockRead(F, Nodes[I], SizeOf(TNodeType), BytesRead);
        Seek(F, FilePos(F) + (NetHdr.SizeOfRec - SizeOf(TNodeType)));
      end;
    end;

    Result := True;
  finally
    CloseFile(F);
  end;
end;

function TPCBNodeData.WriteNodeRecord(NodeNum: Word): Boolean;
var
  F: file;
  Offset: LongInt;
  FlagSize: Integer;
begin
  Result := False;
  if (NodeNum < 1) or (NodeNum > FMaxNodes) then Exit;

  AssignFile(F, FPCBDir + 'USERNET.DAT');
  {$I-}
  Reset(F, 1);
  {$I+}
  if IOResult <> 0 then Exit;

  try
    FlagSize := (FMaxNodes + 7) div 8;
    Offset := SizeOf(TUserNetHdr) + (FlagSize * 2) +
              (LongInt(NodeNum - 1) * NetHdr.SizeOfRec);
    Seek(F, Offset);
    BlockWrite(F, Nodes[NodeNum - 1], NetHdr.SizeOfRec);
    Result := True;
  finally
    CloseFile(F);
  end;
end;

{ ---------- Property helpers ---------- }

function TPCBNodeData.GetCallerName: string;
begin
  Result := ExtractStr(Sys.Name, SizeOf(Sys.Name));
end;

function TPCBNodeData.GetCallerCity: string;
begin
  if Length(Nodes) > 0 then
    Result := ExtractStr(Nodes[0].City, SizeOf(Nodes[0].City))
  else
    Result := '';
end;

function TPCBNodeData.GetNodeStatus(NodeNum: Word): Char;
begin
  if (NodeNum >= 1) and (NodeNum <= FMaxNodes) then
    Result := Nodes[NodeNum - 1].Status
  else
    Result := #0;
end;

function TPCBNodeData.GetNodeOperation(NodeNum: Word): string;
begin
  if (NodeNum >= 1) and (NodeNum <= FMaxNodes) then
    Result := ExtractStr(Nodes[NodeNum - 1].Operation,
                         SizeOf(Nodes[NodeNum - 1].Operation))
  else
    Result := '';
end;

function TPCBNodeData.IsNodeBusy(NodeNum: Word): Boolean;
var
  ByteIdx, BitIdx: Integer;
begin
  Result := False;
  if (NodeNum < 1) or (NodeNum > FMaxNodes) then Exit;
  ByteIdx := (NodeNum - 1) div 8;
  BitIdx  := (NodeNum - 1) mod 8;
  if ByteIdx < Length(BusyFlags) then
    Result := (BusyFlags[ByteIdx] and (1 shl BitIdx)) <> 0;
end;

function TPCBNodeData.IsNodeAvailable(NodeNum: Word): Boolean;
begin
  Result := False;
  if (NodeNum < 1) or (NodeNum > FMaxNodes) then Exit;
  Result := (Nodes[NodeNum - 1].Status = NS_AVAILABLE) and
            not IsNodeBusy(NodeNum);
end;

end.
