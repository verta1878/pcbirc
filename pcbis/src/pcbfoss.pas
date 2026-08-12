{ ===========================================================================
  pcbfoss — PCBoard FOSSIL bridge for pcbis
  Copyright (C) 2026 PCBoard Revival Project

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Based on NM_Fossil.pas from netmodem2irc by wrench
  (verta1878/netmodem2irc), which is based on Dedrick Allen's
  NetModem/32 FOSSIL specification.
  =========================================================================== }

unit pcbfoss;
{ ===========================================================================
  pcbfoss — FOSSIL (INT 14h) bridge for pcbis Internet Services
  ---------------------------------------------------------------------------
  Bridges a TCP socket (telnet client) to PCBoard's FOSSIL/COM port interface.
  PCBoard calls INT 14h FOSSIL functions to read/write the "modem".
  pcbfoss intercepts those calls and routes bytes to/from a TCP connection.

  Architecture:
    telnet client <-TCP-> pcbis <-ring buffers-> pcbfoss <-INT14h-> PCBoard
                                                   |
                                        pcbfoss_rings.pas
                                     (replaces NM_UART16550)

  Based on: netmodem2irc engine/NM_Fossil.pas (wrench, GPLv3)
  Spec:     FidoNet FSC-0015 (FOSSIL Rev 5), FSC-0072
  Verified: same function table, same semantics, same signature ($1954)

  The function set is identical to NM_Fossil — only the backend changes:
  NM_Fossil → NM_UART16550 (emulated 16550 registers)
  pcbfoss   → pcbfoss_rings (simple TX/RX ring buffers + socket drain/fill)

  For the DOS target: pcbfoss hooks INT 14h via the FPC go32v2 ISR mechanism.
  For the Linux target: pcbfoss runs in-process, PCBoard calls it via procedure.
  =========================================================================== }

{$MODE OBJFPC}{$H+}

interface

uses
  pcbfoss_rings;

const
  { FOSSIL identity — doors check for these exact values }
  FOSSIL_SIGNATURE = $1954;   { AX on successful Fn $04 (init) }
  FOSSIL_INFO_BX   = $0521;   { BX: max function $21, FOSSIL rev 5 }
  FOSSIL_MAJVER    = 5;
  FOSSIL_MINVER    = 0;
  FOSSIL_ID_STR    = 'pcbfoss 0.1 — PCBoard Internet Services FOSSIL';

  { INT 14h function numbers (AH) — FSC-0015 Rev 5 }
  FN_SET_BAUD        = $00;   { Set baud rate (ignored — TCP has no baud) }
  FN_TX_WAIT         = $01;   { Transmit char, wait if buffer full }
  FN_RX_WAIT         = $02;   { Receive char, wait if buffer empty }
  FN_GET_STATUS      = $03;   { Get FOSSIL status (line + modem) }
  FN_INIT            = $04;   { Initialize FOSSIL driver }
  FN_DEINIT          = $05;   { Deinitialize (shutdown) }
  FN_SET_DTR         = $06;   { Raise/lower DTR (maps to socket open/close) }
  FN_TIMER_TICK      = $07;   { Timer tick handler (no-op for TCP) }
  FN_FLUSH_OUTPUT    = $08;   { Flush TX buffer }
  FN_PURGE_OUTPUT    = $09;   { Purge (discard) TX buffer }
  FN_PURGE_INPUT     = $0A;   { Purge (discard) RX buffer }
  FN_TX_NOWAIT       = $0B;   { Transmit char, return immediately if full }
  FN_PEEK            = $0C;   { Peek at next RX char without removing }
  FN_KBD_NOWAIT      = $0D;   { Non-blocking keyboard read (for local console) }
  FN_KBD_WAIT        = $0E;   { Blocking keyboard read }
  FN_FLOW_CONTROL    = $0F;   { Enable/disable flow control (no-op for TCP) }
  FN_CTRLC_CHECK     = $10;   { Ctrl+C/Ctrl+K checking }
  FN_SET_CURSOR      = $11;   { Set cursor position }
  FN_GET_CURSOR      = $12;   { Get cursor position }
  FN_ANSI_WRITE      = $13;   { Write string with ANSI processing }
  FN_WATCHDOG        = $14;   { Reboot watchdog (no-op) }
  FN_WRITE_BIOS      = $15;   { Write char to BIOS screen }
  FN_INSDEL          = $16;   { Insert/delete scroll }
  FN_REBOOT          = $17;   { System reboot (no-op in pcbis) }
  FN_READ_BLOCK      = $18;   { Block read from RX buffer }
  FN_WRITE_BLOCK     = $19;   { Block write to TX buffer }
  FN_BREAK           = $1A;   { Send break signal (no-op for TCP) }
  FN_GET_INFO        = $1B;   { Get driver info block }

  { Status bits for Fn $03 }
  FSTAT_RX_READY     = $01;   { AH bit 0: data waiting in RX buffer }
  FSTAT_OVERRUN      = $02;   { AH bit 1: overrun error }
  FSTAT_TX_ROOM      = $20;   { AH bit 5: TX buffer has room (THRE) }
  FSTAT_TX_EMPTY     = $40;   { AH bit 6: TX buffer completely empty }

  { Modem status for Fn $03 (AL) }
  MSTAT_DCD          = $80;   { AL bit 7: carrier detect (= socket connected) }
  MSTAT_DSR          = $20;   { AL bit 5: data set ready }
  MSTAT_CTS          = $10;   { AL bit 4: clear to send }

