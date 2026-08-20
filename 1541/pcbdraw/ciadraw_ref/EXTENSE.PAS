Unit Extense;
{$IFDEF FPC}
  {$MODE TP}
{$ENDIF}

Interface

{$IFDEF FPC}
Uses Go32;
{$ENDIF}

Var Screen : Array[0..3999] of byte;

Procedure InputNums(var Lne : String;Chars : Byte);
Function  VGACard : Boolean;
Procedure WaitRetrace;
Procedure SetTypeRate(Kdelay, Krate:Byte);
Procedure ChangeMode(Mode : Word);
Procedure Setcursorshape(Shape : word);
Function  Extended_Key:Word;
Procedure Blinking(State : Boolean);
Function  IntToStr(Number : LongInt): String;
Function  Pad(S : String;Len : Byte) : String;
Function  Padstring(Outp : String;Number : Byte): String;
Procedure NoCursor;
Procedure YesCursor;
Procedure FastMove(Var Source,Dest;NumToMove:Word);
Procedure Mousesensetivity(x,y:word);
Procedure Clear_Keyboard_Buffer;
Procedure QWrite(X, Y : Byte; S : String; FG, BG, Width : Byte);
Function  LeadingZero(W : Word) : String;
Procedure PipeWrite(S : String);
Procedure Uncrunch(var Addr1,Addr2; BlkLen:Integer);
Function  CapsLock : Boolean;
Function  ScrollLock : Boolean;
Function  InsKey : Boolean;
Procedure GetKeys(Var C1 : Char;Var C2 : Char);
Procedure Beep(Freq,Dely : Word);
Procedure InputLn(var Lne:String;Chars:Byte;Caps:Boolean;Allcaps:Boolean);
Function  Yesno : Boolean;

Implementation

Uses Dos, Crt;

