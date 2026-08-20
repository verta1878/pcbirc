{ ===========================================================================
  pcbfoss_rings — Ring buffers for pcbfoss FOSSIL bridge
  Copyright (C) 2026 PCBoard Revival Project

  Based on NM_UART16550.pas from netmodem2irc (wrench, GPLv3).
  Stripped to just the ring buffer logic — no 16550 register emulation.

  The UART emulation in NM_UART16550 is overkill for pcbfoss — PCBoard
  doesn't touch UART registers directly, it goes through FOSSIL INT 14h.
  So we only need the ring buffers that sit behind the FOSSIL dispatch.
  =========================================================================== }

unit pcbfoss_rings;

{$MODE OBJFPC}{$H+}

interface

const
  DEFAULT_RING_SIZE = 8192;  { 8K per ring — plenty for telnet traffic }

type
  TFossilRings = class
  private
    FTXBuf  : array of Byte;
    FTXHead : Word;
    FTXTail : Word;
    FTXCount: Word;
    FTXCap  : Word;

    FRXBuf  : array of Byte;
    FRXHead : Word;
    FRXTail : Word;
    FRXCount: Word;
    FRXCap  : Word;
  public
    constructor Create(BufSize : Word = DEFAULT_RING_SIZE);
    destructor Destroy; override;

    { TX ring — PCBoard writes here via FOSSIL, pcbis drains to socket }
    procedure TXPut(B : Byte);
    function  TXGet : Byte;
    function  TXAvail : Word;   { bytes waiting to be sent }
    function  TXFree : Word;    { space available for writing }
    function  TXSize : Word;    { total capacity }
    procedure TXClear;
    function  TXBlockRead(var Buf; Count : Word) : Word;

    { RX ring — pcbis fills from socket, PCBoard reads via FOSSIL }
    procedure RXPut(B : Byte);
    function  RXGet : Byte;
    function  RXPeek : Byte;    { look without consuming }
    function  RXAvail : Word;   { bytes waiting to be read }
    function  RXFree : Word;    { space available }
    function  RXSize : Word;    { total capacity }
    procedure RXClear;
    function  RXBlockRead(var Buf; Count : Word) : Word;
    function  RXBlockWrite(const Buf; Count : Word) : Word;
    function  TXBlockWrite(const Buf; Count : Word) : Word;

    { Clear both rings }
    procedure Clear;
  end;

implementation

constructor TFossilRings.Create(BufSize : Word);
begin
  inherited Create;
  FTXCap := BufSize;
  FRXCap := BufSize;
  SetLength(FTXBuf, FTXCap);
  SetLength(FRXBuf, FRXCap);
  Clear;
end;

destructor TFossilRings.Destroy;
begin
  SetLength(FTXBuf, 0);
  SetLength(FRXBuf, 0);
  inherited Destroy;
end;

procedure TFossilRings.Clear;
begin
  TXClear;
  RXClear;
end;

{ === TX Ring === }

procedure TFossilRings.TXPut(B : Byte);
begin
  if FTXCount >= FTXCap then Exit; { full — drop (shouldn't happen with flow control) }
  FTXBuf[FTXHead] := B;
  FTXHead := (FTXHead + 1) mod FTXCap;
  Inc(FTXCount);
end;

function TFossilRings.TXGet : Byte;
begin
  if FTXCount = 0 then begin Result := 0; Exit; end;
  Result := FTXBuf[FTXTail];
  FTXTail := (FTXTail + 1) mod FTXCap;
  Dec(FTXCount);
end;

function TFossilRings.TXAvail : Word;
begin
  Result := FTXCount;
end;

function TFossilRings.TXFree : Word;
begin
  Result := FTXCap - FTXCount;
end;

function TFossilRings.TXSize : Word;
begin
  Result := FTXCap;
end;

procedure TFossilRings.TXClear;
begin
  FTXHead := 0;
  FTXTail := 0;
  FTXCount := 0;
end;

function TFossilRings.TXBlockRead(var Buf; Count : Word) : Word;
var Chunk, Avail : Word;
begin
  { BUG-4 fix: two-phase memcpy instead of per-byte loop }
  Avail := FTXCount;
  if Count > Avail then Count := Avail;
  Result := Count;
  { Phase 1: tail to end of buffer }
  Chunk := FTXCap - FTXTail;
  if Chunk > Count then Chunk := Count;
  Move(FTXBuf[FTXTail], Buf, Chunk);
  FTXTail := (FTXTail + Chunk) mod FTXCap;
  Dec(FTXCount, Chunk);
  Dec(Count, Chunk);
  { Phase 2: wraparound }
  if Count > 0 then begin
    Move(FTXBuf[0], PByte(@Buf)[Chunk], Count);
    FTXTail := Count;
    Dec(FTXCount, Count);
  end;
end;

function TFossilRings.TXBlockWrite(const Buf; Count : Word) : Word;
var
  P : PByte;
  I : Word;
begin
  P := @Buf;
  Result := 0;
  for I := 1 to Count do
  begin
    if FTXCount >= FTXCap then Break;
    TXPut(P^);
    Inc(P);
    Inc(Result);
  end;
end;

{ === RX Ring === }

procedure TFossilRings.RXPut(B : Byte);

begin
  if FRXCount >= FRXCap then begin FRXOverflow := True; Exit; end;
  FRXBuf[FRXHead] := B;
  FRXHead := (FRXHead + 1) mod FRXCap;
  Inc(FRXCount);
end;

function TFossilRings.RXGet : Byte;
begin
  if FRXCount = 0 then begin Result := 0; Exit; end;
  Result := FRXBuf[FRXTail];
  FRXTail := (FRXTail + 1) mod FRXCap;
  Dec(FRXCount);
end;

function TFossilRings.RXPeek : Byte;
begin
  if FRXCount = 0 then Result := 0
  else Result := FRXBuf[FRXTail];
end;

function TFossilRings.RXAvail : Word;
begin
  Result := FRXCount;
end;

function TFossilRings.RXFree : Word;
begin
  Result := FRXCap - FRXCount;
end;

function TFossilRings.RXSize : Word;
begin
  Result := FRXCap;
end;

procedure TFossilRings.RXClear;
begin
  FRXHead := 0;
  FRXTail := 0;
  FRXCount := 0;
  FRXOverflow := False;
end;

function TFossilRings.RXBlockRead(var Buf; Count : Word) : Word;
var Chunk, Avail : Word;
begin
  { BUG-4 fix: two-phase memcpy instead of per-byte loop }
  Avail := FRXCount;
  if Count > Avail then Count := Avail;
  Result := Count;
  Chunk := FRXCap - FRXTail;
  if Chunk > Count then Chunk := Count;
  Move(FRXBuf[FRXTail], Buf, Chunk);
  FRXTail := (FRXTail + Chunk) mod FRXCap;
  Dec(FRXCount, Chunk);
  Dec(Count, Chunk);
  if Count > 0 then begin
    Move(FRXBuf[0], PByte(@Buf)[Chunk], Count);
    FRXTail := Count;
    Dec(FRXCount, Count);
  end;
end;

function TFossilRings.RXBlockWrite(const Buf; Count : Word) : Word;
var
  P : PByte;
  I : Word;
begin
  P := @Buf;
  Result := 0;
  for I := 1 to Count do
  begin
    if FRXCount >= FRXCap then Break;
    RXPut(P^);
    Inc(P);
    Inc(Result);
  end;
end;

end.
