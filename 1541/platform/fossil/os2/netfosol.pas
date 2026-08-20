{ ===========================================================================
  netfosol — OS/2 FOSSIL Driver
  GPLv3 — Copyright (C) 2026 wrench (netmodem2irc)

  OS/2 native FOSSIL driver for netmodem2irc.
  Bridges TCP/IP callers to BBS software via socket I/O.

  Two modes:
    1. Socket mode (default) — uses m_fossil_socket for TCP transport
    2. Serial mode — uses OS/2 DosDevIOCtl for real COM ports

  OS/2 API reference: OS/2 Toolkit 4.5 (bsedev.h)

  Build:
    ppc386 -Tos2 netfosol.pas (FPC cross-compile)
    or via openwatcomirc C port (async_os2.c)
  =========================================================================== }

{$MODE OBJFPC}{$H+}

unit netfosol;

interface

uses
  SysUtils, m_fossil_socket;

const
  NETFOSOL_VERSION = 'netfosol 1.0 — OS/2 FOSSIL';

  { OS/2 DosDevIOCtl constants (from bsedev.h / OS/2 Toolkit 4.5) }
  IOCTL_ASYNC          = $01;   { async device category }
  ASYNC_SETBAUDRATE    = $41;   { set baud rate }
  ASYNC_GETBAUDRATE    = $61;   { get baud rate }
  ASYNC_SETLINECTRL    = $42;   { set line control (data bits, stop, parity) }
  ASYNC_GETLINECTRL    = $62;   { get line control }
  ASYNC_SETMODEMCTRL   = $46;   { set modem control (DTR, RTS) }
  ASYNC_GETMODEMCTRL   = $66;   { get modem control }
  ASYNC_GETMODEMINPUT  = $67;   { get modem input (DCD, DSR, CTS, RI) }
  ASYNC_GETINQUECOUNT  = $68;   { get input queue byte count }
  ASYNC_GETOUTQUECOUNT = $69;   { get output queue byte count }
  ASYNC_SETENHPARM     = $54;   { set enhanced mode params (FIFO trigger) }

  { Modem status bits (from ASYNC_GETMODEMINPUT) }
  MS_CTS_ON  = $10;
  MS_DSR_ON  = $20;
  MS_RI_ON   = $40;
  MS_DCD_ON  = $80;

  { DTR/RTS control bits (for ASYNC_SETMODEMCTRL) }
  DTR_ON  = $01;
  DTR_OFF = $FE;
  RTS_ON  = $02;
  RTS_OFF = $FD;

type
  TFossilBackend = (fbSocket, fbSerial);

  { OS/2 FOSSIL driver }
  TNetFosOL = class
  private
    FBackend    : TFossilBackend;
    FSocket     : TFossilSocket;     { socket backend (from m_fossil_socket) }
    FComHandle  : LongInt;           { OS/2 COM port handle (DosOpen) }
    FComPort    : Integer;           { COM port number (1-16) }
    FBaudRate   : LongInt;           { current baud rate }
    FActive     : Boolean;
    FInBytes    : LongInt;
    FOutBytes   : LongInt;

    { OS/2 serial helpers }
    function  SerialOpen(Port: Integer; Baud: LongInt): Boolean;
    procedure SerialClose;
    function  SerialSend(const Buf; Len: LongInt): LongInt;
    function  SerialRecv(var Buf; MaxLen: LongInt): LongInt;
    function  SerialReady: Boolean;
    function  SerialDCD: Boolean;
    procedure SerialDTR(State: Boolean);
    procedure SerialSetBaud(Baud: LongInt);
  public
    constructor Create;
    destructor  Destroy; override;

    { Socket mode init (netmodem2irc → BBS) }
    function  InitSocket(Port: Word): Boolean;
    function  InitSocketFD(FD: LongInt): Boolean;
    function  InitSocketConnect(Port: Word): Boolean;

    { Serial mode init (real COM port) }
    function  InitSerial(Port: Integer; Baud: LongInt): Boolean;

    procedure Close;

    { FOSSIL-compatible I/O }
    function  SendByte(B: Byte): Boolean;
    function  SendStr(const S: String): LongInt;
    function  RecvByte: Integer;         { -1 = no data }
    function  RecvBuf(var Buf; MaxLen: LongInt): LongInt;
    function  RecvReady: Boolean;
    function  IsConnected: Boolean;
    procedure PurgeInput;
    procedure PurgeOutput;
    procedure HangUp;

    property Backend  : TFossilBackend read FBackend;
    property Active   : Boolean read FActive;
    property InBytes  : LongInt read FInBytes;
    property OutBytes : LongInt read FOutBytes;
    property ComPort  : Integer read FComPort;
    property BaudRate : LongInt read FBaudRate;
  end;

implementation

{ PCBoard-compatible globals — these must exist if PCBoard C code
  references them. Updated by the driver on connect/disconnect. }
