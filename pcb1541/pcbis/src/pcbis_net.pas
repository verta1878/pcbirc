{
  pcbis_net.pas — Network multiplexer and server framework
  Manages TCP listeners and connections via select()/poll().
}

unit pcbis_net;

{$mode objfpc}{$H+}

interface

uses
  SysUtils, Classes, BaseUnix, Sockets,
  pcbis_config, pcbis_log;

const
  MAX_CLIENTS = 128;

type
  TConnectionState = (csListening, csConnected, csClosing);
  TProtocolType = (ptTelnet, ptBinkp, ptFtp, ptHttp, ptSmtpQueue);

  TPcbisConnection = class
  public
    Socket    : longint;
    State     : TConnectionState;
    Protocol  : TProtocolType;
    RemoteIP  : string;
    RemotePort: word;
    ConnectTime : TDateTime;
    LastActivity: TDateTime;
    NodeNumber  : integer;    { for telnet: assigned PCBoard node }
    InBuf     : string;       { incoming data buffer }
    OutBuf    : string;       { outgoing data buffer }

    constructor Create(ASock : longint; AProto : TProtocolType);
    function Age : integer;   { seconds since connect }
    function Idle : integer;  { seconds since last activity }
  end;

  TPcbisServer = class
  private
    FCfg         : TPcbisConfig;
    FTelnetSock  : longint;
    FBinkpSock   : longint;
    FFtpSock     : longint;
    FHttpSock    : longint;
    FConnections : TList;
    FNextNode    : integer;

    function CreateListener(const BindAddr : string; Port : integer) : longint;
    procedure AcceptConnection(ListenSock : longint; Proto : TProtocolType);
    procedure HandleRead(Conn : TPcbisConnection);
    procedure HandleWrite(Conn : TPcbisConnection);
    procedure CloseConnection(Conn : TPcbisConnection);
    procedure RemoveClosed;
  public
    constructor Create(ACfg : TPcbisConfig);
    destructor Destroy; override;

    procedure StartTelnet;
    procedure StartBinkp;
    procedure StartFtp;
    procedure StartHttp;
    procedure StartSmtp;
    procedure StopAll;
    procedure Poll(TimeoutMs : integer);

    function ConnectionCount : integer;
    function TelnetCount : integer;
    function BinkpCount : integer;
    function FtpCount : integer;
    function HttpCount : integer;
  end;

implementation

uses
  pcbis_telnet, pcbis_binkp, pcbis_ftp, pcbis_http;

{ === TPcbisConnection === }

constructor TPcbisConnection.Create(ASock : longint; AProto : TProtocolType);
begin
  inherited Create;
  Socket := ASock;
  State := csConnected;
  Protocol := AProto;
  ConnectTime := Now;
  LastActivity := Now;
  NodeNumber := -1;
  InBuf := '';
  OutBuf := '';
end;

function TPcbisConnection.Age : integer;
begin
  Result := Round((Now - ConnectTime) * 86400);
end;

function TPcbisConnection.Idle : integer;
begin
  Result := Round((Now - LastActivity) * 86400);
end;

{ === TPcbisServer === }

constructor TPcbisServer.Create(ACfg : TPcbisConfig);
begin
  inherited Create;
  FCfg := ACfg;
  FTelnetSock := -1;
  FBinkpSock := -1;
  FFtpSock := -1;
  FHttpSock := -1;
  FConnections := TList.Create;
  FNextNode := ACfg.TelnetNodeStart;
end;

destructor TPcbisServer.Destroy;
begin
  StopAll;
  FConnections.Free;
  inherited Destroy;
end;

function TPcbisServer.CreateListener(const BindAddr : string; Port : integer) : longint;
var
  Sock   : longint;
  Addr   : TInetSockAddr;
  OptVal : longint;
