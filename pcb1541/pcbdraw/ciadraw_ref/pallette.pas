Unit Pallette;
{$IFDEF FPC}{$MODE TP}{$ENDIF}

Interface

Const Dacs : Array[0..15] of Byte = (0,1,2,3,4,5,20,7,56,57,58,59,60,61,62,63);

Var
  Pall           : Array[0..255,1..3] of Byte;
  OrigPal        : Array[0..255,1..3] of Byte;
  GreyPallette   : Array[0..255,1..3] of Byte;
  TempPallette   : Array[0..255,1..3] of Byte;

 Procedure SaveTempPal;
 Procedure RestoreTempPal;
 Procedure GrabPallette;
 Procedure RestorePallette;
 Procedure GrabOrigPallette;
 Procedure RestoreOrigPallette;
 Procedure SetPallette(ColorNo : Byte; R,G,B : Byte);
 Procedure GetPallette(Col : Byte; Var R,G,B : Byte);
 Procedure FadeUp;
 Procedure FadeDown;
 Procedure InitGreyScale;
 Procedure InstantGreyscale;

Implementation

Uses {$IFDEF FPC}Go32,{$ENDIF}Dos,Crt,Extense;

Procedure GetPallette(Col : Byte; Var R,G,B : Byte);
Begin
{$IFDEF FPC}
  outportb($3C7, Col);
  R := inportb($3C9); G := inportb($3C9); B := inportb($3C9);
{$ELSE}
  Port[$3C7] := Col;
  R := Port[$3C9]; G := Port[$3C9]; B := Port[$3C9];
{$ENDIF}
End;

Procedure SetPallette(ColorNo : Byte; R,G,B : Byte);
Begin
{$IFDEF FPC}
  outportb($3C8, ColorNo);
  outportb($3C9, R); outportb($3C9, G); outportb($3C9, B);
{$ELSE}
  Port[$3C8] := ColorNo;
  Port[$3C9] := R; Port[$3C9] := G; Port[$3C9] := B;
{$ENDIF}
End;

Procedure SaveTempPal;
Begin Move(Pall, TempPallette, 768); End;

Procedure RestoreTempPal;
Begin Move(TempPallette, Pall, 768); End;

Procedure GrabPallette;
Var X : Integer;
Begin For X := 0 to 255 do GetPallette(X,Pall[X,1],Pall[X,2],Pall[X,3]); End;

Procedure GrabOrigPallette;
Var X : Integer;
Begin For X := 0 to 255 do GetPallette(X,OrigPal[X,1],OrigPal[X,2],OrigPal[X,3]); End;

Procedure RestorePallette;
Var X : Integer;
Begin For X := 0 to 255 do SetPallette(X,Pall[X,1],Pall[X,2],Pall[X,3]); End;

Procedure RestoreOrigPallette;
Var X : Integer;
Begin For X := 0 to 255 do SetPallette(X,OrigPal[X,1],OrigPal[X,2],OrigPal[X,3]); End;

Procedure FadeUp;
Var loop1,loop2 : Integer; Tmp : Array[1..3] of Byte;
Begin
  For loop1:=1 to 64 do For loop2:=0 to 255 do begin
    GetPallette(loop2,Tmp[1],Tmp[2],Tmp[3]);
    If Tmp[1]<Pall[loop2,1] then inc(Tmp[1]);
    If Tmp[2]<Pall[loop2,2] then inc(Tmp[2]);
    If Tmp[3]<Pall[loop2,3] then inc(Tmp[3]);
    SetPallette(loop2,Tmp[1],Tmp[2],Tmp[3]);
  end;
End;

Procedure FadeDown;
Var loop1,loop2 : Integer; Tmp : Array[1..3] of Byte;
Begin
  For loop1:=1 to 64 do For loop2:=0 to 255 do begin
    GetPallette(loop2,Tmp[1],Tmp[2],Tmp[3]);
    If Tmp[1]>0 then dec(Tmp[1]);
    If Tmp[2]>0 then dec(Tmp[2]);
    If Tmp[3]>0 then dec(Tmp[3]);
    SetPallette(loop2,Tmp[1],Tmp[2],Tmp[3]);
  end;
End;

Procedure InitGreyScale;
Var X : Byte; R,G,B : Byte; Grey : Integer;
Begin
  For X := 0 to 255 do begin
    GetPallette(X,R,G,B);
    Grey := Round((R * 0.30)+(G * 0.59)+(B * 0.11));
    GreyPallette[X,1] := Grey;
    GreyPallette[X,2] := Grey;
    GreyPallette[X,3] := Grey;
  end;
End;

Procedure InstantGreyscale;
Var X : Integer;
Begin For X := 1 to 255 do SetPallette(X,GreyPallette[X,1],GreyPallette[X,2],GreyPallette[X,3]); End;

End.