var
  CDokay       : Byte = 0;    { 1 = connected, 0 = disconnected }
  _CTSokay     : Byte = 1;    { always 1 on sockets }
  _RingDetect  : Byte = 0;    { always 0 }
  CDup         : Byte = 0;    { 1 = carrier up }
  B8250        : Byte = 0;    { UART type flags — all 0 for socket }
  B16550       : Byte = 0;
  B16550A      : Byte = 0;
  B16650       : Byte = 0;
  _InBytes     : LongInt = 0;
  _OutBytes    : LongInt = 0;
  _OverrunErrors : LongInt = 0;
  _ParityErrors  : LongInt = 0;
  _FramingErrors : LongInt = 0;


{$IFDEF OS2}
uses
  DosCalls;  { OS/2 API: DosOpen, DosClose, DosRead, DosWrite, DosDevIOCtl }
{$ENDIF}

{ ---------------------------------------------------------------
  Constructor / Destructor
  --------------------------------------------------------------- }

constructor TNetFosOL.Create;
begin
  inherited Create;
  FBackend := fbSocket;
  FSocket := TFossilSocket.Create;
  FComHandle := -1;
  FComPort := 0;
  FBaudRate := 115200;
  FActive := False;
  FInBytes := 0;
  FOutBytes := 0;
end;

destructor TNetFosOL.Destroy;
begin
  Close;
  FSocket.Free;
  inherited Destroy;
end;

{ ---------------------------------------------------------------
  Socket mode init — delegates to m_fossil_socket
  --------------------------------------------------------------- }

function TNetFosOL.InitSocket(Port: Word): Boolean;
begin
  FBackend := fbSocket;
  Result := FSocket.InitAccept(Port);
  FActive := Result;
end;

function TNetFosOL.InitSocketFD(FD: LongInt): Boolean;
begin
  FBackend := fbSocket;
  Result := FSocket.InitFromFD(FD);
  FActive := Result;
end;

function TNetFosOL.InitSocketConnect(Port: Word): Boolean;
begin
  FBackend := fbSocket;
  Result := FSocket.InitConnect(Port);
  FActive := Result;
end;

{ ---------------------------------------------------------------
  Serial mode init — OS/2 DosOpen + DosDevIOCtl
  --------------------------------------------------------------- }

function TNetFosOL.InitSerial(Port: Integer; Baud: LongInt): Boolean;
begin
  FBackend := fbSerial;
  FComPort := Port;
  FBaudRate := Baud;
  Result := SerialOpen(Port, Baud);
  FActive := Result;
end;

function TNetFosOL.SerialOpen(Port: Integer; Baud: LongInt): Boolean;
{$IFDEF OS2}
var
  Action : ULong;
  DevName: String;
  RC     : ApiRet;
begin
  Result := False;
  DevName := 'COM' + IntToStr(Port);

  RC := DosOpen(PChar(DevName), FComHandle, Action, 0, 0,
    OPEN_ACTION_OPEN_IF_EXISTS,
    OPEN_SHARE_DENYREADWRITE or OPEN_ACCESS_READWRITE, nil);

  if RC <> 0 then begin FComHandle := -1; Exit; end;

  SerialSetBaud(Baud);
  SerialDTR(True);
  Result := True;
end;
{$ELSE}
begin
  { Non-OS/2: stub — serial only works on OS/2 }
  Result := False;
  FComHandle := -1;
end;
{$ENDIF}

procedure TNetFosOL.SerialClose;
{$IFDEF OS2}
begin
  if FComHandle >= 0 then DosClose(FComHandle);
  FComHandle := -1;
end;
{$ELSE}
begin
  FComHandle := -1;
end;
{$ENDIF}

function TNetFosOL.SerialSend(const Buf; Len: LongInt): LongInt;
{$IFDEF OS2}
var Written: ULong;
begin
  Result := 0;
  if FComHandle < 0 then Exit;
  if DosWrite(FComHandle, Buf, Len, Written) = 0 then
    Result := Written;
end;
{$ELSE}
begin Result := 0; end;
{$ENDIF}

function TNetFosOL.SerialRecv(var Buf; MaxLen: LongInt): LongInt;
{$IFDEF OS2}
var BytesRead: ULong;
begin
  Result := 0;
  if FComHandle < 0 then Exit;
  if DosRead(FComHandle, Buf, MaxLen, BytesRead) = 0 then
    Result := BytesRead;
end;
{$ELSE}
begin Result := 0; end;
{$ENDIF}

function TNetFosOL.SerialReady: Boolean;
{$IFDEF OS2}
var
  QueueCount: Word;
  ParmLen, DataLen: ULong;
begin
  Result := False;
  if FComHandle < 0 then Exit;
  ParmLen := 0; DataLen := SizeOf(QueueCount);
  if DosDevIOCtl(FComHandle, IOCTL_ASYNC, ASYNC_GETINQUECOUNT,
    nil, 0, @ParmLen, @QueueCount, SizeOf(QueueCount), @DataLen) = 0 then
    Result := (QueueCount > 0);
end;
{$ELSE}
begin Result := False; end;
{$ENDIF}

function TNetFosOL.SerialDCD: Boolean;
{$IFDEF OS2}
var
  ModemStatus: Byte;
  ParmLen, DataLen: ULong;
