{ ===========================================================================
  pcbis_wfc.pas — Waiting For Caller console UI
  ANSI text UI following PCBoard's color scheme.
  Pattern from MIS (mis_ansiwfc.pas) in mystic-bbs-irc.
  No LCL — raw ANSI escape codes only.

  Layout borrowed from AdeptXBBS 1.20a2 WFC + our protocol tabs.
  =========================================================================== }

unit pcbis_wfc;

{$mode objfpc}{$H+}

interface

uses
  pcbis_net, pcbis_config, pcbis_events;

type
  TWfcProtoTab = (
    wtTelnet, wtBinkp, wtFtp, wtHttp, wtSmtp, wtEvents
  );

  TLastCaller = record
    UserName : string[25];
    Node     : integer;
    TimeOn   : string[8];
    TimeOff  : string[8];
    Via      : string[8];
  end;

  TPcbisWfc = class
  private
    FCfg        : TPcbisConfig;
    FServer     : TPcbisServer;
    FEvents     : TPcbisEvents;
    FActiveTab  : TWfcProtoTab;
    FLogLines   : array[0..7] of string;
    FLogColors  : array[0..7] of byte;
    FLogCount   : integer;
    FCallers    : array[0..2] of TLastCaller;
    FCallerCnt  : integer;
    FTotalCalls : longint;
    FStartTime  : TDateTime;
    FNeedRedraw : boolean;
    FSelectedNode : integer;   { -1 = show connection list, >=0 = detail }
    FNodeCursor   : integer;   { highlight bar in connection list }

    { Screen primitives }
    procedure GotoXY(X, Y : integer);
    procedure SetColor(Fg, Bg : byte);
    procedure ClearScreen;
    procedure WriteAt(X, Y : integer; Fg, Bg : byte; const S : string);
    procedure HLine(X, Y, Len : integer; C : char; Fg, Bg : byte);
    procedure VLine(X, Y, Len : integer; C : char; Fg, Bg : byte);
    procedure Box(X1, Y1, X2, Y2 : integer; Style : byte; Fg, Bg : byte);
    procedure WritePadded(X, Y : integer; Fg, Bg : byte;
                          const S : string; Width : integer);

    { PCBoard color constants }
    function ColHeaderFg : byte;   { White }
    function ColHeaderBg : byte;   { Blue }
    function ColHighFg : byte;     { Yellow }
    function ColNormalFg : byte;   { LightGray }
    function ColNormalBg : byte;   { Black }
    function ColBorderFg : byte;   { Cyan }
    function ColLabelFg : byte;    { Cyan (dark) }
    function ColStatusFg : byte;   { Black }
    function ColStatusBg : byte;   { Cyan }
    function ColErrorFg : byte;    { White }
    function ColErrorBg : byte;    { Red }
    function ColInputFg : byte;    { White }
    function ColSectionFg : byte;  { Yellow — section labels }

    { Log service colors }
    function ColTelnetFg : byte;   { LightGreen }
    function ColBinkpFg : byte;    { LightMagenta }
    function ColFtpFg : byte;      { White }
    function ColHttpFg : byte;     { Cyan }
    function ColSmtpFg : byte;     { LightRed }

    { Drawing }
    procedure DrawFrame;
    procedure DrawTitle;
    procedure DrawProtoTab;
    procedure DrawConnections;
    procedure DrawNodeDetail;
    procedure DrawEvents;
    procedure DrawSystem;
    procedure DrawLastCallers;
    procedure DrawServerLog;
    procedure DrawStatusBar;

    { Helpers }
    function FormatUptime : string;
    function ActiveNodeCount : integer;
    function MaxNodes : integer;
    function ProtoName(P : TProtocolType) : string;
    function ProtoColor(P : TProtocolType) : byte;
    function FormatElapsed(Secs : integer) : string;
    function FormatIdle(Secs : integer) : string;
  public
    constructor Create(ACfg : TPcbisConfig; AServer : TPcbisServer;
                       AEvents : TPcbisEvents);

    procedure Draw;
    procedure Update;
    procedure AddLog(const Line : string; AColor : byte = 7);
    procedure AddCaller(const AUser : string; ANode : integer;
                        const AVia : string);
    procedure HandleKey(Key : char);
    function  ShouldQuit : boolean;
  end;

implementation

uses
  SysUtils, pcbis_log, pcbis_nodedata;