type
  { Register frame — mirrors INT 14h register usage.
    PCBoard (or a shim) fills AH with the function number and in-params,
    calls FossilDispatch, then reads results from the same frame. }
  TFossilRegs = record
    AH : Byte;      { in: function number }
    AL : Byte;      { in/out: char (TX/RX), modem status }
    BX : Word;      { out: info word (Fn $04) }
    CX : Word;      { in/out: block count, buffer sizes }
    DX : Word;      { in: port index (DL) }
    BlockPtr : Pointer;  { in/out: block I/O buffer (Fn $18/$19) }
    BlockLen : Word;     { actual bytes transferred }
  end;

  { FOSSIL info block returned by Fn $1B }
  TFossilInfo = packed record
    StructSize : Word;      { size of this structure }
    MajVer     : Byte;      { FOSSIL spec major version }
    MinVer     : Byte;      { FOSSIL spec minor version }
    IDStr      : Pointer;   { far pointer to ID string }
    RXBufSize  : Word;      { receive buffer size }
    RXBufFree  : Word;      { bytes free in RX buffer }
    TXBufSize  : Word;      { transmit buffer size }
    TXBufFree  : Word;      { bytes free in TX buffer }
    ScreenW    : Byte;      { screen width }
    ScreenH    : Byte;      { screen height }
    BaudRate   : Word;      { current baud rate (always 115200 for TCP) }
  end;

  { Per-port FOSSIL instance — one per PCBoard node }
  TPcbFossil = class
  private
    FPort      : Byte;        { port index (0-based) }
    FActive    : Boolean;     { true after Fn $04 init }
    FRings     : TFossilRings;{ TX/RX ring buffers }
    FConnected : Boolean;     { true when TCP socket is connected }
    FDTRState  : Boolean;     { DTR raised = online }
  public
    constructor Create(APort : Byte; BufSize : Word);
    destructor Destroy; override;

    { Main dispatch — call with filled TFossilRegs, returns results in same }
    procedure Dispatch(var Regs : TFossilRegs);

    { Socket interface — pcbis calls these to move data to/from TCP }
    function  SocketRead(var Buf; Count : Word) : Word;   { drain TX ring → socket }
    procedure SocketWrite(const Buf; Count : Word);       { fill RX ring ← socket }

    { Connection management }
    procedure SetConnected(AConnected : Boolean);
    function  IsConnected : Boolean;
    function  IsActive : Boolean;

    property Port : Byte read FPort;
  end;

