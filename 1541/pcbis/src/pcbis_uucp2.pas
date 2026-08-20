{ ===========================================================================
  pcbis_uucp2.pas — UUCP2: UUCP over TCP/IP
  Same UUCP protocol, TCP transport instead of modem.
  Drop-in replacement for UUIN/UUOUT — just change the transport.

  Equivalent to Wildcat!'s FXUUCP with tcpip transport in SYSTEMS file.
  =========================================================================== }

unit pcbis_uucp2;

{$mode objfpc}{$H+}

interface

uses
  pcbis_config;

type
  TUucpSession = class
  private
    FHost     : string;
    FPort     : integer;
    FLogin    : string;
    FPassword : string;
    FSpoolDir : string;
    FSocket   : longint;

    function  Connect : boolean;
    procedure Disconnect;
    function  SendLogin : boolean;
    function  DoGProtocol : boolean;  { g-protocol file transfer }
  public
    constructor Create(const AHost : string; APort : integer;
                       const ALogin, APassword, ASpoolDir : string);
    destructor Destroy; override;

    { Send outbound files from spool }
    function  SendFiles : integer;

    { Receive inbound files to spool }
    function  ReceiveFiles : integer;

    { Full exchange (send + receive) }
    function  Exchange : boolean;
  end;

{ High-level: export from PCBoard → UUCP spool }
procedure Uucp2Export(const PCBDir, SpoolDir : string);

{ High-level: import from UUCP spool → PCBoard }
procedure Uucp2Import(const SpoolDir, PCBDir : string);

{ High-level: transport (connect to remote, exchange spool files) }
function Uucp2Transfer(const Host : string; Port : integer;
                        const Login, Password, SpoolDir : string) : boolean;

implementation

uses
  SysUtils, Sockets, BaseUnix, pcbis_log;

constructor TUucpSession.Create(const AHost : string; APort : integer;
                                const ALogin, APassword, ASpoolDir : string);
begin
  inherited Create;
  FHost := AHost;
  FPort := APort;
  FLogin := ALogin;
  FPassword := APassword;
  FSpoolDir := ASpoolDir;
  FSocket := -1;
end;

destructor TUucpSession.Destroy;
begin
  if FSocket >= 0 then Disconnect;
  inherited Destroy;
end;

function TUucpSession.Connect : boolean;
var
  Addr : TInetSockAddr;
begin
  Result := False;
  FSocket := fpSocket(AF_INET, SOCK_STREAM, 0);
  if FSocket < 0 then
  begin
    LogMsg(lpMain, llError, 'UUCP2: socket creation failed');
    Exit;
  end;

  FillChar(Addr, SizeOf(Addr), 0);
  Addr.sin_family := AF_INET;
  Addr.sin_port := htons(FPort);
  Addr.sin_addr := StrToNetAddr(FHost);

  if fpConnect(FSocket, @Addr, SizeOf(Addr)) <> 0 then
  begin
    LogMsg(lpMain, llError, 'UUCP2: connect to ' + FHost + ':' + IntToStr(FPort) + ' failed');
    CloseSocket(FSocket);
    FSocket := -1;
    Exit;
  end;

  LogMsg(lpMain, llInfo, 'UUCP2: connected to ' + FHost + ':' + IntToStr(FPort));
  Result := True;
end;

procedure TUucpSession.Disconnect;
begin
  if FSocket >= 0 then
  begin
    CloseSocket(FSocket);
    FSocket := -1;
    LogMsg(lpMain, llInfo, 'UUCP2: disconnected');
  end;
end;

function TUucpSession.SendLogin : boolean;
var
  Buf : array[0..1023] of byte;
  N   : longint;
  Response : string;