begin
  Sock := fpSocket(AF_INET, SOCK_STREAM, 0);
  if Sock < 0 then
  begin
    LogError('Socket creation failed: ' + IntToStr(SocketError));
    Result := -1;
    Exit;
  end;

  { SO_REUSEADDR }
  OptVal := 1;
  fpSetSockOpt(Sock, SOL_SOCKET, SO_REUSEADDR, @OptVal, SizeOf(OptVal));

  { Bind }
  FillChar(Addr, SizeOf(Addr), 0);
  Addr.sin_family := AF_INET;
  Addr.sin_port := htons(Port);
  if BindAddr = '0.0.0.0' then
    Addr.sin_addr.s_addr := 0  { INADDR_ANY }
  else
    Addr.sin_addr := StrToNetAddr(BindAddr);

  if fpBind(Sock, @Addr, SizeOf(Addr)) <> 0 then
  begin
    LogError('Bind failed on port ' + IntToStr(Port) + ': ' + IntToStr(SocketError));
    CloseSocket(Sock);
    Result := -1;
    Exit;
  end;

  { Listen }
  if fpListen(Sock, 16) <> 0 then
  begin
    LogError('Listen failed: ' + IntToStr(SocketError));
    CloseSocket(Sock);
    Result := -1;
    Exit;
  end;

  Result := Sock;
end;

procedure TPcbisServer.AcceptConnection(ListenSock : longint; Proto : TProtocolType);
var
  ClientSock : longint;
  ClientAddr : TInetSockAddr;
  AddrLen    : TSockLen;
  Conn       : TPcbisConnection;
begin
  AddrLen := SizeOf(ClientAddr);
  ClientSock := fpAccept(ListenSock, @ClientAddr, @AddrLen);
  if ClientSock < 0 then Exit;

  if FConnections.Count >= FCfg.MaxConnections then
  begin
    LogWarn('Max connections reached, rejecting');
    CloseSocket(ClientSock);
    Exit;
  end;

  Conn := TPcbisConnection.Create(ClientSock, Proto);
  Conn.RemoteIP := NetAddrToStr(ClientAddr.sin_addr);
  Conn.RemotePort := ntohs(ClientAddr.sin_port);

  if Proto = ptTelnet then
  begin
    Conn.NodeNumber := FNextNode;
    Inc(FNextNode);
    if FNextNode >= FCfg.TelnetNodeStart + FCfg.TelnetMaxNodes then
      FNextNode := FCfg.TelnetNodeStart;
    LogConn('TELNET', Conn.RemoteIP, 'connected → node ' + IntToStr(Conn.NodeNumber));
    TelnetOnConnect(Conn);
  end
  else if Proto = ptBinkp then
  begin
    LogConn('BINKP', Conn.RemoteIP, 'connected');
    BinkpOnConnect(Conn);
  end
  else if Proto = ptFtp then
  begin
    LogConn('FTP', Conn.RemoteIP, 'connected');
    FtpOnConnect(Conn);
  end
  else if Proto = ptHttp then
  begin
    LogConn('HTTP', Conn.RemoteIP, 'connected');
    HttpOnConnect(Conn);
  end;

  FConnections.Add(Conn);
end;

procedure TPcbisServer.HandleRead(Conn : TPcbisConnection);
var
  Buf  : array[0..4095] of byte;
  N    : longint;
begin
  N := fpRecv(Conn.Socket, @Buf, SizeOf(Buf), 0);
  if N <= 0 then
  begin
    Conn.State := csClosing;
    Exit;
  end;

  Conn.LastActivity := Now;
  SetLength(Conn.InBuf, Length(Conn.InBuf) + N);
  Move(Buf, Conn.InBuf[Length(Conn.InBuf) - N + 1], N);

  case Conn.Protocol of
    ptTelnet: TelnetOnData(Conn);
    ptBinkp:  BinkpOnData(Conn);
    ptFtp:    FtpOnData(Conn);
    ptHttp:   HttpOnData(Conn);
  end;
end;

procedure TPcbisServer.HandleWrite(Conn : TPcbisConnection);
var
  N : longint;
