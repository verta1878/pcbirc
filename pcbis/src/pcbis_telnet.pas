{
  pcbis_telnet.pas — Telnet protocol handler for pcbis
  Handles IAC negotiation and bridges to PCBoard nodes.
}

unit pcbis_telnet;

{$mode objfpc}{$H+}

interface

uses
  pcbis_net, pcbfoss;

procedure TelnetOnConnect(Conn : TPcbisConnection);
procedure TelnetOnData(Conn : TPcbisConnection);
procedure TelnetOnDisconnect(Conn : TPcbisConnection);
procedure TelnetPollFossil(Conn : TPcbisConnection);

{ Per-node FOSSIL instances }
function  GetNodeFossil(NodeNum : integer) : TPcbFossil;

implementation

uses
  SysUtils, pcbis_log, pcbis_nodedata;

const
  { Telnet commands }
  IAC  = #255;
  DONT = #254;
  DO_  = #253;
  WONT = #252;
  WILL = #251;
  SB   = #250;
  SE   = #240;

  OPT_ECHO    = #1;
  OPT_SGA     = #3;
  OPT_TTYPE   = #24;
  OPT_NAWS    = #31;

  MAX_NODES = 32;

var
  NodeFossils : array[0..MAX_NODES-1] of TPcbFossil;

function GetNodeFossil(NodeNum : integer) : TPcbFossil;
begin
  if (NodeNum >= 0) and (NodeNum < MAX_NODES) then
    Result := NodeFossils[NodeNum]
  else
    Result := nil;
end;

procedure InitNodeFossil(NodeNum : integer);
begin
  if (NodeNum >= 0) and (NodeNum < MAX_NODES) then
  begin
    if NodeFossils[NodeNum] = nil then
      NodeFossils[NodeNum] := TPcbFossil.Create(NodeNum, 8192);
  end;
end;

procedure SendIAC(Conn : TPcbisConnection; Cmd, Opt : char);
begin
  Conn.OutBuf := Conn.OutBuf + IAC + Cmd + Opt;
end;

procedure TelnetOnConnect(Conn : TPcbisConnection);
var
  Foss   : TPcbFossil;
  Regs   : TFossilRegs;
  Status : TPcbNodeStatus;