const
  { Box drawing chars — single line }
  BOX_H  = #196;  { - }
  BOX_V  = #179;  { | }
  BOX_TL = #218;
  BOX_TR = #191;
  BOX_BL = #192;
  BOX_BR = #217;
  BOX_LT = #195;
  BOX_RT = #180;
  BOX_TT = #194;
  BOX_BT = #193;
  BOX_CR = #197;

  { Double line }
  DBL_H  = #205;
  DBL_V  = #186;
  DBL_TL = #201;
  DBL_TR = #187;
  DBL_BL = #200;
  DBL_BR = #188;
  DBL_LT = #204;
  DBL_RT = #185;
  DBL_TT = #203;
  DBL_BT = #202;

  { Mixed: double horizontal, single vertical }
  MIX_LT = #199;  { single-V left tee on double-H }
  MIX_RT = #182;  { single-V right tee on double-H }

  TAB_NAMES : array[TWfcProtoTab] of string = (
    'Telnet', 'BinkP', 'FTP', 'HTTP', 'SMTP', 'Events'
  );

  { Section label chars }
  SEC_L  = #181;   { right-pointing tee for section label bracket }
  SEC_R  = #198;   { left-pointing tee }

  SCREEN_W = 80;
  SCREEN_H = 25;

  { Layout rows }
  ROW_TITLE       = 1;
  ROW_TABS        = 2;
  ROW_CONN_TOP    = 3;
  ROW_CONN_BOT    = 10;
  ROW_DIV1        = 11;   { divider: connections | last callers }
  ROW_CALLERS_TOP = 12;
  ROW_CALLERS_BOT = 15;
  ROW_DIV2        = 16;   { divider: callers | log }
  ROW_LOG_TOP     = 17;
  ROW_LOG_BOT     = 23;
  ROW_DIV3        = 24;   { divider: log | status bar }
  ROW_STATUS      = 25;

  { Column split: connections left, system right }
  COL_SPLIT       = 42;

var
  QuitFlag : boolean = False;

{ ======================================================================= }
{  Constructor                                                             }
{ ======================================================================= }

constructor TPcbisWfc.Create(ACfg : TPcbisConfig; AServer : TPcbisServer;
                             AEvents : TPcbisEvents);
begin
  inherited Create;
  FCfg := ACfg;
  FServer := AServer;
  FEvents := AEvents;
  FActiveTab := wtTelnet;
  FLogCount := 0;
  FCallerCnt := 0;
  FTotalCalls := 0;
  FStartTime := Now;
  FNeedRedraw := True;
  FSelectedNode := -1;
  FNodeCursor := 0;
  FillChar(FLogLines, SizeOf(FLogLines), 0);
  FillChar(FLogColors, SizeOf(FLogColors), 0);
  FillChar(FCallers, SizeOf(FCallers), 0);
end;

{ ======================================================================= }
{  Screen Primitives                                                       }
{ ======================================================================= }

