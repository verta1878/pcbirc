Unit m_fossil_socket;

// ====================================================================
// m_fossil_socket -- Socket backend for TFossil
// ====================================================================
//
// Third backend for TFossil (alongside fbSerial and fbInt14).
// Used when netmodem2irc passes a connected socket fd to the BBS
// instead of a COM port or FOSSIL driver.
//
// Platforms: Linux, FreeBSD, macOS, Windows
//
// Three init modes (wrench's plan):
//   InitFromFD(fd)    -- fd passed from netmodem2irc via env/arg
//   InitAccept(port)  -- listen on localhost, accept one connection
//   InitConnect(port) -- connect to localhost port
//
// GPLv3 -- The Crew: verta1878, sysop/0, evga, kiddo, wrench
// ====================================================================

{$IFDEF FPC}{$MODE OBJFPC}{$H+}{$ENDIF}

Interface

Uses
{$IFDEF WINDOWS}
  WinSock;
{$ELSE}
  BaseUnix, Sockets;
{$ENDIF}

Type
  TFossilSocket = Class
  Private
    FFD       : LongInt;
    FActive   : Boolean;
    FInBytes  : LongInt;
    FOutBytes : LongInt;
  Public
    Constructor Create;
    Destructor  Destroy; Override;

    Function  InitFromFD (FD: LongInt): Boolean;
    Function  InitAccept (Port: Word): Boolean;
    Function  InitConnect (Port: Word): Boolean;
    Procedure Deinit;

    Function  Send (Const S: String): LongInt;
    Function  SendByte (B: Byte): Boolean;
    Function  Recv (MaxLen: LongInt): String;
    Function  RecvReady: Boolean;
    Function  Connected: Boolean;

    Procedure Flush;
    Procedure PurgeInput;
    Procedure HangUp;

    Property FD       : LongInt Read FFD;
    Property Active   : Boolean Read FActive;
    Property InBytes  : LongInt Read FInBytes;
    Property OutBytes : LongInt Read FOutBytes;
  End;

Implementation

Constructor TFossilSocket.Create;
Begin
  Inherited Create;
  FFD := -1; FActive := False;
  FInBytes := 0; FOutBytes := 0;
End;

Destructor TFossilSocket.Destroy;
Begin
  If FActive Then Deinit;
  Inherited Destroy;
End;

Function TFossilSocket.InitFromFD (FD: LongInt): Boolean;
Begin
  FFD := FD; FActive := (FFD >= 0); Result := FActive;
End;

Function TFossilSocket.InitAccept (Port: Word): Boolean;
Var
  ListenFD : LongInt;
  Addr     : TInetSockAddr;
  AddrLen  : LongInt;
Begin
  Result := False;
  ListenFD := fpSocket(AF_INET, SOCK_STREAM, 0);
  If ListenFD < 0 Then Exit;
  FillChar(Addr, SizeOf(Addr), 0);
  Addr.sin_family := AF_INET;
  Addr.sin_port := htons(Port);
  Addr.sin_addr.s_addr := htonl($7F000001);
  If fpBind(ListenFD, @Addr, SizeOf(Addr)) < 0 Then Begin CloseSocket(ListenFD); Exit; End;
  If fpListen(ListenFD, 1) < 0 Then Begin CloseSocket(ListenFD); Exit; End;
  AddrLen := SizeOf(Addr);
  FFD := fpAccept(ListenFD, @Addr, @AddrLen);
  CloseSocket(ListenFD);
  FActive := (FFD >= 0); Result := FActive;
End;

Function TFossilSocket.InitConnect (Port: Word): Boolean;
Var Addr : TInetSockAddr;
Begin
  Result := False;
  FFD := fpSocket(AF_INET, SOCK_STREAM, 0);
  If FFD < 0 Then Exit;
  FillChar(Addr, SizeOf(Addr), 0);
  Addr.sin_family := AF_INET;
  Addr.sin_port := htons(Port);
  Addr.sin_addr.s_addr := htonl($7F000001);
  If fpConnect(FFD, @Addr, SizeOf(Addr)) < 0 Then Begin CloseSocket(FFD); FFD := -1; Exit; End;
  FActive := True; Result := True;
End;

Procedure TFossilSocket.Deinit;
Begin
  If FFD >= 0 Then CloseSocket(FFD);
  FFD := -1; FActive := False;
End;

Function TFossilSocket.Send (Const S: String): LongInt;
Begin
  Result := 0;
  If (Not FActive) or (Length(S) = 0) Then Exit;
  Result := fpSend(FFD, @S[1], Length(S), 0);
  If Result < 0 Then Begin FActive := False; Result := 0; End
  Else Inc(FOutBytes, Result);
End;

Function TFossilSocket.SendByte (B: Byte): Boolean;
Begin
  Result := False;
  If Not FActive Then Exit;
  If fpSend(FFD, @B, 1, 0) = 1 Then Begin Inc(FOutBytes); Result := True; End
  Else FActive := False;
End;

Function TFossilSocket.Recv (MaxLen: LongInt): String;
Var Buf : Array[0..4095] Of Byte; N : LongInt;
Begin
  Result := '';
  If Not FActive Then Exit;
  If MaxLen > 4096 Then MaxLen := 4096;
  N := fpRecv(FFD, @Buf[0], MaxLen, 0);
  If N > 0 Then Begin SetLength(Result, N); Move(Buf[0], Result[1], N); Inc(FInBytes, N); End
  Else If N = 0 Then FActive := False;
End;

Function TFossilSocket.RecvReady: Boolean;
Var FDS : TFDSet; TV : TTimeVal;
Begin
  Result := False;
  If Not FActive Then Exit;
  fpFD_ZERO(FDS); fpFD_SET(FFD, FDS);
  TV.tv_sec := 0; TV.tv_usec := 10000; { 10ms yield -- no CPU hog }
  Result := (fpSelect(FFD + 1, @FDS, nil, nil, @TV) > 0);
End;

Function TFossilSocket.Connected: Boolean;
Var Buf : Byte; N : LongInt;
Begin
  Result := FActive;
  If Not FActive Then Exit;
  N := fpRecv(FFD, @Buf, 1, MSG_PEEK or MSG_DONTWAIT);
  If N = 0 Then Begin FActive := False; Result := False; End;
End;

Procedure TFossilSocket.Flush;
Begin { TCP handles flushing } End;

Procedure TFossilSocket.PurgeInput;
Var Buf : Array[0..4095] Of Byte;
Begin
  If Not FActive Then Exit;
  While RecvReady Do fpRecv(FFD, @Buf[0], 4096, 0);
End;

Procedure TFossilSocket.HangUp;
Begin
  If Not FActive Then Exit;
  fpShutdown(FFD, 1); { SHUT_WR -- simulate DTR drop }
  FActive := False;
End;

End.
