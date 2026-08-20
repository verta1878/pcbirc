{ ===========================================================================
  pcbis_qwk.pas — QWK/QWKE mail networking
  Board-to-board message exchange via QWK packets over internet.
  Replaces dial-up QWK networking with FTP/HTTP transport.
  =========================================================================== }

unit pcbis_qwk;

{$mode objfpc}{$H+}

interface

uses
  pcbis_config;

const
  QWK_MSG_PER_BLOCK = 128;  { bytes per QWK block }
  QWK_HEADER_BLOCKS = 1;    { 1 block = 128 bytes per message header }

type
  { QWK message header — exactly 128 bytes }
  TQwkMsgHeader = packed record
    Status    : char;           { ' '=public, '*'=private, '+' =public/read, '~'=private/read }
    MsgNum    : array[1..7] of char;  { message number, space-padded }
    DateTime  : array[1..13] of char; { MM-DD-YYHH:MM }
    ToUser    : array[1..25] of char; { recipient }
    FromUser  : array[1..25] of char; { sender }
    Subject   : array[1..25] of char; { subject line }
    Password  : array[1..12] of char; { message password }
    RefNum    : array[1..8] of char;  { reference message number }
    NumBlocks : array[1..6] of char;  { total blocks (header + text) }
    Alive     : char;           { #225 = alive, #226 = deleted }
    ConfNum   : Word;           { conference number (little-endian) }
    Filler    : Word;           { unused }
    NetTag    : char;           { '*' = has network tagline }
  end;

  { CONTROL.DAT fields }
  TQwkControl = record
    BBSName     : string;
    BBSCity     : string;
    BBSPhone    : string;
    SysopName   : string;
    BoardID     : string;    { QWK board ID (8 chars) }
    DateTime    : string;
    NumConfs    : integer;
    ConfNames   : array of string;
    ConfNums    : array of integer;
  end;

  { QWKE extensions }
  TQwkeHeader = record
    ToAddr      : string;    { internet email address }
    FromAddr    : string;    { internet email address }
    Subject     : string;    { longer subject (up to 72 chars) }
    MsgID       : string;    { unique message ID }
    ReplyID     : string;    { reply-to message ID }
  end;

  { Conference mapping — QWK conf# ↔ PCBoard conference }
  TConfMap = record
    QwkNum      : integer;
    PcbConfName : string;
    PcbConfNum  : integer;
    Enabled     : boolean;
  end;

  TPcbisQwk = class
  private
    FCfg      : TPcbisConfig;
    FConfMap  : array of TConfMap;
    FBoardID  : string;

    procedure BuildControlDat(const Filename : string);
    procedure ExportMessages(const OutDir : string);
    procedure ImportMessages(const InDir : string);
    function  ReadMsgHeader(var F : file; var Hdr : TQwkMsgHeader) : boolean;
    procedure WriteMsgHeader(var F : file; const Hdr : TQwkMsgHeader);
  public
    constructor Create(ACfg : TPcbisConfig);
    destructor Destroy; override;

    { Export PCBoard messages → QWK packet }
    procedure ExportQwk(const OutFile : string);

    { Import REP packet → PCBoard messages }
    procedure ImportRep(const RepFile : string);

    { Full exchange cycle }
    procedure DoExchange;

    { Add conference mapping }
    procedure MapConference(QwkNum, PcbNum : integer; const PcbName : string);
  end;

implementation

uses
  SysUtils, Classes, pcbis_log;

constructor TPcbisQwk.Create(ACfg : TPcbisConfig);
begin
  inherited Create;
  FCfg := ACfg;
  FBoardID := 'PCBREV';   { TODO: from config }
  SetLength(FConfMap, 0);
end;

destructor TPcbisQwk.Destroy;
begin
  SetLength(FConfMap, 0);
  inherited Destroy;
end;

procedure TPcbisQwk.MapConference(QwkNum, PcbNum : integer; const PcbName : string);
var
  N : integer;
begin
  N := Length(FConfMap);
  SetLength(FConfMap, N + 1);
  FConfMap[N].QwkNum := QwkNum;
  FConfMap[N].PcbConfNum := PcbNum;
  FConfMap[N].PcbConfName := PcbName;
  FConfMap[N].Enabled := True;
end;

procedure TPcbisQwk.BuildControlDat(const Filename : string);
var
  F : TextFile;
  I : integer;
begin
  AssignFile(F, Filename);
  Rewrite(F);

  WriteLn(F, 'PCBoard 15.4 Revival');      { line 1: BBS name }
  WriteLn(F, '');                           { line 2: city/state }
  WriteLn(F, '000-000-0000');               { line 3: phone }
  WriteLn(F, 'SYSOP');                      { line 4: sysop name }
  WriteLn(F, '0,' + FBoardID);             { line 5: serial#, board ID }
  WriteLn(F, FormatDateTime('mm-dd-yyyy,hh:nn:ss', Now)); { line 6: date/time }
  WriteLn(F, '');                           { line 7: user name }
  WriteLn(F, '');                           { line 8: blank }
  WriteLn(F, '0');                          { line 9: blank }
  WriteLn(F, '0');                          { line 10: total messages }
  WriteLn(F, IntToStr(Length(FConfMap)));    { line 11: number of conferences }

  for I := 0 to High(FConfMap) do
  begin
    WriteLn(F, IntToStr(FConfMap[I].QwkNum));
    WriteLn(F, FConfMap[I].PcbConfName);
  end;

  { Footer }
  WriteLn(F, 'HELLO');
  WriteLn(F, 'NEWS');
  WriteLn(F, 'GOODBYE');

  CloseFile(F);
end;

procedure TPcbisQwk.ExportMessages(const OutDir : string);
var
  MsgFile : file;
  Hdr     : TQwkMsgHeader;
  Block   : array[1..128] of byte;
begin
  { TODO: Read PCBoard message bases for each mapped conference,
    find new messages since last export, write to MESSAGES.DAT.

    For each message:
    1. Fill TQwkMsgHeader (128 bytes)
    2. Write header block
    3. Write message text in 128-byte blocks
    4. Pad last block to 128 bytes

    Message text: strip PCBoard @-codes, convert to CP437.
    Track last exported message number per conference in .PTR file. }

  AssignFile(MsgFile, OutDir + DirectorySeparator + 'MESSAGES.DAT');
  Rewrite(MsgFile, 1);

  { Write copyright block (first 128 bytes) }
  FillChar(Block, SizeOf(Block), ' ');
  Move('Produced by pcbis QWK', Block, 21);
  BlockWrite(MsgFile, Block, 128);

  { TODO: iterate conferences and export messages }

  CloseFile(MsgFile);
  LogSmtp(llInfo, 'QWK export: wrote MESSAGES.DAT');
end;

procedure TPcbisQwk.ImportMessages(const InDir : string);
var
  MsgFile : file;
  Hdr     : TQwkMsgHeader;
  Block   : array[1..128] of byte;
  NumBlks : integer;
  MsgText : string;
  I, N    : integer;
begin
  { TODO: Read incoming .REP file's MESSAGES.DAT,
    parse each QWK message header, inject into PCBoard message base
    for the mapped conference.

    For each message:
    1. Read 128-byte header → TQwkMsgHeader
    2. Read (NumBlocks - 1) * 128 bytes of text
    3. Find PCBoard conference from ConfNum via FConfMap
    4. Write to PCBoard message base }

  if not FileExists(InDir + DirectorySeparator + 'MESSAGES.DAT') then
  begin
    LogSmtp(llInfo, 'QWK import: no MESSAGES.DAT found');
    Exit;
  end;

  AssignFile(MsgFile, InDir + DirectorySeparator + 'MESSAGES.DAT');
  Reset(MsgFile, 1);

  { Skip copyright block }
  BlockRead(MsgFile, Block, 128);

  N := 0;
  while not EOF(MsgFile) do
  begin
    { Read header }
    BlockRead(MsgFile, Hdr, 128);
    if Hdr.Alive <> #225 then Continue;

    { Parse number of blocks }
    NumBlks := StrToIntDef(Trim(string(Hdr.NumBlocks)), 1);

    { Read message text blocks }
    MsgText := '';
    for I := 2 to NumBlks do
    begin
      FillChar(Block, 128, 0);
      BlockRead(MsgFile, Block, 128);
      SetLength(MsgText, Length(MsgText) + 128);
      Move(Block, MsgText[Length(MsgText) - 127], 128);
    end;

    { Trim trailing spaces/nulls }
    MsgText := TrimRight(MsgText);

    { TODO: inject into PCBoard message base for conference Hdr.ConfNum }
    Inc(N);
  end;

  CloseFile(MsgFile);
  LogSmtp(llInfo, 'QWK import: processed ' + IntToStr(N) + ' messages');
end;

function TPcbisQwk.ReadMsgHeader(var F : file; var Hdr : TQwkMsgHeader) : boolean;
var
  BytesRead : integer;
begin
  BlockRead(F, Hdr, SizeOf(Hdr), BytesRead);
  Result := (BytesRead = SizeOf(Hdr));
end;

procedure TPcbisQwk.WriteMsgHeader(var F : file; const Hdr : TQwkMsgHeader);
begin
  BlockWrite(F, Hdr, SizeOf(Hdr));
end;

procedure TPcbisQwk.ExportQwk(const OutFile : string);
var
  WorkDir : string;
begin
  WorkDir := ExtractFilePath(OutFile) + 'qwkwork';
  ForceDirectories(WorkDir);

  BuildControlDat(WorkDir + DirectorySeparator + 'CONTROL.DAT');
  ExportMessages(WorkDir);

  { TODO: ZIP WorkDir contents into OutFile (.QWK) }

  LogSmtp(llInfo, 'QWK packet exported: ' + OutFile);
end;

procedure TPcbisQwk.ImportRep(const RepFile : string);
var
  WorkDir : string;
begin
  WorkDir := ExtractFilePath(RepFile) + 'repwork';
  ForceDirectories(WorkDir);

  { TODO: unZIP RepFile into WorkDir }

  ImportMessages(WorkDir);

  LogSmtp(llInfo, 'REP packet imported: ' + RepFile);
end;

procedure TPcbisQwk.DoExchange;
begin
  { Full QWK exchange cycle:
    1. Export new messages → .QWK packet
    2. Transport .QWK to hub (FTP upload or HTTP POST)
    3. Download .REP from hub (FTP download or HTTP GET)
    4. Import .REP → PCBoard messages

    Transport configured in [qwk] section of pcbis.cfg:
      hub_url = ftp://hub.example.com/qwk/
      hub_user = boardid
      hub_pass = password
  }
  LogEvent(llInfo, 'QWK exchange: starting');
  ExportQwk('qwkout' + DirectorySeparator + FBoardID + '.QWK');
  { TODO: FTP/HTTP transport }
  { TODO: download and import .REP }
  LogEvent(llInfo, 'QWK exchange: complete');
end;

end.