Procedure InputNums(var Lne : String;Chars : Byte);
Var Key : Char;
Begin
  Write(Lne);
  Repeat
    Key := ReadKey;
    If (Key in ['0'..'9']) and (Length(Lne) < Chars) then begin
      Lne := Lne + Key; Write(Key);
    End;
    if (Key = #8) and (Length(Lne) > 0) then begin
      Delete(Lne,Length(Lne),1); Write(#8,' ',#8);
    End;
  Until Key = #13;
End;

Function VGACard : Boolean;
Var Regs : Registers;
Begin
  Regs.AX := $1A00; Intr($10,Regs);
  VGACard := (Regs.AL = $1A) and (Regs.BL >= 7);
End;

Procedure WaitRetrace;
Begin
{$IFDEF FPC}
  While (inportb($3DA) and 8) <> 0 do;
  While (inportb($3DA) and 8) = 0 do;
{$ELSE}
  asm
    mov dx,$3da
  @l1: in al,dx; test al,8; jnz @l1
  @l2: in al,dx; test al,8; jz @l2
  end;
{$ENDIF}
End;

Procedure Mousesensetivity(x,y:word);
Var Regs : Registers;
Begin
  Regs.AX := $1A; Regs.BX := x; Regs.CX := y; Regs.DX := 0;
  Intr($33, Regs);
End;

Procedure ChangeMode(Mode : Word);
Var Regs : Registers;
Begin Regs.AX := Mode; Intr($10, Regs); End;

Procedure Setcursorshape(Shape : word);
Var Regs : Registers;
Begin Regs.AH := 1; Regs.CX := Shape; Intr($10, Regs); End;

Function Extended_Key:Word;
Var Regs: Registers;
Begin
  Regs.AH := $10; Intr($16, Regs);
  If Regs.AL = 0 then Extended_Key := Regs.AH + 256
  Else Extended_Key := Regs.AL;
End;

Procedure SetTypeRate(Kdelay, Krate:Byte);
Var Regs : Registers;
Begin
  Regs.AH := 3; Regs.AL := 5; Regs.BH := Kdelay; Regs.BL := Krate;
  Intr($16, Regs);
End;

Procedure Blinking(State : Boolean);
Var Regs : Registers;
Begin
  Regs.AX := $1003;
  if State then Regs.BL := 1 else Regs.BL := 0;
  Intr($10, Regs);
End;

Function IntToStr(Number : LongInt): String;
Var Tmp : String;
Begin Str(Number, Tmp); IntToStr := Tmp; End;

Function Pad(S : String;Len : Byte) : String;
Begin While Length(S) < Len Do S := S + ' '; Pad := S; End;

Function Padstring(Outp : String;Number : Byte): String;
Begin While Length(Outp) < Number do Outp := Outp + ' '; Padstring := Outp; End;

Procedure NoCursor;
Var Regs : Registers;
Begin Regs.AH := 1; Regs.CX := $2000; Intr($10, Regs); End;

Procedure YesCursor;
Var Regs : Registers;
Begin Regs.AH := 1; Regs.CX := $0607; Intr($10, Regs); End;

Procedure FastMove(Var Source,Dest;NumToMove:Word);
Begin Move(Source, Dest, NumToMove); End;

Procedure Clear_Keyboard_Buffer;
Begin While KeyPressed do ReadKey; End;

Procedure QWrite(X, Y : Byte; S : String; FG, BG, Width : Byte);
Var I, Attr : Byte;
    Buf : Array[1..160] of Byte;
Begin
  Attr := (BG shl 4) or FG;
  While Length(S) < Width do S := S + ' ';
  For I := 1 to Length(S) do begin
    Buf[I*2-1] := Ord(S[I]);
    Buf[I*2]   := Attr;
  end;
{$IFDEF FPC}
  dosmemput($B800, Word(((Y-1)*80+(X-1))*2), Buf, Length(S)*2);
{$ELSE}
  For I := 1 to Length(S) do begin
    Mem[$B800:((Y-1)*80+(X-1)+I-1)*2] := Ord(S[I]);
    Mem[$B800:((Y-1)*80+(X-1)+I-1)*2+1] := Attr;
  end;
{$ENDIF}
End;


Function LeadingZero(W : Word) : String;
Var S : String;
Begin
  Str(w:0,S);
  If Length(s) = 1 then S := '0'+S;
  LeadingZero := S;
End;

Procedure PipeWrite(S : String);
Var X,Code : Byte;
    Error  : Integer;
Begin
  For X := 1 to Length(S) do begin
    If S[X] <> '|' then Write(S[X])
    Else Begin
      Val(Copy(S,X + 1,2),Code,Error);
      If Error>0 then Write(S[X])
      Else Begin
        If Code<16 then Textcolor(Code) Else Textbackground(Code-16);
        If X + 2 > Length(S) then Inc(X,1) else Inc(X,2);
      End;
    End;
  end;
End;


Procedure Uncrunch(var Addr1,Addr2; BlkLen:Integer);
Begin Move(Addr1, Addr2, BlkLen); End;
Procedure Beep(Freq,Dely : Word);
Begin Sound(Freq); Delay(Dely); Nosound; End;

Procedure InputLn(var Lne:String;Chars:Byte;Caps:Boolean;Allcaps:Boolean);
Var StartedX,X,Y,Counter,OldAttr:Byte; CH:Char; P:String; Done:boolean;
Begin
  OldAttr:=Textattr; StartedX:=WhereX; P:=''; Counter:=0; Yescursor; Done:=False;
  For X:=1 to Chars do Write(' ');GotoXy(StartedX,WhereY);
  Repeat X:=WhereX;Y:=WhereY; CH:=Readkey;If CH=#0 then CH:=Readkey;
    Case CH of
      #8:If Counter>0 then Begin GotoXy(X-1,Y);Write(' ');GotoXy(X-1,Y);Delete(P,Length(P),1);Dec(Counter);End;
      #13:Done:=True;
      #27:Begin P:='Aborted';Done:=True;End;
      #32..#126:If Counter<Chars then Begin Inc(Counter);
        If Caps then If(P[Counter-1]=#0)or(P[Counter-1]=#32)then CH:=Upcase(CH);
        If Allcaps then CH:=Upcase(CH); P:=P+CH; Write(CH); End;
    End;
  Until Done; Lne:=P; GotoXy(StartedX,WhereY);
  Textattr:=7;For X:=1 to Chars do Write(' ');
  If Lne<>'Aborted' then Begin GotoXy(StartedX,WhereY);Write(Lne);End;
  Nocursor; Textattr:=OldAttr;
End;

Function Yesno:Boolean;
Var X,Y:Byte;CH:Char;Yes:Boolean;
Begin X:=WhereX;Y:=WhereY;
  GotoXy(X,Y);PipeWrite('|15|17 Yes |16|07 No ');
  Yes:=True; CH:=#0;
  While not(CH in['Y','N',#13])do Begin
    CH:=Readkey;If CH=#0 then CH:=Readkey;CH:=Upcase(CH);
    If CH in[#75,'Y'] then Yes:=True;
    If CH in[#77,'N'] then Yes:=False;
    GotoXy(X,Y);
    If Yes then Begin Textattr:=$1F;Write(' Yes ');Textattr:=$7;Write(' No ');End
    else Begin Textattr:=$7;Write(' Yes ');Textattr:=$1F;Write(' No ');End;
  End;
  Yesno:=Yes;
End;

Function CapsLock : Boolean;
{$IFDEF FPC}
Var Regs : Registers;
Begin Regs.AH := 2; Intr($16, Regs); CapsLock := (Regs.AL and $40) <> 0; End;
{$ELSE}
Begin if (Mem[$0040:$0017] and $40) = $40 then CapsLock := True else CapsLock := False; End;
{$ENDIF}

Function ScrollLock : Boolean;
{$IFDEF FPC}
Var Regs : Registers;
Begin Regs.AH := 2; Intr($16, Regs); ScrollLock := (Regs.AL and $10) <> 0; End;
{$ELSE}
Begin If (Mem[$0040:$0017] and $10) = $10 then ScrollLock := True Else ScrollLock := False; End;
{$ENDIF}

Function InsKey : Boolean;
Var Regs : Registers;
Begin Regs.AH := 2; Intr($16, Regs); InsKey := (Regs.AL and 128) <> 0; End;

Procedure GetKeys(Var C1 : Char; Var C2 : Char);
Begin C1 := ReadKey; If C1 = #0 then C2 := ReadKey else C2 := #0; End;
end.
