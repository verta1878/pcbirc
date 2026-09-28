Unit FontUnit;
{$IFDEF FPC}{$MODE TP}{$ENDIF}

Interface

Uses Dos;

Type CharRec = Record Data : Array[1..16] of Byte; End;

Var CharSet        : Array[0..255] of CharRec;
    DefaultCharSet : Array[0..255] of CharRec;
    TempFont       : Array[0..255] of CharRec;
    ClipBoard      : CharRec;

Procedure GetImageChar(chrVal : Byte; Var CharInfo);
Procedure SetImageChar(chrVal : Byte; Var CharInfo);
Procedure SaveTempFont;
Procedure RestoreTempFont;
Procedure SavetoClipBoard(Character : Byte);
Procedure PasteFromClipBoard(Character : Byte);
Procedure GetFont(var P:Pointer);
Procedure PutFont(P:Pointer);

Implementation

{$IFDEF FPC}
Uses Go32;

Procedure WritePortW(port, val: Word);
Begin outportb(port, Lo(val)); outportb(port+1, Hi(val)); End;
{$ENDIF}

Procedure GetImageChar(chrVal : Byte; Var CharInfo);
Var Offset : Word;
Begin
  Offset := chrVal * 32;
{$IFDEF FPC}
  WritePortW($3C4, $0402); WritePortW($3C4, $0704);
  WritePortW($3CE, $0204); WritePortW($3CE, $0005); WritePortW($3CE, $0006);
  dosmemget($A000, Offset, CharInfo, 16);
  WritePortW($3C4, $0302); WritePortW($3C4, $0304);
  WritePortW($3CE, $0004); WritePortW($3CE, $1005); WritePortW($3CE, $0E06);
{$ELSE}
  Inline($FA);
  PortW[$3C4] := $0402; PortW[$3C4] := $0704;
  PortW[$3CE] := $0204; PortW[$3CE] := $0005; PortW[$3CE] := $0006;
  Move(Ptr($A000, Offset)^, CharInfo, 16);
  PortW[$3C4] := $0302; PortW[$3C4] := $0304;
  PortW[$3CE] := $0004; PortW[$3CE] := $1005; PortW[$3CE] := $0E06;
  Inline($FB);
{$ENDIF}
End;

Procedure SetImageChar(chrVal : Byte; Var CharInfo);
Var Offset : Word;
Begin
  Offset := chrVal * 32;
{$IFDEF FPC}
  WritePortW($3C4, $0402); WritePortW($3C4, $0704);
  WritePortW($3CE, $0204); WritePortW($3CE, $0005); WritePortW($3CE, $0006);
  dosmemput($A000, Offset, CharInfo, 16);
  WritePortW($3C4, $0302); WritePortW($3C4, $0304);
  WritePortW($3CE, $0004); WritePortW($3CE, $1005); WritePortW($3CE, $0E06);
{$ELSE}
  Inline($FA);
  PortW[$3C4] := $0402; PortW[$3C4] := $0704;
  PortW[$3CE] := $0204; PortW[$3CE] := $0005; PortW[$3CE] := $0006;
  Move(CharInfo, Ptr($A000, Offset)^, 16);
  PortW[$3C4] := $0302; PortW[$3C4] := $0304;
  PortW[$3CE] := $0004; PortW[$3CE] := $1005; PortW[$3CE] := $0E06;
  Inline($FB);
{$ENDIF}
End;

Procedure SaveTempFont;
Var X : Byte;
Begin For X := 0 to 255 do GetImageChar(X,TempFont[X]); End;

Procedure RestoreTempFont;
Var X : Byte;
Begin For X := 0 to 255 do SetImageChar(X,TempFont[X]); End;

Procedure SavetoClipBoard(Character : Byte);
Begin GetImageChar(Character,ClipBoard); End;

Procedure PasteFromClipBoard(Character : Byte);
Begin SetImageChar(Character,Clipboard); End;

Procedure GetFont(var P:Pointer);
Var Regs : Registers;
Begin
  Regs.AX := $1130; Regs.BH := $06; Intr($10, Regs);
{$IFDEF FPC}
  GetMem(P, 256*16);
  dosmemget(Regs.ES, Regs.BP, P^, 256*16);
{$ELSE}
  P := Ptr(Regs.ES, Regs.BP);
{$ENDIF}
End;

Procedure PutFont(P:Pointer);
Begin
{$IFDEF FPC}
  WritePortW($3C4, $0402); WritePortW($3C4, $0704);
  WritePortW($3CE, $0204); WritePortW($3CE, $0005); WritePortW($3CE, $0006);
  dosmemput($A000, 0, P^, 256*16);
  WritePortW($3C4, $0302); WritePortW($3C4, $0304);
  WritePortW($3CE, $0004); WritePortW($3CE, $1005); WritePortW($3CE, $0E06);
{$ELSE}
  PortW[$3C4] := $0402; PortW[$3C4] := $0704;
  PortW[$3CE] := $0204; PortW[$3CE] := $0005; PortW[$3CE] := $0006;
  Move(P^, Ptr($A000, 0)^, 256*16);
  PortW[$3C4] := $0302; PortW[$3C4] := $0304;
  PortW[$3CE] := $0004; PortW[$3CE] := $1005; PortW[$3CE] := $0E06;
{$ENDIF}
End;

End.
