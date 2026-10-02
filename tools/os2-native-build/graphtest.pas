program graphtest;
uses graph, crt, sysutils, classes, objects, app, dialogs, zipper, fpjson, process;
var gd,gm:smallint;
begin
  gd:=Detect; gm:=0;
  InitGraph(gd,gm,'');
  if GraphResult=grOk then begin
    SetColor(15); Line(0,0,100,100); Circle(50,50,20);
    CloseGraph;
  end;
  writeln('graphtest ok ', IntToStr(GetMaxX));
end.