procedure TPcbisWfc.GotoXY(X, Y : integer);
begin
  Write(#27'[', Y, ';', X, 'H');
end;

procedure TPcbisWfc.SetColor(Fg, Bg : byte);
var
  FgCode, BgCode : byte;
  Bright : boolean;
begin
  Bright := Fg > 7;
  FgCode := Fg and 7;
  BgCode := Bg and 7;
  if Bright then
    Write(#27'[1;', 30 + FgCode, ';', 40 + BgCode, 'm')
  else
    Write(#27'[0;', 30 + FgCode, ';', 40 + BgCode, 'm');
end;

procedure TPcbisWfc.ClearScreen;
begin
  Write(#27'[2J');
  GotoXY(1, 1);
end;

procedure TPcbisWfc.WriteAt(X, Y : integer; Fg, Bg : byte; const S : string);
begin
  GotoXY(X, Y);
  SetColor(Fg, Bg);
  Write(S);
end;

procedure TPcbisWfc.WritePadded(X, Y : integer; Fg, Bg : byte;
                                const S : string; Width : integer);
begin
  WriteAt(X, Y, Fg, Bg, Copy(S + StringOfChar(' ', Width), 1, Width));
end;

procedure TPcbisWfc.HLine(X, Y, Len : integer; C : char; Fg, Bg : byte);
var
  I : integer;
begin
  GotoXY(X, Y);
  SetColor(Fg, Bg);
  for I := 1 to Len do Write(C);
end;

procedure TPcbisWfc.VLine(X, Y, Len : integer; C : char; Fg, Bg : byte);
var
  I : integer;
begin
  SetColor(Fg, Bg);
  for I := 0 to Len - 1 do
  begin
    GotoXY(X, Y + I);
    Write(C);
  end;
end;

procedure TPcbisWfc.Box(X1, Y1, X2, Y2 : integer; Style : byte;
                        Fg, Bg : byte);
var
  I : integer;
  H, V, TL, TR, BL, BR : char;
begin
  if Style = 2 then
  begin
    H := DBL_H; V := DBL_V;
    TL := DBL_TL; TR := DBL_TR; BL := DBL_BL; BR := DBL_BR;
  end
  else
  begin
    H := BOX_H; V := BOX_V;
    TL := BOX_TL; TR := BOX_TR; BL := BOX_BL; BR := BOX_BR;
  end;

  SetColor(Fg, Bg);

  { Top }
  GotoXY(X1, Y1); Write(TL);
  for I := X1 + 1 to X2 - 1 do Write(H);
  Write(TR);

  { Sides }
  for I := Y1 + 1 to Y2 - 1 do
  begin
    GotoXY(X1, I); Write(V);
    GotoXY(X2, I); Write(V);
  end;

  { Bottom }
  GotoXY(X1, Y2); Write(BL);
  for I := X1 + 1 to X2 - 1 do Write(H);
  Write(BR);
end;

{ ======================================================================= }
{  PCBoard Colors                                                          }
{ ======================================================================= }

function TPcbisWfc.ColHeaderFg : byte;  begin Result := 15; end;  { White }
function TPcbisWfc.ColHeaderBg : byte;  begin Result := 1;  end;  { Blue }
function TPcbisWfc.ColHighFg : byte;    begin Result := 14; end;  { Yellow }
function TPcbisWfc.ColNormalFg : byte;  begin Result := 7;  end;  { LightGray }
function TPcbisWfc.ColNormalBg : byte;  begin Result := 0;  end;  { Black }
function TPcbisWfc.ColBorderFg : byte;  begin Result := 3;  end;  { Cyan }
function TPcbisWfc.ColLabelFg : byte;   begin Result := 11; end;  { LightCyan }
function TPcbisWfc.ColStatusFg : byte;  begin Result := 0;  end;  { Black }
function TPcbisWfc.ColStatusBg : byte;  begin Result := 3;  end;  { Cyan }
function TPcbisWfc.ColErrorFg : byte;   begin Result := 15; end;  { White }
function TPcbisWfc.ColErrorBg : byte;   begin Result := 4;  end;  { Red }
function TPcbisWfc.ColInputFg : byte;   begin Result := 15; end;  { White }
function TPcbisWfc.ColSectionFg : byte; begin Result := 14; end;  { Yellow — section labels }

{ Log service colors — timestamp colored by protocol }
function TPcbisWfc.ColTelnetFg : byte;  begin Result := 10; end;  { LightGreen }
function TPcbisWfc.ColBinkpFg : byte;   begin Result := 13; end;  { LightMagenta }
function TPcbisWfc.ColFtpFg : byte;     begin Result := 15; end;  { White }
function TPcbisWfc.ColHttpFg : byte;    begin Result := 3;  end;  { Cyan }
function TPcbisWfc.ColSmtpFg : byte;    begin Result := 12; end;  { LightRed }

{ ======================================================================= }
{  Helpers                                                                 }
{ ======================================================================= }

function TPcbisWfc.FormatUptime : string;
var
  Elapsed : TDateTime;
  Days, Hours, Mins : integer;
begin
  Elapsed := Now - FStartTime;
  Days := Trunc(Elapsed);
  Hours := Trunc(Frac(Elapsed) * 24);
  Mins := Trunc(Frac(Frac(Elapsed) * 24) * 60);
  Result := Format('%dd %dh %02dm', [Days, Hours, Mins]);
end;

function TPcbisWfc.ActiveNodeCount : integer;
begin
  Result := FServer.TelnetCount;
end;

function TPcbisWfc.MaxNodes : integer;
begin
  Result := FCfg.MaxConnections;
end;

{ ProtoName — short protocol label for display }
function TPcbisWfc.ProtoName(P : TProtocolType) : string;
begin
  case P of
    ptTelnet:    Result := 'Telnet';
    ptBinkp:     Result := 'BinkP';
    ptFtp:       Result := 'FTP';
    ptHttp:      Result := 'HTTP';
    ptSmtpQueue: Result := 'SMTP';
  else
    Result := '???';
  end;
end;

{ ProtoColor — service color for timestamps and via labels }
function TPcbisWfc.ProtoColor(P : TProtocolType) : byte;
begin
  case P of
    ptTelnet:    Result := ColTelnetFg;
    ptBinkp:     Result := ColBinkpFg;
    ptFtp:       Result := ColFtpFg;
    ptHttp:      Result := ColHttpFg;
    ptSmtpQueue: Result := ColSmtpFg;
  else
    Result := ColNormalFg;
  end;
end;

{ FormatElapsed — "H:MM:SS" from seconds }
function TPcbisWfc.FormatElapsed(Secs : integer) : string;
begin
  Result := Format('%d:%2.2d:%2.2d',
            [Secs div 3600, (Secs div 60) mod 60, Secs mod 60]);
end;

{ FormatIdle — "M:SS" from seconds }
function TPcbisWfc.FormatIdle(Secs : integer) : string;
begin
  Result := Format('%d:%2.2d', [Secs div 60, Secs mod 60]);
end;

{ ======================================================================= }
{  Frame                                                                   }
{ ======================================================================= }

procedure TPcbisWfc.DrawFrame;
begin
  ClearScreen;

  { Outer frame — double line }
  Box(1, 1, 80, 25, 2, ColBorderFg, ColNormalBg);

  { === Row 11: divider connections/callers === }
  GotoXY(1, ROW_DIV1); SetColor(ColBorderFg, ColNormalBg);
  Write(DBL_LT);
  HLine(2, ROW_DIV1, 78, DBL_H, ColBorderFg, ColNormalBg);
  GotoXY(80, ROW_DIV1); Write(DBL_RT);

  { === Row 16: divider callers/log === }
  GotoXY(1, ROW_DIV2); SetColor(ColBorderFg, ColNormalBg);
  Write(DBL_LT);
  HLine(2, ROW_DIV2, 78, DBL_H, ColBorderFg, ColNormalBg);
  GotoXY(80, ROW_DIV2); Write(DBL_RT);

  { === Row 24: divider log/status === }
  GotoXY(1, ROW_DIV3); SetColor(ColBorderFg, ColNormalBg);
  Write(DBL_LT);
  HLine(2, ROW_DIV3, 78, DBL_H, ColBorderFg, ColNormalBg);
  GotoXY(80, ROW_DIV3); Write(DBL_RT);

  { === Vertical split: col 42, rows 3-10 (connections | system) === }
  VLine(COL_SPLIT, ROW_CONN_TOP, ROW_CONN_BOT - ROW_CONN_TOP + 1,
        DBL_V, ColBorderFg, ColNormalBg);
  { Top tee }
  GotoXY(COL_SPLIT, ROW_TABS); SetColor(ColBorderFg, ColNormalBg);
  { Bottom tee at divider }
  GotoXY(COL_SPLIT, ROW_DIV1); Write(DBL_BT);
end;

{ ======================================================================= }
{  Title bar — "PCBIS v0.1.0:  BBS Name      Day DD Mon YYYY HH:MM:SS x/y nodes" }
{ ======================================================================= }

procedure TPcbisWfc.DrawTitle;
var
  Left, Right : string;
  Gap : integer;
begin
  Left := ' PCBIS v0.1.0:  ' + FCfg.BBSName;
  Right := FormatDateTime('ddd dd mmm yyyy hh:nn:ss', Now) +
           Format(' %d/%d nodes ', [ActiveNodeCount, MaxNodes]);

  { Left-justify Left, right-justify Right, fill gap with spaces }
  Gap := 78 - Length(Left) - Length(Right);

  GotoXY(2, ROW_TITLE);
  SetColor(ColHeaderFg, ColHeaderBg);
  Write(Left);
  if Gap > 0 then
    Write(StringOfChar(' ', Gap));
  Write(Right);
end;

{ ======================================================================= }
{  Protocol tabs                                                           }
{ ======================================================================= }

procedure TPcbisWfc.DrawProtoTab;
var
  T   : TWfcProtoTab;
  Col : integer;
  Lab : string;
begin
  Col := 3;
  for T := Low(TWfcProtoTab) to High(TWfcProtoTab) do
  begin
    Lab := ' ' + TAB_NAMES[T] + ' ';
    if T = FActiveTab then
      WriteAt(Col, ROW_TABS, ColHeaderFg, ColHeaderBg, Lab)
    else
      WriteAt(Col, ROW_TABS, ColNormalFg, ColNormalBg, Lab);
    Inc(Col, Length(Lab));
  end;
  { Clear to the vertical split }
  if Col < COL_SPLIT then
    WritePadded(Col, ROW_TABS, ColNormalFg, ColNormalBg, '',
                COL_SPLIT - Col);
end;

{ ======================================================================= }
{  Connections panel (left, rows 3-10)                                     }
{ ======================================================================= }

procedure TPcbisWfc.DrawConnections;
var
  Y, I, Row  : integer;
  Conn       : TPcbisConnection;
  Line       : string;
  Fg, Bg     : byte;
  Proto      : TProtocolType;
  Count      : integer;
  PW         : integer;   { panel width }
begin
  PW := COL_SPLIT - 4;

  { If a node is selected, show detail view instead }
  if FSelectedNode >= 0 then
  begin
    DrawNodeDetail;
    Exit;
  end;

  { Events tab gets its own draw routine }
  if FActiveTab = wtEvents then
  begin
    DrawEvents;
    Exit;
  end;

  { Section label — yellow, shows protocol name }
  case FActiveTab of
    wtTelnet: WriteAt(2, ROW_CONN_TOP, ColSectionFg, ColNormalBg, 'Telnet     ');
    wtBinkp:  WriteAt(2, ROW_CONN_TOP, ColSectionFg, ColNormalBg, 'BinkP      ');
    wtFtp:    WriteAt(2, ROW_CONN_TOP, ColSectionFg, ColNormalBg, 'FTP        ');
    wtHttp:   WriteAt(2, ROW_CONN_TOP, ColSectionFg, ColNormalBg, 'HTTP       ');
    wtSmtp:   WriteAt(2, ROW_CONN_TOP, ColSectionFg, ColNormalBg, 'SMTP       ');
  end;

  { Column header — light cyan, per-protocol layout }
  case FActiveTab of
    wtTelnet:
      WritePadded(3, ROW_CONN_TOP + 1, ColLabelFg, ColNormalBg,
                  'Node User          Time   Idle  Via', PW);
    wtBinkp:
      WritePadded(3, ROW_CONN_TOP + 1, ColLabelFg, ColNormalBg,
                  'Address            Time   Idle  Status', PW);
    wtFtp:
      WritePadded(3, ROW_CONN_TOP + 1, ColLabelFg, ColNormalBg,
                  'User          IP              Time  ', PW);
    wtHttp:
      WritePadded(3, ROW_CONN_TOP + 1, ColLabelFg, ColNormalBg,
                  'IP              Request          Code', PW);
    wtSmtp:
      WritePadded(3, ROW_CONN_TOP + 1, ColLabelFg, ColNormalBg,
                  'Queue         Recipient        Status', PW);
  end;

  { Determine which protocol to filter }
  case FActiveTab of
    wtTelnet: Proto := ptTelnet;
    wtBinkp:  Proto := ptBinkp;
    wtFtp:    Proto := ptFtp;
    wtHttp:   Proto := ptHttp;
    wtSmtp:   Proto := ptSmtpQueue;
  else
    Proto := ptTelnet;
  end;

  { Walk server connections, show matching protocol }
  Row := 0;
  Count := FServer.ConnectionCount;
  for I := 0 to Count - 1 do
  begin
    Conn := FServer.GetConnection(I);
    if Conn = nil then Continue;
    if Conn.Protocol <> Proto then Continue;

    Y := ROW_CONN_TOP + 2 + Row;
    if Y > ROW_CONN_BOT then Break;

    { Build line — fixed-width fields, per-protocol columns }
    case FActiveTab of
      wtTelnet:
        Line := Format('%4d %-14s %7s %5s  Tel',
                [Conn.NodeNumber,
                 Copy(Conn.RemoteIP, 1, 14),
                 FormatElapsed(Conn.Age),
                 FormatIdle(Conn.Idle)]);
      wtBinkp:
        Line := Format('%-18s %7s %5s  conn',
                [Copy(Conn.RemoteIP, 1, 18),
                 FormatElapsed(Conn.Age),
                 FormatIdle(Conn.Idle)]);
      wtFtp:
        Line := Format('%-13s %-15s %7s',
                ['anonymous',
                 Copy(Conn.RemoteIP, 1, 15),
                 FormatElapsed(Conn.Age)]);
      wtHttp:
        Line := Format('%-15s %-16s %4s',
                [Copy(Conn.RemoteIP, 1, 15),
                 'GET /',
                 '200']);
      wtSmtp:
        Line := Format('%-13s %-16s %s',
                ['outbound',
                 '',
                 'queued']);
    else
      Line := Copy(Conn.RemoteIP, 1, PW);
    end;

    { Highlight if cursor is on this row }
    if Row = FNodeCursor then
    begin
      Fg := 0;  { Black on Cyan = highlight bar }
      Bg := 3;
    end
    else
    begin
      Fg := ColNormalFg;
      Bg := ColNormalBg;
    end;

    WritePadded(3, Y, Fg, Bg, Line, PW);
    Inc(Row);
  end;

  { Fill remaining rows — idle message per protocol }
  for Y := ROW_CONN_TOP + 2 + Row to ROW_CONN_BOT do
  begin
    if (Row = 0) and (Y = ROW_CONN_TOP + 2) then
    begin
      case FActiveTab of
        wtTelnet: WritePadded(3, Y, ColNormalFg, ColNormalBg,
                              '(waiting for call)', PW);
        wtBinkp:  WritePadded(3, Y, ColNormalFg, ColNormalBg,
                              '(no sessions)', PW);
        wtFtp:    WritePadded(3, Y, ColNormalFg, ColNormalBg,
                              '(no transfers)', PW);
        wtHttp:   WritePadded(3, Y, ColNormalFg, ColNormalBg,
                              '(no requests)', PW);
        wtSmtp:   WritePadded(3, Y, ColNormalFg, ColNormalBg,
                              '(queue empty)', PW);
      else
        WritePadded(3, Y, ColNormalFg, ColNormalBg, '', PW);
      end;
    end
    else
      WritePadded(3, Y, ColNormalFg, ColNormalBg, '', PW);
  end;
end;

{ ======================================================================= }
{  Node Detail (left panel, rows 3-10) — selected node's full info        }
{  10 fields: User, Via, TimeOn+Idle, Speed+Sec, IP, Action, City, Calls  }
{ ======================================================================= }

procedure TPcbisWfc.DrawNodeDetail;
var
  Conn : TPcbisConnection;
  PW   : integer;
  Info : TPcbNodeInfo;
begin
  PW := COL_SPLIT - 4;

  { Section label }
  WritePadded(2, ROW_CONN_TOP, ColSectionFg, ColNormalBg,
              Format('Node %d Detail', [FSelectedNode]), PW + 2);

  Conn := FServer.FindNode(FSelectedNode);

  if Conn = nil then
  begin
    { Node went away — clear and reset }
    WritePadded(3, ROW_CONN_TOP + 1, ColNormalFg, ColNormalBg,
                '(node disconnected)', PW);
    WritePadded(3, ROW_CONN_TOP + 2, ColNormalFg, ColNormalBg, '', PW);
    WritePadded(3, ROW_CONN_TOP + 3, ColNormalFg, ColNormalBg, '', PW);
    WritePadded(3, ROW_CONN_TOP + 4, ColNormalFg, ColNormalBg, '', PW);
    WritePadded(3, ROW_CONN_TOP + 5, ColNormalFg, ColNormalBg, '', PW);
    WritePadded(3, ROW_CONN_TOP + 6, ColNormalFg, ColNormalBg, '', PW);
    WritePadded(3, ROW_CONN_TOP + 7, ColNormalFg, ColNormalBg, '', PW);
    Exit;
  end;

  { Row 1: User }
  WriteAt(3, ROW_CONN_TOP + 1, ColLabelFg, ColNormalBg, '     User: ');
  WritePadded(14, ROW_CONN_TOP + 1, ColInputFg, ColNormalBg,
              Conn.RemoteIP, PW - 11);  { TODO: real username from nodedata }

  { Row 2: Via — colored by service }
  WriteAt(3, ROW_CONN_TOP + 2, ColLabelFg, ColNormalBg, '      Via: ');
  WritePadded(14, ROW_CONN_TOP + 2, ProtoColor(Conn.Protocol), ColNormalBg,
              ProtoName(Conn.Protocol), PW - 11);

  { Row 3: Time On + Idle }
  WriteAt(3, ROW_CONN_TOP + 3, ColLabelFg, ColNormalBg, '  Time On: ');
  WriteAt(14, ROW_CONN_TOP + 3, ColInputFg, ColNormalBg,
          FormatElapsed(Conn.Age));
  WriteAt(24, ROW_CONN_TOP + 3, ColLabelFg, ColNormalBg, '  Idle: ');
  WritePadded(32, ROW_CONN_TOP + 3, ColInputFg, ColNormalBg,
              FormatIdle(Conn.Idle), PW - 29);

  { Row 4: Speed + Security — speed shows protocol name for TCP connections }
  WriteAt(3, ROW_CONN_TOP + 4, ColLabelFg, ColNormalBg, '    Speed: ');
  WriteAt(14, ROW_CONN_TOP + 4, ColInputFg, ColNormalBg,
          ProtoName(Conn.Protocol));
  WriteAt(24, ROW_CONN_TOP + 4, ColLabelFg, ColNormalBg, '   Sec: ');
  WritePadded(32, ROW_CONN_TOP + 4, ColInputFg, ColNormalBg,
              '0', PW - 29);  { TODO: real security level from nodedata }

  { Row 5: IP address }
  WriteAt(3, ROW_CONN_TOP + 5, ColLabelFg, ColNormalBg, '       IP: ');
  WritePadded(14, ROW_CONN_TOP + 5, ColInputFg, ColNormalBg,
              Conn.RemoteIP, PW - 11);

  { Row 6: Current action }
  WriteAt(3, ROW_CONN_TOP + 6, ColLabelFg, ColNormalBg, '   Action: ');
  WritePadded(14, ROW_CONN_TOP + 6, ColHighFg, ColNormalBg,
              'Connected', PW - 11);  { TODO: real activity from nodedata }

  { Row 7: City }
  WriteAt(3, ROW_CONN_TOP + 7, ColLabelFg, ColNormalBg, '     City: ');
  WritePadded(14, ROW_CONN_TOP + 7, ColInputFg, ColNormalBg,
              '', PW - 11);  { TODO: real city from USERS record }
end;

{ ======================================================================= }
{  Events tab — show all 6 batch slots + built-in internet events          }
{ ======================================================================= }

procedure TPcbisWfc.DrawEvents;
var
  Y, I  : integer;
  PW    : integer;
  Line  : string;
  SlotY : integer;
begin
  PW := COL_SPLIT - 4;

  { Section label }
  WriteAt(2, ROW_CONN_TOP, ColSectionFg, ColNormalBg, 'Events');

  { Header }
  WritePadded(3, ROW_CONN_TOP + 1, ColLabelFg, ColNormalBg,
              'Slot Time  Days    Description', PW);

  { Batch event slots 1-6 }
  SlotY := 0;
  for I := 1 to MAX_EVENT_SLOTS do
  begin
    Y := ROW_CONN_TOP + 2 + SlotY;
    if Y > ROW_CONN_BOT then Break;

    if FEvents.GetSlotEnabled(I) then
    begin
      if FEvents.GetSlotRunning(I) then
      begin
        Line := Format(' %d   %s  %-7s %s',
                [I, FEvents.GetSlotTime(I), 'MTWTFSS',
                 'RUNNING']);
        WritePadded(3, Y, 10, ColNormalBg, Line, PW);  { bright green }
      end
      else
      begin
        Line := Format(' %d   %s  %-7s %s',
                [I, FEvents.GetSlotTime(I), 'MTWTFSS',
                 FEvents.GetSlotDesc(I)]);
        WritePadded(3, Y, ColNormalFg, ColNormalBg, Line, PW);
      end;
    end
    else
    begin
      Line := Format(' %d   --:--  ------- (not configured)', [I]);
      WritePadded(3, Y, 8, ColNormalBg, Line, PW);  { dark gray }
    end;
    Inc(SlotY);
  end;

  { Built-in events on remaining rows }
  for I := 0 to FEvents.BuiltinCount - 1 do
  begin
    Y := ROW_CONN_TOP + 2 + SlotY;
    if Y > ROW_CONN_BOT then Break;

    if FEvents.GetBuiltinEnabled(I) then
      Line := Format(' *   %-5s  auto    %s',
              [FEvents.GetBuiltinInterval(I),
               FEvents.GetBuiltinDesc(I)])
    else
      Line := Format(' *   off    auto    %s',
              [FEvents.GetBuiltinDesc(I)]);

    WritePadded(3, Y, ColNormalFg, ColNormalBg, Line, PW);
    Inc(SlotY);
  end;

  { Clear remaining }
  for Y := ROW_CONN_TOP + 2 + SlotY to ROW_CONN_BOT do
    WritePadded(3, Y, ColNormalFg, ColNormalBg, '', PW);
end;

{ ======================================================================= }
{  System panel (right, rows 3-10) — always visible regardless of tab      }
{ ======================================================================= }

procedure TPcbisWfc.DrawSystem;
var
  PortLine : string;
begin
  { Section label — yellow }
  WriteAt(COL_SPLIT + 1, ROW_CONN_TOP, ColSectionFg, ColNormalBg,
          'System');

  { Uptime }
  WriteAt(COL_SPLIT + 2, ROW_CONN_TOP + 1, ColLabelFg, ColNormalBg,
          '  Uptime: ');
  WritePadded(COL_SPLIT + 12, ROW_CONN_TOP + 1, ColInputFg, ColNormalBg,
              FormatUptime, 24);

  { Total calls }
  WriteAt(COL_SPLIT + 2, ROW_CONN_TOP + 2, ColLabelFg, ColNormalBg,
          '   Calls: ');
  WritePadded(COL_SPLIT + 12, ROW_CONN_TOP + 2, ColInputFg, ColNormalBg,
              IntToStr(FTotalCalls), 24);

  { Users online }
  WriteAt(COL_SPLIT + 2, ROW_CONN_TOP + 3, ColLabelFg, ColNormalBg,
          '   Users: ');
  WritePadded(COL_SPLIT + 12, ROW_CONN_TOP + 3, ColHighFg, ColNormalBg,
              IntToStr(ActiveNodeCount), 24);

  { Protocol counts — live from server }
  WriteAt(COL_SPLIT + 2, ROW_CONN_TOP + 4, ColLabelFg, ColNormalBg,
          '  Telnet: ');
  WritePadded(COL_SPLIT + 12, ROW_CONN_TOP + 4, ColNormalFg, ColNormalBg,
              Format('%d/%d  port %d',
              [FServer.TelnetCount, FCfg.TelnetMaxNodes, FCfg.TelnetPort]), 24);

  WriteAt(COL_SPLIT + 2, ROW_CONN_TOP + 5, ColLabelFg, ColNormalBg,
          '   BinkP: ');
  WritePadded(COL_SPLIT + 12, ROW_CONN_TOP + 5, ColNormalFg, ColNormalBg,
              Format('%d  port %d',
              [FServer.BinkpCount, FCfg.BinkpPort]), 24);

  WriteAt(COL_SPLIT + 2, ROW_CONN_TOP + 6, ColLabelFg, ColNormalBg,
          '     FTP: ');
  if FCfg.FtpEnabled then
    WritePadded(COL_SPLIT + 12, ROW_CONN_TOP + 6, ColNormalFg, ColNormalBg,
                Format('%d  port 21', [FServer.FtpCount]), 24)
  else
    WritePadded(COL_SPLIT + 12, ROW_CONN_TOP + 6, 8, ColNormalBg,
                'off', 24);

  { Next event }
  WriteAt(COL_SPLIT + 2, ROW_CONN_TOP + 7, ColLabelFg, ColNormalBg,
          '   Event: ');
  WritePadded(COL_SPLIT + 12, ROW_CONN_TOP + 7, ColNormalFg, ColNormalBg,
              FEvents.NextEventDesc, 24);
end;

{ ======================================================================= }
{  Last Callers (rows 12-15)                                               }
{ ======================================================================= }

procedure TPcbisWfc.DrawLastCallers;
var
  I, Y : integer;
  Line : string;
begin
  { Section label — yellow }
  WriteAt(2, ROW_CALLERS_TOP, ColSectionFg, ColNormalBg,
          'Last Callers');

  { Header — light cyan }
  WritePadded(3, ROW_CALLERS_TOP + 1, ColLabelFg, ColNormalBg,
              'User                      Node      On         Off       Via', 76);

  { Caller rows }
  for I := 0 to 2 do
  begin
    Y := ROW_CALLERS_TOP + 2 + I;
    if I < FCallerCnt then
    begin
      Line := Format('%-25s %2d    %-8s %-8s %-8s',
                [FCallers[I].UserName, FCallers[I].Node,
                 FCallers[I].TimeOn, FCallers[I].TimeOff,
                 FCallers[I].Via]);
      WritePadded(3, Y, ColNormalFg, ColNormalBg, Line, 76);
    end
    else
      WritePadded(3, Y, 8, ColNormalBg,
                  '(none yet)', 76);
  end;
end;

{ ======================================================================= }
{  Server log (rows 17-23)                                                 }
{ ======================================================================= }

procedure TPcbisWfc.DrawServerLog;
var
  I, Y : integer;
begin
  { Section label — yellow }
  WriteAt(2, ROW_LOG_TOP, ColSectionFg, ColNormalBg, 'Log');

  for I := 0 to 6 do
  begin
    Y := ROW_LOG_TOP + 1 + I;
    if Y > ROW_LOG_BOT then Break;
    if I < FLogCount then
    begin
      { Timestamp (first 9 chars "HH:MM:SS ") colored by service }
      WriteAt(3, Y, FLogColors[I], ColNormalBg,
              Copy(FLogLines[I], 1, 9));
      { Rest of line in normal gray }
      WritePadded(12, Y, ColNormalFg, ColNormalBg,
                  Copy(FLogLines[I], 10, Length(FLogLines[I]) - 9), 67);
    end
    else
      WritePadded(3, Y, ColNormalFg, ColNormalBg, '', 76);
  end;
end;

{ ======================================================================= }
{  Status bar                                                              }
{ ======================================================================= }

procedure TPcbisWfc.DrawStatusBar;
begin
  if FSelectedNode >= 0 then
    WriteAt(2, ROW_STATUS, ColStatusFg, ColStatusBg,
      ' ESC Back  SPACE Local  ALT-K Kill  Q Quit' +
      '                                   ')
  else
    WriteAt(2, ROW_STATUS, ColStatusFg, ColStatusBg,
      ' TAB Switch  ' + #24#25 + ' Select  ENTER Detail  SPACE Local  Q Quit' +
      '                    ');
end;

{ ======================================================================= }
{  Full draw / partial update                                              }
{ ======================================================================= }

procedure TPcbisWfc.Draw;
begin
  DrawFrame;
  DrawTitle;
  DrawProtoTab;
  DrawConnections;
  DrawSystem;
  DrawLastCallers;
  DrawServerLog;
  DrawStatusBar;
  FNeedRedraw := False;
end;

procedure TPcbisWfc.Update;
begin
  if FNeedRedraw then
    Draw
  else
  begin
    DrawTitle;         { refresh date/time + node count }
    DrawProtoTab;
    DrawConnections;
    DrawSystem;        { refresh uptime + calls + users }
    DrawLastCallers;
    DrawServerLog;
  end;
end;

{ ======================================================================= }
{  Log + caller ring buffers                                               }
{ ======================================================================= }

procedure TPcbisWfc.AddLog(const Line : string; AColor : byte = 7);
var
  I : integer;
  Stamped : string;
begin
  Stamped := FormatDateTime('hh:nn:ss', Now) + ' ' + Line;

  if FLogCount >= 7 then
  begin
    { Scroll up — shift both lines and colors }
    for I := 0 to 5 do
    begin
      FLogLines[I]  := FLogLines[I + 1];
      FLogColors[I] := FLogColors[I + 1];
    end;
    FLogLines[6]  := Stamped;
    FLogColors[6] := AColor;
  end
  else
  begin
    FLogLines[FLogCount]  := Stamped;
    FLogColors[FLogCount] := AColor;
    Inc(FLogCount);
  end;
end;

procedure TPcbisWfc.AddCaller(const AUser : string; ANode : integer;
                              const AVia : string);
var
  I : integer;
begin
  Inc(FTotalCalls);

  { Shift down — newest on top }
  for I := 2 downto 1 do
    FCallers[I] := FCallers[I - 1];

  FCallers[0].UserName := AUser;
  FCallers[0].Node := ANode;
  FCallers[0].TimeOn := FormatDateTime('hh:nn:ss', Now);
  FCallers[0].TimeOff := '';
  FCallers[0].Via := AVia;

  if FCallerCnt < 3 then
    Inc(FCallerCnt);
end;

{ ======================================================================= }
{  Keyboard                                                                }
{ ======================================================================= }

procedure TPcbisWfc.HandleKey(Key : char);
begin
  case Key of
    #9: { TAB — next protocol tab }
      begin
        if FActiveTab = High(TWfcProtoTab) then
          FActiveTab := Low(TWfcProtoTab)
        else
          Inc(FActiveTab);
        FSelectedNode := -1;    { back to list view on tab change }
        FNodeCursor := 0;
        FNeedRedraw := True;
      end;

    'q', 'Q': { Q — quit }
      QuitFlag := True;

    #27: { ESC — back to connection list if in detail, else quit }
      begin
        if FSelectedNode >= 0 then
        begin
          FSelectedNode := -1;
          FNeedRedraw := True;
        end
        else
          QuitFlag := True;
      end;

    ' ': { SPACE — local login }
      begin
        AddLog('Local login requested');
        { TODO: spawn local PCBoard session }
      end;

    #13: { ENTER — select node at cursor for detail view }
      begin
        if (FSelectedNode < 0) and (FActiveTab <> wtEvents) then
        begin
          { Find which connection the cursor is on }
          if FNodeCursor < FServer.ConnectionCount then
          begin
            FSelectedNode := FServer.GetConnection(FNodeCursor).NodeNumber;
            FNeedRedraw := True;
          end;
        end;
      end;

    { Arrow keys arrive as ESC [ A/B sequences — we get just A/B here
      after the caller strips the ESC [ prefix }
    'A': { UP arrow — move cursor up }
      begin
        if FSelectedNode < 0 then  { only in list view }
        begin
          if FNodeCursor > 0 then
            Dec(FNodeCursor);
        end;
      end;

    'B': { DOWN arrow — move cursor down }
      begin
        if FSelectedNode < 0 then
        begin
          if FNodeCursor < FServer.ConnectionCount - 1 then
            Inc(FNodeCursor);
        end;
      end;
  end;
end;

function TPcbisWfc.ShouldQuit : boolean;
begin
  Result := QuitFlag;
end;

end.
