{ ===========================================================================
  pcbfoss_rings.pas — Lock-free ring buffers for FOSSIL TX/RX
  ===========================================================================
  Two-phase block I/O: head-to-end then wrap-around in one call.
  Fixes BUG-4 from audit: per-byte block I/O replaced with memcpy.
  GPLv3 — the crew 4free (wrench + byte)
  =========================================================================== }

{$MODE OBJFPC}{$H-}

Unit pcbfoss_rings;

Interface

Const
  RING_SIZE = 4096;

Type
  TRingBuf = Record
    Buf   : Array[0..RING_SIZE-1] Of Byte;
    Head  : Word;   { write position }
    Tail  : Word;   { read position }
    Count : Word;   { bytes in buffer }
  End;

Procedure RingInit(Var R: TRingBuf);
Function  RingAvail(Const R: TRingBuf): Word;
Function  RingFree(Const R: TRingBuf): Word;
Procedure RingPutByte(Var R: TRingBuf; B: Byte);
Function  RingGetByte(Var R: TRingBuf): Byte;
Function  RingPeekByte(Const R: TRingBuf): Byte;

{ Block I/O — two-phase memcpy, fixes BUG-4 }
Function  RingBlockWrite(Var R: TRingBuf; Var Src; Len: Word): Word;
Function  RingBlockRead(Var R: TRingBuf; Var Dst; Len: Word): Word;

Implementation

Procedure RingInit(Var R: TRingBuf);
Begin
  FillChar(R.Buf, RING_SIZE, 0);
  R.Head  := 0;
  R.Tail  := 0;
  R.Count := 0;
End;

Function RingAvail(Const R: TRingBuf): Word;
Begin
  RingAvail := R.Count;
End;

Function RingFree(Const R: TRingBuf): Word;
Begin
  RingFree := RING_SIZE - R.Count;
End;

Procedure RingPutByte(Var R: TRingBuf; B: Byte);
Begin
  If R.Count >= RING_SIZE Then Exit;  { Full — drop byte }
  R.Buf[R.Head] := B;
  R.Head := (R.Head + 1) MOD RING_SIZE;
  Inc(R.Count);
End;

Function RingGetByte(Var R: TRingBuf): Byte;
Begin
  If R.Count = 0 Then Begin RingGetByte := 0; Exit; End;
  RingGetByte := R.Buf[R.Tail];
  R.Tail := (R.Tail + 1) MOD RING_SIZE;
  Dec(R.Count);
End;

Function RingPeekByte(Const R: TRingBuf): Byte;
Begin
  If R.Count = 0 Then RingPeekByte := 0
  Else RingPeekByte := R.Buf[R.Tail];
End;

{ BUG-4 FIX: Block write via two-phase memcpy instead of per-byte loop.
  Phase 1: head to end of buffer. Phase 2: wrap to start. }
Function RingBlockWrite(Var R: TRingBuf; Var Src; Len: Word): Word;
Var
  P: PByte;
  Phase1, Phase2, Avail: Word;
Begin
  Avail := RING_SIZE - R.Count;
  If Len > Avail Then Len := Avail;
  If Len = 0 Then Begin RingBlockWrite := 0; Exit; End;

  P := @Src;

  { Phase 1: Head to end of buffer }
  Phase1 := RING_SIZE - R.Head;
  If Phase1 > Len Then Phase1 := Len;
  Move(P^, R.Buf[R.Head], Phase1);
  Inc(P, Phase1);

  { Phase 2: Wrap around }
  Phase2 := Len - Phase1;
  If Phase2 > 0 Then
    Move(P^, R.Buf[0], Phase2);

  R.Head := (R.Head + Len) MOD RING_SIZE;
  Inc(R.Count, Len);
  RingBlockWrite := Len;
End;

{ Block read: same two-phase approach }
Function RingBlockRead(Var R: TRingBuf; Var Dst; Len: Word): Word;
Var
  P: PByte;
  Phase1, Phase2: Word;
Begin
  If Len > R.Count Then Len := R.Count;
  If Len = 0 Then Begin RingBlockRead := 0; Exit; End;

  P := @Dst;

  Phase1 := RING_SIZE - R.Tail;
  If Phase1 > Len Then Phase1 := Len;
  Move(R.Buf[R.Tail], P^, Phase1);
  Inc(P, Phase1);

  Phase2 := Len - Phase1;
  If Phase2 > 0 Then
    Move(R.Buf[0], P^, Phase2);

  R.Tail := (R.Tail + Len) MOD RING_SIZE;
  Dec(R.Count, Len);
  RingBlockRead := Len;
End;

End.