begin
  Result := False;

  { Wait for "login:" prompt }
  N := fpRecv(FSocket, @Buf, SizeOf(Buf), 0);
  if N <= 0 then Exit;
  Response := '';
  SetLength(Response, N);
  Move(Buf, Response[1], N);

  if Pos('login:', LowerCase(Response)) = 0 then
  begin
    LogMsg(lpMain, llWarn, 'UUCP2: expected login prompt, got: ' + Response);
    Exit;
  end;

  { Send login }
  fpSend(FSocket, @FLogin[1], Length(FLogin), 0);
  fpSend(FSocket, PChar(#13#10), 2, 0);

  { Wait for "password:" or "ssword:" }
  N := fpRecv(FSocket, @Buf, SizeOf(Buf), 0);
  if N <= 0 then Exit;

  { Send password }
  fpSend(FSocket, @FPassword[1], Length(FPassword), 0);
  fpSend(FSocket, PChar(#13#10), 2, 0);

  { Wait for acknowledgment }
  N := fpRecv(FSocket, @Buf, SizeOf(Buf), 0);
  if N <= 0 then Exit;

  LogMsg(lpMain, llInfo, 'UUCP2: authenticated as ' + FLogin);
  Result := True;
end;

function TUucpSession.DoGProtocol : boolean;
begin
  { TODO: implement UUCP g-protocol file transfer
    The g-protocol sends files in 64-byte packets with checksums.
    This is the same protocol used over modem — just the transport
    changed from serial to TCP.

    Reference: Taylor UUCP source, or Wildcat!'s FXUUCICO.

    For the initial implementation, we can use the simpler 't' protocol
    (transparent/TCP) which sends raw file data without packetization,
    since we're on a reliable TCP connection. }

  LogMsg(lpMain, llInfo, 'UUCP2: g-protocol transfer (stub)');
  Result := True;
end;

function TUucpSession.SendFiles : integer;
var
  SR : TSearchRec;
begin
  Result := 0;

  { Scan spool directory for outbound files (C.* command files) }
  if FindFirst(FSpoolDir + '/C.*', faAnyFile, SR) = 0 then
  begin
    repeat
      { TODO: parse C.* file for data file reference (D.*),
        send via g-protocol or t-protocol }
      LogMsg(lpMain, llInfo, 'UUCP2: sending ' + SR.Name);
      Inc(Result);
    until FindNext(SR) <> 0;
    FindClose(SR);
  end;
end;

function TUucpSession.ReceiveFiles : integer;
begin
  Result := 0;
  { TODO: receive files from remote into spool directory }
  LogMsg(lpMain, llInfo, 'UUCP2: receiving files (stub)');
end;

function TUucpSession.Exchange : boolean;
begin
  Result := False;
  if not Connect then Exit;
  try
    if not SendLogin then Exit;
    if not DoGProtocol then Exit;
    SendFiles;
    ReceiveFiles;
    Result := True;
  finally
    Disconnect;
  end;
end;

{ === High-level functions (called from events/batch) === }

procedure Uucp2Export(const PCBDir, SpoolDir : string);
begin
  { Read PCBoard message bases, generate UUCP spool files (D.* and C.*)
    This is what UUOUT does — same logic, just called from pcbis. }
  LogMsg(lpMain, llInfo, 'UUCP2 export: PCBoard → spool (' + SpoolDir + ')');
  { TODO: implement message base → spool conversion }
end;

procedure Uucp2Import(const SpoolDir, PCBDir : string);
begin
  { Read incoming UUCP spool files, inject into PCBoard message bases.
    This is what UUIN does. }
  LogMsg(lpMain, llInfo, 'UUCP2 import: spool → PCBoard (' + PCBDir + ')');
  { TODO: implement spool → message base conversion }
end;

function Uucp2Transfer(const Host : string; Port : integer;
                        const Login, Password, SpoolDir : string) : boolean;
var
  Session : TUucpSession;
begin
  Session := TUucpSession.Create(Host, Port, Login, Password, SpoolDir);
  try
    Result := Session.Exchange;
  finally
    Session.Free;
  end;
end;

end.