begin
  if Length(Conn.OutBuf) = 0 then Exit;

  N := fpSend(Conn.Socket, @Conn.OutBuf[1], Length(Conn.OutBuf), 0);
  if N > 0 then
    Delete(Conn.OutBuf, 1, N)
  else if N < 0 then
    Conn.State := csClosing;
end;

procedure TPcbisServer.CloseConnection(Conn : TPcbisConnection);
begin
  if Conn.Protocol = ptTelnet then
    LogConn('TELNET', Conn.RemoteIP, 'disconnected (node ' + IntToStr(Conn.NodeNumber) + ')')
  else
    LogConn('BINKP', Conn.RemoteIP, 'disconnected');

  CloseSocket(Conn.Socket);
  Conn.State := csClosing;
end;

procedure TPcbisServer.RemoveClosed;
var
  I    : integer;
  Conn : TPcbisConnection;
begin
  for I := FConnections.Count - 1 downto 0 do
  begin
    Conn := TPcbisConnection(FConnections[I]);
    if Conn.State = csClosing then
    begin
      CloseSocket(Conn.Socket);
      FConnections.Delete(I);
      Conn.Free;
    end;
  end;
end;

procedure TPcbisServer.StartTelnet;
begin
  FTelnetSock := CreateListener(FCfg.TelnetListen, FCfg.TelnetPort);
  if FTelnetSock < 0 then
    LogError('Failed to start telnet listener')
  else
    LogInfo('Telnet listening on ' + FCfg.TelnetListen + ':' + IntToStr(FCfg.TelnetPort));
end;

procedure TPcbisServer.StartBinkp;
begin
  FBinkpSock := CreateListener(FCfg.BinkpListen, FCfg.BinkpPort);
  if FBinkpSock < 0 then
    LogError('Failed to start BinkP listener')
  else
    LogInfo('BinkP listening on ' + FCfg.BinkpListen + ':' + IntToStr(FCfg.BinkpPort));
end;

procedure TPcbisServer.StartFtp;
begin
  FFtpSock := CreateListener('0.0.0.0', 21);  { TODO: from config }
  if FFtpSock < 0 then
    LogError('Failed to start FTP listener')
  else
    LogInfo('FTP listening on port 21');
end;

procedure TPcbisServer.StartHttp;
begin
  FHttpSock := CreateListener('0.0.0.0', 8080);  { TODO: from config }
  if FHttpSock < 0 then
    LogError('Failed to start HTTP listener')
  else
    LogInfo('HTTP listening on port 8080');
end;

procedure TPcbisServer.StartSmtp;
begin
  { SMTP is outbound only — no listener needed.
    The queue scanner checks SmtpTriggerDir periodically. }
  LogInfo('SMTP outbound queue: ' + FCfg.SmtpTriggerDir);
end;

procedure TPcbisServer.StopAll;
var
  I : integer;
begin
  if FTelnetSock >= 0 then begin CloseSocket(FTelnetSock); FTelnetSock := -1; end;
  if FBinkpSock >= 0 then begin CloseSocket(FBinkpSock); FBinkpSock := -1; end;
  if FFtpSock >= 0 then begin CloseSocket(FFtpSock); FFtpSock := -1; end;
  if FHttpSock >= 0 then begin CloseSocket(FHttpSock); FHttpSock := -1; end;

  for I := 0 to FConnections.Count - 1 do
    CloseConnection(TPcbisConnection(FConnections[I]));
  RemoveClosed;
end;

procedure TPcbisServer.Poll(TimeoutMs : integer);
var
  ReadSet, WriteSet : TFDSet;
  MaxFD, I          : longint;
  Conn              : TPcbisConnection;
  TV                : TTimeVal;