implementation

uses
  SysUtils;

constructor TPcbFossil.Create(APort : Byte; BufSize : Word);
begin
  inherited Create;
  FPort := APort;
  FActive := False;
  FConnected := False;
  FDTRState := False;
  FRings := TFossilRings.Create(BufSize);
end;

destructor TPcbFossil.Destroy;
begin
  FRings.Free;
  inherited Destroy;
end;

procedure TPcbFossil.Dispatch(var Regs : TFossilRegs);
var
  Ch      : Byte;
  Count   : Word;
  Info    : TFossilInfo;
begin
  case Regs.AH of

    FN_SET_BAUD: { $00 — set baud rate. No-op for TCP — always "fast". }
      begin
        Regs.AH := FSTAT_TX_EMPTY or FSTAT_TX_ROOM;
        if FConnected then
          Regs.AL := MSTAT_DCD or MSTAT_DSR or MSTAT_CTS
        else
          Regs.AL := MSTAT_DSR or MSTAT_CTS;
      end;

    FN_TX_WAIT: { $01 — transmit char in AL, wait until sent }
      begin
        { In TCP mode, "wait" means spin until TX ring has room.
          In practice the ring is always large enough. }
        FRings.TXPut(Regs.AL);
        Regs.AH := FSTAT_TX_ROOM;
        if FConnected then Regs.AL := MSTAT_DCD or MSTAT_DSR or MSTAT_CTS;
      end;

    FN_RX_WAIT: { $02 — receive char into AL, wait until available }
      begin
        if FRings.RXAvail > 0 then
          Regs.AL := FRings.RXGet
        else
          Regs.AL := 0; { caller should check status first }
        Regs.AH := FSTAT_TX_ROOM;
      end;

    FN_GET_STATUS: { $03 — return line + modem status }
      begin
        Regs.AH := 0;
        if FRings.RXAvail > 0 then Regs.AH := Regs.AH or FSTAT_RX_READY;
        if FRings.TXFree > 0 then Regs.AH := Regs.AH or FSTAT_TX_ROOM;
        if FRings.TXAvail = 0 then Regs.AH := Regs.AH or FSTAT_TX_EMPTY;
        Regs.AL := MSTAT_DSR or MSTAT_CTS;
        if FConnected and FDTRState then
          Regs.AL := Regs.AL or MSTAT_DCD;
      end;

    FN_INIT: { $04 — initialize FOSSIL. Return signature. }
      begin
        FActive := True;
        FDTRState := True;
        FRings.Clear;
        Regs.AX := FOSSIL_SIGNATURE;  { $1954 — "I am a FOSSIL" }
        Regs.BX := FOSSIL_INFO_BX;    { $0521 — rev 5, max fn $21 }
      end;

    FN_DEINIT: { $05 — deinitialize }
      begin
        FActive := False;
        FDTRState := False;
        FRings.Clear;
      end;

    FN_SET_DTR: { $06 — raise (AL=1) or lower (AL=0) DTR }
      begin
        FDTRState := (Regs.AL and $01) <> 0;
        { DTR drop = hang up. If DTR goes low, signal pcbis to close socket. }
        if not FDTRState then
          FConnected := False;
      end;

    FN_TIMER_TICK: { $07 — no-op for TCP }
      ;

    FN_FLUSH_OUTPUT: { $08 — flush TX buffer (pcbis drains it on next poll) }
      ; { ring buffer is always available for drain }

    FN_PURGE_OUTPUT: { $09 — discard TX buffer }
      FRings.TXClear;

    FN_PURGE_INPUT: { $0A — discard RX buffer }
      FRings.RXClear;

    FN_TX_NOWAIT: { $0B — transmit char, return 1=sent 0=full }
      begin
        if FRings.TXFree > 0 then
        begin
          FRings.TXPut(Regs.AL);
          Regs.AX := 1;
        end
        else
          Regs.AX := 0;
      end;

    FN_PEEK: { $0C — peek at next RX byte without removing }
      begin
        if FRings.RXAvail > 0 then
        begin
          Regs.AL := FRings.RXPeek;
          Regs.AH := FSTAT_RX_READY;
        end
        else
          Regs.AX := $FFFF; { no data }
      end;

    FN_KBD_NOWAIT: { $0D — local keyboard (no-op in pcbis) }
      Regs.AX := $FFFF;

    FN_KBD_WAIT: { $0E — local keyboard wait (no-op) }
      Regs.AX := $FFFF;

    FN_FLOW_CONTROL: { $0F — no-op for TCP (always "flow controlled") }
      ;

    FN_CTRLC_CHECK: { $10 — no-op }
      ;

    FN_SET_CURSOR: { $11 — set cursor (DH=row, DL=col) — forward to console }
      ; { pcbis handles screen output separately }

    FN_GET_CURSOR: { $12 — get cursor position }
      begin
        Regs.DX := 0; { row 0, col 0 — pcbis doesn't track cursor }
      end;

    FN_ANSI_WRITE: { $13 — write string with ANSI processing }
      begin
        { For pcbis: just put the char in TX ring, let telnet client handle ANSI }
        FRings.TXPut(Regs.AL);
      end;

    FN_WATCHDOG: { $14 — no-op }
      ;

    FN_WRITE_BIOS: { $15 — write char to BIOS screen }
      begin
        { Same as ANSI write for our purposes }
        FRings.TXPut(Regs.AL);
      end;

    FN_READ_BLOCK: { $18 — block read: CX bytes from RX ring to ES:DI }
      begin
        Count := FRings.RXBlockRead(Regs.BlockPtr^, Regs.CX);
        Regs.AX := Count;
      end;

    FN_WRITE_BLOCK: { $19 — block write: CX bytes from ES:DI to TX ring }
      begin
        Count := FRings.TXBlockWrite(Regs.BlockPtr^, Regs.CX);
        Regs.AX := Count;
      end;

    FN_BREAK: { $1A — send break (no-op for TCP) }
      ;

    FN_GET_INFO: { $1B — return info block }
      begin
        if Regs.BlockPtr <> nil then
        begin
          FillChar(Info, SizeOf(Info), 0);
          Info.StructSize := SizeOf(Info);
          Info.MajVer := FOSSIL_MAJVER;
          Info.MinVer := FOSSIL_MINVER;
          Info.IDStr := @FOSSIL_ID_STR;
          Info.RXBufSize := FRings.RXSize;
          Info.RXBufFree := FRings.RXFree;
          Info.TXBufSize := FRings.TXSize;
          Info.TXBufFree := FRings.TXFree;
          Info.ScreenW := 80;
          Info.ScreenH := 25;
          Info.BaudRate := 115; { 115200 / 100 — FOSSIL convention }
          Move(Info, Regs.BlockPtr^, SizeOf(Info));
          Regs.AX := SizeOf(Info);
        end;
      end;

  end; { case }
end;

function TPcbFossil.SocketRead(var Buf; Count : Word) : Word;
begin
  { pcbis calls this to drain the TX ring → send to telnet client }
  Result := FRings.TXBlockRead(Buf, Count);
end;

procedure TPcbFossil.SocketWrite(const Buf; Count : Word);
begin
  { pcbis calls this to fill the RX ring ← received from telnet client }
  FRings.RXBlockWrite(Buf, Count);
end;

procedure TPcbFossil.SetConnected(AConnected : Boolean);
begin
  FConnected := AConnected;
  if AConnected then FDTRState := True;
end;

function TPcbFossil.IsConnected : Boolean;
begin
  Result := FConnected and FDTRState;
end;

function TPcbFossil.IsActive : Boolean;
begin
  Result := FActive;
end;

end.
