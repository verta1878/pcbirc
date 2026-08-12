{ ===========================================================================
  pcbis_nntp.pas — NNTP client for pcbis (PCBNNTP)
  Pulls articles from internet news servers, injects into PCBoard
  message bases. Outbound posting of local messages to newsgroups.
  Reference: MIS mis_client_nntp.pas (847 lines)
  =========================================================================== }

unit pcbis_nntp;

{$mode objfpc}{$H+}

interface

uses
  pcbis_config;

type
  { Newsgroup ↔ PCBoard conference mapping }
  TNewsgroupMap = record
    Newsgroup  : string;   { e.g. comp.lang.pascal }
    PcbConf    : integer;  { PCBoard conference number }
    PcbName    : string;   { PCBoard conference name }
    LastArticle: longint;  { last article number pulled }
    PostAllowed: boolean;  { allow outbound posting }
  end;

  TPcbisNntp = class
  private
    FCfg       : TPcbisConfig;
    FSocket    : longint;
    FGroups    : array of TNewsgroupMap;
    FConnected : boolean;

    function  Connect : boolean;
    procedure Disconnect;
    function  SendCmd(const Cmd : string) : string;
    function  ReadLine : string;
    function  ReadMultiLine : TStringList;
    procedure PullGroup(var Group : TNewsgroupMap);
    procedure PostArticle(const Group, From, Subject, Body : string);
  public
    constructor Create(ACfg : TPcbisConfig);
    destructor Destroy; override;

    procedure AddGroup(const Newsgroup : string; PcbConf : integer;
                       const PcbName : string; PostAllowed : boolean);

    { Pull new articles from all mapped newsgroups }
    procedure PullAll;

    { Post pending outbound messages to newsgroups }
    procedure PostAll;

    { Full exchange }
    procedure DoExchange;
  end;

implementation

uses
  SysUtils, Classes, Sockets, BaseUnix, pcbis_log;

constructor TPcbisNntp.Create(ACfg : TPcbisConfig);
begin
  inherited Create;
  FCfg := ACfg;
  FSocket := -1;
  FConnected := False;
  SetLength(FGroups, 0);
end;

destructor TPcbisNntp.Destroy;
begin
  if FConnected then Disconnect;
  SetLength(FGroups, 0);
  inherited Destroy;
end;

procedure TPcbisNntp.AddGroup(const Newsgroup : string; PcbConf : integer;
                              const PcbName : string; PostAllowed : boolean);
var N : integer;
begin
  N := Length(FGroups);
  SetLength(FGroups, N + 1);
  FGroups[N].Newsgroup := Newsgroup;
  FGroups[N].PcbConf := PcbConf;
  FGroups[N].PcbName := PcbName;
  FGroups[N].LastArticle := 0;
  FGroups[N].PostAllowed := PostAllowed;
end;

function TPcbisNntp.Connect : boolean;
var
  Addr : TInetSockAddr;
begin
  Result := False;
  FSocket := fpSocket(AF_INET, SOCK_STREAM, 0);
  if FSocket < 0 then
  begin
    LogMsg(lpMain, llError, 'NNTP: socket creation failed');
    Exit;
  end;

  FillChar(Addr, SizeOf(Addr), 0);
  Addr.sin_family := AF_INET;
  Addr.sin_port := htons(FCfg.NntpPort);
  Addr.sin_addr := StrToNetAddr(FCfg.NntpServer);

  if fpConnect(FSocket, @Addr, SizeOf(Addr)) <> 0 then
  begin
    LogMsg(lpMain, llError, 'NNTP: connect to ' + FCfg.NntpServer + ' failed');
    CloseSocket(FSocket);
    FSocket := -1;
    Exit;
  end;

  { Read greeting }
  ReadLine;

  { Authenticate if credentials provided }
  if FCfg.NntpUser <> '' then
  begin
    SendCmd('AUTHINFO USER ' + FCfg.NntpUser);
    SendCmd('AUTHINFO PASS ' + FCfg.NntpPass);
  end;

  FConnected := True;
  LogMsg(lpMain, llInfo, 'NNTP: connected to ' + FCfg.NntpServer);
  Result := True;
end;

procedure TPcbisNntp.Disconnect;
begin
  if FSocket >= 0 then
  begin
    SendCmd('QUIT');
    CloseSocket(FSocket);
    FSocket := -1;
  end;
  FConnected := False;
end;

function TPcbisNntp.ReadLine : string;
var
  C : char;
  N : longint;
begin
  Result := '';
  repeat
    N := fpRecv(FSocket, @C, 1, 0);
    if N <= 0 then Exit;
    if C = #10 then Break;
    if C <> #13 then Result := Result + C;
  until False;
end;

function TPcbisNntp.SendCmd(const Cmd : string) : string;
var
  S : string;
begin
  S := Cmd + #13#10;
  fpSend(FSocket, @S[1], Length(S), 0);
  Result := ReadLine;
  LogMsg(lpMain, llDebug, 'NNTP: ' + Cmd + ' → ' + Copy(Result, 1, 30));
end;

function TPcbisNntp.ReadMultiLine : TStringList;
var
  Line : string;
begin
  Result := TStringList.Create;
  repeat
    Line := ReadLine;
    if Line = '.' then Break;
    if (Length(Line) > 1) and (Line[1] = '.') then
      Delete(Line, 1, 1);  { dot-stuffing }
    Result.Add(Line);
  until False;
