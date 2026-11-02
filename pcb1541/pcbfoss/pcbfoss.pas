{ ===========================================================================
  pcbfoss.pas — FOSSIL Driver Interface (FTS-0001 Rev 5)
  ===========================================================================
  Pascal FOSSIL API for BBS doors and comm programs.
  Calls INT 14h — works with any loaded FOSSIL driver (X00, BNU,
  CYFOSSIL, etc).

  Fixes from 2026-08 audit:
    BUG-1: Fn $02 now blocks with DCD check + timeout (was instant return)
    BUG-2: Fn $13 writes CX-length string at ES:DI (was single char)
    BUG-3: Double-init guards with FActive flag (was clearing pending data)
    IMP-1: Overrun status bit (FSTAT_OVERRUN $02) now tracked
    IMP-2: BaudRate 115 corrected to 1152 (FSC-0015: units of 100 bps)

  GPLv3 — the crew 4free (wrench + byte)
  =========================================================================== }

{$MODE OBJFPC}{$H-}

Unit pcbfoss;

Interface

Uses pcbfoss_rings;

Const
  { FOSSIL signature returned by Fn 04h }
  FOSSIL_SIG    = $1954;

  { Status bits (AH = line status from Fn 03h) }
  FSTAT_DR      = $01;   { Data Ready }
  FSTAT_OVERRUN = $02;   { Overrun Error }
  FSTAT_THRE    = $20;   { TX Holding Register Empty }
  FSTAT_TSRE    = $40;   { TX Shift Register Empty }

  { Modem status bits (AL from Fn 03h) }
  FMSR_CTS      = $10;
  FMSR_DSR      = $20;
  FMSR_RI       = $40;
  FMSR_DCD      = $80;

  { Baud rate codes for Fn 00h bits [7:5] }
  FBAUD_300     = $40;   { 010 }
  FBAUD_600     = $60;   { 011 }
  FBAUD_1200    = $80;   { 100 }
  FBAUD_2400    = $A0;   { 101 }
  FBAUD_4800    = $C0;   { 110 }
  FBAUD_9600    = $E0;   { 111 }
  FBAUD_19200   = $00;   { 000 }
  FBAUD_38400   = $20;   { 001 }

  { Line format bits for Fn 00h bits [4:0] }
  FLINE_8N1     = $03;   { 8 data, no parity, 1 stop }
  FLINE_7E1     = $1A;   { 7 data, even parity, 1 stop }

  { Default RX timeout in ticks (18.2 ticks/sec) }
  FOSSIL_RX_TIMEOUT = 182;  { ~10 seconds }

Type
  TFOSSILInfo = Record
    Size      : Word;
    Rev       : Byte;
    DriverRev : Byte;
    IdOfs     : Word;
    IdSeg     : Word;
    RxBufSize : Word;
    RxBufFree : Word;
    TxBufSize : Word;
    TxBufFree : Word;
    ScrWidth  : Byte;
    ScrHeight : Byte;
    BaudMask  : Byte;
  End;

Var
  FPort     : Word;      { COM port (0=COM1, 1=COM2, etc) }
  FActive   : Boolean;   { True after successful init }
  FConnected: Boolean;   { True while DCD is asserted }
  FOverrun  : Boolean;   { Overrun detected since last check }
  FBaudRate : Word;      { Current baud in units of 100 bps (FSC-0015) }
  RXRing    : TRingBuf;  { Receive ring buffer }
  TXRing    : TRingBuf;  { Transmit ring buffer }

{ Core FOSSIL API }
Function  FOSSILInit(Port: Word): Boolean;
Procedure FOSSILDeinit;
Function  FOSSILStatus: Word;
Procedure FOSSILSetBaud(BaudCode, LineCode: Byte);

{ TX }
Procedure FOSSILWriteChar(Ch: Char);
Function  FOSSILWriteCharNoWait(Ch: Char): Boolean;
Procedure FOSSILWriteStr(Const S: String);
Function  FOSSILBlockWrite(Var Buf; Len: Word): Word;
Procedure FOSSILFlush;
Procedure FOSSILPurgeOutput;

