{$IFDEF FPC}{$MODE TP}{$ENDIF}
unit exec;
{
  Memory-efficient EXEC unit.
  Original: Keld R. Hansen (TP 6/7 real-mode memory shrink + exec).
  FPC port: uses standard Dos.Exec which handles memory natively.
}
interface
USES DOS;
TYPE STR127 = STRING[127];
FUNCTION EXECUTE(Name : PathStr ; Tail : STR127) : WORD;

implementation

FUNCTION EXECUTE(Name : PathStr ; Tail : STR127) : WORD;
BEGIN
{$IFDEF FPC}
  Dos.Exec(Name, Tail);
  EXECUTE := DosError;
{$ELSE}
  SwapVectors;
  Dos.Exec(Name, Tail);
  SwapVectors;
  EXECUTE := DosError;
{$ENDIF}
END;

end.