begin
  Result := False;
  if FComHandle < 0 then Exit;
  ParmLen := 0; DataLen := SizeOf(ModemStatus);
  if DosDevIOCtl(FComHandle, IOCTL_ASYNC, ASYNC_GETMODEMINPUT,
    nil, 0, @ParmLen, @ModemStatus, SizeOf(ModemStatus), @DataLen) = 0 then
    Result := (ModemStatus and MS_DCD_ON) <> 0;
end;
{$ELSE}
begin Result := False; end;
{$ENDIF}

procedure TNetFosOL.SerialDTR(State: Boolean);
{$IFDEF OS2}
var
  ModemCtrl: Byte;
  ParmLen, DataLen: ULong;
begin
  if FComHandle < 0 then Exit;
  if State then ModemCtrl := DTR_ON else ModemCtrl := 0;
  ParmLen := SizeOf(ModemCtrl); DataLen := 0;
  DosDevIOCtl(FComHandle, IOCTL_ASYNC, ASYNC_SETMODEMCTRL,
    @ModemCtrl, SizeOf(ModemCtrl), @ParmLen, nil, 0, @DataLen);
end;
{$ELSE}
begin end;
{$ENDIF}

procedure TNetFosOL.SerialSetBaud(Baud: LongInt);
{$IFDEF OS2}
var
  BaudWord: Word;
  ParmLen, DataLen: ULong;
begin
  if FComHandle < 0 then Exit;
  BaudWord := Word(Baud);
  ParmLen := SizeOf(BaudWord); DataLen := 0;
  DosDevIOCtl(FComHandle, IOCTL_ASYNC, ASYNC_SETBAUDRATE,
    @BaudWord, SizeOf(BaudWord), @ParmLen, nil, 0, @DataLen);
  FBaudRate := Baud;
end;
{$ELSE}
begin FBaudRate := Baud; end;
{$ENDIF}

{ ---------------------------------------------------------------
  Common I/O — dispatches to socket or serial backend
  --------------------------------------------------------------- }

procedure TNetFosOL.Close;
begin
  if FBackend = fbSocket then
    FSocket.Deinit
  else
    SerialClose;
  FActive := False;
end;

function TNetFosOL.SendByte(B: Byte): Boolean;
begin
  if not FActive then begin Result := False; Exit; end;
  if FBackend = fbSocket then
    Result := FSocket.SendByte(B)
  else
    Result := (SerialSend(B, 1) = 1);
  if Result then Inc(FOutBytes);
end;

function TNetFosOL.SendStr(const S: String): LongInt;
begin
  if not FActive then begin Result := 0; Exit; end;
  if FBackend = fbSocket then
    Result := FSocket.Send(S)
  else
    Result := SerialSend(S[1], Length(S));
  Inc(FOutBytes, Result);
end;

function TNetFosOL.RecvByte: Integer;
var
  Buf: Byte;
  S: String;
begin
  Result := -1;
  if not FActive then Exit;
  if FBackend = fbSocket then
  begin
    if not FSocket.RecvReady then Exit;
    S := FSocket.Recv(1);
    if Length(S) > 0 then begin Result := Ord(S[1]); Inc(FInBytes); end;
  end
  else
  begin
    if not SerialReady then Exit;
    if SerialRecv(Buf, 1) = 1 then begin Result := Buf; Inc(FInBytes); end;
  end;
end;

function TNetFosOL.RecvBuf(var Buf; MaxLen: LongInt): LongInt;
var S: String;
begin
  Result := 0;
  if not FActive then Exit;
  if FBackend = fbSocket then
  begin
    S := FSocket.Recv(MaxLen);
    Result := Length(S);
    if Result > 0 then Move(S[1], Buf, Result);
  end
  else
    Result := SerialRecv(Buf, MaxLen);
  Inc(FInBytes, Result);
end;

function TNetFosOL.RecvReady: Boolean;
begin
  if not FActive then begin Result := False; Exit; end;
  if FBackend = fbSocket then
    Result := FSocket.RecvReady
  else
    Result := SerialReady;
end;

function TNetFosOL.IsConnected: Boolean;
begin
  if not FActive then begin Result := False; Exit; end;
  if FBackend = fbSocket then
    Result := FSocket.Connected
  else
    Result := SerialDCD;
end;

procedure TNetFosOL.PurgeInput;
begin
  if not FActive then Exit;
  if FBackend = fbSocket then
    FSocket.PurgeInput;
  { OS/2 serial: DosDevIOCtl ASYNC_FLUSHINPUT — not critical }
end;

procedure TNetFosOL.PurgeOutput;
begin
  if not FActive then Exit;
  if FBackend = fbSocket then
    FSocket.Flush;
  { OS/2 serial: DosDevIOCtl ASYNC_FLUSHOUTPUT — not critical }
end;

procedure TNetFosOL.HangUp;
begin
  if not FActive then Exit;
  if FBackend = fbSocket then
    FSocket.HangUp
  else
    SerialDTR(False);   { drop DTR = hangup }
  FActive := False;
  CDokay := 0; CDup := 0;
end;

end.