begin
  fpFD_ZERO(ReadSet);
  fpFD_ZERO(WriteSet);
  MaxFD := 0;

  { Add listeners }
  if FTelnetSock >= 0 then begin fpFD_SET(FTelnetSock, ReadSet); if FTelnetSock > MaxFD then MaxFD := FTelnetSock; end;
  if FBinkpSock >= 0 then begin fpFD_SET(FBinkpSock, ReadSet); if FBinkpSock > MaxFD then MaxFD := FBinkpSock; end;
  if FFtpSock >= 0 then begin fpFD_SET(FFtpSock, ReadSet); if FFtpSock > MaxFD then MaxFD := FFtpSock; end;
  if FHttpSock >= 0 then begin fpFD_SET(FHttpSock, ReadSet); if FHttpSock > MaxFD then MaxFD := FHttpSock; end;

  { Add client connections }
  for I := 0 to FConnections.Count - 1 do
  begin
    Conn := TPcbisConnection(FConnections[I]);
    if Conn.State = csConnected then
    begin
      fpFD_SET(Conn.Socket, ReadSet);
      if Length(Conn.OutBuf) > 0 then
        fpFD_SET(Conn.Socket, WriteSet);
      if Conn.Socket > MaxFD then MaxFD := Conn.Socket;
    end;
  end;

  TV.tv_sec := TimeoutMs div 1000;
  TV.tv_usec := (TimeoutMs mod 1000) * 1000;

  if fpSelect(MaxFD + 1, @ReadSet, @WriteSet, nil, @TV) <= 0 then
  begin
    { Check idle timeouts }
    for I := 0 to FConnections.Count - 1 do
    begin
      Conn := TPcbisConnection(FConnections[I]);
      if (Conn.Protocol = ptTelnet) and (Conn.Idle > FCfg.TelnetIdleTimeout) then
      begin
        LogConn('TELNET', Conn.RemoteIP, 'idle timeout');
        Conn.State := csClosing;
      end;
    end;
    RemoveClosed;
    Exit;
  end;

  { Accept new connections }
  if (FTelnetSock >= 0) and fpFD_ISSET(FTelnetSock, ReadSet) then
    AcceptConnection(FTelnetSock, ptTelnet);
  if (FBinkpSock >= 0) and fpFD_ISSET(FBinkpSock, ReadSet) then
    AcceptConnection(FBinkpSock, ptBinkp);
  if (FFtpSock >= 0) and fpFD_ISSET(FFtpSock, ReadSet) then
    AcceptConnection(FFtpSock, ptFtp);
  if (FHttpSock >= 0) and fpFD_ISSET(FHttpSock, ReadSet) then
    AcceptConnection(FHttpSock, ptHttp);

  { Handle client I/O }
  for I := 0 to FConnections.Count - 1 do
  begin
    Conn := TPcbisConnection(FConnections[I]);
    if Conn.State <> csConnected then Continue;

    if fpFD_ISSET(Conn.Socket, ReadSet) then
      HandleRead(Conn);
    if fpFD_ISSET(Conn.Socket, WriteSet) then
      HandleWrite(Conn);

    { Poll FOSSIL TX ring for telnet connections }
    if Conn.Protocol = ptTelnet then
      TelnetPollFossil(Conn);
  end;

  RemoveClosed;
end;

function TPcbisServer.ConnectionCount : integer;
begin
  Result := FConnections.Count;
end;

function TPcbisServer.TelnetCount : integer;
var
  I : integer;
begin
  Result := 0;
  for I := 0 to FConnections.Count - 1 do
    if TPcbisConnection(FConnections[I]).Protocol = ptTelnet then Inc(Result);
end;

function TPcbisServer.BinkpCount : integer;
var
  I : integer;
begin
  Result := 0;
  for I := 0 to FConnections.Count - 1 do
    if TPcbisConnection(FConnections[I]).Protocol = ptBinkp then Inc(Result);
end;

function TPcbisServer.FtpCount : integer;
var
  I : integer;
begin
  Result := 0;
  for I := 0 to FConnections.Count - 1 do
    if TPcbisConnection(FConnections[I]).Protocol = ptFtp then Inc(Result);
end;

function TPcbisServer.HttpCount : integer;
var
  I : integer;
begin
  Result := 0;
  for I := 0 to FConnections.Count - 1 do
    if TPcbisConnection(FConnections[I]).Protocol = ptHttp then Inc(Result);
end;

end.
