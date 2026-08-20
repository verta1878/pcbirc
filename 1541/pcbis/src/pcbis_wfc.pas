{ ===========================================================================
  pcbis_wfc.pas — Waiting For Caller console UI
  ANSI text UI following PCBoard's color scheme.
  Pattern from MIS (mis_ansiwfc.pas) in mystic-bbs-irc.
  No LCL — raw ANSI escape codes only.
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

  TPcbisWfc = class
  private
    FCfg        : TPcbisConfig;
    FServer     : TPcbisServer;
    FEvents     : TPcbisEvents;
    FActiveTab  : TWfcProtoTab;
    FLogLines   : array[0..19] of string;
    FLogCount   : integer;
    FNeedRedraw : boolean;

    { Screen primitives }
    procedure GotoXY(X, Y : integer);
    procedure SetColor(Fg, Bg : byte);
    procedure ClearScreen;
    procedure WriteAt(X, Y : integer; Fg, Bg : byte; const S : string);
    procedure HLine(X, Y, Len : integer; C : char; Fg, Bg : byte);
    procedure VLine(X, Y, Len : integer; C : char; Fg, Bg : byte);
    procedure Box(X1, Y1, X2, Y2 : integer; Style : byte; Fg, Bg : byte);

    { PCBoard color constants }
    function ColHeaderFg : byte;   { White }
    function ColHeaderBg : byte;   { Blue }
    function ColHighFg : byte;     { Yellow }
    function ColNormalFg : byte;   { LightGray }
    function ColNormalBg : byte;   { Black }
    function ColBorderFg : byte;   { Cyan }
    function ColStatusFg : byte;   { Black }
    function ColStatusBg : byte;   { Cyan }
    function ColErrorFg : byte;    { White }
    function ColErrorBg : byte;    { Red }
    function ColInputFg : byte;    { White }

    { Drawing }
    procedure DrawFrame;
    procedure DrawTitle;
    procedure DrawConnections;
    procedure DrawStats;
    procedure DrawServerLog;
    procedure DrawStatusBar;
    procedure DrawProtoTab;
  public
    constructor Create(ACfg : TPcbisConfig; AServer : TPcbisServer;
                       AEvents : TPcbisEvents);

    procedure Draw;
    procedure Update;
    procedure AddLog(const Line : string);
    procedure HandleKey(Key : char);
    function  ShouldQuit : boolean;
  end;

implementation

uses
  SysUtils, pcbis_log;

const
  { Box drawing chars — single line }
  BOX_H  = #196;  { ─ }
  BOX_V  = #179;  { │ }
  BOX_TL = #218;  { ┌ }
  BOX_TR = #191;  { ┐ }
  BOX_BL = #192;  { └ }
  BOX_BR = #217;  { ┘ }
  BOX_LT = #195;  { ├ }
  BOX_RT = #180;  { ┤ }
  BOX_TT = #194;  { ┬ }
  BOX_BT = #193;  { ┴ }
  BOX_CR = #197;  { ┼ }

  { Double line }
  DBL_H  = #205;  { ═ }
  DBL_V  = #186;  { ║ }
  DBL_TL = #201;  { ╔ }
  DBL_TR = #187;  { ╗ }
  DBL_BL = #200;  { ╚ }
  DBL_BR = #188;  { ╝ }
  DBL_LT = #204;  { ╠ }
  DBL_RT = #185;  { ╣ }
  DBL_TT = #203;  { ╦ }
  DBL_BT = #202;  { ╩ }

  TAB_NAMES : array[TWfcProtoTab] of string = (
    'Telnet', 'BinkP', 'FTP', 'HTTP', 'SMTP', 'Events'
  );

  SCREEN_W = 80;
  SCREEN_H = 25;

var
  QuitFlag : boolean = False;

constructor TPcbisWfc.Create(ACfg : TPcbisConfig; AServer : TPcbisServer;
                             AEvents : TPcbisEvents);
begin
  inherited Create;
  FCfg := ACfg;
  FServer := AServer;
  FEvents := AEvents;
  FActiveTab := wtTelnet;
  FLogCount := 0;
  FNeedRedraw := True;
  FillChar(FLogLines, SizeOf(FLogLines), 0);
end;

{ === Screen Primitives === }

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

procedure TPcbisWfc.Box(X1, Y1, X2, Y2 : integer; Style : byte; Fg, Bg : byte);
var
  I : integer;
  H, V, TL, TR, BL, BR : char;
begin
  if Style = 2 then
  begin
    H := DBL_H; V := DBL_V; TL := DBL_TL; TR := DBL_TR; BL := DBL_BL; BR := DBL_BR;
  end
  else
  begin
    H := BOX_H; V := BOX_V; TL := BOX_TL; TR := BOX_TR; BL := BOX_BL; BR := BOX_BR;
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

{ === PCBoard Colors === }
function TPcbisWfc.ColHeaderFg : byte; begin Result := 15; end;  { White }
function TPcbisWfc.ColHeaderBg : byte; begin Result := 1; end;   { Blue }
function TPcbisWfc.ColHighFg : byte; begin Result := 14; end;    { Yellow }
function TPcbisWfc.ColNormalFg : byte; begin Result := 7; end;   { LightGray }
function TPcbisWfc.ColNormalBg : byte; begin Result := 0; end;   { Black }
function TPcbisWfc.ColBorderFg : byte; begin Result := 3; end;   { Cyan }
function TPcbisWfc.ColStatusFg : byte; begin Result := 0; end;   { Black }
function TPcbisWfc.ColStatusBg : byte; begin Result := 3; end;   { Cyan }
function TPcbisWfc.ColErrorFg : byte; begin Result := 15; end;   { White }
function TPcbisWfc.ColErrorBg : byte; begin Result := 4; end;    { Red }
function TPcbisWfc.ColInputFg : byte; begin Result := 15; end;   { White }

{ === Drawing === }

procedure TPcbisWfc.DrawFrame;
begin
  ClearScreen;

  { Outer frame — double line }
  Box(1, 1, 80, 25, 2, ColBorderFg, ColNormalBg);

  { Title bar }
  HLine(2, 1, 78, DBL_H, ColBorderFg, ColNormalBg);

  { Divider after connections (row 11) }
  GotoXY(1, 11); SetColor(ColBorderFg, ColNormalBg);
  Write(DBL_LT);
  HLine(2, 11, 78, DBL_H, ColBorderFg, ColNormalBg);
  GotoXY(80, 11); Write(DBL_RT);

  { Divider for stats (col 31) }
  VLine(31, 2, 9, DBL_V, ColBorderFg, ColNormalBg);
  GotoXY(31, 1); Write(DBL_TT);
  GotoXY(31, 11); Write(DBL_BT);

  { Status bar divider (row 24) }
  GotoXY(1, 24); SetColor(ColBorderFg, ColNormalBg);
  Write(DBL_LT);
  HLine(2, 24, 78, DBL_H, ColBorderFg, ColNormalBg);
  GotoXY(80, 24); Write(DBL_RT);
end;

procedure TPcbisWfc.DrawTitle;
begin
  WriteAt(3, 1, ColHeaderFg, ColHeaderBg,
    ' PCBoard Internet Services v0.1.0 — 15.4 Revival ');
end;

procedure TPcbisWfc.DrawProtoTab;
var
  T   : TWfcProtoTab;
  X   : integer;
begin
  X := 3;
  GotoXY(X, 2);
  for T := Low(TWfcProtoTab) to High(TWfcProtoTab) do
  begin
    if T = FActiveTab then
    begin
      SetColor(ColHeaderFg, ColHeaderBg);
      Write(' ', TAB_NAMES[T], ' ');
    end
    else
    begin
      SetColor(ColNormalFg, ColNormalBg);
      Write(' ', TAB_NAMES[T], ' ');
    end;
    Inc(X, Length(TAB_NAMES[T]) + 2);
  end;
end;

procedure TPcbisWfc.DrawConnections;
var
  Y : integer;
begin
  WriteAt(3, 3, ColHighFg, ColNormalBg, 'Connections');

  { Clear the connection area }
  for Y := 4 to 10 do
    WriteAt(3, Y, ColNormalFg, ColNormalBg, StringOfChar(' ', 27));

  { TODO: list active connections for the selected protocol tab
    Format: "Node  User          Time   Idle" for telnet
            "Address       File         Bytes" for binkp
            "User          File         Speed" for ftp
            "IP            Request      Code" for http }

  case FActiveTab of
    wtTelnet:
      begin
        WriteAt(3, 4, ColNormalFg, ColNormalBg, 'Node User          Time');
        { TODO: iterate telnet connections }
        WriteAt(3, 5, 8, ColNormalBg, '(no connections)');
      end;
    wtBinkp:
      begin
        WriteAt(3, 4, ColNormalFg, ColNormalBg, 'Address       Status');
        WriteAt(3, 5, 8, ColNormalBg, '(no sessions)');
      end;
    wtFtp:
      begin
        WriteAt(3, 4, ColNormalFg, ColNormalBg, 'User          Activity');
        WriteAt(3, 5, 8, ColNormalBg, '(no connections)');
      end;
    wtHttp:
      begin
        WriteAt(3, 4, ColNormalFg, ColNormalBg, 'IP            Request');
        WriteAt(3, 5, 8, ColNormalBg, '(idle)');
      end;
    wtSmtp:
      begin
        WriteAt(3, 4, ColNormalFg, ColNormalBg, 'Queue         Status');
        WriteAt(3, 5, 8, ColNormalBg, '(empty)');
      end;
    wtEvents:
      begin
        WriteAt(3, 4, ColNormalFg, ColNormalBg, 'Slot Time  Description');
        { Show event slots from FEvents }
        for Y := 1 to 6 do
          WriteAt(3, 4 + Y, 8, ColNormalBg,
            FEvents.GetSlotDesc(Y));
      end;
  end;
end;

procedure TPcbisWfc.DrawStats;
var
  Port, Active, Total : integer;
  ProtoName : string;
begin
  WriteAt(33, 3, ColHighFg, ColNormalBg, 'Statistics');

  case FActiveTab of
    wtTelnet: begin ProtoName := 'Telnet'; Port := FCfg.TelnetPort;
              Active := FServer.TelnetCount; end;
    wtBinkp:  begin ProtoName := 'BinkP'; Port := FCfg.BinkpPort;
              Active := FServer.BinkpCount; end;
    wtFtp:    begin ProtoName := 'FTP'; Port := 21;
              Active := FServer.FtpCount; end;
    wtHttp:   begin ProtoName := 'HTTP'; Port := 8080;
              Active := FServer.HttpCount; end;
    wtSmtp:   begin ProtoName := 'SMTP'; Port := 0; Active := 0; end;
    wtEvents: begin ProtoName := 'Events'; Port := 0; Active := 0; end;
  end;

  WriteAt(33, 4, ColBorderFg, ColNormalBg, '     Proto: ');
  WriteAt(45, 4, ColInputFg, ColNormalBg, ProtoName);

  WriteAt(33, 5, ColBorderFg, ColNormalBg, '      Port: ');
  if Port > 0 then
    WriteAt(45, 5, ColInputFg, ColNormalBg, IntToStr(Port))
  else
    WriteAt(45, 5, 8, ColNormalBg, 'n/a');

  WriteAt(33, 6, ColBorderFg, ColNormalBg, '       Max: ');
  WriteAt(45, 6, ColInputFg, ColNormalBg, IntToStr(FCfg.MaxConnections));

  WriteAt(33, 7, ColBorderFg, ColNormalBg, '    Active: ');
  WriteAt(45, 7, ColHighFg, ColNormalBg, IntToStr(Active));

  WriteAt(33, 8, ColBorderFg, ColNormalBg, '     Total: ');
  WriteAt(45, 8, ColNormalFg, ColNormalBg, IntToStr(FServer.ConnectionCount));

  WriteAt(33, 9, ColBorderFg, ColNormalBg, '    Uptime: ');
  WriteAt(45, 9, ColNormalFg, ColNormalBg, FormatDateTime('hh:nn:ss', Now));
end;

procedure TPcbisWfc.DrawServerLog;
var
  I, Y : integer;
begin
  WriteAt(3, 12, ColHighFg, ColNormalBg, 'Server Status');

  for I := 0 to 10 do
  begin
    Y := 13 + I;
    if Y > 23 then Break;
    GotoXY(3, Y);
    SetColor(ColNormalFg, ColNormalBg);
    if I < FLogCount then
      Write(Copy(FLogLines[I] + StringOfChar(' ', 76), 1, 76))
    else
      Write(StringOfChar(' ', 76));
  end;
end;

procedure TPcbisWfc.DrawStatusBar;
begin
  WriteAt(2, 25, ColStatusFg, ColStatusBg,
    ' TAB/Switch   SPACE/Local   ALT-K/Kill   ESC/Shutdown                          ');
end;

procedure TPcbisWfc.Draw;
begin
  DrawFrame;
  DrawTitle;
  DrawProtoTab;
  DrawConnections;
  DrawStats;
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
    DrawProtoTab;
    DrawConnections;
    DrawStats;
    DrawServerLog;
  end;
end;

procedure TPcbisWfc.AddLog(const Line : string);
var
  I : integer;
begin
  if FLogCount >= 11 then
  begin
    { Scroll up }
    for I := 0 to 9 do
      FLogLines[I] := FLogLines[I + 1];
    FLogLines[10] := Line;
  end
  else
  begin
    FLogLines[FLogCount] := Line;
    Inc(FLogCount);
  end;
end;

procedure TPcbisWfc.HandleKey(Key : char);
begin
  case Key of
    #9: { TAB — next protocol tab }
      begin
        if FActiveTab = High(TWfcProtoTab) then
          FActiveTab := Low(TWfcProtoTab)
        else
          Inc(FActiveTab);
        FNeedRedraw := True;
      end;

    #27: { ESC — quit }
      QuitFlag := True;

    ' ': { SPACE — local login }
      begin
        AddLog(FormatDateTime('hh:nn:ss', Now) + ' Local login requested');
        { TODO: spawn local PCBoard session }
      end;
  end;
end;

function TPcbisWfc.ShouldQuit : boolean;
begin
  Result := QuitFlag;
end;

end.
