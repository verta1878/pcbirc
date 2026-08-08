{ ===========================================================================
  netfoswl — Windows FOSSIL Driver
  GPLv3 — Copyright (C) 2026 wrench (netmodem2irc)

  Windows native FOSSIL driver for netmodem2irc.
  Bridges TCP/IP callers to BBS software via socket I/O.
  Replaces NetFoss (PCMicro) for DOS apps under DOSBox/NTVDM.

  Two modes:
    1. Socket mode (default) — uses m_fossil_socket for TCP transport
    2. Serial mode — uses Win32 CreateFile/ReadFile/WriteFile for COM ports

  Build:
    ppc386 -Twin32 netfoswl.pas (FPC)
  =========================================================================== }

{$MODE OBJFPC}{$H+}

unit netfoswl;

interface

uses
  SysUtils, Windows, m_fossil_socket;

const
  NETFOSWL_VERSION = 'netfoswl 1.0 — Windows FOSSIL';

type
  TFossilBackend = (fbSocket, fbSerial);

  { Windows FOSSIL driver }
  TNetFosWL = class
  private
    FBackend    : TFossilBackend;
    FSocket     : TFossilSocket;     { socket backend }
    FComHandle  : THandle;           { Win32 COM port handle }
    FComPort    : Integer;
    FBaudRate   : LongInt;
    FActive     : Boolean;
    FInBytes    : LongInt;
    FOutBytes   : LongInt;

    { Win32 serial helpers }
    function  SerialOpen(Port: Integer; Baud: LongInt): Boolean;
    procedure SerialClose;
    function  SerialSend(const Buf; Len: DWORD): DWORD;
    function  SerialRecv(var Buf; MaxLen: DWORD): DWORD;
    function  SerialReady: Boolean;
    function  SerialDCD: Boolean;
    procedure SerialDTR(State: Boolean);
    procedure SerialSetBaud(Baud: LongInt);
  public
    constructor Create;
    destructor  Destroy; override;

    { Socket mode init }
    function  InitSocket(Port: Word): Boolean;
    function  InitSocketFD(FD: LongInt): Boolean;
    function  InitSocketConnect(Port: Word): Boolean;

    { Serial mode init }
    function  InitSerial(Port: Integer; Baud: LongInt): Boolean;

    procedure Close;

    { FOSSIL-compatible I/O }
    function  SendByte(B: Byte): Boolean;
    function  SendStr(const S: String): LongInt;
    function  RecvByte: Integer;
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


{ ---------------------------------------------------------------
  Constructor / Destructor
  --------------------------------------------------------------- }

constructor TNetFosWL.Create;
begin
  inherited Create;
  FBackend := fbSocket;
  FSocket := TFossilSocket.Create;
  FComHandle := INVALID_HANDLE_VALUE;
  FComPort := 0;
  FBaudRate := 115200;
  FActive := False;
  FInBytes := 0;
  FOutBytes := 0;
end;

destructor TNetFosWL.Destroy;
begin
  Close;
  FSocket.Free;
  inherited Destroy;
end;

{ ---------------------------------------------------------------
  Socket mode init
  --------------------------------------------------------------- }

function TNetFosWL.InitSocket(Port: Word): Boolean;
begin
  FBackend := fbSocket;
  Result := FSocket.InitAccept(Port);
  FActive := Result;
end;

function TNetFosWL.InitSocketFD(FD: LongInt): Boolean;
begin
  FBackend := fbSocket;
  Result := FSocket.InitFromFD(FD);
  FActive := Result;
end;

function TNetFosWL.InitSocketConnect(Port: Word): Boolean;
begin
  FBackend := fbSocket;
  Result := FSocket.InitConnect(Port);
  FActive := Result;
end;

{ ---------------------------------------------------------------
  Serial mode init — Win32 CreateFile + DCB
  --------------------------------------------------------------- }

function TNetFosWL.InitSerial(Port: Integer; Baud: LongInt): Boolean;
begin
  FBackend := fbSerial;
  FComPort := Port;
  FBaudRate := Baud;
  Result := SerialOpen(Port, Baud);
  FActive := Result;
end;

function TNetFosWL.SerialOpen(Port: Integer; Baud: LongInt): Boolean;
var
  DevName: String;
  DCB: TDCB;
  Timeouts: TCommTimeouts;
begin
  Result := False;

  { COM10+ needs \\.\COM10 syntax }
  if Port >= 10 then
    DevName := '\\.\COM' + IntToStr(Port)
  else
    DevName := 'COM' + IntToStr(Port);

  FComHandle := CreateFile(PChar(DevName),
    GENERIC_READ or GENERIC_WRITE, 0, nil,
    OPEN_EXISTING, 0, 0);

  if FComHandle = INVALID_HANDLE_VALUE then Exit;

  { Configure port }
  FillChar(DCB, SizeOf(DCB), 0);
  DCB.DCBlength := SizeOf(DCB);
  if not GetCommState(FComHandle, DCB) then
  begin
    CloseHandle(FComHandle);
    FComHandle := INVALID_HANDLE_VALUE;
    Exit;
  end;

  DCB.BaudRate := Baud;
  DCB.ByteSize := 8;
  DCB.Parity := NOPARITY;
  DCB.StopBits := ONESTOPBIT;
  DCB.Flags := DCB.Flags or $01;  { fBinary = 1 }

  if not SetCommState(FComHandle, DCB) then
  begin
    CloseHandle(FComHandle);
    FComHandle := INVALID_HANDLE_VALUE;
    Exit;
  end;

  { Set timeouts — non-blocking read }
  Timeouts.ReadIntervalTimeout := MAXDWORD;
  Timeouts.ReadTotalTimeoutMultiplier := 0;
  Timeouts.ReadTotalTimeoutConstant := 10;  { 10ms timeout }
  Timeouts.WriteTotalTimeoutMultiplier := 0;
  Timeouts.WriteTotalTimeoutConstant := 1000;
  SetCommTimeouts(FComHandle, Timeouts);

  { Raise DTR }
  EscapeCommFunction(FComHandle, SETDTR);

  FBaudRate := Baud;
  Result := True;
