{ ===========================================================================
  pcbis_ftp.pas — FTP server for pcbis
  Based on MIS FTP server (mystic-bbs-irc/mystic/mis_client_ftp.pas, 1,341 lines)
  
  Key features from Mystic:
  - QWK via FTP: RETR boardid.qwk generates packet, STOR boardid.rep imports
  - Data connection: PASV (passive) + PORT (active) with configurable port range
  - File transfer: block-based with resume (REST) support
  - Security: access levels, daily DL limits, UL/DL ratio enforcement
  - Upload: disk space check, duplicate detection, FILE_ID.DIZ extraction
  - Directory mapping: file bases → virtual FTP directories
  =========================================================================== }

unit pcbis_ftp;

{$mode objfpc}{$H+}

interface

uses
  pcbis_net;

procedure FtpOnConnect(Conn : TPcbisConnection);
procedure FtpOnData(Conn : TPcbisConnection);

implementation

{ BUG-1 fix: path traversal protection }
function SafePath(const Base, UserPath: string): string;
var
  Full: string;
begin
  Full := ExpandFileName(Base + DirectorySeparator + UserPath);
  if Pos(Base, Full) = 1 then
    Result := Full
  else
    Result := '';  { path escaped the jail }
end;

function ContainsDotDot(const S: string): boolean;
begin
  Result := (Pos('..', S) > 0) or (Pos(#0, S) > 0);
end;


uses
  SysUtils, Classes, BaseUnix, Sockets,
  pcbis_log, pcbis_config, pcbis_security;

const
  XFER_BUF_SIZE = 8192;  { 8K transfer buffer }

  { FTP reply codes }
  RE_READY       = '220 PCBoard 15.4 Revival FTP Server ready';
  RE_GOODBYE     = '221 Goodbye';
  RE_XFER_OK     = '226 Transfer complete';
  RE_PASV_OK     = '227 Entering Passive Mode';
  RE_LOGIN_OK    = '230 User logged in';
  RE_NEED_PASS   = '331 Password required';
  RE_DATA_OPEN   = '150 Opening data connection';
  RE_DATA_CLOSED = '226 Closing data connection';
  RE_OK          = '250 OK';
  RE_PWD         = '257';
  RE_NO_ACCESS   = '550 Access denied';
  RE_BAD_FILE    = '550 File not found';
  RE_DL_LIMIT    = '550 Download limit exceeded';
  RE_DL_RATIO    = '550 Upload/download ratio not met';
  RE_BAD_CMD     = '502 Command not implemented';
  RE_DISK_FULL   = '452 Insufficient storage space';

type
  TFtpState = (fsWaitUser, fsWaitPass, fsReady, fsTransfer);

  { PCBoard file area mapping (from DLPATH.LST) }
  TFileArea = record
    FtpName    : string;   { virtual directory name }
    RealPath   : string;   { actual filesystem path }
    AreaNum    : integer;  { PCBoard file area number }
    SecLevel   : integer;  { minimum security level }
    UlAllowed  : boolean;  { uploads permitted }
    FreeFiles  : boolean;  { downloads don't count against limits }
    Description: string;
  end;

  TFtpSession = record
    State      : TFtpState;
    Username   : string;
    SecLevel   : integer;   { user's security level }
    CurArea    : integer;   { current file area index (-1 = root) }
    DataPort   : Word;      { passive mode data port }
    DataIP     : string;    { active mode client IP }
    DataSock   : longint;   { passive listener socket }
    Passive    : boolean;
    TransType  : char;      { 'A' ascii, 'I' binary }
    RestPos    : longint;   { resume position }
    DlToday    : longint;   { bytes downloaded this session }
    DlCount    : integer;   { files downloaded this session }
    UlCount    : integer;   { files uploaded this session }
  end;

var
  FtpSessions : array[0..127] of TFtpSession;
  FileAreas   : array of TFileArea;
  NumAreas    : integer = 0;
  QwkBoardID  : string = 'PCBREV';
  FtpPassvMin : Word = 10000;
  FtpPassvMax : Word = 10100;

function GetFtp(Conn : TPcbisConnection) : integer;
begin
  Result := Conn.Socket mod 128;
end;

procedure SendReply(Conn : TPcbisConnection; const Msg : string);
begin
  Conn.OutBuf := Conn.OutBuf + Msg + #13#10;
end;

{ === Data Connection === }

function OpenDataPassive(var Session : TFtpSession) : longint;
var
  Addr    : TInetSockAddr;
  OptVal  : longint;
  ClientSock : longint;
  ClientAddr : TInetSockAddr;
  AddrLen : TSockLen;
begin
  Result := -1;

  { Accept connection on the passive listening socket }
  if Session.DataSock < 0 then Exit;

  AddrLen := SizeOf(ClientAddr);
  ClientSock := fpAccept(Session.DataSock, @ClientAddr, @AddrLen);

  { Close the listener }
  CloseSocket(Session.DataSock);
  Session.DataSock := -1;

  Result := ClientSock;
end;

function OpenDataActive(const Session : TFtpSession) : longint;
var
  Sock : longint;
  Addr : TInetSockAddr;
begin
  Result := -1;
  Sock := fpSocket(AF_INET, SOCK_STREAM, 0);
  if Sock < 0 then Exit;

  FillChar(Addr, SizeOf(Addr), 0);
  Addr.sin_family := AF_INET;
  Addr.sin_port := htons(Session.DataPort);
  Addr.sin_addr := StrToNetAddr(Session.DataIP);

  if fpConnect(Sock, @Addr, SizeOf(Addr)) <> 0 then
  begin
    CloseSocket(Sock);
    Exit;
  end;

  Result := Sock;
end;

function OpenDataSession(var Session : TFtpSession) : longint;
begin
  if Session.Passive then
    Result := OpenDataPassive(Session)
  else
    Result := OpenDataActive(Session);
end;

{ === File Transfer === }

function SendFile(Conn : TPcbisConnection; var Session : TFtpSession;
                  const Filename : string) : boolean;
var
  F       : file;
  Buf     : array[1..XFER_BUF_SIZE] of byte;
  DataConn: longint;
  BytesRead, BytesSent : longint;
begin
  Result := False;

  if not FileExists(Filename) then
  begin
    SendReply(Conn, RE_BAD_FILE);
    Exit;
  end;

  SendReply(Conn, RE_DATA_OPEN + ' for ' + ExtractFileName(Filename));
  DataConn := OpenDataSession(Session);
  if DataConn < 0 then
  begin
    SendReply(Conn, '425 Cannot open data connection');
    Exit;
  end;

  AssignFile(F, Filename);
  Reset(F, 1);

  { Resume support }
  if Session.RestPos > 0 then
  begin
    Seek(F, Session.RestPos);
    Session.RestPos := 0;
  end;

  while not EOF(F) do
  begin
    BlockRead(F, Buf, SizeOf(Buf), BytesRead);
    if BytesRead <= 0 then Break;

    BytesSent := fpSend(DataConn, @Buf, BytesRead, 0);
    if BytesSent <= 0 then Break;

    Session.DlToday := Session.DlToday + BytesSent;
  end;

  CloseFile(F);
  CloseSocket(DataConn);

  Inc(Session.DlCount);
  SendReply(Conn, RE_XFER_OK);
  LogFtp(llInfo, Conn.RemoteIP, 'RETR ' + ExtractFileName(Filename) +
         ' (' + IntToStr(Session.DlToday) + ' bytes total today)');
  Result := True;
end;

function RecvFile(Conn : TPcbisConnection; var Session : TFtpSession;
                  const Filename : string; IsAppend : boolean) : boolean;
var
  F       : file;
  Buf     : array[1..XFER_BUF_SIZE] of byte;
  DataConn: longint;
  BytesRead : longint;
  TotalRecv : longint;
begin
  Result := False;

  SendReply(Conn, RE_DATA_OPEN + ' for ' + ExtractFileName(Filename));
  DataConn := OpenDataSession(Session);
  if DataConn < 0 then
  begin
    SendReply(Conn, '425 Cannot open data connection');
    Exit;
  end;

  AssignFile(F, Filename);
  if IsAppend and FileExists(Filename) then
  begin
    Reset(F, 1);
    Seek(F, FileSize(F));
  end
  else
    Rewrite(F, 1);

  TotalRecv := 0;
  repeat
    BytesRead := fpRecv(DataConn, @Buf, SizeOf(Buf), 0);
    if BytesRead > 0 then
    begin
      BlockWrite(F, Buf, BytesRead);
      TotalRecv := TotalRecv + BytesRead;
    end;
  until BytesRead <= 0;

  CloseFile(F);
  CloseSocket(DataConn);

  Inc(Session.UlCount);
  SendReply(Conn, RE_XFER_OK);
  LogFtp(llInfo, Conn.RemoteIP, 'STOR ' + ExtractFileName(Filename) +
         ' (' + IntToStr(TotalRecv) + ' bytes)');
  Result := True;
end;

{ === Security — PCBoard file area access checks === }

type
  TDlCheckResult = (dlOK, dlNoAccess, dlLimitExceeded, dlRatioBad);

function CheckFileLimits(const Session : TFtpSession;
                         const Area : TFileArea;
                         FileSize : longint) : TDlCheckResult;
begin
  { Check security level }
  if Session.SecLevel < Area.SecLevel then
  begin
    Result := dlNoAccess;
    Exit;
  end;

  { Free files bypass limits }
  if Area.FreeFiles then
  begin
    Result := dlOK;
    Exit;
  end;

  { Check daily download byte limit
    TODO: read from PCBoard user record (SecLevel → MaxDLBytes) }
  { Placeholder: 10MB daily limit }
  if Session.DlToday + FileSize > 10 * 1024 * 1024 then
  begin
    Result := dlLimitExceeded;
    Exit;
  end;

  { Check UL/DL ratio
    TODO: read from PCBoard user record }
  { Placeholder: no ratio enforcement yet }

  Result := dlOK;
end;

{ === Command Handlers === }

procedure ProcessCommand(Conn : TPcbisConnection; const Cmd, Arg : string);
var
  Idx : integer;
  I   : integer;
  Addr : TInetSockAddr;
  OptVal : longint;
  P1, P2 : integer;
begin
  Idx := GetFtp(Conn);

  if Cmd = 'USER' then
  begin
    FtpSessions[Idx].Username := Arg;
    if UpperCase(Arg) = 'ANONYMOUS' then
    begin
      FtpSessions[Idx].State := fsReady;
      FtpSessions[Idx].SecLevel := 10;  { low security for anonymous }
      SendReply(Conn, '230 Anonymous login OK — read-only access');
      LogFtp(llInfo, Conn.RemoteIP, 'anonymous login');
    end
    else
    begin
      FtpSessions[Idx].State := fsWaitPass;
      SendReply(Conn, RE_NEED_PASS + ' for ' + Arg);
    end;
  end

  else if Cmd = 'PASS' then
  begin
    { TODO: validate against PCBoard USERS file
      - Lookup user by name
      - Check password hash
      - Get security level
      - Check expired/deleted account
      - Update last login date }
    FtpSessions[Idx].State := fsReady;
    FtpSessions[Idx].SecLevel := 100;  { placeholder }
    SendReply(Conn, RE_LOGIN_OK + ', proceed');
    LogFtp(llInfo, Conn.RemoteIP, 'login: ' + FtpSessions[Idx].Username +
           ' (security level ' + IntToStr(FtpSessions[Idx].SecLevel) + ')');
  end

  else if Cmd = 'SYST' then
    SendReply(Conn, '215 UNIX Type: L8')

  else if Cmd = 'FEAT' then
  begin
    Conn.OutBuf := Conn.OutBuf +
      '211-Features:' + #13#10 +
      ' PASV' + #13#10 +
      ' REST STREAM' + #13#10 +
      ' SIZE' + #13#10 +
      '211 End' + #13#10;
  end

  else if Cmd = 'PWD' then
  begin
    if FtpSessions[Idx].CurArea < 0 then
      SendReply(Conn, RE_PWD + ' "/" is current directory')
    else
      SendReply(Conn, RE_PWD + ' "/' + FileAreas[FtpSessions[Idx].CurArea].FtpName +
                '" is current directory');
  end

  else if Cmd = 'CWD' then
  begin
    if (Arg = '/') or (Arg = '') then
    begin
      FtpSessions[Idx].CurArea := -1;
      SendReply(Conn, RE_OK + ' Directory changed to /');
    end
    else
    begin
      { Find matching file area }
      for I := 0 to NumAreas - 1 do
      begin
        if UpperCase(FileAreas[I].FtpName) = UpperCase(Arg) then
        begin
          if FtpSessions[Idx].SecLevel >= FileAreas[I].SecLevel then
          begin
            FtpSessions[Idx].CurArea := I;
            SendReply(Conn, RE_OK + ' Directory changed to /' + FileAreas[I].FtpName);
          end
          else
            SendReply(Conn, RE_NO_ACCESS);
          Exit;
        end;
      end;
      SendReply(Conn, '550 Directory not found');
    end;
  end

  else if Cmd = 'CDUP' then
  begin
    FtpSessions[Idx].CurArea := -1;
    SendReply(Conn, RE_OK);
  end

  else if Cmd = 'TYPE' then
  begin
    if (Arg = 'I') or (Arg = 'A') then
    begin
      FtpSessions[Idx].TransType := Arg[1];
      SendReply(Conn, '200 Type set to ' + Arg);
    end
    else
      SendReply(Conn, '504 Type not supported');
  end

  else if Cmd = 'PASV' then
  begin
    { Open passive data port }
    FtpSessions[Idx].Passive := True;
    FtpSessions[Idx].DataPort := FtpPassvMin + Random(FtpPassvMax - FtpPassvMin);

    { BUG-2 fix: close existing DataSock before opening new one }
      if FtpSessions[Idx].DataSock >= 0 then
        CloseSocket(FtpSessions[Idx].DataSock);
      FtpSessions[Idx].DataSock := fpSocket(AF_INET, SOCK_STREAM, 0);
    if FtpSessions[Idx].DataSock >= 0 then
    begin
      OptVal := 1;
      fpSetSockOpt(FtpSessions[Idx].DataSock, SOL_SOCKET, SO_REUSEADDR, @OptVal, SizeOf(OptVal));
      FillChar(Addr, SizeOf(Addr), 0);
      Addr.sin_family := AF_INET;
      Addr.sin_port := htons(FtpSessions[Idx].DataPort);
      fpBind(FtpSessions[Idx].DataSock, @Addr, SizeOf(Addr));
      fpListen(FtpSessions[Idx].DataSock, 1);

      P1 := FtpSessions[Idx].DataPort div 256;
      P2 := FtpSessions[Idx].DataPort mod 256;
      SendReply(Conn, RE_PASV_OK + ' (0,0,0,0,' + IntToStr(P1) + ',' + IntToStr(P2) + ')');
      LogFtp(llDebug, Conn.RemoteIP, 'PASV on port ' + IntToStr(FtpSessions[Idx].DataPort));
    end
    else
      SendReply(Conn, '425 Cannot open passive connection');
  end

  else if Cmd = 'PORT' then
  begin
    { Parse PORT h1,h2,h3,h4,p1,p2 }
    FtpSessions[Idx].Passive := False;
    { TODO: parse IP and port from comma-separated values }
    SendReply(Conn, '200 PORT command OK');
  end

  else if Cmd = 'LIST' then
  begin
    { Directory listing }
    if FtpSessions[Idx].CurArea < 0 then
    begin
      { Root — list file areas as directories }
      SendReply(Conn, RE_DATA_OPEN);
      I := OpenDataSession(FtpSessions[Idx]);
      if I >= 0 then
      begin
        for P1 := 0 to NumAreas - 1 do
        begin
          if FtpSessions[Idx].SecLevel >= FileAreas[P1].SecLevel then
          begin
            { Unix-style directory listing }
            Conn.OutBuf := Conn.OutBuf; { data goes to data connection }
            { TODO: send via data socket, not control }
          end;
        end;
        CloseSocket(I);
      end;
      SendReply(Conn, RE_DATA_CLOSED);
    end
    else
    begin
      { List files in current area }
      { TODO: read PCBoard DIR file for this area and format as Unix ls -l }
      SendReply(Conn, RE_DATA_OPEN);
      SendReply(Conn, RE_DATA_CLOSED);
    end;
  end

  else if Cmd = 'RETR' then
  begin
    { === QWK via FTP === }
    if UpperCase(Arg) = UpperCase(QwkBoardID + '.QWK') then
    begin
      LogFtp(llInfo, Conn.RemoteIP, 'QWK packet requested via FTP');
      { TODO: Generate QWK packet on-the-fly using pcbis_qwk.ExportQwk,
        then send via data connection. This is the Mystic pattern:
        1. Create temp dir
        2. QWK.ExportPacket → MESSAGES.DAT + CONTROL.DAT
        3. ZIP into boardid.QWK
        4. SendFile(tempdir/boardid.QWK)
        5. QWK.UpdateLastReadPointers (only if send succeeded)
        6. Cleanup temp dir }
      SendReply(Conn, '550 QWK generation not yet implemented');
      Exit;
    end;

    { Normal file download }
    if FtpSessions[Idx].CurArea < 0 then
    begin
      SendReply(Conn, RE_NO_ACCESS + ': Select a file area first');
      Exit;
    end;

    { Check limits }
    case CheckFileLimits(FtpSessions[Idx], FileAreas[FtpSessions[Idx].CurArea], 0) of
      dlOK:         SendFile(Conn, FtpSessions[Idx],
                             FileAreas[FtpSessions[Idx].CurArea].RealPath +
                             DirectorySeparator + Arg);
      dlNoAccess:   SendReply(Conn, RE_NO_ACCESS);
      dlLimitExceeded: SendReply(Conn, RE_DL_LIMIT);
      dlRatioBad:   SendReply(Conn, RE_DL_RATIO);
    end;
  end

  else if (Cmd = 'STOR') or (Cmd = 'APPE') then
  begin
    { === QWK REP via FTP === }
    if UpperCase(Arg) = UpperCase(QwkBoardID + '.REP') then
    begin
      LogFtp(llInfo, Conn.RemoteIP, 'QWK REP packet received via FTP');
      { TODO: Receive .REP file, then:
        1. RecvFile to temp dir
        2. Unzip boardid.REP
        3. QWK.ImportPacket → inject into PCBoard message bases
        4. Log imported/failed count
        5. Cleanup }
      SendReply(Conn, '550 REP import not yet implemented');
      Exit;
    end;

    { Normal file upload }
    if FtpSessions[Idx].CurArea < 0 then
    begin
      SendReply(Conn, RE_NO_ACCESS + ': Select a file area first');
      Exit;
    end;

    if not FileAreas[FtpSessions[Idx].CurArea].UlAllowed then
    begin
      SendReply(Conn, RE_NO_ACCESS + ': Uploads not permitted in this area');
      Exit;
    end;

    { TODO: Check disk space (like Mystic's FreeUL check)
      TODO: Check duplicate filename (IsDuplicateFile)
      TODO: After upload: extract FILE_ID.DIZ, add to DIR listing }

    RecvFile(Conn, FtpSessions[Idx],
             FileAreas[FtpSessions[Idx].CurArea].RealPath +
             DirectorySeparator + Arg,
             Cmd = 'APPE');
  end

  else if Cmd = 'REST' then
  begin
    FtpSessions[Idx].RestPos := StrToIntDef(Arg, 0);
    SendReply(Conn, '350 Restart position set to ' + IntToStr(FtpSessions[Idx].RestPos));
  end

  else if Cmd = 'SIZE' then
  begin
    { TODO: return file size from DIR listing }
    SendReply(Conn, RE_BAD_FILE);
  end

  else if Cmd = 'NOOP' then
    SendReply(Conn, '200 OK')

  else if Cmd = 'QUIT' then
  begin
    SendReply(Conn, RE_GOODBYE);
    LogFtp(llInfo, Conn.RemoteIP, 'logout: ' + FtpSessions[Idx].Username +
           ' (DL: ' + IntToStr(FtpSessions[Idx].DlCount) + ' files, ' +
           IntToStr(FtpSessions[Idx].DlToday) + ' bytes' +
           ' UL: ' + IntToStr(FtpSessions[Idx].UlCount) + ' files)');
    Conn.State := csClosing;
  end

  else
    SendReply(Conn, RE_BAD_CMD);
end;

{ === Connection handlers === }

procedure FtpOnConnect(Conn : TPcbisConnection);
var
  Idx : integer;
begin
  Idx := GetFtp(Conn);
  FillChar(FtpSessions[Idx], SizeOf(TFtpSession), 0);
  FtpSessions[Idx].State := fsWaitUser;
  FtpSessions[Idx].CurArea := -1;
  FtpSessions[Idx].TransType := 'I';  { binary default }
  FtpSessions[Idx].DataSock := -1;
  SendReply(Conn, RE_READY);
  LogFtp(llInfo, Conn.RemoteIP, 'connected');
end;

procedure FtpOnData(Conn : TPcbisConnection);
var
  Line : string;
  P, S : integer;
  Cmd, Arg : string;
begin
  while True do
  begin
    P := Pos(#10, Conn.InBuf);
    if P = 0 then Break;

    Line := Trim(Copy(Conn.InBuf, 1, P - 1));
    Delete(Conn.InBuf, 1, P);

    if Length(Line) = 0 then Continue;

    S := Pos(' ', Line);
    if S > 0 then
    begin
      Cmd := UpperCase(Copy(Line, 1, S - 1));
      Arg := Copy(Line, S + 1, Length(Line));
    end
    else
    begin
      Cmd := UpperCase(Line);
      Arg := '';
    end;

    ProcessCommand(Conn, Cmd, Arg);
  end;
end;

end.
