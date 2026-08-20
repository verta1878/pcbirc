{ ===========================================================================
  pcbis_http.pas — HTTP server for pcbis
  Static file serving + dynamic endpoints (/status, /callers, /online).
  =========================================================================== }

unit pcbis_http;

{$mode objfpc}{$H+}

interface

uses
  pcbis_net;

procedure HttpOnConnect(Conn : TPcbisConnection);
procedure HttpOnData(Conn : TPcbisConnection);

implementation

uses
  SysUtils, Classes, pcbis_log, pcbis_config;

const
  SERVER_NAME = 'pcbis/0.1.0';
  CRLF = #13#10;

procedure SendResponse(Conn : TPcbisConnection;
                       Code : integer; const Status, ContentType, Body : string);
var
  Header : string;
begin
  Header := 'HTTP/1.0 ' + IntToStr(Code) + ' ' + Status + CRLF +
            'Server: ' + SERVER_NAME + CRLF +
            'Content-Type: ' + ContentType + CRLF +
            'Content-Length: ' + IntToStr(Length(Body)) + CRLF +
            'Connection: close' + CRLF +
            CRLF;
  Conn.OutBuf := Conn.OutBuf + Header + Body;
  Conn.State := csClosing; { HTTP/1.0 — close after response }
end;

procedure Send404(Conn : TPcbisConnection; const Path : string);
begin
  SendResponse(Conn, 404, 'Not Found', 'text/html',
    '<html><head><title>404</title></head><body>' +
    '<h1>404 Not Found</h1><p>' + Path + ' not found on this server.</p>' +
    '<hr><em>' + SERVER_NAME + '</em></body></html>');
end;

function BuildStatusPage : string;
begin
  Result :=
    '<html><head><title>PCBoard BBS Status</title>' +
    '<style>body{font-family:monospace;background:#000;color:#0f0;padding:20px}' +
    'h1{color:#0ff}table{border-collapse:collapse;margin:10px 0}' +
    'td,th{border:1px solid #0f0;padding:4px 12px;text-align:left}' +
    'th{background:#003;color:#0ff}</style></head><body>' +
    '<h1>PCBoard 15.4 Revival — Server Status</h1>' +
    '<table><tr><th>Service</th><th>Status</th><th>Port</th></tr>' +
    '<tr><td>Telnet</td><td>Online</td><td>2323</td></tr>' +
    '<tr><td>BinkP</td><td>Online</td><td>24554</td></tr>' +
    '<tr><td>FTP</td><td>Online</td><td>21</td></tr>' +
    '<tr><td>HTTP</td><td>Online</td><td>8080</td></tr>' +
    '<tr><td>SMTP</td><td>Queue Active</td><td>outbound</td></tr>' +
    '</table>' +
    '<p>Uptime: ' + FormatDateTime('hh:nn:ss', Now - ServerStartTime) + '</p>' +
    '<hr><em>' + SERVER_NAME + '</em></body></html>';
end;

function BuildCallersPage : string;
begin
  { TODO: read PCBoard CALLERS log file and format as HTML table }
  Result :=
    '<html><head><title>Last Callers</title>' +
    '<style>body{font-family:monospace;background:#000;color:#0f0;padding:20px}' +
    'h1{color:#0ff}table{border-collapse:collapse}' +
    'td,th{border:1px solid #0f0;padding:4px 12px}' +
    'th{background:#003;color:#0ff}</style></head><body>' +
    '<h1>PCBoard 15.4 Revival — Last Callers</h1>' +
    '<table><tr><th>#</th><th>Name</th><th>Date</th><th>Node</th></tr>' +
    '<tr><td>1</td><td>SYSOP</td><td>' + DateToStr(Now) + '</td><td>1</td></tr>' +
    '</table>' +
    '<p><a href="/">Home</a> | <a href="/status">Status</a> | <a href="/online">Online</a></p>' +
    '<hr><em>' + SERVER_NAME + '</em></body></html>';
end;

