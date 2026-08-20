{
  pcbis_smtp.pas — Outbound SMTP client for pcbis
  Sends validation emails triggered by PCBoard user signup.
  NOT a mail server — outbound only, relays through configured SMTP host.
}

unit pcbis_smtp;

{$mode objfpc}{$H+}

interface

uses
  pcbis_config;

{ Send a single email via SMTP relay }
function SmtpSendMail(const Cfg : TPcbisConfig;
                      const ToAddr, Subject, Body : string) : boolean;

{ Check trigger directory for pending emails and send them }
procedure SmtpProcessQueue(const Cfg : TPcbisConfig);

implementation

uses
  SysUtils, Classes, Sockets, BaseUnix, pcbis_log;

function ReadLine(Sock : longint) : string;
var
  C   : char;
  N   : longint;
begin
  Result := '';
  repeat
    N := fpRecv(Sock, @C, 1, 0);
    if N <= 0 then Exit;
    if C = #10 then Break;
    if C <> #13 then
      Result := Result + C;
  until False;
end;

function SendLine(Sock : longint; const Line : string) : boolean;
var
  S : string;
begin
  S := Line + #13#10;
  Result := fpSend(Sock, @S[1], Length(S), 0) = Length(S);
end;

function ExpectReply(Sock : longint; ExpectedCode : integer) : boolean;
var
  Reply : string;
  Code  : integer;
begin
  Reply := ReadLine(Sock);
  Code := StrToIntDef(Copy(Reply, 1, 3), 0);
  Result := (Code = ExpectedCode);
  if not Result then
    LogWarn('SMTP: expected ' + IntToStr(ExpectedCode) +
            ' got: ' + Reply);
end;

function SmtpSendMail(const Cfg : TPcbisConfig;
                      const ToAddr, Subject, Body : string) : boolean;
var
  Sock    : longint;
  Addr    : TInetSockAddr;
  HostAddr: THostAddr;
begin
  Result := False;

  if Cfg.SmtpRelay = '' then
  begin
    LogError('SMTP: no relay configured');
    Exit;
  end;

  { Resolve relay host }
  Sock := fpSocket(AF_INET, SOCK_STREAM, 0);
  if Sock < 0 then
  begin
    LogError('SMTP: socket failed');
    Exit;
  end;

  try
    FillChar(Addr, SizeOf(Addr), 0);
    Addr.sin_family := AF_INET;
    Addr.sin_port := htons(Cfg.SmtpRelayPort);
    Addr.sin_addr := StrToNetAddr(Cfg.SmtpRelay);

    if fpConnect(Sock, @Addr, SizeOf(Addr)) <> 0 then
    begin
      LogError('SMTP: connect to ' + Cfg.SmtpRelay + ':' +
               IntToStr(Cfg.SmtpRelayPort) + ' failed');
      Exit;
    end;

    { SMTP conversation }
    if not ExpectReply(Sock, 220) then Exit;  { greeting }

    SendLine(Sock, 'EHLO pcbis');
    if not ExpectReply(Sock, 250) then
    begin
      { Try HELO fallback }
      SendLine(Sock, 'HELO pcbis');
      if not ExpectReply(Sock, 250) then Exit;
    end;

    { Skip multi-line 250 responses }
    { TODO: properly handle multi-line EHLO response }

    SendLine(Sock, 'MAIL FROM:<' + Cfg.SmtpFrom + '>');
    if not ExpectReply(Sock, 250) then Exit;

    SendLine(Sock, 'RCPT TO:<' + ToAddr + '>');
    if not ExpectReply(Sock, 250) then Exit;

    SendLine(Sock, 'DATA');
    if not ExpectReply(Sock, 354) then Exit;

    { Message headers + body }
    SendLine(Sock, 'From: ' + Cfg.SmtpFrom);
    SendLine(Sock, 'To: ' + ToAddr);
    SendLine(Sock, 'Subject: ' + Subject);
    SendLine(Sock, 'Date: ' + FormatDateTime('ddd, dd mmm yyyy hh:nn:ss', Now) + ' +0000');
    SendLine(Sock, 'X-Mailer: pcbis 0.1.0');
    SendLine(Sock, 'MIME-Version: 1.0');
    SendLine(Sock, 'Content-Type: text/plain; charset=us-ascii');
    SendLine(Sock, '');
    SendLine(Sock, Body);
    SendLine(Sock, '.');
    if not ExpectReply(Sock, 250) then Exit;

    SendLine(Sock, 'QUIT');
    ExpectReply(Sock, 221); { don't care if QUIT fails }

    LogInfo('SMTP: sent to ' + ToAddr + ' via ' + Cfg.SmtpRelay);
    Result := True;
  finally
    CloseSocket(Sock);
  end;
end;

procedure SmtpProcessQueue(const Cfg : TPcbisConfig);
var
  SR  : TSearchRec;
  F   : TStringList;
  Fn  : string;
begin
  if not Cfg.SmtpEnabled then Exit;
  if Cfg.SmtpTriggerDir = '' then Exit;
  if not DirectoryExists(Cfg.SmtpTriggerDir) then Exit;

  { Scan for .eml files in trigger directory }
  if FindFirst(Cfg.SmtpTriggerDir + '/*.eml', faAnyFile, SR) = 0 then
  begin
    repeat
      Fn := Cfg.SmtpTriggerDir + '/' + SR.Name;
      F := TStringList.Create;
      try
        F.LoadFromFile(Fn);
        { Format: line 1 = To address, line 2 = Subject, rest = body }
        if F.Count >= 3 then
        begin
          if SmtpSendMail(Cfg, F[0], F[1],
                          Copy(F.Text, Length(F[0]) + Length(F[1]) + 3, MaxInt)) then
          begin
            DeleteFile(Fn);
            LogInfo('SMTP: queued message delivered, removed ' + SR.Name);
          end
          else
            LogWarn('SMTP: failed to send ' + SR.Name + ', will retry');
        end;
      finally
        F.Free;
      end;
    until FindNext(SR) <> 0;
    FindClose(SR);
  end;
end;

end.
