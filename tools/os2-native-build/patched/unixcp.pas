unit unixcp;
{$mode objfpc}
interface
{ Stub for non-Unix platforms — returns CP437 }
function GetSystemCodepage: word;
implementation
function GetSystemCodepage: word;
begin
  Result := 437; { DOS CP437 }
end;
end.