function BuildOnlinePage : string;
begin
  { TODO: read PCBoard USERS.SYS / node status files }
  Result :=
    '<html><head><title>Who''s Online</title>' +
    '<style>body{font-family:monospace;background:#000;color:#0f0;padding:20px}' +
    'h1{color:#0ff}table{border-collapse:collapse}' +
    'td,th{border:1px solid #0f0;padding:4px 12px}' +
    'th{background:#003;color:#0ff}</style></head><body>' +
    '<h1>PCBoard 15.4 Revival — Who''s Online</h1>' +
    '<table><tr><th>Node</th><th>User</th><th>Activity</th><th>Time</th></tr>' +
    '<tr><td colspan="4"><em>No users online</em></td></tr>' +
    '</table>' +
    '<p><a href="/">Home</a> | <a href="/status">Status</a> | <a href="/callers">Callers</a></p>' +
    '<hr><em>' + SERVER_NAME + '</em></body></html>';
end;

function LoadStaticFile(const DocRoot, Path : string) : string;
var
  FullPath : string;
  F : TFileStream;
begin
  Result := '';
  if Path = '/' then
    FullPath := DocRoot + '/index.htm'
  else
    FullPath := DocRoot + Path;

  { Security: no .. traversal }
  if Pos('..', FullPath) > 0 then Exit;

  if not FileExists(FullPath) then Exit;

  try
    F := TFileStream.Create(FullPath, fmOpenRead or fmShareDenyNone);
    try
      SetLength(Result, F.Size);
      if F.Size > 0 then
        F.Read(Result[1], F.Size);
    finally
      F.Free;
    end;
  except
    Result := '';
  end;
end;

function ContentTypeFor(const Path : string) : string;
var
  Ext : string;
begin
  Ext := LowerCase(ExtractFileExt(Path));
  if (Ext = '.htm') or (Ext = '.html') then Result := 'text/html'
  else if Ext = '.txt' then Result := 'text/plain'
  else if Ext = '.css' then Result := 'text/css'
  else if Ext = '.js' then Result := 'application/javascript'
  else if Ext = '.json' then Result := 'application/json'
  else if (Ext = '.jpg') or (Ext = '.jpeg') then Result := 'image/jpeg'
  else if Ext = '.png' then Result := 'image/png'
  else if Ext = '.gif' then Result := 'image/gif'
  else if Ext = '.ico' then Result := 'image/x-icon'
  else Result := 'application/octet-stream';
end;

procedure HttpOnConnect(Conn : TPcbisConnection);
begin
  LogConn('HTTP', Conn.RemoteIP, 'connected');
end;

procedure HttpOnData(Conn : TPcbisConnection);
var
  P        : integer;
  Request  : string;
  Method   : string;
  Path     : string;
  Body     : string;
  SP1, SP2 : integer;
begin
  { Wait for complete request (ends with double CRLF) }
  P := Pos(CRLF + CRLF, Conn.InBuf);
  if P = 0 then Exit;

  Request := Copy(Conn.InBuf, 1, P - 1);
  Conn.InBuf := '';

  { Parse request line: "GET /path HTTP/1.x" }
  SP1 := Pos(' ', Request);
  if SP1 = 0 then begin Send404(Conn, '/'); Exit; end;

  Method := Copy(Request, 1, SP1 - 1);
  SP2 := Pos(' ', Request, SP1 + 1);
  if SP2 = 0 then SP2 := Length(Request) + 1;
  Path := Copy(Request, SP1 + 1, SP2 - SP1 - 1);

  LogConn('HTTP', Conn.RemoteIP, Method + ' ' + Path);

  if Method <> 'GET' then
  begin
    SendResponse(Conn, 405, 'Method Not Allowed', 'text/plain', 'Only GET supported');
    Exit;
  end;

  { Dynamic endpoints }
  if Path = '/status' then
  begin
    SendResponse(Conn, 200, 'OK', 'text/html', BuildStatusPage);
    Exit;
  end;

  if Path = '/callers' then
  begin
    SendResponse(Conn, 200, 'OK', 'text/html', BuildCallersPage);
    Exit;
  end;

  if Path = '/online' then
  begin
    SendResponse(Conn, 200, 'OK', 'text/html', BuildOnlinePage);
    Exit;
  end;

  { Static files }
  Body := LoadStaticFile('.', Path);  { TODO: get docroot from config }
  if Body <> '' then
  begin
    SendResponse(Conn, 200, 'OK', ContentTypeFor(Path), Body);
    Exit;
  end;

  Send404(Conn, Path);
end;

end.