end;

procedure TNetFosWL.SerialClose;
begin
  if FComHandle <> INVALID_HANDLE_VALUE then
  begin
    EscapeCommFunction(FComHandle, CLRDTR);
    CloseHandle(FComHandle);
  end;
  FComHandle := INVALID_HANDLE_VALUE;
end;

function TNetFosWL.SerialSend(const Buf; Len: DWORD): DWORD;
var Written: DWORD;
begin
  Result := 0;
  if FComHandle = INVALID_HANDLE_VALUE then Exit;
  if WriteFile(FComHandle, Buf, Len, Written, nil) then
    Result := Written;
end;

function TNetFosWL.SerialRecv(var Buf; MaxLen: DWORD): DWORD;
var BytesRead: DWORD;
begin
  Result := 0;
  if FComHandle = INVALID_HANDLE_VALUE then Exit;
  if ReadFile(FComHandle, Buf, MaxLen, BytesRead, nil) then
    Result := BytesRead;
end;

function TNetFosWL.SerialReady: Boolean;
var Errors: DWORD; Stat: TComStat;
begin
  Result := False;
  if FComHandle = INVALID_HANDLE_VALUE then Exit;
  ClearCommError(FComHandle, Errors, @Stat);
  Result := (Stat.cbInQue > 0);
end;

function TNetFosWL.SerialDCD: Boolean;
var Status: DWORD;
begin
  Result := False;
  if FComHandle = INVALID_HANDLE_VALUE then Exit;
  GetCommModemStatus(FComHandle, Status);
  Result := (Status and MS_RLSD_ON) <> 0;
end;

procedure TNetFosWL.SerialDTR(State: Boolean);
begin
  if FComHandle = INVALID_HANDLE_VALUE then Exit;
  if State then
    EscapeCommFunction(FComHandle, SETDTR)
  else
    EscapeCommFunction(FComHandle, CLRDTR);
end;

procedure TNetFosWL.SerialSetBaud(Baud: LongInt);
var DCB: TDCB;
begin
  if FComHandle = INVALID_HANDLE_VALUE then Exit;
  FillChar(DCB, SizeOf(DCB), 0);
  DCB.DCBlength := SizeOf(DCB);
  if GetCommState(FComHandle, DCB) then
  begin
    DCB.BaudRate := Baud;
    SetCommState(FComHandle, DCB);
    FBaudRate := Baud;
  end;
end;

{ ---------------------------------------------------------------
  Common I/O — dispatches to socket or serial backend
  --------------------------------------------------------------- }

procedure TNetFosWL.Close;
begin
  if FBackend = fbSocket then
    FSocket.Deinit
  else
    SerialClose;
  FActive := False;
end;

function TNetFosWL.SendByte(B: Byte): Boolean;
begin
  if not FActive then begin Result := False; Exit; end;
  if FBackend = fbSocket then
    Result := FSocket.SendByte(B)
  else
    Result := (SerialSend(B, 1) = 1);
  if Result then Inc(FOutBytes);
end;

function TNetFosWL.SendStr(const S: String): LongInt;
begin
  if not FActive then begin Result := 0; Exit; end;
  if FBackend = fbSocket then
    Result := FSocket.Send(S)
  else
    Result := SerialSend(S[1], Length(S));
  Inc(FOutBytes, Result);
end;

function TNetFosWL.RecvByte: Integer;
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

function TNetFosWL.RecvBuf(var Buf; MaxLen: LongInt): LongInt;
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

function TNetFosWL.RecvReady: Boolean;
begin
  if not FActive then begin Result := False; Exit; end;
  if FBackend = fbSocket then
    Result := FSocket.RecvReady
  else
    Result := SerialReady;
end;

function TNetFosWL.IsConnected: Boolean;
begin
  if not FActive then begin Result := False; Exit; end;
  if FBackend = fbSocket then
    Result := FSocket.Connected
  else
    Result := SerialDCD;
end;

procedure TNetFosWL.PurgeInput;
begin
  if not FActive then Exit;
  if FBackend = fbSocket then
    FSocket.PurgeInput
  else if FComHandle <> INVALID_HANDLE_VALUE then
    PurgeComm(FComHandle, PURGE_RXCLEAR);
end;

procedure TNetFosWL.PurgeOutput;
begin
  if not FActive then Exit;
  if FBackend = fbSocket then
    FSocket.Flush
  else if FComHandle <> INVALID_HANDLE_VALUE then
    PurgeComm(FComHandle, PURGE_TXCLEAR);
end;

procedure TNetFosWL.HangUp;
begin
  if not FActive then Exit;
  if FBackend = fbSocket then
    FSocket.HangUp
  else
    SerialDTR(False);
  FActive := False;
  CDokay := 0; CDup := 0;
end;

end.