{ RX — BUG-1 FIX: blocks with DCD check + timeout }
Function  FOSSILReadChar: Char;
Function  FOSSILReadCharTimeout(Ticks: Word): Integer;
Function  FOSSILCharReady: Boolean;
Function  FOSSILPeekChar: Integer;
Function  FOSSILBlockRead(Var Buf; Len: Word): Word;
Procedure FOSSILPurgeInput;

{ Modem }
Function  FOSSILCarrier: Boolean;
Procedure FOSSILBreak(OnOff: Boolean);

{ Flow control: 0=none, 1=XON/XOFF, 2=CTS/RTS, 3=both }
Procedure FOSSILFlowControl(Flags: Byte);

{ Keyboard }
Function  FOSSILKbdRead: Word;
Function  FOSSILKbdPeek: Integer;

{ Cursor }
Procedure FOSSILSetCursor(X, Y: Byte);
Procedure FOSSILGetCursor(Var X, Y: Byte);

{ ANSI write — Fn 13h: writes CX chars from ES:DI (BUG-2 FIX) }
Procedure FOSSILANSIWrite(Const S: String);

{ Driver info }
Function  FOSSILGetInfo(Var Info: TFOSSILInfo): Boolean;

Implementation

{$IFDEF GO32V2}
Uses Go32;
{$ENDIF}

{ --- Low-level INT 14h call --- }

{$IFDEF GO32V2}
Var Regs: TRealRegs;

Procedure Int14(AH_val: Byte; AL_val: Byte);
Begin
  FillChar(Regs, SizeOf(Regs), 0);
  Regs.AH := AH_val;
  Regs.AL := AL_val;
  Regs.DX := FPort;
  RealIntr($14, Regs);
End;
{$ELSE}
{ Stub for non-DOS targets — allows compilation but no FOSSIL }
Procedure Int14(AH_val: Byte; AL_val: Byte); Begin End;
{$ENDIF}