begin
  { Negotiate telnet options }
  SendIAC(Conn, WILL, OPT_ECHO);
  SendIAC(Conn, WILL, OPT_SGA);
  SendIAC(Conn, DO_, OPT_NAWS);
  SendIAC(Conn, DO_, OPT_TTYPE);

  { Initialize FOSSIL for this node }
  InitNodeFossil(Conn.NodeNumber);
  Foss := GetNodeFossil(Conn.NodeNumber);
  if Foss <> nil then
  begin
    FillChar(Regs, SizeOf(Regs), 0);
    Regs.AH := FN_INIT;
    Regs.DX := Conn.NodeNumber;
    Foss.Dispatch(Regs);
    Foss.SetConnected(True);
    LogConn('TELNET', Conn.RemoteIP,
            'FOSSIL initialized for node ' + IntToStr(Conn.NodeNumber) +
            ' (sig=$' + IntToHex(Regs.AX, 4) + ')');
  end;

  { Write node status for PCBoard who's online }
  FillChar(Status, SizeOf(Status), 0);
  Status.NodeNum := Conn.NodeNumber;
  Status.UserName := '(connecting)';
  Status.City := Conn.RemoteIP;
  Status.ConnectStr := 'TELNET';
  Status.ConnectTime := Now;
  Status.Activity := 'Logging in';
  WriteNodeStatus('.', Status);  { TODO: get PCB dir from config }

  { Banner }
  Conn.OutBuf := Conn.OutBuf +
    #13#10 +
    'PCBoard 15.4 Revival — Internet Services' + #13#10 +
    'Node ' + IntToStr(Conn.NodeNumber) + ' — FOSSIL bridge active' + #13#10 +
    #13#10;
end;

procedure ProcessIAC(Conn : TPcbisConnection; var Data : string);
var
  I   : integer;
  Cmd : char;
  Opt : char;
begin
  I := 1;
  while I <= Length(Data) do
  begin
    if Data[I] = IAC then
    begin
      if I + 2 <= Length(Data) then
      begin
        Cmd := Data[I + 1];
        Opt := Data[I + 2];
        case Cmd of
          DO_:  if (Opt = OPT_ECHO) or (Opt = OPT_SGA) then
                  SendIAC(Conn, WILL, Opt)
                else
                  SendIAC(Conn, WONT, Opt);
          DONT: SendIAC(Conn, WONT, Opt);
          WILL: if (Opt = OPT_NAWS) or (Opt = OPT_TTYPE) then
                  SendIAC(Conn, DO_, Opt)
                else
                  SendIAC(Conn, DONT, Opt);
          WONT: SendIAC(Conn, DONT, Opt);
          SB:   while (I + 2 < Length(Data)) and (Data[I + 2] <> SE) do Inc(I);
        end;
        Delete(Data, I, 3);
        Continue;
      end
      else
        Break;
    end;
    Inc(I);
  end;
end;

procedure TelnetOnData(Conn : TPcbisConnection);
var
  CleanData : string;
  Foss      : TPcbFossil;
begin
  CleanData := Conn.InBuf;
  Conn.InBuf := '';

  ProcessIAC(Conn, CleanData);
  if Length(CleanData) = 0 then Exit;

  { Feed data into FOSSIL RX ring — PCBoard reads it via INT 14h }
  Foss := GetNodeFossil(Conn.NodeNumber);
  if Foss <> nil then
    Foss.SocketWrite(CleanData[1], Length(CleanData))
  else
    { Fallback echo mode if no FOSSIL }
    Conn.OutBuf := Conn.OutBuf + CleanData;
end;

procedure TelnetPollFossil(Conn : TPcbisConnection);
var
  Foss : TPcbFossil;
  Buf  : array[0..4095] of byte;
  N    : Word;
begin
  { Drain FOSSIL TX ring → send to telnet client }
  Foss := GetNodeFossil(Conn.NodeNumber);
  if Foss = nil then Exit;

  N := Foss.SocketRead(Buf, SizeOf(Buf));
  if N > 0 then
  begin
    SetLength(Conn.OutBuf, Length(Conn.OutBuf) + N);
    Move(Buf, Conn.OutBuf[Length(Conn.OutBuf) - N + 1], N);
  end;

  { Check if FOSSIL dropped DTR (PCBoard hung up) }
  if not Foss.IsConnected then
  begin
    LogConn('TELNET', Conn.RemoteIP,
            'FOSSIL DTR drop on node ' + IntToStr(Conn.NodeNumber));
    Conn.State := csClosing;
  end;
end;

procedure TelnetOnDisconnect(Conn : TPcbisConnection);
var
  Foss    : TPcbFossil;
  Regs    : TFossilRegs;
  Caller  : TPcbCallerRec;
begin
  { Deinit FOSSIL }
  Foss := GetNodeFossil(Conn.NodeNumber);
  if Foss <> nil then
  begin
    FillChar(Regs, SizeOf(Regs), 0);
    Regs.AH := FN_DEINIT;
    Foss.Dispatch(Regs);
    Foss.SetConnected(False);
  end;

  { Clear node status }
  ClearNodeStatus('.', Conn.NodeNumber);

  { Log to CALLERS }
  FillChar(Caller, SizeOf(Caller), 0);
  Caller.NodeNum := Conn.NodeNumber;
  Caller.UserName := '(telnet)';  { TODO: get actual username from PCBoard }
  Caller.City := Conn.RemoteIP;
  Caller.ConnectStr := 'TELNET';
  Caller.LoginTime := Conn.ConnectTime;
  Caller.LogoutTime := Now;
  AppendCaller('.', Caller);

  LogConn('TELNET', Conn.RemoteIP,
          'disconnected from node ' + IntToStr(Conn.NodeNumber));
end;

var
  I : integer;
initialization
  for I := 0 to MAX_NODES - 1 do
    NodeFossils[I] := nil;

finalization
  for I := 0 to MAX_NODES - 1 do
    if NodeFossils[I] <> nil then
      NodeFossils[I].Free;

end.