end;

procedure TPcbisNntp.PullGroup(var Group : TNewsgroupMap);
var
  Reply     : string;
  Articles  : TStringList;
  I         : integer;
  ArtNum    : longint;
  Headers   : TStringList;
  Body      : TStringList;
  From, Subj, MsgBody : string;
  NewCount  : integer;
begin
  { Select group }
  Reply := SendCmd('GROUP ' + Group.Newsgroup);
  if Copy(Reply, 1, 3) <> '211' then
  begin
    LogMsg(lpMain, llWarn, 'NNTP: cannot select ' + Group.Newsgroup + ': ' + Reply);
    Exit;
  end;

  { Get new article numbers since last pull }
  if Group.LastArticle > 0 then
    Reply := SendCmd('NEWNEWS ' + Group.Newsgroup + ' ' +
                     FormatDateTime('yymmdd hhnnss', Now - 1) + ' GMT')
  else
    Reply := SendCmd('XOVER 1-');

  { For simplicity, use XOVER to list articles }
  Reply := SendCmd('XOVER ' + IntToStr(Group.LastArticle + 1) + '-');
  if Copy(Reply, 1, 3) <> '224' then
  begin
    LogMsg(lpMain, llInfo, 'NNTP: no new articles in ' + Group.Newsgroup);
    Exit;
  end;

  Articles := ReadMultiLine;
  NewCount := 0;

  try
    for I := 0 to Articles.Count - 1 do
    begin
      { XOVER format: num\tsubject\tfrom\tdate\tmsg-id\treferences\tbytes\tlines }
      ArtNum := StrToIntDef(Copy(Articles[I], 1, Pos(#9, Articles[I]) - 1), 0);
      if ArtNum <= Group.LastArticle then Continue;

      { Fetch article }
      Reply := SendCmd('ARTICLE ' + IntToStr(ArtNum));
      if Copy(Reply, 1, 3) <> '220' then Continue;

      Body := ReadMultiLine;
      try
        { Parse headers and body }
        From := '';
        Subj := '';
        MsgBody := '';

        { Find blank line separating headers from body }
        I := 0;
        while I < Body.Count do
        begin
          if Body[I] = '' then Break;
          if Pos('From:', Body[I]) = 1 then
            From := Trim(Copy(Body[I], 6, Length(Body[I])))
          else if Pos('Subject:', Body[I]) = 1 then
            Subj := Trim(Copy(Body[I], 9, Length(Body[I])));
          Inc(I);
        end;
        { Rest is body }
        Inc(I);
        while I < Body.Count do
        begin
          MsgBody := MsgBody + Body[I] + #13#10;
          Inc(I);
        end;

        { TODO: Inject into PCBoard message base for conference Group.PcbConf
          - Create message header record
          - Write to conference message file
          - Update high message number }

        Group.LastArticle := ArtNum;
        Inc(NewCount);
      finally
        Body.Free;
      end;
    end;
  finally
    Articles.Free;
  end;

  if NewCount > 0 then
    LogMsg(lpMain, llInfo, 'NNTP: pulled ' + IntToStr(NewCount) +
           ' articles from ' + Group.Newsgroup +
           ' → conference ' + IntToStr(Group.PcbConf));
end;

procedure TPcbisNntp.PostArticle(const Group, From, Subject, Body : string);
var
  Reply : string;
begin
  Reply := SendCmd('POST');
  if Copy(Reply, 1, 3) <> '340' then
  begin
    LogMsg(lpMain, llWarn, 'NNTP: POST rejected: ' + Reply);
    Exit;
  end;

  { Send article }
  SendCmd('From: ' + From);
  SendCmd('Newsgroups: ' + Group);
  SendCmd('Subject: ' + Subject);
  SendCmd('Organization: PCBoard 15.4 Revival');
  SendCmd('X-Mailer: pcbis 0.1.0 (PCBNNTP)');
  SendCmd('');  { blank line = end of headers }
  SendCmd(Body);
  Reply := SendCmd('.');

  if Copy(Reply, 1, 3) = '240' then
    LogMsg(lpMain, llInfo, 'NNTP: posted to ' + Group + ': ' + Subject)
  else
    LogMsg(lpMain, llWarn, 'NNTP: post failed: ' + Reply);
end;

procedure TPcbisNntp.PostAll;
var
  I : integer;
begin
  { TODO: scan PCBoard message bases for outbound messages
    - Check each mapped conference for new messages since last scan
    - For messages with NNTP origin or flagged for newsgroup posting
    - Call PostArticle with proper headers }

  for I := 0 to High(FGroups) do
  begin
    if not FGroups[I].PostAllowed then Continue;
    { TODO: scan conference FGroups[I].PcbConf for outbound messages }
  end;
end;

procedure TPcbisNntp.PullAll;
var I : integer;
begin
  if not Connect then Exit;
  try
    for I := 0 to High(FGroups) do
      PullGroup(FGroups[I]);
  finally
    Disconnect;
  end;
end;

procedure TPcbisNntp.DoExchange;
begin
  if not FCfg.NntpEnabled then Exit;
  LogEvent(llInfo, 'NNTP exchange starting');

  if not Connect then Exit;
  try
    { Pull first, then post }
    PullAll;
    PostAll;
  finally
    Disconnect;
  end;

  LogEvent(llInfo, 'NNTP exchange complete');
end;

end.