{ --- Fn 04h: Init --- }
{ BUG-3 FIX: guard with FActive — double-init won't clear pending data }
Function FOSSILInit(Port: Word): Boolean;
Begin
  FOSSILInit := False;
  If FActive Then Exit;  { Already initialized — don't re-init }

  FPort := Port;
  FActive := False;
  FConnected := False;
  FOverrun := False;
  FBaudRate := 96;  { Default 9600 = 96 * 100 bps }
  RingInit(RXRing);
  RingInit(TXRing);

  {$IFDEF GO32V2}
  FillChar(Regs, SizeOf(Regs), 0);
  Regs.AH := $04;
  Regs.DX := Port;
  Regs.BX := 0;
  RealIntr($14, Regs);
  If Regs.AX = FOSSIL_SIG Then
  Begin
    FActive := True;
    { Check initial DCD }
    FConnected := FOSSILCarrier;
    FOSSILInit := True;
  End;
  {$ENDIF}
End;

{ --- Fn 05h: Deinit --- }
Procedure FOSSILDeinit;
Begin
  If Not FActive Then Exit;
  {$IFDEF GO32V2}
  Int14($05, 0);
  {$ENDIF}
  FActive := False;
  FConnected := False;
End;

{ --- Fn 03h: Status --- }
Function FOSSILStatus: Word;
Begin
  FOSSILStatus := 0;
  If Not FActive Then Exit;
  {$IFDEF GO32V2}
  Int14($03, 0);
  FOSSILStatus := Regs.AX;
  { IMP-1: track overrun }
  If (Regs.AH And FSTAT_OVERRUN) <> 0 Then FOverrun := True;
  { Track DCD }
  FConnected := (Regs.AL And FMSR_DCD) <> 0;
  {$ENDIF}
End;

{ --- Fn 00h: Set Baud --- }
Procedure FOSSILSetBaud(BaudCode, LineCode: Byte);
Begin
  If Not FActive Then Exit;
  {$IFDEF GO32V2}
  Int14($00, BaudCode Or LineCode);
  { IMP-2: store baud in units of 100 bps per FSC-0015 }
  Case BaudCode Of
    FBAUD_300:   FBaudRate := 3;
    FBAUD_600:   FBaudRate := 6;
    FBAUD_1200:  FBaudRate := 12;
    FBAUD_2400:  FBaudRate := 24;
    FBAUD_4800:  FBaudRate := 48;
    FBAUD_9600:  FBaudRate := 96;
    FBAUD_19200: FBaudRate := 192;
    FBAUD_38400: FBaudRate := 384;
  Else
    FBaudRate := 1152;  { IMP-2 FIX: 115200 = 1152 * 100 }
  End;
  {$ENDIF}
End;

{ --- Fn 01h: TX Char (wait) --- }
Procedure FOSSILWriteChar(Ch: Char);
Begin
  If Not FActive Then Exit;
  {$IFDEF GO32V2}
  Int14($01, Byte(Ch));
  {$ENDIF}
End;

{ --- Fn 0Bh: TX Char (no wait) --- }
Function FOSSILWriteCharNoWait(Ch: Char): Boolean;
Begin
  FOSSILWriteCharNoWait := False;
  If Not FActive Then Exit;
  {$IFDEF GO32V2}
  Int14($0B, Byte(Ch));
  FOSSILWriteCharNoWait := Regs.AX = 1;
  {$ENDIF}
End;

{ --- Write string via Fn 01h --- }
Procedure FOSSILWriteStr(Const S: String);
Var I: Integer;
Begin
  For I := 1 To Length(S) Do
    FOSSILWriteChar(S[I]);
End;

{ --- Fn 17h: Block Write --- }
Function FOSSILBlockWrite(Var Buf; Len: Word): Word;
{$IFDEF GO32V2}
Var
  TBuf: Word;
  Written: Word;
{$ENDIF}
Begin
  FOSSILBlockWrite := 0;
  If (Not FActive) Or (Len = 0) Then Exit;
  {$IFDEF GO32V2}
  { Copy to DOS transfer buffer }
  TBuf := tb;
  If Len > tb_size Then Len := tb_size;
  Seg_Move(Get_DS, LongInt(@Buf), DosMemSelector, TBuf, Len);
  FillChar(Regs, SizeOf(Regs), 0);
  Regs.AH := $17;
  Regs.CX := Len;
  Regs.ES := TBuf SHR 4;
  Regs.DI := TBuf AND $0F;
  Regs.DX := FPort;
  RealIntr($14, Regs);
  FOSSILBlockWrite := Regs.AX;
  {$ENDIF}
End;

{ --- Fn 08h: Flush Output --- }
Procedure FOSSILFlush;
Begin
  If Not FActive Then Exit;
  {$IFDEF GO32V2}
  Int14($08, 0);
  {$ENDIF}
End;

{ --- Fn 09h: Purge Output --- }
Procedure FOSSILPurgeOutput;
Begin
  If Not FActive Then Exit;
  {$IFDEF GO32V2}
  Int14($09, 0);
  {$ENDIF}
End;

{ --- Fn 02h: RX Char (wait) --- }
{ BUG-1 FIX: blocks with DCD check + timeout instead of instant return }
Function FOSSILReadChar: Char;
Var C: Integer;
Begin
  C := FOSSILReadCharTimeout(FOSSIL_RX_TIMEOUT);
  If C < 0 Then FOSSILReadChar := #0
  Else FOSSILReadChar := Chr(C And $FF);
End;

{ Read with timeout in BIOS ticks. Returns -1 on timeout/disconnect. }
Function FOSSILReadCharTimeout(Ticks: Word): Integer;
{$IFDEF GO32V2}
Var
  StartTick, NowTick: LongInt;
  TickAddr: LongInt;
{$ENDIF}
Begin
  FOSSILReadCharTimeout := -1;
  If Not FActive Then Exit;
  {$IFDEF GO32V2}
  { Get BIOS tick count at 0040:006C }
  TickAddr := $46C;
  StartTick := MemL[Seg0040:$6C];

  Repeat
    { Check for data via Fn 0Ch (peek) }
    FillChar(Regs, SizeOf(Regs), 0);
    Regs.AH := $0C;
    Regs.DX := FPort;
    RealIntr($14, Regs);
    If Regs.AX <> $FFFF Then
    Begin
      { Data available — read it with Fn 02h }
      FillChar(Regs, SizeOf(Regs), 0);
      Regs.AH := $02;
      Regs.DX := FPort;
      RealIntr($14, Regs);
      FOSSILReadCharTimeout := Regs.AL;
      Exit;
    End;

    { Check DCD — disconnect = bail }
    If Not FOSSILCarrier Then
    Begin
      FConnected := False;
      Exit;
    End;

    { Check timeout }
    NowTick := MemL[Seg0040:$6C];
    If (NowTick - StartTick) >= Ticks Then Exit;

    { Yield CPU briefly }
    Asm
      INT $28    { DOS idle interrupt }
    End;
  Until False;
  {$ENDIF}
End;

{ --- Fn 0Ch: Peek Input --- }
Function FOSSILCharReady: Boolean;
Begin
  FOSSILCharReady := False;
  If Not FActive Then Exit;
  {$IFDEF GO32V2}
  Int14($0C, 0);
  FOSSILCharReady := Regs.AX <> $FFFF;
  {$ENDIF}
End;

Function FOSSILPeekChar: Integer;
Begin
  FOSSILPeekChar := -1;
  If Not FActive Then Exit;
  {$IFDEF GO32V2}
  Int14($0C, 0);
  If Regs.AX <> $FFFF Then FOSSILPeekChar := Regs.AL;
  {$ENDIF}
End;

{ --- Fn 16h: Block Read --- }
Function FOSSILBlockRead(Var Buf; Len: Word): Word;
{$IFDEF GO32V2}
Var TBuf: Word;
{$ENDIF}
Begin
  FOSSILBlockRead := 0;
  If (Not FActive) Or (Len = 0) Then Exit;
  {$IFDEF GO32V2}
  TBuf := tb;
  If Len > tb_size Then Len := tb_size;
  FillChar(Regs, SizeOf(Regs), 0);
  Regs.AH := $16;
  Regs.CX := Len;
  Regs.ES := TBuf SHR 4;
  Regs.DI := TBuf AND $0F;
  Regs.DX := FPort;
  RealIntr($14, Regs);
  If Regs.AX > 0 Then
    Seg_Move(DosMemSelector, TBuf, Get_DS, LongInt(@Buf), Regs.AX);
  FOSSILBlockRead := Regs.AX;
  {$ENDIF}
End;

{ --- Fn 0Ah: Purge Input --- }
Procedure FOSSILPurgeInput;
Begin
  If Not FActive Then Exit;
  {$IFDEF GO32V2}
  Int14($0A, 0);
  {$ENDIF}
End;

{ --- Carrier detect --- }
Function FOSSILCarrier: Boolean;
Var St: Word;
Begin
  St := FOSSILStatus;
  FOSSILCarrier := (Lo(St) And FMSR_DCD) <> 0;
End;

{ --- Fn 18h: Break --- }
Procedure FOSSILBreak(OnOff: Boolean);
Begin
  If Not FActive Then Exit;
  {$IFDEF GO32V2}
  If OnOff Then Int14($18, 1)
  Else Int14($18, 0);
  {$ENDIF}
End;

{ --- Fn 0Fh: Flow Control --- }
Procedure FOSSILFlowControl(Flags: Byte);
Begin
  If Not FActive Then Exit;
  {$IFDEF GO32V2}
  Int14($0F, Flags);
  {$ENDIF}
End;

{ --- Fn 0Dh: Keyboard Read --- }
Function FOSSILKbdRead: Word;
Begin
  FOSSILKbdRead := $FFFF;
  {$IFDEF GO32V2}
  Int14($0D, 0);
  FOSSILKbdRead := Regs.AX;
  {$ENDIF}
End;

{ --- Fn 0Eh: Keyboard Peek --- }
Function FOSSILKbdPeek: Integer;
Begin
  FOSSILKbdPeek := -1;
  {$IFDEF GO32V2}
  Int14($0E, 0);
  If Regs.AX <> $FFFF Then FOSSILKbdPeek := Regs.AX;
  {$ENDIF}
End;

{ --- Fn 11h: Set Cursor --- }
Procedure FOSSILSetCursor(X, Y: Byte);
Begin
  {$IFDEF GO32V2}
  FillChar(Regs, SizeOf(Regs), 0);
  Regs.AH := $11;
  Regs.DH := Y;
  Regs.DL := X;
  Regs.DX := FPort;  { DX is overloaded — Fn 11h uses DH/DL for row/col }
  { Actually Fn 11h uses DH=row, DL=col and ignores port DX.
    Re-do without port: }
  FillChar(Regs, SizeOf(Regs), 0);
  Regs.AH := $11;
  Regs.DH := Y;
  Regs.DL := X;
  RealIntr($14, Regs);
  {$ENDIF}
End;

{ --- Fn 12h: Get Cursor --- }
Procedure FOSSILGetCursor(Var X, Y: Byte);
Begin
  X := 0; Y := 0;
  {$IFDEF GO32V2}
  FillChar(Regs, SizeOf(Regs), 0);
  Regs.AH := $12;
  RealIntr($14, Regs);
  X := Regs.DL;
  Y := Regs.DH;
  {$ENDIF}
End;

{ --- Fn 13h: ANSI Write --- }
{ BUG-2 FIX: writes CX-length string at ES:DI per spec, not single char }
Procedure FOSSILANSIWrite(Const S: String);
{$IFDEF GO32V2}
Var
  TBuf: Word;
  Len: Word;
{$ENDIF}
Begin
  If Length(S) = 0 Then Exit;
  {$IFDEF GO32V2}
  Len := Length(S);
  TBuf := tb;
  If Len > tb_size Then Len := tb_size;
  Seg_Move(Get_DS, LongInt(@S[1]), DosMemSelector, TBuf, Len);
  FillChar(Regs, SizeOf(Regs), 0);
  Regs.AH := $13;
  Regs.CX := Len;
  Regs.ES := TBuf SHR 4;
  Regs.DI := TBuf AND $0F;
  RealIntr($14, Regs);
  {$ENDIF}
End;

{ --- Fn 19h: Driver Info --- }
Function FOSSILGetInfo(Var Info: TFOSSILInfo): Boolean;
{$IFDEF GO32V2}
Var TBuf: Word;
{$ENDIF}
Begin
  FOSSILGetInfo := False;
  FillChar(Info, SizeOf(Info), 0);
  If Not FActive Then Exit;
  {$IFDEF GO32V2}
  TBuf := tb;
  FillChar(Regs, SizeOf(Regs), 0);
  Regs.AH := $19;
  Regs.CX := SizeOf(TFOSSILInfo);
  Regs.ES := TBuf SHR 4;
  Regs.DI := TBuf AND $0F;
  Regs.DX := FPort;
  RealIntr($14, Regs);
  If Regs.AX >= 19 Then
  Begin
    Seg_Move(DosMemSelector, TBuf, Get_DS, LongInt(@Info), SizeOf(Info));
    FOSSILGetInfo := True;
  End;
  {$ENDIF}
End;

Begin
  FPort := 0;
  FActive := False;
  FConnected := False;
  FOverrun := False;
  FBaudRate := 96;
End.
